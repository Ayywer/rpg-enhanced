#include "item_database.hpp"

#include <json-c/json.h>

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

std::string read_string(json_object* object, const char* name,
                        const std::string& context) {
  json_object* value = require_member(object, name, context);
  if (!json_object_is_type(value, json_type_string)) {
    throw std::runtime_error("Expected string '" + std::string(name) +
                             "' in " + context);
  }
  return json_object_get_string(value);
}

std::uint64_t read_unsigned(json_object* object, const char* name,
                            const std::string& context) {
  json_object* value = require_member(object, name, context);
  if (!json_object_is_type(value, json_type_int) ||
      json_object_get_int64(value) < 0) {
    throw std::runtime_error("Expected non-negative integer '" +
                             std::string(name) + "' in " + context);
  }
  return static_cast<std::uint64_t>(json_object_get_int64(value));
}

ItemType parse_type(const std::string& type, const std::string& context,
                    bool equipment_catalog) {
  if (equipment_catalog && type == "equipment") return ItemType::EQUIPMENT;
  if (!equipment_catalog && type == "equipment") {
    throw std::runtime_error("Equipment definition found in usable items catalog in " + context);
  }
  if (type == "potions") return ItemType::POTION;
  if (!equipment_catalog && type == "relic") return ItemType::RELIC;
  if (type == "scrolls") return ItemType::SCROLL;
  throw std::runtime_error("Unsupported item type '" + type + "' in " + context);
}

ItemEffect parse_effect(const std::string& effect, const std::string& context) {
  if (effect == "heal") return ItemEffect::HEAL;
  if (effect == "restore_mana") return ItemEffect::RESTORE_MANA;
  if (effect == "damage") return ItemEffect::DAMAGE;
  if (effect == "learn_spell") return ItemEffect::LEARN_SPELL;
  if (effect == "activate_relic") return ItemEffect::ACTIVATE_RELIC;
  if (effect == "none") return ItemEffect::HEAL;
  throw std::runtime_error("Unsupported item effect '" + effect + "' in " + context);
}

EquipmentSlot parse_slot(const std::string& slot, const std::string& context) {
  if (slot == "helmet") return EquipmentSlot::HELMET;
  if (slot == "chestplate") return EquipmentSlot::CHESTPLATE;
  if (slot == "boots") return EquipmentSlot::BOOTS;
  if (slot == "necklace") return EquipmentSlot::NECKLACE;
  if (slot == "ring") return EquipmentSlot::RING;
  if (slot == "weapon") return EquipmentSlot::WEAPON;
  if (slot == "shield") return EquipmentSlot::SHIELD;
  throw std::runtime_error("Unsupported equipment slot '" + slot + "' in " + context);
}

} // namespace

ItemDatabase::ItemDatabase()
  : ItemDatabase(RPG_ITEM_DATABASE_PATH, RPG_EQUIPMENT_DATABASE_PATH) {}

ItemDatabase::ItemDatabase(const std::string& database_path) {
  load_items(database_path, false);
}

ItemDatabase::ItemDatabase(const std::string& items_path,
                           const std::string& equipment_path) {
  load_items(items_path, false);
  load_items(equipment_path, true);
}

