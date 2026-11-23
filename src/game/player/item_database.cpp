#include "item_database.hpp"
#include "item_use_registry.hpp"
#include "passive_ability_registry.hpp"

#include <json-c/json.h>

#include <algorithm>
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

std::vector<std::string> read_allowed_classes(json_object* equipment,
                                              const std::string& context) {
  json_object* classes = nullptr;
  if (!json_object_object_get_ex(equipment, "allowed_classes", &classes)) {
    return {};
  }
  if (!json_object_is_type(classes, json_type_array)) {
    throw std::runtime_error("Expected array 'allowed_classes' in " + context);
  }

  std::vector<std::string> allowed_classes;
  const std::size_t count = json_object_array_length(classes);
  allowed_classes.reserve(count);
  for (std::size_t index = 0; index < count; ++index) {
    json_object* class_name = json_object_array_get_idx(classes, index);
    if (!json_object_is_type(class_name, json_type_string)) {
      throw std::runtime_error("Expected class name in 'allowed_classes' in " +
                               context);
    }
    const std::string name = json_object_get_string(class_name);
    if (name.empty() ||
        std::find(allowed_classes.begin(), allowed_classes.end(), name) !=
            allowed_classes.end()) {
      throw std::runtime_error("Invalid class name in 'allowed_classes' in " +
                               context);
    }
    allowed_classes.push_back(name);
  }
  return allowed_classes;
}

} // namespace

ItemDatabase::ItemDatabase()
  : ItemDatabase(RPG_ITEM_DATABASE_PATH,
                 RPG_EQUIPMENT_DATABASE_DIRECTORY) {}

ItemDatabase::ItemDatabase(const std::string& database_path) {
  load_items(database_path, false);
}

ItemDatabase::ItemDatabase(const std::string& items_path,
                           const std::string& equipment_directory) {
  load_items(items_path, false);
  for (const char* filename : {"armor.json", "weapons.json", "shields.json",
                               "accessories.json"}) {
    load_items(equipment_directory + "/" + filename, true);
  }
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
  std::vector<std::pair<json_object*, std::string>> entries;
  if (equipment_catalog && json_object_is_type(items, json_type_object)) {
    json_object_object_foreach(items, category, category_items) {
      if (!json_object_is_type(category_items, json_type_array)) {
        throw std::runtime_error("Expected equipment array for category '" +
                                 std::string(category) + "' in " + database_path);
      }
      const std::size_t category_count =
          json_object_array_length(category_items);
      for (std::size_t index = 0; index < category_count; ++index) {
        entries.emplace_back(json_object_array_get_idx(category_items, index),
                             category);
      }
    }
  } else if (json_object_is_type(items, json_type_array)) {
    const std::size_t count = json_object_array_length(items);
    for (std::size_t index = 0; index < count; ++index) {
      entries.emplace_back(json_object_array_get_idx(items, index), "");
    }
  } else {
    throw std::runtime_error(std::string("Expected '") + array_name +
                             (equipment_catalog ? "' object or array in "
                                                : "' array in ") +
                             database_path);
  }
  if (!equipment_catalog && entries.empty()) {
    throw std::runtime_error("Item database must contain a non-empty 'items' array");
  }

  if (equipment_catalog) {
    m_Equipment.reserve(m_Equipment.size() + entries.size());
  } else {
    m_Items.reserve(entries.size());
  }
  for (std::size_t index = 0; index < entries.size(); ++index) {
    json_object* entry = entries[index].first;
    const std::string& category = entries[index].second;
    const std::string context = category.empty()
                                    ? "item entry " + std::to_string(index)
                                    : category + " equipment entry " +
                                          std::to_string(index);
    if (!json_object_is_type(entry, json_type_object)) {
      throw std::runtime_error("Expected object in " + context);
    }

    ItemDefinition definition;
    definition.item.id = read_string(entry, "id", context);
    definition.item.name = read_string(entry, "name", context);
    definition.item.type = parse_type(read_string(entry, "type", context),
                      context, equipment_catalog);
    if (!equipment_catalog) {
      definition.item.effect =
          parse_effect(read_string(entry, "effect", context), context);
      definition.item.use_function = read_string(entry, "use_function", context);
      definition.item.power = read_unsigned(entry, "power", context);
    }
    definition.item.max_quantity = equipment_catalog
                       ? 1
                       : read_unsigned(entry, "max_quantity", context);
    definition.price = read_unsigned(entry, "price", context);

    json_object* availability = nullptr;
    if (!json_object_object_get_ex(entry, "available_in_shop", &availability)) {
      json_object_object_get_ex(entry, "available_in_shops", &availability);
    }
    if (availability != nullptr) {
      if (!json_object_is_type(availability, json_type_array)) {
        throw std::runtime_error("Expected array 'available_in_shop' in " + context);
      }
      const std::size_t shop_count = json_object_array_length(availability);
      for (std::size_t shop_index = 0; shop_index < shop_count; ++shop_index) {
        json_object* shop_id = json_object_array_get_idx(availability, shop_index);
        if (!json_object_is_type(shop_id, json_type_string)) {
          throw std::runtime_error("Expected shop ID in 'available_in_shop' in " + context);
        }
        const std::string id = json_object_get_string(shop_id);
        if (id.empty() ||
            std::find(definition.item.available_in_shops.begin(),
                      definition.item.available_in_shops.end(), id) !=
                definition.item.available_in_shops.end()) {
          throw std::runtime_error("Invalid or duplicate shop ID in 'available_in_shop' in " + context);
        }
        definition.item.available_in_shops.push_back(id);
      }
    }

    if (equipment_catalog) {
      json_object* passive_ability = nullptr;
      if (json_object_object_get_ex(entry, "passive_ability", &passive_ability)) {
        if (!json_object_is_type(passive_ability, json_type_string)) {
          throw std::runtime_error("Expected string 'passive_ability' in " +
                                   context);
        }
        definition.item.passive_ability =
            json_object_get_string(passive_ability);
        if (definition.item.passive_ability == "none") {
          definition.item.passive_ability.clear();
        }
      }
      if (!definition.item.passive_ability.empty() &&
          !PassiveAbilityRegistry::has_builtin_function(
              definition.item.passive_ability)) {
        throw std::runtime_error("Unknown passive ability '" +
                                 definition.item.passive_ability + "' in " +
                                 context);
      }
    }

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
      definition.item.allowed_classes = read_allowed_classes(equipment, context);
      if (equipment_catalog && !category.empty()) {
        if (category == "all_classes") {
          if (!definition.item.allowed_classes.empty()) {
            throw std::runtime_error(
                "Items in 'all_classes' cannot define 'allowed_classes' in " +
                context);
          }
        } else if (category == "shared_classes") {
          if (definition.item.allowed_classes.size() < 2) {
            throw std::runtime_error(
                "Items in 'shared_classes' require at least two allowed classes in " +
                context);
          }
        } else {
          if (!definition.item.allowed_classes.empty()) {
            throw std::runtime_error(
                "Class category items must not define 'allowed_classes' in " +
                context);
          }
          definition.item.allowed_classes = {category};
        }
      }
    }

    const bool invalid_equipment =
      equipment_catalog != definition.item.equipment_slot.has_value() ||
      equipment_catalog != (definition.item.type == ItemType::EQUIPMENT);
    const bool invalid_effect =
        !equipment_catalog && read_string(entry, "effect", context) == "none";
    const bool invalid_function =
      !equipment_catalog &&
      (definition.item.use_function.empty() ||
                definition.item.use_function == "none" ||
                !ItemUseRegistry::has_builtin_function(
                    definition.item.use_function));
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