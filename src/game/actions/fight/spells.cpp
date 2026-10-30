#include "spells.hpp"

#include <json-c/json.h>

#include <algorithm>
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

} // namespace

SpellDatabase::SpellDatabase() : SpellDatabase(RPG_SPELL_DATABASE_PATH) {}

SpellDatabase::SpellDatabase(const std::string& database_path) {
  JsonObject root(json_object_from_file(database_path.c_str()), json_object_put);
  if (!root || !json_object_is_type(root.get(), json_type_object)) {
    throw std::runtime_error("Unable to load spell database: " + database_path);
  }

  json_object* spells = require_member(root.get(), "spells", database_path);
  if (!json_object_is_type(spells, json_type_array) ||
      json_object_array_length(spells) == 0) {
    throw std::runtime_error("Spell database must contain a non-empty 'spells' array");
  }

  const std::size_t count = json_object_array_length(spells);
  m_Spells.reserve(count);
  for (std::size_t index = 0; index < count; ++index) {
    json_object* entry = json_object_array_get_idx(spells, index);
    const std::string context = "spell entry " + std::to_string(index);
    if (!json_object_is_type(entry, json_type_object)) {
      throw std::runtime_error("Expected object in " + context);
    }

    SpellDefinition definition;
    definition.id = read_string(entry, "id", context);
    definition.name = read_string(entry, "name", context);
    definition.description = read_string(entry, "description", context);
    const std::string effect = read_string(entry, "effect", context);
    if (effect == "heal") {
      definition.effect = SpellEffect::HEAL;
    } else if (effect == "damage") {
      definition.effect = SpellEffect::DAMAGE;
    } else {
      throw std::runtime_error("Unsupported spell effect '" + effect + "' in " +
                               context);
    }
    definition.mana_cost = read_unsigned(entry, "mana_cost", context);
    definition.power = read_unsigned(entry, "power", context);
    definition.success_chance = read_unsigned(entry, "success_chance", context);

    json_object* unlocks = require_member(entry, "learn_by_class", context);
    if (!json_object_is_type(unlocks, json_type_object) ||
        json_object_object_length(unlocks) == 0) {
      throw std::runtime_error("Spell must have class unlock levels in " + context);
    }
    json_object_object_foreach(unlocks, class_name, unlock_level) {
      if (!json_object_is_type(unlock_level, json_type_int)) {
        throw std::runtime_error("Invalid class unlock level in " + context);
      }
      const std::int64_t level = json_object_get_int64(unlock_level);
      if (level < 1) {
        throw std::runtime_error("Class unlock levels must be positive in " +
                                 context);
      }
      definition.class_unlock_levels.emplace(
          class_name, static_cast<std::uint64_t>(level));
    }

    if (definition.id.empty() || definition.name.empty() ||
        definition.description.empty() || definition.power == 0 ||
        definition.success_chance > 100 || find(definition.id) != nullptr) {
      throw std::runtime_error("Invalid or duplicate spell definition in " + context);
    }
    m_Spells.push_back(std::move(definition));
  }
}

std::vector<std::string> SpellDatabase::learn_available(Player& player) const {
  std::vector<std::string> learned;
  for (const SpellDefinition& spell : m_Spells) {
    const auto unlock = spell.class_unlock_levels.find(player.m_ClassName);
    if (unlock != spell.class_unlock_levels.end() &&
        player.m_Level >= unlock->second && player.learn_spell(spell.id)) {
      learned.push_back(spell.name);
    }
  }
  return learned;
}

bool SpellDatabase::learn(Player& player, const std::string& spell_id) const {
  return find(spell_id) != nullptr && player.learn_spell(spell_id);
}

std::vector<const SpellDefinition*> SpellDatabase::known_spells(
    const Player& player) const {
  std::vector<const SpellDefinition*> known;
  for (const SpellDefinition& spell : m_Spells) {
    if (player.knows_spell(spell.id)) {
      known.push_back(&spell);
    }
  }
  return known;
}

const SpellDefinition* SpellDatabase::find(const std::string& spell_id) const {
  for (const SpellDefinition& spell : m_Spells) {
    if (spell.id == spell_id) {
      return &spell;
    }
  }
  return nullptr;
}

SpellResult SpellDatabase::cast(Player& player, Enemy& enemy,
                                const SpellDefinition& spell,
                                bool succeeds) const {
  SpellResult result;
  if (!player.knows_spell(spell.id)) {
    result.not_known = true;
    return result;
  }
  if (player.m_Mana < spell.mana_cost) {
    result.insufficient_mana = true;
    return result;
  }
  if (!succeeds) {
    return result;
  }

  player.m_Mana -= spell.mana_cost;
  result.mana_spent = spell.mana_cost;
  result.succeeded = true;
  if (spell.effect == SpellEffect::HEAL) {
    const std::uint64_t max_heal = static_cast<std::uint64_t>(
        std::numeric_limits<std::int64_t>::max() - player.m_Health);
    result.heal = std::min(spell.power, max_heal);
    player.m_Health += static_cast<std::int64_t>(result.heal);
  } else if (enemy.m_Health > 0) {
    const std::uint64_t max_damage =
      static_cast<std::uint64_t>(enemy.m_Health);
    const std::uint64_t damage_multiplier = player.m_Intelligence;
    const std::uint64_t scaled_power =
      damage_multiplier != 0 &&
          spell.power > std::numeric_limits<std::uint64_t>::max() /
                    damage_multiplier
        ? max_damage
        : std::min(spell.power * damage_multiplier / 10, max_damage);
    result.damage = scaled_power;
    enemy.m_Health -= static_cast<std::int64_t>(result.damage);
  }
  return result;
}

} // namespace rpg