void ItemDatabase::load_items(const std::string& database_path,
                              bool equipment_catalog) {
  JsonObject root(json_object_from_file(database_path.c_str()), json_object_put);
  if (!root || !json_object_is_type(root.get(), json_type_object)) {
    throw std::runtime_error("Unable to load " +
                             std::string(equipment_catalog ? "equipment" : "item") +
                             " database: " + database_path);
  }
  const char* array_name = equipment_catalog ? "equipment" : "items";
  json_object* items = require_member(root.get(), array_name, database_path);
  if (!json_object_is_type(items, json_type_array) ||
      json_object_array_length(items) == 0) {
    throw std::runtime_error(std::string(equipment_catalog ? "Equipment" : "Item") +
                             " database must contain a non-empty '" + array_name +
                             "' array");
  }

  const std::size_t count = json_object_array_length(items);
  m_Items.reserve(count);
  for (std::size_t index = 0; index < count; ++index) {
    json_object* entry = json_object_array_get_idx(items, index);
    const std::string context = "item entry " + std::to_string(index);
    if (!json_object_is_type(entry, json_type_object)) {
      throw std::runtime_error("Expected object in " + context);
    }

    ItemDefinition definition;
    definition.item.id = read_string(entry, "id", context);
    definition.item.name = read_string(entry, "name", context);
    definition.item.type = parse_type(read_string(entry, "type", context),
                      context, equipment_catalog);
    definition.item.effect = parse_effect(read_string(entry, "effect", context), context);
    definition.item.use_function = read_string(entry, "use_function", context);
    definition.item.power = read_unsigned(entry, "power", context);
    definition.item.max_quantity = read_unsigned(entry, "max_quantity", context);
    definition.price = read_unsigned(entry, "price", context);

    json_object* spell_id = nullptr;
    if (json_object_object_get_ex(entry, "spell_id", &spell_id)) {
      if (!json_object_is_type(spell_id, json_type_string)) {
        throw std::runtime_error("Expected string 'spell_id' in " + context);
      }
      definition.item.spell_id = json_object_get_string(spell_id);
    }

    json_object* bonuses = nullptr;
    if (json_object_object_get_ex(entry, "bonuses", &bonuses)) {
      if (!json_object_is_type(bonuses, json_type_object)) {
        throw std::runtime_error("Expected bonuses object in " + context);
      }
      definition.item.bonus_strength =
          read_unsigned(bonuses, "strength", context);
      definition.item.bonus_defense =
          read_unsigned(bonuses, "defense", context);
    }

    json_object* equipment = nullptr;
    if (json_object_object_get_ex(entry, "equipment", &equipment)) {
      if (!json_object_is_type(equipment, json_type_object)) {
        throw std::runtime_error("Expected equipment object in " + context);
      }
      definition.item.equipment_slot =
          parse_slot(read_string(equipment, "slot", context), context);
      definition.item.protection = read_unsigned(equipment, "protection", context);
      definition.item.strength = read_unsigned(equipment, "strength", context);
    }

    const bool invalid_equipment =
      equipment_catalog != definition.item.equipment_slot.has_value() ||
      equipment_catalog != (definition.item.type == ItemType::EQUIPMENT);
    const bool invalid_effect =
        definition.item.type == ItemType::EQUIPMENT
            ? read_string(entry, "effect", context) != "none"
            : read_string(entry, "effect", context) == "none";
    const bool invalid_function =
      equipment_catalog
        ? definition.item.use_function != "none"
        : definition.item.use_function.empty() ||
            definition.item.use_function == "none";
    const bool invalid_spell =
      (definition.item.effect == ItemEffect::LEARN_SPELL) !=
      !definition.item.spell_id.empty();
    const bool invalid_relic =
      (definition.item.type == ItemType::RELIC) !=
          (definition.item.effect == ItemEffect::ACTIVATE_RELIC) ||
      (definition.item.type == ItemType::RELIC &&
       definition.item.bonus_strength == 0 && definition.item.bonus_defense == 0);
    if (definition.item.id.empty() || definition.item.name.empty() ||
        definition.item.max_quantity == 0 || invalid_equipment || invalid_effect ||
      invalid_function || invalid_spell || invalid_relic ||
      find(definition.item.id) != nullptr) {
      throw std::runtime_error("Invalid or duplicate item definition in " + context);
    }
    if (equipment_catalog) {
      m_Equipment.push_back(std::move(definition));
    } else {
      m_Items.push_back(std::move(definition));
    }
  }
}

const std::vector<ItemDefinition>& ItemDatabase::items() const { return m_Items; }

const std::vector<EquipmentDefinition>& ItemDatabase::equipment() const {
  return m_Equipment;
}

const ItemDefinition* ItemDatabase::find(const std::string& item_id) const {
  for (const ItemDefinition& definition : m_Items) {
    if (definition.item.id == item_id) return &definition;
  }
  for (const EquipmentDefinition& definition : m_Equipment) {
    if (definition.item.id == item_id) return &definition;
  }
  return nullptr;
}

} // namespace rpg