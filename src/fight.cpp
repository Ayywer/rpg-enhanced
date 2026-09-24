// header
#include "fight.hpp"

namespace rpg {
Fight::Fight(std::shared_ptr<Player> player, std::shared_ptr<Enemy> enemy)
  : m_pEnemy(enemy), m_pPlayer(player), m_Reward(enemy->m_Exp) {}

std::uint64_t Player::attack(std::shared_ptr<Enemy> enemy,
                             std::uint64_t attack_damage,
                             BodyPart body_part) {
  const std::uint64_t raw_damage = this->m_Strength * attack_damage;
  const std::uint64_t defense =
      enemy->m_Equipment[body_part].m_Protection * enemy->m_Defense;
  const std::uint64_t damage = raw_damage > defense ? raw_damage - defense : 0;
  enemy->m_Health -= damage;
  return damage;
}

std::uint64_t Enemy::attack(std::shared_ptr<Player> player,
                            std::uint64_t attack_damage,
                            BodyPart body_part) {
  const std::uint64_t raw_damage = this->m_Strength * attack_damage;
  const std::uint64_t defense =
      player->m_Equipment[body_part].m_Protection * player->m_Defense;
  const std::uint64_t damage = raw_damage > defense ? raw_damage - defense : 0;
  player->m_Health -= damage;
  return damage;
}

std::uint64_t Fight::PlayerHit(std::shared_ptr<Player> who,
                               std::shared_ptr<Enemy> whom,
                               std::uint64_t attack_damage,
                               BodyPart body_part) {
  return who->attack(whom, attack_damage, body_part);
}

std::uint64_t Fight::EnemyHit(std::shared_ptr<Enemy> who,
                              std::shared_ptr<Player> whom,
                              std::uint64_t attack_damage,
                              BodyPart body_part) {
  return who->attack(whom, attack_damage, body_part);
}

void Fight::loop() {
  std::random_device rd;
  std::uniform_int_distribution<std::uint64_t> distribution(1, 100);
  std::mt19937 engine(rd());

  bool players_turn = true;

  uint64_t spell_damage = 0;
  uint64_t spell_heal = 0;

  uint64_t turn_count = 0;

  while (m_pEnemy->m_Health > 0 && m_pPlayer->m_Health > 0) {
    if (turn_count >= 5) {
      spell_damage = 0;
      spell_heal = 0;
    }

    std::cout << "\n----------------------------------------\n"
          << "              BATTLE STATUS\n"
          << "----------------------------------------\n"
          << "  Your health:  " << m_pPlayer->m_Health << '\n'
          << "  Enemy health: " << m_pEnemy->m_Health << '\n'
          << "----------------------------------------\n";

    if (players_turn) {
      BodyPart body_part;
      std::uint64_t chance;
      std::uint64_t attack_damage;

      char input;
      std::cout << "Your turn\n"
            << "  [a] Attack\n"
            << "  [h] Heal yourself (costs 1 EXP, 50% chance)\n"
            << "  [d] Cast additional damage (costs 2 EXP, 50% chance)\n"
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
          body_part = BodyPart::BOOTS;
          chance = 0;
          attack_damage = 0;
          break;
        }

        if (distribution(engine) < chance) {
          const std::uint64_t damage =
              PlayerHit(m_pPlayer, m_pEnemy, attack_damage, body_part);
          std::cout << "You hit the enemy for " << damage << " damage!\n";
        } else {
          std::cout << "You missed!\n";
        }
      } else if (input == 'h') {
        if (1 <= m_pPlayer->m_Exp) {
          if (distribution(engine) < 50) {
            spell_heal = 10;

            std::cout << "\n  Heal spell succeeded: +10 health.\n";
          } else {
            std::cout << "\n  The heal spell failed.\n";
          }

          m_pPlayer->m_Exp -= 1;
        } else {
          std::cout << "\n  Not enough EXP.\n";
          continue;
        }
      } else if (input == 'd') {
        if (2 <= m_pPlayer->m_Exp) {

          if (distribution(engine) < 50) {
            spell_damage = 10;

            std::cout << "\n  Damage spell succeeded: +10 damage.\n";
          } else {
            std::cout << "\n  The damage spell failed.\n";
          }

          m_pPlayer->m_Exp -= 2;
        } else {
          std::cout << "\n  Not enough EXP.\n";
          continue;
        }
      }

      m_pPlayer->m_Health += spell_heal;
      m_pEnemy->m_Health -= spell_damage;

      players_turn = false;
    } else {
      std::cout << "Enemy turn\n";

      if (distribution(engine) < 50) {
        std::uint64_t random = distribution(engine);

        std::uint64_t damage = 0;
        if (random <= 33) {
          damage = EnemyHit(m_pEnemy, m_pPlayer, 1, BodyPart::BOOTS);
        } else if (random > 33 && random <= 66) {
          damage = EnemyHit(m_pEnemy, m_pPlayer, 1, BodyPart::CHEST);
        } else if (random > 66) {
          damage = EnemyHit(m_pEnemy, m_pPlayer, 1, BodyPart::HEAD);
        }

        std::cout << "  Enemy dealt " << damage << " damage.\n";
      } else {
        std::cout << "  The enemy missed.\n";
      }

      players_turn = true;
    }

    std::cout << '\n';
  }

  if (m_pEnemy->m_Health <= 0) {
    std::cout << "\n========================================\n"
          << "              VICTORY!\n"
          << "========================================\n"
          << "  Reward: " << m_Reward << " EXP and " << m_Reward
          << " gold.\n";
    m_pPlayer->m_Money += m_Reward;
    m_pPlayer->m_Exp += m_Reward;
  } else {
    std::cout << "\n========================================\n"
          << "              DEFEAT\n"
          << "========================================\n"
          << "  You were defeated by the enemy.\n";
  }
}
} // namespace rpg
