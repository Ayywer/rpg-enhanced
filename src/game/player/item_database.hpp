#pragma once

#include "item.hpp"

#include <cstdint>
#include <string>
#include <vector>

namespace rpg {

struct ItemDefinition {
  Item item;
  std::uint64_t price;
};

using EquipmentDefinition = ItemDefinition;

class ItemDatabase {
public:
  ItemDatabase();
  explicit ItemDatabase(const std::string& database_path);
  ItemDatabase(const std::string& items_path,
               const std::string& equipment_path);

  const std::vector<ItemDefinition>& items() const;
  const std::vector<EquipmentDefinition>& equipment() const;
  const ItemDefinition* find(const std::string& item_id) const;

private:
  void load_items(const std::string& database_path, bool equipment_catalog);

  std::vector<ItemDefinition> m_Items;
  std::vector<EquipmentDefinition> m_Equipment;
};

} // namespace rpg