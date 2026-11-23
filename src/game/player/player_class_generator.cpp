#include "player_class_generator.hpp"

#include <json-c/json.h>

#include <limits>
#include <memory>
#include <stdexcept>
#include <string>
#include <utility>

namespace rpg {
namespace {

using JsonObject = std::unique_ptr<json_object, decltype(&json_object_put)>;

json_object* require_member(json_object* object, const char* name,
                            const std::string& context) {
  json_object* value = nullptr;
  if (!json_object_object_get_ex(object, name, &value)) {
    throw std::runtime_error("Missing '" + std::string(name) + "' in " + context);
  }
  return value;
}

std::uint64_t read_unsigned(json_object* object, const char* name,
                            const std::string& context) {
  json_object* value = require_member(object, name, context);
  if (!json_object_is_type(value, json_type_int)) {
    throw std::runtime_error("Expected non-negative integer '" +
                             std::string(name) + "' in " + context);
  }
  const std::int64_t number = json_object_get_int64(value);
  if (number < 0) {
    throw std::runtime_error("Expected non-negative integer '" +
                             std::string(name) + "' in " + context);
  }
  return static_cast<std::uint64_t>(number);
}

std::string read_string(json_object* object, const char* name,
                        const std::string& context) {
  json_object* value = require_member(object, name, context);
  if (!json_object_is_type(value, json_type_string)) {
    throw std::runtime_error("Expected string '" + std::string(name) +
                             "' in " + context);
  }
  return json_object_get_string(value);
}

std::vector<std::string> read_starting_item_ids(json_object* entry,
                                               const std::string& context) {
  json_object* items = nullptr;
  if (!json_object_object_get_ex(entry, "starting_items", &items)) {
    return {};
  }
  if (!json_object_is_type(items, json_type_array)) {
    throw std::runtime_error("Expected array 'starting_items' in " + context);
  }

  std::vector<std::string> item_ids;
  const std::size_t count = json_object_array_length(items);
  item_ids.reserve(count);
  for (std::size_t index = 0; index < count; ++index) {
    json_object* item = json_object_array_get_idx(items, index);
    if (!json_object_is_type(item, json_type_string)) {
      throw std::runtime_error("Expected string in 'starting_items' in " + context);
    }
    const std::string item_id = json_object_get_string(item);
    if (item_id.empty()) {
      throw std::runtime_error("Starting item IDs must not be empty in " + context);
    }
    item_ids.push_back(item_id);
  }
  return item_ids;
}

} // namespace

PlayerClassGenerator::PlayerClassGenerator()
    : PlayerClassGenerator(RPG_PLAYER_CLASS_DATABASE_PATH) {}

PlayerClassGenerator::PlayerClassGenerator(const std::string& database_path) {
  JsonObject root(json_object_from_file(database_path.c_str()), json_object_put);
  if (!root || !json_object_is_type(root.get(), json_type_object)) {
    throw std::runtime_error("Unable to load player class database: " +
                             database_path);
  }

  json_object* classes = require_member(root.get(), "classes", database_path);
  if (!json_object_is_type(classes, json_type_array) ||
      json_object_array_length(classes) == 0) {
    throw std::runtime_error(
        "Player class database must contain a non-empty 'classes' array");
  }

  const std::size_t count = json_object_array_length(classes);
  m_Definitions.reserve(count);
  m_Choices.reserve(count);
  for (std::size_t index = 0; index < count; ++index) {
    json_object* entry = json_object_array_get_idx(classes, index);
    const std::string context = "player class entry " + std::to_string(index);
    if (!json_object_is_type(entry, json_type_object)) {
      throw std::runtime_error("Expected object in " + context);
    }

    const std::string name = read_string(entry, "name", context);
    if (name.empty()) {
      throw std::runtime_error("Class name must not be empty in " + context);
    }

    json_object* stats = require_member(entry, "stats", context);
    if (!json_object_is_type(stats, json_type_object)) {
      throw std::runtime_error("Expected stats object in " + context);
    }
    const std::string stats_context = context + ".stats";
    Definition definition{
      {name},
        {
            read_unsigned(stats, "health", stats_context),
            read_unsigned(stats, "strength", stats_context),
            read_unsigned(stats, "intelligence", stats_context),
            read_unsigned(stats, "defense", stats_context),
            read_unsigned(stats, "precision", stats_context),
            read_unsigned(stats, "mana", stats_context),
            read_unsigned(stats, "max_mana", stats_context),
            read_unsigned(stats, "mana_regeneration", stats_context),
            read_unsigned(stats, "experience", stats_context),
            read_unsigned(stats, "max_experience", stats_context),
            read_unsigned(stats, "level", stats_context),
            read_unsigned(stats, "money", stats_context),
            read_unsigned(stats, "gems", stats_context),
        },
          read_starting_item_ids(entry, context),
    };
    if (definition.stats.health == 0 || definition.stats.max_experience == 0 ||
      definition.stats.precision > 100 ||
        definition.stats.health > static_cast<std::uint64_t>(
                                      std::numeric_limits<std::int64_t>::max())) {
      throw std::runtime_error("Invalid health or max_experience in " + context);
    }
    for (const Definition& existing : m_Definitions) {
      if (existing.choice.name == definition.choice.name) {
        throw std::runtime_error("Duplicate player class name in " + context);
      }
    }

    m_Choices.push_back(definition.choice);
    m_Definitions.push_back(std::move(definition));
  }
}

const std::vector<PlayerClassChoice>& PlayerClassGenerator::choices() const {
  return m_Choices;
}

std::shared_ptr<Player> PlayerClassGenerator::create(
    const std::string& class_name) const {
  for (const Definition& definition : m_Definitions) {
    if (definition.choice.name != class_name) {
      continue;
    }

    auto player = std::make_shared<Player>();
    player->m_ClassName = definition.choice.name;
    player->m_Health = static_cast<std::int64_t>(definition.stats.health);
    player->m_MaxHealth = definition.stats.health;
    player->m_Strength = definition.stats.strength;
    player->m_Intelligence = definition.stats.intelligence;
    player->m_Defense = definition.stats.defense;
    player->m_Precision = definition.stats.precision;
    player->m_MaxMana = definition.stats.max_mana;
    player->m_Mana = player->m_MaxMana;
    player->m_ManaRegenerationMultiplier = definition.stats.mana_regeneration;
    player->m_Exp = definition.stats.experience;
    player->m_MaxExp = definition.stats.max_experience;
    player->m_Level = definition.stats.level;
    player->m_Money = definition.stats.money;
    player->m_Gems = definition.stats.gems;
    return player;
  }
  return nullptr;
}

std::vector<std::string> PlayerClassGenerator::starting_item_ids(
    const std::string& class_name) const {
  for (const Definition& definition : m_Definitions) {
    if (definition.choice.name == class_name) {
      return definition.starting_item_ids;
    }
  }
  return {};
}

} // namespace rpg