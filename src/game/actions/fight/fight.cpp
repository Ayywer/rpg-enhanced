// header
#include "fight.hpp"

#include "attack.hpp"
#include "../../leveling.hpp"

#include <iostream>
#include <random>
#include <utility>

namespace rpg {
Fight::Fight(std::shared_ptr<Player> player, std::shared_ptr<Enemy> enemy) : 
  m_pEnemy(enemy), m_pPlayer(player), m_ExpReward(enemy->m_RewardExp),
  m_MoneyReward(enemy->m_RewardMoney), m_GemsReward(enemy->m_RewardGems) {}

void Fight::register_action(char key, std::string label,
                            FightActionHandler handler) {
  m_Actions.register_action(key, std::move(label), std::move(handler));
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
  m_pPlayer->m_Mana = m_pPlayer->m_MaxMana;

  std::random_device rd;
  std::mt19937 engine(rd());
  FightActionContext context{m_pPlayer, m_pEnemy, engine};

  bool players_turn = true;

  while (m_pEnemy->m_Health > 0 && m_pPlayer->m_Health > 0) {
    std::cout << "<---------------------------------------->" << '\n'
              << "              BATTLE STATUS               " << '\n'
              << "<---------------------------------------->" << '\n'
              << "                 PLAYER                   " << '\n'
              << "Your health:"   << m_pPlayer->m_Health      << '\n'
              << "Your mana:"     << m_pPlayer->m_Mana        << '\n'
              << "Your level:"    << m_pPlayer->m_Level       << '\n'
              << "<---------------------------------------->" << '\n'
              << "                 ENEMY                    " << '\n'
              << "Enemy health:"  << m_pEnemy->m_Health       << '\n'
              << "Enemy level:"   << m_pEnemy->m_Level        << '\n'
              << "<---------------------------------------->" << '\n';

    if (players_turn) {
      char input;
      std::cout << "YOUR TURN\n";
      m_Actions.print_actions();
      std::cout << "Choose an action: ";
      std::cin >> input;

      if (m_Actions.execute(input, context) == FightActionResult::RETRY) {
        continue;
      }

      m_pPlayer->m_Mana += m_pPlayer->m_ManaRegenerationMultiplier;
      std::cout << "  Mana regenerated: +" << m_pPlayer->m_ManaRegenerationMultiplier << ".\n";

      if (m_pPlayer->m_Mana > m_pPlayer->m_MaxMana) {
        m_pPlayer->m_Mana = m_pPlayer->m_MaxMana;
      }

      players_turn = false;
    } else {
      std::cout << "Enemy turn.\n";
      perform_enemy_turn(context);
      players_turn = true;
    }

  }

  if (m_pEnemy->m_Health <= 0) {
    m_pPlayer->m_Money += m_MoneyReward;
    m_pPlayer->m_Gems += m_GemsReward;

    std::cout << "\n========================================\n"
              << "              VICTORY!\n"
              << "========================================\n"
              << "  Reward: " << m_ExpReward << " EXP and " << m_MoneyReward
              << " gold.\n";

    apply_level_progress(m_pPlayer, m_ExpReward);
    std::cout << " Your Level progress: " << m_pPlayer->m_Exp << " / "
              << m_pPlayer->m_MaxExp << " EXP\n";
  } else {
    std::cout << "\n========================================\n"
              << "              DEFEAT\n"
              << "========================================\n"
              << "  You were defeated by the enemy.\n";
  }
}
} // namespace rpg
