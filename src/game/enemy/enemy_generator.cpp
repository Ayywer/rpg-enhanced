#include "enemy_generator.hpp"

#include <json-c/json.h>

#include <algorithm>
#include <limits>
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_set>
#include <utility>

namespace rpg {
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

std::uint64_t read_unsigned(json_object *object, const char *name,
                            const std::string &context) {
  json_object *value = require_member(object, name, context);
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

std::string read_string(json_object *object, const char *name,
                        const std::string &context) {
  json_object *value = require_member(object, name, context);
  if (!json_object_is_type(value, json_type_string)) {
    throw std::runtime_error("Expected string '" + std::string(name) + "' in " +
                             context);
  }
  return json_object_get_string(value);
}

std::uint64_t scale_stat(std::uint64_t base, std::uint64_t per_level,
                         std::uint64_t level, const std::string &name) {
  const std::uint64_t levels_above_one = level - 1;
  const std::uint64_t maximum = std::numeric_limits<std::uint64_t>::max();
  if (levels_above_one != 0 &&
      per_level > (maximum - base) / levels_above_one) {
    throw std::overflow_error("Scaled enemy stat overflows: " + name);
  }
  return base + per_level * levels_above_one;
}

std::uint64_t scale_precision(std::uint64_t base, std::uint64_t per_level,
                              std::uint64_t level) {
  const std::uint64_t remaining = 100 - base;
  const std::uint64_t levels_above_one = level - 1;
  if (per_level == 0 || levels_above_one <= remaining / per_level) {
    return base + per_level * levels_above_one;
  }
  return 100;
}

} // namespace

EnemyGenerator::EnemyGenerator() : EnemyGenerator(RPG_ENEMY_DATABASE_PATH) {}

EnemyGenerator::EnemyGenerator(const std::string &database_path)
    : m_RandomEngine(std::random_device{}()) {
  const auto read_stats = [](json_object *object, const std::string &context) {
    if (!json_object_is_type(object, json_type_object)) {
      throw std::runtime_error("Expected stats object in " + context);
    }
    return Stats{
        read_unsigned(object, "health", context),
        read_unsigned(object, "strength", context),
        read_unsigned(object, "defense", context),
        read_unsigned(object, "precision", context),
        read_unsigned(object, "experience", context),
        read_unsigned(object, "money", context),
        read_unsigned(object, "gems", context),
    };
  };

  JsonObject root(json_object_from_file(database_path.c_str()),
                  json_object_put);
  if (!root || !json_object_is_type(root.get(), json_type_object)) {
    throw std::runtime_error("Unable to load enemy database: " + database_path);
  }

  json_object *enemies = require_member(root.get(), "enemies", database_path);
  if (!json_object_is_type(enemies, json_type_array) ||
      json_object_array_length(enemies) == 0) {
    throw std::runtime_error(
        "Enemy database must contain a non-empty 'enemies' array");
  }

  const std::size_t count = json_object_array_length(enemies);
  m_Definitions.reserve(count);
  for (std::size_t index = 0; index < count; ++index) {
    json_object *entry = json_object_array_get_idx(enemies, index);
    const std::string context = "enemy entry " + std::to_string(index);
    if (!json_object_is_type(entry, json_type_object)) {
      throw std::runtime_error("Expected object in " + context);
    }

    json_object *paths = require_member(entry, "paths", context);
    if (!json_object_is_type(paths, json_type_array) ||
        json_object_array_length(paths) == 0) {
      throw std::runtime_error("Expected non-empty 'paths' array in " + context);
    }
    std::vector<std::string> path_ids;
    std::unordered_set<std::string> unique_path_ids;
    const std::size_t path_count = json_object_array_length(paths);
    path_ids.reserve(path_count);
    for (std::size_t path_index = 0; path_index < path_count; ++path_index) {
      json_object *path = json_object_array_get_idx(paths, path_index);
      if (!json_object_is_type(path, json_type_string)) {
        throw std::runtime_error("Expected string path ID in " + context);
      }
      std::string path_id = json_object_get_string(path);
      if (path_id.empty() || !unique_path_ids.insert(path_id).second) {
        throw std::runtime_error("Invalid or duplicate path ID in " + context);
      }
      path_ids.push_back(std::move(path_id));
    }

    Definition definition{
        read_string(entry, "name", context),
        read_unsigned(entry, "weight", context),
        std::move(path_ids),
        read_stats(require_member(entry, "stats", context), context + ".stats"),
        read_stats(require_member(entry, "per_level", context),
                   context + ".per_level"),
    };
    if (definition.name.empty() || definition.weight == 0 ||
      definition.stats.health == 0 || definition.stats.precision > 100 ||
      definition.per_level.precision > 100) {
      throw std::runtime_error(
          "Enemy name, weight, and health must be positive in " + context);
    }
    m_Definitions.push_back(std::move(definition));
  }
}

std::shared_ptr<Enemy> EnemyGenerator::generate(std::uint64_t level,
                                                const std::string& path_id) {
  std::uint64_t total_weight = 0;
  for (const Definition &definition : m_Definitions) {
    if (std::find(definition.paths.begin(), definition.paths.end(), path_id) ==
        definition.paths.end()) {
      continue;
    }
    if (definition.weight >
        std::numeric_limits<std::uint64_t>::max() - total_weight) {
      throw std::overflow_error(
          "Enemy selection weights exceed supported range");
    }
    total_weight += definition.weight;
  }
  if (total_weight == 0) {
    throw std::invalid_argument("No enemies are configured for fighting path: " +
                                path_id);
  }

  std::uniform_int_distribution<std::uint64_t> selection(1, total_weight);
  const std::uint64_t selected_weight = selection(m_RandomEngine);
  std::uint64_t cumulative_weight = 0;
  const Definition *selected = nullptr;
  for (const Definition &definition : m_Definitions) {
    if (std::find(definition.paths.begin(), definition.paths.end(), path_id) ==
        definition.paths.end()) {
      continue;
    }
    cumulative_weight += definition.weight;
    if (selected_weight <= cumulative_weight) {
      selected = &definition;
      break;
    }
  }

  const std::uint64_t enemy_level = level == 0 ? 1 : level;
  auto enemy = std::make_shared<Enemy>();
  enemy->m_Name = selected->name;
  enemy->m_Level = enemy_level;
  const std::uint64_t health =
      scale_stat(selected->stats.health, selected->per_level.health,
                 enemy_level, "health");
  if (health >
      static_cast<std::uint64_t>(std::numeric_limits<std::int64_t>::max())) {
    throw std::overflow_error("Scaled enemy health exceeds supported range");
  }
  enemy->m_Health = static_cast<std::int64_t>(health);
  enemy->m_MaxHealth = health;
  enemy->m_Strength =
      scale_stat(selected->stats.strength, selected->per_level.strength,
                 enemy_level, "strength");
  enemy->m_Defense =
      scale_stat(selected->stats.defense, selected->per_level.defense,
                 enemy_level, "defense");
    enemy->m_Precision = scale_precision(
      selected->stats.precision, selected->per_level.precision, enemy_level);
  enemy->m_RewardExp =
      scale_stat(selected->stats.experience, selected->per_level.experience,
                 enemy_level, "experience");
  enemy->m_RewardMoney = scale_stat(
      selected->stats.money, selected->per_level.money, enemy_level, "money");
  enemy->m_RewardGems = scale_stat(
      selected->stats.gems, selected->per_level.gems, enemy_level, "gems");
  return enemy;
}

} // namespace rpg