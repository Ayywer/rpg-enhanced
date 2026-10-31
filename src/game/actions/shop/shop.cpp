// header
#include "shop.hpp"

#include "../../console.hpp"

#include <array>
#include <string>
#include <vector>

namespace rpg {
void Shop::loop() {
  const std::vector<std::string> categories{
      "Items", "Armor & Accessories", "Weapons & Shields"};
  std::array<std::vector<const ItemDefinition*>, 3> stock_by_category;
  for (const ItemDefinition& definition : m_ItemDatabase.items()) {
    stock_by_category[0].push_back(&definition);
  }
  for (const EquipmentDefinition& definition : m_ItemDatabase.equipment()) {
    const auto slot = definition.item.equipment_slot;
    if (slot == EquipmentSlot::WEAPON || slot == EquipmentSlot::SHIELD) {
      stock_by_category[2].push_back(&definition);
    } else {
      stock_by_category[1].push_back(&definition);
    }
  }

  while (true) {
    const auto wallet = ui::panel(
        "YOUR WALLET",
        ftxui::vbox({
            ftxui::text("GOLD  " + std::to_string(m_pPlayer->m_Money)) |
                ftxui::bold | ftxui::color(ftxui::Color::Gold1),
            ftxui::text("GEMS  " + std::to_string(m_pPlayer->m_Gems)) |
                ftxui::color(ftxui::Color::RedLight),
        }),
        ftxui::Color::RGB(232, 194, 111));

    int selected_category = 0;
    int selected_item = 0;
    std::vector<std::string> item_labels;
    const auto refresh_item_labels = [&] {
      item_labels.clear();
      const auto& stock = stock_by_category[
          static_cast<std::size_t>(selected_category)];
      for (const ItemDefinition* definition : stock) {
        item_labels.push_back(definition->item.name + "  /  " +
                              std::to_string(definition->price) + " gold");
      }
      if (item_labels.empty()) {
        item_labels.push_back("No stock in this category");
      }
      selected_item = std::clamp(
          selected_item, 0, static_cast<int>(item_labels.size()) - 1);
    };

    refresh_item_labels();
    const ItemDefinition* selected_definition = nullptr;
    bool leave_shop = false;
    auto screen = ftxui::ScreenInteractive::Fullscreen();
    auto category_menu = ftxui::Menu(
        &categories, &selected_category, ftxui::MenuOption::Vertical());
    auto item_menu = ftxui::Menu(
        &item_labels, &selected_item, ftxui::MenuOption::Vertical());
    auto selector = ftxui::CatchEvent(
        ftxui::Container::Horizontal({category_menu, item_menu}),
        [&](ftxui::Event event) {
            if (event == ftxui::Event::Escape) {
            leave_shop = true;
            screen.ExitLoopClosure()();
            return true;
          }
          if (event == ftxui::Event::Return && item_menu->Focused()) {
            const auto& stock = stock_by_category[
                static_cast<std::size_t>(selected_category)];
            if (selected_item >= 0 &&
                selected_item < static_cast<int>(stock.size())) {
              selected_definition =
                  stock[static_cast<std::size_t>(selected_item)];
              screen.ExitLoopClosure()();
              return true;
            }
          }
          return false;
        });
    auto renderer = ftxui::Renderer(selector, [&] {
      refresh_item_labels();
      return ui::page(
          "Smith and provisions",
          ftxui::vbox({
              wallet,
              ftxui::hbox({
                  ui::panel("Categories", category_menu->Render() | ftxui::flex),
                  ui::panel(categories[static_cast<std::size_t>(selected_category)],
                            item_menu->Render() | ftxui::flex),
              }) | ftxui::flex,
          }) | ftxui::flex,
          "");
    });
    screen.Loop(renderer);

    if (leave_shop || selected_definition == nullptr) {
      return;
    }

    const ItemDefinition& definition = *selected_definition;
    const std::uint64_t price = definition.price;
    const Item& item = definition.item;
    if (price > m_pPlayer->m_Money) {
      ui::notify("Not enough gold", "You need " + std::to_string(price) +
                                      " gold, but have " +
                                      std::to_string(m_pPlayer->m_Money) + ".",
                 ftxui::Color::RGB(224, 95, 91));
      continue;
    }
    if (m_pPlayer->add_item(item) == 0) {
      ui::notify("Inventory full", "That item stack is already at its limit.",
                 ftxui::Color::RGB(224, 95, 91));
      continue;
    }
    m_pPlayer->m_Money -= price;
    ui::notify("Purchased", item.name + " added to inventory. Remaining gold: " +
                                      std::to_string(m_pPlayer->m_Money) + ".");
  }
}

} // namespace rpg