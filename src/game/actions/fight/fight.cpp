// header
#include "fight.hpp"

#include "../../console.hpp"
#include "attack.hpp"
#include "../../leveling.hpp"

#include <random>
#include <string>
#include <utility>

namespace rpg {
namespace {

ftxui::Element battle_status(const std::shared_ptr<Player>& player,
               const std::shared_ptr<Enemy>& enemy) {
  const auto player_panel = ui::panel(
    "YOU  /  " + player->m_ClassName,
    ftxui::vbox({
      ui::meter("HEALTH", static_cast<std::uint64_t>(player->m_Health), player->m_MaxHealth, ui::health_color()),
      ui::meter("MANA", player->m_Mana, player->m_MaxMana, ui::mana_color()),
      ftxui::text("LEVEL " + std::to_string(player->m_Level)) | ftxui::color(ui::text_color()),

    }),
    ui::accent_color());

  const auto enemy_panel = ui::panel(
    "FOE  /  " + enemy->m_Name,
    ftxui::vbox({
      ui::meter("HEALTH", static_cast<std::uint64_t>( std::max<std::int64_t>(0, enemy->m_Health)), enemy->m_MaxHealth, ui::error_color()),
      ftxui::text("LEVEL  " + std::to_string(enemy->m_Level)) | ftxui::color(ui::text_color()),
      ui::spacer(),
      ftxui::text("STRENGTH  " + std::to_string(enemy->m_Strength)) | ftxui::color(ui::text_color()),
      ftxui::text("DEFENCE  " + std::to_string(enemy->m_Defense)) | ftxui::color(ui::text_color()),
      ftxui::text("PRECISION  " + std::to_string(enemy->m_Precision) + "%") | ftxui::color(ui::text_color()),
    }),
    ui::error_color());
  return ftxui::hbox({player_panel, enemy_panel});
}

} // namespace

Fight::Fight(std::shared_ptr<Player> player, std::shared_ptr<Enemy> enemy,
             const SpellDatabase& spell_database,
             const ItemUseRegistry& item_use_registry)
    : m_pEnemy(enemy),
      m_pPlayer(player),
      m_Actions(spell_database, item_use_registry),
      m_ExpReward(enemy->m_RewardExp),
      m_MoneyReward(enemy->m_RewardMoney),
      m_GemsReward(enemy->m_RewardGems) {}

void Fight::register_action(std::string label,
                            FightActionHandler handler) {
  m_Actions.register_action(std::move(label), std::move(handler));
}

std::uint64_t Fight::PlayerHit(std::shared_ptr<Player> who,
                               std::shared_ptr<Enemy> whom,
                               std::uint64_t attack_damage,
                               BodyPart body_part) {
  return perform_attack(who, whom, attack_damage, body_part);
}

std::uint64_t Fight::EnemyHit(std::shared_ptr<Enemy> who,
                              std::shared_ptr<Player> whom,
                              std::uint64_t attack_damage, BodyPart body_part) {
  return perform_enemy_attack(who, whom, attack_damage, body_part);
}

void Fight::loop() {
  if (m_pPlayer->m_ClassName == "Mage") {
    m_pPlayer->m_Mana = m_pPlayer->m_MaxMana;
  }

  std::random_device rd;
  std::mt19937 engine(rd());
  FightActionContext context{m_pPlayer, m_pEnemy, engine, {}};

  bool players_turn = true;

  while (m_pEnemy->m_Health > 0 && m_pPlayer->m_Health > 0) {
    if (players_turn) {
      const std::vector<std::string> actions = m_Actions.labels(*m_pPlayer);
      const int selected = ui::choose_option(
          "Your turn", actions, battle_status(m_pPlayer, m_pEnemy));
      if (selected < 0) {
        break;
      }

      context.message.clear();
      if (m_Actions.execute(static_cast<std::size_t>(selected), context) ==
          FightActionResult::RETRY) {
        ui::notify("Action unavailable", ui::combat_log_message(context.message),
             ui::warning_color());
        continue;
      }

      m_pPlayer->m_Mana += m_pPlayer->m_ManaRegenerationMultiplier;
      if (m_pPlayer->m_Mana > m_pPlayer->m_MaxMana) {
        m_pPlayer->m_Mana = m_pPlayer->m_MaxMana;
      }
      context.message += "\nMana regenerated: +" +
                         std::to_string(m_pPlayer->m_ManaRegenerationMultiplier) +
                         ".";
      ui::notify("Your turn", ui::combat_log_message(context.message));
      players_turn = false;
    } else {
      context.message.clear();
      perform_enemy_turn(context);
      ui::notify("Enemy turn", ui::combat_log_message(context.message),
             ui::error_color());
      players_turn = true;
    }

  }

  if (m_pEnemy->m_Health <= 0) {
    m_pPlayer->m_Money += m_MoneyReward;
    m_pPlayer->m_Gems += m_GemsReward;

    const std::uint64_t previous_level = m_pPlayer->m_Level;
    apply_level_progress(m_pPlayer, m_ExpReward);
    const std::vector<std::string> learned_spells = m_Actions.learn_available(*m_pPlayer);
    
    std::string result = "You defeated " + m_pEnemy->m_Name +
                         ".\n\nRewards: " + "\n" + 
                         std::to_string(m_ExpReward) + " EXP\n" + 
                         std::to_string(m_MoneyReward) + " Gold\n" + 
                         std::to_string(m_GemsReward) + " Gems" + 
                         "\n\nLevel:" + std::to_string(m_pPlayer->m_Level) + 
                         "\nProgress: " + std::to_string(m_pPlayer->m_Exp) +
                         "/" + std::to_string(m_pPlayer->m_MaxExp) + " EXP.";
    
    if (m_pPlayer->m_Level > previous_level) {
      result += "\n\nYou reached level " + std::to_string(m_pPlayer->m_Level) + "!";
    }

    if (!learned_spells.empty()) {
      result += "\n\nNew spells learned: ";
      
      for (std::size_t index = 0; index < learned_spells.size(); ++index) {
        if (index != 0) {
          result += ", ";
        }
        result += learned_spells[index];
      }

      result += ".";
    }
    ui::notify("Victory", ui::combat_log_message(result), ui::success_color());
  } else {
    ui::notify("GAME OVER", "You were defeated by " + m_pEnemy->m_Name + ".", ui::error_color());
  }
}
} // namespace rpg
