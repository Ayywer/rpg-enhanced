// header
#include "fight.hpp"

namespace rpg {
Fight::Fight(std::shared_ptr<Player> player, std::shared_ptr<Enemy> enemy) : 
  m_pEnemy(enemy), m_pPlayer(player), m_ExpReward(enemy->m_RewardExp),
  m_MoneyReward(enemy->m_RewardMoney), m_GemsReward(enemy->m_RewardGems) {}

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
  std::uniform_int_distribution<std::uint64_t> distribution(1, 100);
  std::mt19937 engine(rd());

  bool players_turn = true;

  while (m_pEnemy->m_Health > 0 && m_pPlayer->m_Health > 0) {
    std::cout << "\n----------------------------------------\n"
              << "              BATTLE STATUS\n"
              << "----------------------------------------\n"
              << "  Your health:  " << m_pPlayer->m_Health << '\n'
              << "  Your mana:    " << m_pPlayer->m_Mana << '\n'
              << "  Your level:   " << m_pPlayer->m_Level << '\n'
              << '\n'
              << "  Enemy health: " << m_pEnemy->m_Health << '\n'
              << "----------------------------------------\n";

    if (players_turn) {
      BodyPart body_part = BodyPart::CHEST;
      std::uint64_t chance = 90;
      std::uint64_t attack_damage = 2;

      char input;
      std::cout << "Your turn\n"
                << "  [a] Attack\n"
                << "  [h] Heal yourself (costs 1 mana, 50% chance)\n"
                << "  [d] Cast additional damage (costs 2 mana, 50% chance)\n"
                << "Choose an action: ";
      std::cin >> input;

      if (input == 'a') {
        std::cout << "Choose a target\n"
                  << "  [b] Boots  (50% chance, power 4)\n"
                  << "  [c] Chest  (90% chance, power 2)\n"
                  << "  [h] Head   (20% chance, power 5)\n"
                  << "Choose a target: ";
        std::cin >> input;
        switch (input) {
        case 'b':
          body_part = BodyPart::BOOTS;
          chance = 50;
          attack_damage = 4;
          break;

        case 'c':
          body_part = BodyPart::CHEST;
          chance = 90;
          attack_damage = 2;
          break;

        case 'h':
          body_part = BodyPart::HEAD;
          chance = 20;
          attack_damage = 5;
          break;

        default:
          body_part = BodyPart::CHEST;
          chance = 90;
          attack_damage = 2;
          break;
        }

        if (distribution(engine) < chance) {
          const std::uint64_t damage = PlayerHit(m_pPlayer, m_pEnemy, attack_damage, body_part);
          std::cout << "You hit the enemy for " << damage << " damage!\n";
        } else {
          std::cout << "You missed!\n";
        }
      } else if (input == 'h') {
        if (m_pPlayer->m_Mana < 1) {
          std::cout << "\n  Not enough mana.\n";
          continue;
        }

        if (distribution(engine) < 50) {
          const SpellResult result = cast_heal(m_pPlayer);
          std::cout << "\n  Heal spell succeeded: +" << result.heal << " health.\n";
        } else {
          std::cout << "\n  The heal spell failed.\n";
        }
      } else if (input == 'd') {
        if (m_pPlayer->m_Mana < 2) {
          std::cout << "\n  Not enough mana.\n";
          continue;
        }

        if (distribution(engine) < 50) {
          const SpellResult result = cast_damage_spell(m_pPlayer, m_pEnemy);
          std::cout << "\n  Damage spell succeeded: +" << result.damage << " damage.\n";
        } else {
          std::cout << "\n  The damage spell failed.\n";
        }
      }

      m_pPlayer->m_Mana += m_pPlayer->m_ManaRegenerationMultiplier;
      std::cout << "  Mana regenerated: +" << m_pPlayer->m_ManaRegenerationMultiplier << ".\n";

      if (m_pPlayer->m_Mana > m_pPlayer->m_MaxMana) {
        m_pPlayer->m_Mana = m_pPlayer->m_MaxMana;
      }

      players_turn = false;
    } else {
      std::cout << "Enemy turn.\n";

      if (distribution(engine) < 50) {
        std::uint64_t random = distribution(engine);

        std::uint64_t damage = 1;
        if (random <= 33) {
          damage = EnemyHit(m_pEnemy, m_pPlayer, 4, BodyPart::BOOTS);
        } else if (random > 33 && random <= 66) {
          damage = EnemyHit(m_pEnemy, m_pPlayer, 2, BodyPart::CHEST);
        } else if (random > 66) {
          damage = EnemyHit(m_pEnemy, m_pPlayer, 5, BodyPart::HEAD);
        }

        std::cout << "Enemy dealt " << damage << " damage.\n";
      } else {
        std::cout << "The enemy missed.\n";
      }

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
