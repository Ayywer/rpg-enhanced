// header
#include "game.hpp"

// local
#include "actions/fight/fight.hpp"
#include "console.hpp"

#include <ftxui/dom/elements.hpp>
#include <functional>

namespace rpg {
void Game::setup() {
  std::vector<std::string> classes;
  for (const PlayerClassChoice &choice : m_PlayerClassGenerator.choices()) {
    classes.push_back(choice.name);
  }

  const int selected = ui::choose_option("Choose your class", classes);

  if (selected >= 0 && selected < static_cast<int>(m_PlayerClassGenerator.choices().size())) {
    const std::string& class_name =
      m_PlayerClassGenerator.choices()[selected].name;
    m_pPlayer = m_PlayerClassGenerator.create(class_name);
    for (const std::string& item_id :
       m_PlayerClassGenerator.starting_item_ids(class_name)) {
      const ItemDefinition* definition = m_ItemDatabase.find(item_id);
      if (definition == nullptr) {
        throw std::runtime_error("Unknown starting item '" + item_id +
                                 "' for class '" + m_pPlayer->m_ClassName + "'");
      }
      if (m_pPlayer->add_item(definition->item) == 0) {
        throw std::runtime_error("Unable to add starting item '" + item_id +
                                 "' for class '" + m_pPlayer->m_ClassName + "'");
      }
      if (definition->item.type == ItemType::EQUIPMENT &&
          !m_pPlayer->equip_item(item_id)) {
        throw std::runtime_error("Unable to equip starting item '" + item_id +
                                 "' for class '" + m_pPlayer->m_ClassName + "'");
      }
    }
    m_SpellDatabase.learn_available(*m_pPlayer);
  }
}

void Game::loop() {
  while (true) {
    const int menu_choice = ui::choose_option("", {"New Game", "Settings", "Quit"});
    
    if (menu_choice == 1) {
      ui::settings_menu();
      continue;
    }
    
    if (menu_choice != 0) return;
    break;
  }

  setup();

  struct GameAction {
    std::string category;
    std::string label;
    std::function<bool()> is_available;
    std::function<void()> execute;
  };

  bool continue_adventure = true;

  const auto show_statistics = [this] {
    const auto equipment_name = [this](EquipmentSlot slot) {
      const auto equipment = m_pPlayer->m_Equipment.find(slot);
      return equipment == m_pPlayer->m_Equipment.end() ? std::string("Empty") : equipment->second.m_Name;
    };

    const auto stat = [](const std::string &label, const std::string &value) {
      return ftxui::hbox({
          ftxui::text(label) | ftxui::color(ui::text_color()),
          ftxui::text(value) | ftxui::bold | ftxui::color(ui::text_color()),
      });
    };
    const std::vector<std::string> stat_sections{"Overview", "Statistic", "Equipment", "Spell book", "Return"};
    
    while (true) {
      const int selected = ui::choose_option("Player statistics  /  " + m_pPlayer->m_ClassName, stat_sections);
      
      if (selected == 0) {
        ui::choose_option("Overview", {"Back"},
            ftxui::vbox({
                stat("HEALTH:", std::to_string(m_pPlayer->m_Health)),
                stat("DEFENSE:", std::to_string(m_pPlayer->m_Defense)),
                ftxui::text(""),
                stat("LEVEL:", std::to_string(m_pPlayer->m_Level)),
                stat("EXPERIENCE:", std::to_string(m_pPlayer->m_Exp) + " / " + std::to_string(m_pPlayer->m_MaxExp)),
                ftxui::text(""),
                stat("GOLD:", std::to_string(m_pPlayer->m_Money)),
                stat("GEMS:", std::to_string(m_pPlayer->m_Gems)),
            }), 3);
      } else if (selected == 1) {
        ui::choose_option(
            "Attributes", {"Back"},
            ftxui::vbox({
                stat("STRENGTH:", std::to_string(m_pPlayer->m_Strength)),
                stat("INTELLIGENCE:", std::to_string(m_pPlayer->m_Intelligence)),
                stat("PRECISION:", std::to_string(m_pPlayer->m_Precision) + "%"),
                stat("MANA REGENERATION:", std::to_string(m_pPlayer->m_ManaRegenerationMultiplier) + "x"),
            }), 3);
      } else if (selected == 2) {
        ui::choose_option(
            "Equipment", {"Back"},
            ftxui::vbox({
              stat("HELMET:", equipment_name(EquipmentSlot::HELMET)),
              stat("CHESTPLATE:", equipment_name(EquipmentSlot::CHESTPLATE)),
              stat("BOOTS:", equipment_name(EquipmentSlot::BOOTS)),
              ftxui::text(""),
              stat("NECKLACE:", equipment_name(EquipmentSlot::NECKLACE)),
              stat("RING:", equipment_name(EquipmentSlot::RING)),
              ftxui::text(""),
              stat("WEAPON:", equipment_name(EquipmentSlot::WEAPON)),
              stat("SHIELD:", equipment_name(EquipmentSlot::SHIELD)),
            }), 3);
      } else if (selected == 3) {
        std::vector<std::string> spell_options = m_pPlayer->m_KnownSpells;
        if (spell_options.empty()) {
          //spell_options.push_back("No known spells");
          ftxui::text("");
        }
        spell_options.push_back("Back");
        ui::choose_option("Known spells", spell_options);
      } else {
        return;
      }
    }
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
          stack.item.type == ItemType::RELIC && m_pPlayer->has_active_relic(stack.item.id) ? "  /  Active" : "";
        
          choices.push_back(stack.item.name + "  x" + std::to_string(stack.quantity) + "  /  " + type + active);
      }
      choices.push_back("Return");

      const int selected = ui::choose_option("Inventory", choices);
      if (selected < 0 || selected >= static_cast<int>(m_pPlayer->m_Inventory.size())) return; 

      const Item item = m_pPlayer->m_Inventory[static_cast<std::size_t>(selected)].item;
      
      if (item.type == ItemType::EQUIPMENT) {
        if (m_pPlayer->equip_item(item.id)) {
          ui::notify("Equipment changed", item.name + " equipped.", ui::success_color());
        } else {
          ui::notify("Cannot equip", "That item has no valid equipment slot.", ui::error_color());
        }        
      } else {
        const ItemUseResult result = m_ItemUseRegistry.use_item(*m_pPlayer, item.id);
        
        ui::notify(result.succeeded ? "Item used" : "Cannot use item", result.message,
            result.succeeded ? ui::success_color() : ui::warning_color());
      }
    }
  };

  const auto build_region_actions = [this] {
    std::vector<GameAction> actions;
    actions.reserve(16);
    const Region& current_region = m_RegionWorld.current_region();
    const Place* current_place = m_RegionWorld.current_place();

    if (current_place != nullptr) {
      for (const FightingPath& path : current_place->fighting_paths) {
        actions.push_back({"Fighting", path.name, [] { return true; },
                           [this, path]() {
                             const std::uint64_t enemy_level =
                                 m_RegionWorld.encounter_level(m_pPlayer->m_Level, path);
                             Fight fight(m_pPlayer,
                                         m_EnemyGenerator.generate(enemy_level, path.id),
                                         m_SpellDatabase, m_ItemUseRegistry);
                             fight.loop();
                           }});
      }
      for (const PlaceInteraction& interaction : current_place->interactions) {
        actions.push_back({"Place", interaction.name,
                           [] { return true; },
                           [this, interaction]() {
                             m_RegionInteractionRegistry.execute(
                                 interaction.function_id,
                                 {m_pPlayer, m_ItemDatabase},
                               interaction);
                           }});
      }

      for (const PlaceExit& exit : current_place->exits) {
        const auto target = std::find_if(
            current_region.places.begin(), current_region.places.end(),
            [&exit](const Place& candidate) {
              return candidate.id == exit.target_place_id;
            });
        if (target == current_region.places.end()) {
          continue;
        }
        const Place destination = *target;
        actions.push_back({"Place", exit.name, [] { return true; },
                           [this, exit, destination]() {
                             const RegionEntryCheckResult condition_result =
                                 m_RegionEntryConditionRegistry.check(
                                     destination.entry_conditions, *m_pPlayer);
                             if (!condition_result.allowed) {
                               ui::notify("Cannot enter " + destination.name,
                                          condition_result.message,
                                          ui::error_color());
                               return;
                             }
                             const int confirmation = ui::choose_option(
                                 "Travel to " + destination.name,
                                 {"Enter " + destination.name, "Stay here"},
                                 ftxui::paragraph(destination.description));
                             if (confirmation == 0 &&
                                 m_RegionWorld.travel_to_place(exit.target_place_id)) {
                               const Region& arrived_region = m_RegionWorld.current_region();
                               ui::notify("Arrived in " + arrived_region.name,
                                          destination.name);
                             }
                           }});
      }
    } else {
      for (const Place& place : current_region.places) {
        actions.push_back({"Place", "Enter " + place.name,
                           [this] { return m_RegionWorld.current_place() == nullptr; },
                           [this, place]() {
                             const RegionEntryCheckResult condition_result =
                                 m_RegionEntryConditionRegistry.check(
                                     place.entry_conditions, *m_pPlayer);
                             if (!condition_result.allowed) {
                               ui::notify("Cannot enter " + place.name,
                                          condition_result.message,
                                          ui::error_color());
                               return;
                             }
                             const int confirmation = ui::choose_option(
                                 "Enter " + place.name,
                                 {"Enter " + place.name, "Stay in the region"},
                                 ftxui::paragraph(place.description));
                             if (confirmation != 0 ||
                                 !m_RegionWorld.enter_place(place.id)) {
                               return;
                             }
                             const Region& arrived_region = m_RegionWorld.current_region();
                             ui::notify("Entered " + place.name + " / " + arrived_region.name,
                                        arrived_region.name);
                           }});
      }
    }

    return actions;
  };

  while (continue_adventure && m_pPlayer->m_Health > 0) {
    std::vector<GameAction> actions = build_region_actions();

    actions.push_back({"Player", "Open inventory", [] { return true; }, show_inventory});
    actions.push_back({"Player", "View player statistics", [] { return true; }, show_statistics});
    
    actions.push_back({"Game", "Settings", [] { return true; }, [] { ui::settings_menu(); }});
    const Region& region = m_RegionWorld.current_region();

    actions.push_back({"Game", "Leave the adventure", [] { return true; },
      [&continue_adventure] { continue_adventure = false; }});
    
    const auto player_summary = ui::panel(
        "PLAYER",
        ftxui::vbox({
            ftxui::text(m_pPlayer->m_ClassName + "  /  LEVEL " + std::to_string(m_pPlayer->m_Level)) |
                ftxui::bold | ftxui::color(ftxui::Color::White),
            
            ui::spacer(),

              ui::meter("HEALTH", static_cast<std::uint64_t>(m_pPlayer->m_Health), 
                m_pPlayer->m_MaxHealth, ui::health_color()),
              ui::meter("MANA", m_pPlayer->m_Mana, m_pPlayer->m_MaxMana, ui::mana_color()),

              ui::spacer(),

              ftxui::text("GOLD: " + std::to_string(m_pPlayer->m_Money)) | ftxui::color(ui::wealth_color()),
            ftxui::text("GEMS: " + std::to_string(m_pPlayer->m_Gems)) | ftxui::color(ftxui::Color::RedLight),
        }),
        ui::accent_color());
      const Place* current_place = m_RegionWorld.current_place();
      const std::string place_title = current_place == nullptr
          ? std::string()
          : "  /  PLACE  /  " + current_place->name;
      ftxui::Elements region_details{
          ftxui::text(region.name) |
              ftxui::bold | ftxui::color(ui::text_color()),
      };
      if (current_place != nullptr) {
        region_details.push_back(ftxui::separator());
        region_details.push_back(ftxui::text("PLACE  /  " + current_place->name) |
                                 ftxui::bold | ftxui::color(ui::text_color()));
        region_details.push_back(ftxui::paragraph(current_place->description) |
                                 ftxui::color(ui::text_color()));
      }
      const auto region_summary = ui::panel(
          "REGION  /  " + region.name + place_title,
          ftxui::vbox(std::move(region_details)), ui::accent_color());

      std::vector<std::string> categories{"Fighting", "Place", "Player", "Game"};
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
    auto action_container =
      ftxui::Container::Horizontal({category_menu, action_menu});
    auto select_action = ftxui::CatchEvent(
      action_container,
        [&](ftxui::Event event) {
          if (event == ftxui::Event::Return && action_menu->Focused()) {
            const auto &current_group = grouped_actions[static_cast<std::size_t>(selected_category)];
            if (!current_group.empty() && selected_action >= 0 &&
                selected_action < static_cast<int>(current_group.size())) {
              current_group[static_cast<std::size_t>(selected_action)]->execute();
              screen.ExitLoopClosure()();
              return true;
            }
          }
          return ui::handle_navigation_key(event, action_container);
        });

    auto renderer = ftxui::Renderer(select_action, [&] {
      refresh_action_labels();
      const std::string current_category =
          selected_category >= 0 && selected_category < static_cast<int>(categories.size())
              ? categories[static_cast<std::size_t>(selected_category)]
              : "";
        ftxui::Element menus = ftxui::vbox({
          region_summary,
          ftxui::hbox({
                ui::panel("Categories", category_menu->Render() | ftxui::flex),
                ui::panel(current_category.empty() ? "Actions" : current_category,
                          action_menu->Render() | ftxui::flex),
                player_summary | ftxui::flex,
          }) | ftxui::flex,
        });
      return ui::page(
          "Current region: " + region.name +
            (current_place == nullptr ? "" : "  /  Place: " + current_place->name),
          menus | ftxui::flex,
          "");
    });

    screen.Loop(renderer);

    if (!continue_adventure && m_pPlayer->m_Health <= 0) {
      break;
    }
  }
}
} // namespace rpg
