#pragma once

// local
#include "equipment.hpp"
#include "item.hpp"

// std
#include <cstdint>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace rpg {

class Enemy;

class Player {
public:
  virtual ~Player() = default;

  virtual std::uint64_t attack(std::shared_ptr<Enemy> enemy, std::uint64_t attack_damage, BodyPart body_part);

  std::string m_ClassName;
  std::vector<std::string> m_KnownSpells;

  bool knows_spell(const std::string& spell_id) const {
    for (const std::string& known_spell : m_KnownSpells) {
      if (known_spell == spell_id) {
        return true;
      }
    }
    return false;
  }

  bool learn_spell(const std::string& spell_id) {
    if (spell_id.empty() || knows_spell(spell_id)) {
      return false;
    }
    m_KnownSpells.push_back(spell_id);
    return true;
  }

  std::uint64_t add_item(const Item& item, std::uint64_t quantity = 1) {
    if (item.id.empty() || item.name.empty() || item.max_quantity == 0 ||
        quantity == 0) {
      return 0;
    }
    for (ItemStack& stack : m_Inventory) {
      if (stack.item.id == item.id) {
        const std::uint64_t room = item.max_quantity - stack.quantity;
        const std::uint64_t added = quantity < room ? quantity : room;
        stack.quantity += added;
        return added;
      }
    }
    const std::uint64_t added = quantity < item.max_quantity
                                    ? quantity
                                    : item.max_quantity;
    m_Inventory.push_back({item, added});
    return added;
  }

  bool equip_item(const std::string& item_id) {
    for (std::size_t index = 0; index < m_Inventory.size(); ++index) {
      const ItemStack& stack = m_Inventory[index];
      if (stack.item.id != item_id || stack.item.type != ItemType::EQUIPMENT ||
          !stack.item.equipment_slot || stack.quantity == 0) {
        continue;
      }
      const EquipmentSlot slot = *stack.item.equipment_slot;
      const Item equipped_item = stack.item;
      const auto current = m_Equipment.find(slot);
      if (current != m_Equipment.end() && !current->second.m_Item.id.empty()) {
        if (add_item(current->second.m_Item) == 0) {
          return false;
        }
      }
      --m_Inventory[index].quantity;
      if (m_Inventory[index].quantity == 0) {
        m_Inventory.erase(m_Inventory.begin() + index);
      }
      m_Equipment[slot] = Equipment(equipped_item, slot);
      return true;
    }
    return false;
  }

  std::uint64_t equipment_protection(BodyPart body_part) const {
    const EquipmentSlot armor_slot =
        body_part == BodyPart::HEAD ? EquipmentSlot::HELMET
        : body_part == BodyPart::CHEST ? EquipmentSlot::CHESTPLATE
                                       : EquipmentSlot::BOOTS;
    std::uint64_t protection = 0;
    for (EquipmentSlot slot : {armor_slot, EquipmentSlot::NECKLACE,
                               EquipmentSlot::RING, EquipmentSlot::SHIELD}) {
      const auto equipment = m_Equipment.find(slot);
      if (equipment != m_Equipment.end()) {
        protection += equipment->second.m_Protection;
      }
    }
    return protection;
  }

  std::uint64_t weapon_strength() const {
    const auto weapon = m_Equipment.find(EquipmentSlot::WEAPON);
    return weapon == m_Equipment.end() ? 0 : weapon->second.m_Strength;
  }

  bool has_active_relic(const std::string& item_id) const {
    for (const Item& relic : m_ActiveRelics) {
      if (relic.id == item_id) {
        return true;
      }
    }
    return false;
  }

  std::uint64_t active_relic_strength() const {
    std::uint64_t bonus = 0;
    for (const Item& relic : m_ActiveRelics) {
      bonus += relic.bonus_strength;
    }
    return bonus;
  }

  std::uint64_t active_relic_defense() const {
    std::uint64_t bonus = 0;
    for (const Item& relic : m_ActiveRelics) {
      bonus += relic.bonus_defense;
    }
    return bonus;
  }
  
  std::int64_t m_Health;
  std::uint64_t m_MaxHealth = 0;
  std::uint64_t m_Defense;

  std::uint64_t m_Strength;
  std::uint64_t m_Mana = 0;
  std::uint64_t m_MaxMana = 0;
  std::uint64_t m_ManaRegenerationMultiplier = 1;

  std::uint64_t m_Exp = 0;
  std::uint64_t m_MaxExp = 10;
  std::uint64_t m_Level = 1;

  std::uint64_t m_Money;
  std::uint64_t m_Gems;
  std::vector<ItemStack> m_Inventory{};
  std::vector<Item> m_ActiveRelics{};
  std::unordered_map<EquipmentSlot, Equipment> m_Equipment{};
};

} // namespace rpg
