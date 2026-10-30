// header
#include "game.hpp"

// local
#include "actions/fight/fight.hpp"
#include "actions/shop/shop.hpp"
#include "console.hpp"

#include <functional>

namespace rpg {
void Game::setup() {
  std::vector<std::string> classes;
  std::vector<char> class_shortcuts;
  for (const PlayerClassChoice &choice : m_PlayerClassGenerator.choices()) {
    classes.push_back(choice.name + "  [" + choice.key + "]");
    class_shortcuts.push_back(choice.key);
  }

  const int selected = ui::choose_option("Choose your class", classes, ftxui::emptyElement(), class_shortcuts);

  if (selected >= 0 && selected < static_cast<int>(m_PlayerClassGenerator.choices().size())) {
    m_pPlayer = m_PlayerClassGenerator.create(m_PlayerClassGenerator.choices()[selected].key);
    m_SpellDatabase.learn_available(*m_pPlayer);
  }
}

void Game::loop() {

  struct GameAction {
    char shortcut;
    std::string label;
    std::function<bool()> is_available;
    std::function<void()> execute;
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

  const std::vector<GameAction> actions{
      {'a', "Enter the arena", [] { return true; },
       [this] {
         Fight fight(m_pPlayer, m_EnemyGenerator.generate(m_pPlayer->m_Level),
                     m_SpellDatabase, m_ItemUseRegistry);
         fight.loop();
       }},
      {'s', "Visit the armor smith", [] { return true; },
       [this] {
         Shop shop(m_pPlayer, m_ItemDatabase);
         shop.loop();
       }},
      {'i', "Open inventory", [] { return true; }, show_inventory},
      {'v', "View player statistics", [] { return true; }, show_statistics},
      {'q', "Leave the adventure", [] { return true; },
       [&continue_adventure] { continue_adventure = false; }},
  };

  while (continue_adventure && m_pPlayer->m_Health > 0) {
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

    std::vector<const GameAction *> available_actions;
    std::vector<std::string> action_labels;
    std::vector<char> action_shortcuts;

    for (const GameAction &game_action : actions) {
      if (game_action.is_available()) {
        available_actions.push_back(&game_action);
        action_labels.push_back("[" + std::string(1, game_action.shortcut) + "]  " + game_action.label);
        action_shortcuts.push_back(game_action.shortcut);
      }
    }

    const int selected = ui::choose_option("", action_labels, player_summary, action_shortcuts);

    if (selected < 0 || selected >= static_cast<int>(available_actions.size())) break;
    
    available_actions[selected]->execute();
  }
}
} // namespace rpg
