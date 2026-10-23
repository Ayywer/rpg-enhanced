#pragma once

// std
#include <cstdint>
#include <string>
#include <utility>
#include "item.hpp"

namespace rpg {

enum BodyPart : std::uint64_t {
  BOOTS,
  CHEST,
  HEAD,
};

class Equipment {
public:
  Equipment() = default;
  Equipment(std::uint64_t protection) : m_Protection(protection) {}
  Equipment(Item item, EquipmentSlot slot)
      : m_Name(item.name), m_Protection(item.protection),
        m_Strength(item.strength), m_Item(std::move(item)), m_Slot(slot) {}
  Equipment(std::string name, std::uint64_t protection,
            std::uint64_t strength = 0)
      : m_Name(std::move(name)), m_Protection(protection),
        m_Strength(strength) {}

  std::string m_Name;
  std::uint64_t m_Protection = 0;
  std::uint64_t m_Strength = 0;
  Item m_Item{};
  EquipmentSlot m_Slot = EquipmentSlot::HELMET;
};

} // namespace rpg
