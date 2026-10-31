// header
#include "game.hpp"

// local
#include "actions/fight/fight.hpp"
#include "console.hpp"

#include <functional>

namespace rpg {
void Game::setup() {
  std::vector<std::string> classes;
  for (const PlayerClassChoice &choice : m_PlayerClassGenerator.choices()) {
    classes.push_back(choice.name);
  }

  const int selected = ui::choose_option("Choose your class", classes);

  if (selected >= 0 && selected < static_cast<int>(m_PlayerClassGenerator.choices().size())) {
    m_pPlayer = m_PlayerClassGenerator.create(m_PlayerClassGenerator.choices()[selected].key);
    m_SpellDatabase.learn_available(*m_pPlayer);
  }
}

void Game::loop() {

  struct GameAction {
    std::string category;
    std::string label;
    std::function<bool()> is_available;
    std::function<void()> execute;
  };

  const auto location_label = [](const RegionLocation& location) {
    switch (location.type) {
      case RegionLocationType::Shop:
        return std::string("Shop: " + location.name);
      case RegionLocationType::Npc:
        return std::string("NPC: " + location.name);
      case RegionLocationType::HealingBane:
        return std::string("Healing: " + location.name);
    }
    return location.name;
  };

  const auto exit_label = [](const RegionExit& exit) {
    return std::string("Travel: " + exit.name);
  };

  bool continue_adventure = true;
  const auto show_statistics = [this] {
    const auto equipment_name = [this](EquipmentSlot slot) {
      const auto equipment = m_pPlayer->m_Equipment.find(slot);
      return equipment == m_pPlayer->m_Equipment.end()
                 ? std::string("Empty")
                 : equipment->second.m_Name;
    };

    std::string known_spells = "None";
    if (!m_pPlayer->m_KnownSpells.empty()) {
      known_spells.clear();
      for (const std::string &spell : m_pPlayer->m_KnownSpells) {
        if (!known_spells.empty())
          known_spells += ", ";
        known_spells += spell;
      }
    }

    const auto stat = [](const std::string &label, const std::string &value) {
      return ftxui::hbox({
          ftxui::text(label) | ftxui::color(ftxui::Color::GrayLight),
          ftxui::text(value) | ftxui::bold | ftxui::color(ftxui::Color::White),
      });
    };
    const auto pair = [&stat](const std::string &left_label,
                              const std::string &left_value,
                              const std::string &right_label,
                              const std::string &right_value) {
      return ftxui::hbox({stat(left_label, left_value) | ftxui::flex,
                          stat(right_label, right_value) | ftxui::flex});
    };
    const auto section = [](const std::string &title) {
      return ftxui::text(title) | ftxui::bold |
             ftxui::color(ftxui::Color::RGB(91, 205, 183));
    };
    const auto stats = ui::panel(
        "PLAYER  /  " + m_pPlayer->m_ClassName,
        ftxui::vbox({

            section("INFO"),
            section(""),
            stat("HEALTH:", std::to_string(m_pPlayer->m_Health)),
            stat("DEFENSE:", std::to_string(m_pPlayer->m_Defense)),

            pair("LEVEL:", std::to_string(m_pPlayer->m_Level), "", 
                              std::to_string(m_pPlayer->m_Exp) + " / " +
                              std::to_string(m_pPlayer->m_MaxExp)),

            section(""),
            stat("GOLD:", std::to_string(m_pPlayer->m_Money)),
            stat("GEMS:", std::to_string(m_pPlayer->m_Gems)),

            ftxui::separator(),

            section("STATISTIC"),
            section(""),
            stat("STRENGTH:", std::to_string(m_pPlayer->m_Strength)),
            stat("INTELLIGENCE:", std::to_string(m_pPlayer->m_Intelligence)),
            stat("PRECISION:", std::to_string(m_pPlayer->m_Precision) + "%"),
            stat("MANA REGENERATION:", std::to_string(m_pPlayer->m_ManaRegenerationMultiplier) + "x"),

            ftxui::separator(),

            section("EQUIPMENT & WEALTH:"),
            section(""),
            stat("HELMET:", equipment_name(EquipmentSlot::HELMET)),
            stat("CHESTPLATE:", equipment_name(EquipmentSlot::CHESTPLATE)),
            stat("BOOTS:", equipment_name(EquipmentSlot::BOOTS)),
            stat("NECKLACE:", equipment_name(EquipmentSlot::NECKLACE)),
            stat("RING:", equipment_name(EquipmentSlot::RING)),
            stat("WEAPON:", equipment_name(EquipmentSlot::WEAPON)),
            stat("SHIELD:", equipment_name(EquipmentSlot::SHIELD)),

            ftxui::separator(),

            section("KNOWN SPELLS"),

            ftxui::paragraph(known_spells) | ftxui::color(ftxui::Color::White),
        }),
        ftxui::Color::RGB(91, 205, 183));
    ui::choose_option("Player statistics", {"Return to the road"}, stats);
  };
  const auto show_inventory = [this] {
    while (true) {
      std::vector<std::string> choices;
      choices.reserve(m_pPlayer->m_Inventory.size() + 1);
      for (const ItemStack& stack : m_pPlayer->m_Inventory) {
        const std::string type =
          stack.item.type == ItemType::POTION ? "Potions"
          : stack.item.type == ItemType::SCROLL ? "Scrolls"
            : stack.item.type == ItemType::RELIC ? "Relics"
                               : "Equipment";
          const std::string active =
            stack.item.type == ItemType::RELIC &&
                m_pPlayer->has_active_relic(stack.item.id)
              ? "  /  Active"
              : "";
        choices.push_back(stack.item.name + "  x" +
                    std::to_string(stack.quantity) + "  /  " + type +
                    active);
      }
      choices.push_back("Return to the road");
      const int selected = ui::choose_option("Inventory", choices);
      if (selected < 0 ||
          selected >= static_cast<int>(m_pPlayer->m_Inventory.size())) {
        return;
      }
      const Item item = m_pPlayer->m_Inventory[static_cast<std::size_t>(selected)].item;
      if (item.type == ItemType::EQUIPMENT) {
        if (m_pPlayer->equip_item(item.id)) {
          ui::notify("Equipment changed", item.name + " equipped.",
                     ftxui::Color::RGB(91, 205, 183));
        } else {
          ui::notify("Cannot equip", "That item has no valid equipment slot.",
                     ftxui::Color::RGB(224, 95, 91));
        }
      } else {
        const ItemUseResult result =
            m_ItemUseRegistry.use_item(*m_pPlayer, item.id);
        ui::notify(result.succeeded ? "Item used" : "Cannot use item",
                   result.message,
                   result.succeeded ? ftxui::Color::RGB(91, 205, 183)
                                    : ftxui::Color::RGB(232, 166, 76));
      }
    }
  };

  const auto build_region_actions = [this, &location_label, &exit_label] {
    std::vector<GameAction> actions;
    actions.reserve(16);
    const Region& current_region = m_RegionWorld.current_region();

  for (const FightingPath& path : current_region.fighting_paths) {
    actions.push_back({"Fighting", path.name, [] { return true; },
                       [this, path]() {
                         const std::uint64_t enemy_level =
                             m_RegionWorld.encounter_level(m_pPlayer->m_Level, path);
                         Fight fight(m_pPlayer,
                                     m_EnemyGenerator.generate(enemy_level,
                                                               path.id),
                                     m_SpellDatabase, m_ItemUseRegistry);
                         fight.loop();
                       }});
  }

  for (const RegionLocation& location : current_region.locations) {
    actions.push_back({"Region", location_label(location),
                       [] { return true; },
                       [this, location]() {
                         m_RegionLocationRegistry.execute(
                             location.function_id,
                             {m_pPlayer, m_ItemDatabase}, location);
                       }});
  }

  for (const RegionExit& exit : current_region.exits) {
    const auto target = std::find_if(
        m_RegionWorld.regions().begin(), m_RegionWorld.regions().end(),
        [&exit](const Region& candidate) {
          return candidate.id == exit.target_region_id;
        });
    const Region destination = *target;
    actions.push_back({"Region", exit_label(exit) + "  /  " + destination.name,
                       [this] { return !m_RegionWorld.current_region().exits.empty(); },
                       [this, exit, destination]() {
                         const RegionEntryCheckResult condition_result =
                             m_RegionEntryConditionRegistry.check(
                                 destination.entry_conditions, *m_pPlayer);
                         if (!condition_result.allowed) {
                           ui::notify("Cannot enter " + destination.name,
                                      condition_result.message,
                                      ftxui::Color::RGB(224, 95, 91));
                           return;
                         }
                         if (m_RegionWorld.travel_to(exit.target_region_id)) {
                           const Region& destination = m_RegionWorld.current_region();
                           ui::notify("Arrived in " + destination.name,
                                      destination.description + " Suggested levels: " +
                                          std::to_string(destination.minimum_level) + "-" +
                                          std::to_string(destination.maximum_level) + ".");
                         }
                       }});
  }

    return actions;
  };

  while (continue_adventure && m_pPlayer->m_Health > 0) {
    std::vector<GameAction> actions = build_region_actions();
    actions.push_back({"Player", "Open inventory", [] { return true; }, show_inventory});
    actions.push_back({"Player", "View player statistics", [] { return true; }, show_statistics});
    actions.push_back({"Game", "Leave the adventure", [] { return true; },
                       [&continue_adventure] { continue_adventure = false; }});
    const Region& region = m_RegionWorld.current_region();
    const auto player_summary = ui::panel(
        "PLAYER",
        ftxui::vbox({
            ftxui::text(m_pPlayer->m_ClassName + "  /  LEVEL " + std::to_string(m_pPlayer->m_Level)) |
                ftxui::bold | ftxui::color(ftxui::Color::White),
            
            ftxui::text(""),

            ui::meter("HEALTH", static_cast<std::uint64_t>(m_pPlayer->m_Health), 
                m_pPlayer->m_MaxHealth, ftxui::Color::Green),
            ui::meter("MANA", m_pPlayer->m_Mana, m_pPlayer->m_MaxMana, ftxui::Color::BlueViolet),

            ftxui::text(""),

            ftxui::text("GOLD: " + std::to_string(m_pPlayer->m_Money)) | ftxui::color(ftxui::Color::Gold1), //RGB(232, 194, 111)
            ftxui::text("GEMS: " + std::to_string(m_pPlayer->m_Gems)) | ftxui::color(ftxui::Color::RedLight),
        }),
        ftxui::Color::RGB(91, 205, 183));
      const auto region_summary = ui::panel(
        "REGION  /  " + region.name,
        ftxui::vbox({
          ftxui::paragraph(region.description) |
            ftxui::color(ftxui::Color::White),
          ftxui::text("SUGGESTED LEVEL  " +
                std::to_string(region.minimum_level) + "-" +
                std::to_string(region.maximum_level)) |
            ftxui::color(ftxui::Color::GrayLight),
        }),
        ftxui::Color::RGB(91, 205, 183));

    std::vector<std::string> categories{"Player", "Fighting", "Region", "Game"};
    std::vector<std::vector<const GameAction *>> grouped_actions(categories.size());
    for (const GameAction &game_action : actions) {
      if (game_action.is_available()) {
        auto category_it = std::find(categories.begin(), categories.end(), game_action.category);
        if (category_it != categories.end()) {
          const std::size_t category_index = static_cast<std::size_t>(
              std::distance(categories.begin(), category_it));
          grouped_actions[category_index].push_back(&game_action);
        }
      }
    }

    int selected_category = 0;
    int selected_action = 0;
    std::vector<std::string> action_labels;
    const auto refresh_action_labels = [&]() {
      action_labels.clear();
      if (selected_category < static_cast<int>(grouped_actions.size())) {
        for (const GameAction *game_action : grouped_actions[static_cast<std::size_t>(selected_category)]) {
          action_labels.push_back(game_action->label);
        }
      }
      if (action_labels.empty()) {
        action_labels.push_back("No actions available");
      }
      if (selected_action >= static_cast<int>(action_labels.size())) {
        selected_action = static_cast<int>(action_labels.size()) - 1;
      }
    };

    refresh_action_labels();

    auto screen = ftxui::ScreenInteractive::Fullscreen();
    auto category_menu = ftxui::Menu(&categories, &selected_category, ftxui::MenuOption::Vertical());
    auto action_menu = ftxui::Menu(&action_labels, &selected_action, ftxui::MenuOption::Vertical());
    auto select_action = ftxui::CatchEvent(
        ftxui::Container::Horizontal({category_menu, action_menu}),
        [&](ftxui::Event event) {
          if (event == ftxui::Event::Return) {
            const auto &current_group = grouped_actions[static_cast<std::size_t>(selected_category)];
            if (!current_group.empty() && selected_action >= 0 &&
                selected_action < static_cast<int>(current_group.size())) {
              current_group[static_cast<std::size_t>(selected_action)]->execute();
              screen.ExitLoopClosure()();
              return true;
            }
          }
          return false;
        });

    auto renderer = ftxui::Renderer(select_action, [&] {
      refresh_action_labels();
      const std::string current_category =
          selected_category >= 0 && selected_category < static_cast<int>(categories.size())
              ? categories[static_cast<std::size_t>(selected_category)]
              : "";
      return ui::page(
          "Current region: " + region.name,
          ftxui::hbox({
              ui::panel("Categories", category_menu->Render() | ftxui::flex),
              ui::panel(current_category.empty() ? "Actions" : current_category,
                        action_menu->Render() | ftxui::flex),
              player_summary | ftxui::flex,
          }) | ftxui::flex,
          "");
    });

    screen.Loop(renderer);

    if (!continue_adventure && m_pPlayer->m_Health <= 0) {
      break;
    }
  }
}
} // namespace rpg
