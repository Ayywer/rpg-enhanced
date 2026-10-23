#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <utility>

namespace rpg {

enum class ItemType {
  POTION,
  EQUIPMENT,
  RELIC,
  SCROLL,
};

enum class ItemEffect {
  HEAL,
  RESTORE_MANA,
  DAMAGE,
  LEARN_SPELL,
  ACTIVATE_RELIC,
};

enum class EquipmentSlot {
  HELMET,
  CHESTPLATE,
  BOOTS,
  NECKLACE,
  RING,
  WEAPON,
  SHIELD,
};

struct Item {
  Item() = default;
  Item(std::string item_id, std::string item_name, ItemType item_type,
       ItemEffect item_effect, std::uint64_t item_power,
       std::uint64_t item_max_quantity,
       std::optional<EquipmentSlot> slot = std::nullopt,
       std::uint64_t item_protection = 0, std::uint64_t item_strength = 0,
       std::string function = {}, std::string spell = {})
      : id(std::move(item_id)), name(std::move(item_name)), type(item_type),
        effect(item_effect), power(item_power),
        max_quantity(item_max_quantity), equipment_slot(slot),
        protection(item_protection), strength(item_strength),
        use_function(std::move(function)), spell_id(std::move(spell)) {}

  std::string id;
  std::string name;
  ItemType type;
  ItemEffect effect;
  std::uint64_t power;
  std::uint64_t max_quantity;
  std::optional<EquipmentSlot> equipment_slot{};
  std::uint64_t protection = 0;
  std::uint64_t strength = 0;
  std::string use_function;
  std::string spell_id;
  std::uint64_t bonus_strength = 0;
  std::uint64_t bonus_defense = 0;
};

struct ItemStack {
  Item item;
  std::uint64_t quantity;
};

} // namespace rpg