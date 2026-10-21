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
    const auto protection = [this](BodyPart part) {
      const auto equipment = m_pPlayer->m_Equipment.find(part);
      return equipment == m_pPlayer->m_Equipment.end() ? std::uint64_t{0} : equipment->second.m_Protection;
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
            stat("MANA REGENERATION:", std::to_string(m_pPlayer->m_ManaRegenerationMultiplier) + "x"),

            ftxui::separator(),

            section("EQUIPMENT & WEALTH:"),
            section(""),
            stat("HEAD:", std::to_string(protection(BodyPart::HEAD))),
            stat("CHEST:", std::to_string(protection(BodyPart::CHEST))),
            stat("BOOTS:", std::to_string(protection(BodyPart::BOOTS))),

            ftxui::separator(),

            section("KNOWN SPELLS"),

            ftxui::paragraph(known_spells) | ftxui::color(ftxui::Color::White),
        }),
        ftxui::Color::RGB(91, 205, 183));
    ui::choose_option("Player statistics", {"Return to the road"}, stats);
  };

  const std::vector<GameAction> actions{
      {'a', "Enter the arena", [] { return true; },
       [this] {
         Fight fight(m_pPlayer, m_EnemyGenerator.generate(m_pPlayer->m_Level), m_SpellDatabase);
         fight.loop();
       }},
      {'s', "Visit the armor smith", [] { return true; },
       [this] {
         Shop shop(m_pPlayer);
         shop.loop();
       }},
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
