#include "region_database.hpp"

#include <json-c/json.h>

#include <cstdint>
#include <memory>
#include <stdexcept>
#include <string>
#include <utility>

namespace rpg::region_data {
namespace {

using JsonObject = std::unique_ptr<json_object, decltype(&json_object_put)>;

json_object *require_member(json_object *object, const char *name,
                            const std::string &context) {
  json_object *value = nullptr;
  if (!json_object_object_get_ex(object, name, &value)) {
    throw std::runtime_error("Missing '" + std::string(name) + "' in " +
                             context);
  }
  return value;
}

std::string read_string(json_object *object, const char *name,
                        const std::string &context) {
  json_object *value = require_member(object, name, context);
  if (!json_object_is_type(value, json_type_string)) {
    throw std::runtime_error("Expected string '" + std::string(name) + "' in " +
                             context);
  }
  return json_object_get_string(value);
}

std::uint64_t read_unsigned(json_object *object, const char *name,
                            const std::string &context) {
  json_object *value = require_member(object, name, context);
  if (!json_object_is_type(value, json_type_int) ||
      json_object_get_int64(value) < 0) {
    throw std::runtime_error("Expected non-negative integer '" +
                             std::string(name) + "' in " + context);
  }
  return static_cast<std::uint64_t>(json_object_get_int64(value));
}

int read_int(json_object *object, const char *name,
             const std::string &context) {
  json_object *value = require_member(object, name, context);
  if (!json_object_is_type(value, json_type_int)) {
    throw std::runtime_error("Expected integer '" + std::string(name) +
                             "' in " + context);
  }
  const std::int64_t number = json_object_get_int64(value);
  if (number < INT32_MIN || number > INT32_MAX) {
    throw std::runtime_error("Integer out of range '" + std::string(name) +
                             "' in " + context);
  }
  return static_cast<int>(number);
}

json_object *require_array(json_object *object, const char *name,
                           const std::string &context) {
  json_object *value = require_member(object, name, context);
  if (!json_object_is_type(value, json_type_array)) {
    throw std::runtime_error("Expected array '" + std::string(name) + "' in " +
                             context);
  }
  return value;
}

RegionEntryParameter read_parameter(json_object *value,
                                    const std::string &context) {
  switch (json_object_get_type(value)) {
  case json_type_boolean:
    return json_object_get_boolean(value) != 0;
  case json_type_int:
    return static_cast<std::int64_t>(json_object_get_int64(value));
  case json_type_double:
    return json_object_get_double(value);
  case json_type_string:
    return std::string(json_object_get_string(value));
  default:
    throw std::runtime_error("Expected a scalar condition parameter in " +
                             context);
  }
}

std::vector<RegionEntryCondition> read_entry_conditions(
    json_object *region, const std::string &context) {
  json_object *conditions = nullptr;
  if (!json_object_object_get_ex(region, "entry_conditions", &conditions)) {
    return {};
  }
  if (!json_object_is_type(conditions, json_type_array)) {
    throw std::runtime_error("Expected array 'entry_conditions' in " +
                             context);
  }

  std::vector<RegionEntryCondition> result;
  const std::size_t count = json_object_array_length(conditions);
  result.reserve(count);
  for (std::size_t index = 0; index < count; ++index) {
    json_object *entry = json_object_array_get_idx(conditions, index);
    const std::string condition_context =
        context + ".entry_conditions[" + std::to_string(index) + "]";
    if (!json_object_is_type(entry, json_type_object)) {
      throw std::runtime_error("Expected object in " + condition_context);
    }

    RegionEntryCondition condition;
    condition.function_id = read_string(entry, "function_id", condition_context);
    if (condition.function_id.empty()) {
      throw std::runtime_error("Condition function ID cannot be empty in " +
                               condition_context);
    }

    json_object *parameters = nullptr;
    if (json_object_object_get_ex(entry, "parameters", &parameters)) {
      if (!json_object_is_type(parameters, json_type_object)) {
        throw std::runtime_error("Expected object 'parameters' in " +
                                 condition_context);
      }
      json_object_object_foreach(parameters, key, value) {
        condition.parameters.emplace(
            key, read_parameter(value, condition_context + ".parameters." + key));
      }
    }
    result.push_back(std::move(condition));
  }
  return result;
}

PlaceInteractionType read_interaction_type(json_object *object,
                                          const std::string &context) {
  const std::string type = read_string(object, "type", context);
  if (type == "shop") return PlaceInteractionType::Shop;
  if (type == "npc") return PlaceInteractionType::Npc;
  if (type == "healing_bane") return PlaceInteractionType::HealingBane;
  if (type == "location") return PlaceInteractionType::Location;
  throw std::runtime_error("Unknown interaction type '" + type + "' in " + context);
}

WorldData load_database() {
  const std::string path = RPG_REGION_DATABASE_PATH;
  JsonObject root(json_object_from_file(path.c_str()), json_object_put);
  if (!root || !json_object_is_type(root.get(), json_type_object)) {
    throw std::runtime_error("Unable to load region database: " + path);
  }

  WorldData data;
  data.starting_region_id = read_string(root.get(), "starting_region_id", path);
  json_object *starting_place = nullptr;
  if (json_object_object_get_ex(root.get(), "starting_place_id",
                                &starting_place)) {
    if (!json_object_is_type(starting_place, json_type_string)) {
      throw std::runtime_error("Expected string 'starting_place_id' in " + path);
    }
    data.starting_place_id = json_object_get_string(starting_place);
  }
  json_object *regions = require_array(root.get(), "regions", path);
  const std::size_t region_count = json_object_array_length(regions);
  data.regions.reserve(region_count);
  for (std::size_t index = 0; index < region_count; ++index) {
    json_object *region_object = json_object_array_get_idx(regions, index);
    const std::string context = "region entry " + std::to_string(index);
    if (!json_object_is_type(region_object, json_type_object)) {
      throw std::runtime_error("Expected object in " + context);
    }

    Region region{
        read_string(region_object, "id", context),
        read_string(region_object, "name", context),
        {},
        {},
    };

    json_object *places = require_array(region_object, "places", context);
    for (std::size_t item = 0; item < json_object_array_length(places); ++item) {
      json_object *place_object = json_object_array_get_idx(places, item);
      const std::string place_context =
          context + ".places[" + std::to_string(item) + "]";
      if (!json_object_is_type(place_object, json_type_object)) {
        throw std::runtime_error("Expected object in " + place_context);
      }

      Place place{
          read_string(place_object, "id", place_context),
          read_string(place_object, "name", place_context),
          read_string(place_object, "description", place_context),
          {}, {}, {}, {},
      };

      json_object *interactions =
          require_array(place_object, "interactions", place_context);
      for (std::size_t place_item = 0;
           place_item < json_object_array_length(interactions); ++place_item) {
        json_object *entry = json_object_array_get_idx(interactions, place_item);
        const std::string item_context =
            place_context + ".interactions[" + std::to_string(place_item) + "]";
        if (!json_object_is_type(entry, json_type_object)) {
          throw std::runtime_error("Expected object in " + item_context);
        }
        place.interactions.push_back({
            read_string(entry, "id", item_context),
            read_string(entry, "name", item_context),
            read_interaction_type(entry, item_context),
            read_string(entry, "description", item_context),
            read_string(entry, "function_id", item_context),
        });
      }

      json_object *place_paths =
          require_array(place_object, "fighting_paths", place_context);
      for (std::size_t place_item = 0;
           place_item < json_object_array_length(place_paths); ++place_item) {
        json_object *entry = json_object_array_get_idx(place_paths, place_item);
        const std::string item_context =
            place_context + ".fighting_paths[" + std::to_string(place_item) + "]";
        if (!json_object_is_type(entry, json_type_object)) {
          throw std::runtime_error("Expected object in " + item_context);
        }
        place.fighting_paths.push_back({
            read_string(entry, "id", item_context),
            read_string(entry, "name", item_context),
            read_unsigned(entry, "encounters", item_context),
            read_int(entry, "level_adjustment", item_context),
        });
      }

      json_object *place_exits =
          require_array(place_object, "exits", place_context);
      for (std::size_t place_item = 0;
           place_item < json_object_array_length(place_exits); ++place_item) {
        json_object *entry = json_object_array_get_idx(place_exits, place_item);
        const std::string item_context =
            place_context + ".exits[" + std::to_string(place_item) + "]";
        if (!json_object_is_type(entry, json_type_object)) {
          throw std::runtime_error("Expected object in " + item_context);
        }
        place.exits.push_back({
            read_string(entry, "id", item_context),
            read_string(entry, "name", item_context),
            read_string(entry, "target_place_id", item_context),
        });
      }
      place.entry_conditions = read_entry_conditions(place_object, place_context);
      region.places.push_back(std::move(place));
    }

    json_object *bridges = require_array(region_object, "bridges", context);
    for (std::size_t item = 0; item < json_object_array_length(bridges); ++item) {
      json_object *bridge = json_object_array_get_idx(bridges, item);
      const std::string bridge_context =
          context + ".bridges[" + std::to_string(item) + "]";
      if (!json_object_is_type(bridge, json_type_object)) {
        throw std::runtime_error("Expected object in " + bridge_context);
      }
      region.bridges.push_back({
          read_string(bridge, "place_id", bridge_context),
          read_string(bridge, "target_region_id", bridge_context),
      });
    }

    data.regions.push_back(std::move(region));
  }
  return data;
}

} // namespace

const WorldData &database() {
  static const WorldData data = load_database();
  return data;
}

} // namespace rpg::region_data