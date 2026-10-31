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

RegionLocationType read_location_type(json_object *object,
                                      const std::string &context) {
  const std::string type = read_string(object, "type", context);
  if (type == "shop") {
    return RegionLocationType::Shop;
  }
  if (type == "npc") {
    return RegionLocationType::Npc;
  }
  if (type == "healing_bane") {
    return RegionLocationType::HealingBane;
  }
  throw std::runtime_error("Unknown location type '" + type + "' in " +
                           context);
}

WorldData load_database() {
  const std::string path = RPG_REGION_DATABASE_PATH;
  JsonObject root(json_object_from_file(path.c_str()), json_object_put);
  if (!root || !json_object_is_type(root.get(), json_type_object)) {
    throw std::runtime_error("Unable to load region database: " + path);
  }

  WorldData data;
  data.starting_region_id = read_string(root.get(), "starting_region_id", path);
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
        read_string(region_object, "description", context),
        read_unsigned(region_object, "minimum_level", context),
        read_unsigned(region_object, "maximum_level", context),
        {},
        {},
        {},
        {},
    };

    json_object *locations = require_array(region_object, "locations", context);
    for (std::size_t item = 0; item < json_object_array_length(locations);
         ++item) {
      json_object *entry = json_object_array_get_idx(locations, item);
      const std::string item_context =
          context + ".locations[" + std::to_string(item) + "]";
      if (!json_object_is_type(entry, json_type_object)) {
        throw std::runtime_error("Expected object in " + item_context);
      }
      region.locations.push_back({
          read_string(entry, "id", item_context),
          read_string(entry, "name", item_context),
          read_location_type(entry, item_context),
          read_string(entry, "description", item_context),
          read_string(entry, "function_id", item_context),
      });
    }

    json_object *paths =
        require_array(region_object, "fighting_paths", context);
    for (std::size_t item = 0; item < json_object_array_length(paths); ++item) {
      json_object *entry = json_object_array_get_idx(paths, item);
      const std::string item_context =
          context + ".fighting_paths[" + std::to_string(item) + "]";
      if (!json_object_is_type(entry, json_type_object)) {
        throw std::runtime_error("Expected object in " + item_context);
      }
      region.fighting_paths.push_back({
          read_string(entry, "id", item_context),
          read_string(entry, "name", item_context),
          read_unsigned(entry, "encounters", item_context),
          read_int(entry, "level_adjustment", item_context),
      });
    }

    json_object *exits = require_array(region_object, "exits", context);
    for (std::size_t item = 0; item < json_object_array_length(exits); ++item) {
      json_object *entry = json_object_array_get_idx(exits, item);
      const std::string item_context =
          context + ".exits[" + std::to_string(item) + "]";
      if (!json_object_is_type(entry, json_type_object)) {
        throw std::runtime_error("Expected object in " + item_context);
      }
      region.exits.push_back({
          read_string(entry, "id", item_context),
          read_string(entry, "name", item_context),
          read_string(entry, "target_region_id", item_context),
      });
    }
    region.entry_conditions = read_entry_conditions(region_object, context);
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