#include "fight_actions.hpp"

#include "../../console.hpp"
#include "attack.hpp"
#include "spells.hpp"

#include <iostream>
#include <utility>

namespace rpg {
namespace {

struct AttackChoice {
    BodyPart body_part;
    std::uint64_t hit_chance;
    std::uint64_t damage_multiplier;
};

AttackChoice choose_attack() {
    char input;
    clear_console();
    std::cout << std::endl;
    std::cout << "Choose a hit point \n"
              << "[h] Head  (20% chance)\n"
              << "[c] Chest  (90% chance)\n"
              << "[b] Boots   (50% chance)\n"
              << "Your action: ";
    std::cin >> input;

    switch (input) {
    case 'b':
        return {BodyPart::BOOTS, 50, 4};
    case 'h':
        return {BodyPart::HEAD, 20, 5};
    default:
        return {BodyPart::CHEST, 90, 2};
    }
}

} // namespace

std::uint64_t FightActionContext::roll_percent() {
    std::uniform_int_distribution<std::uint64_t> distribution(1, 100);
    return distribution(random_engine);
}

FightActions::FightActions() {
    register_action('a', "Attack", [](FightActionContext& context) {
        const AttackChoice choice = choose_attack();
        if (context.roll_percent() <= choice.hit_chance) {
            const std::uint64_t damage = perform_attack(
                    context.player, context.enemy, choice.damage_multiplier,
                    choice.body_part);
            std::cout << std::endl;
            std::cout << "You hit the enemy for " << damage << " damage!\n";
        } else {
            std::cout << std::endl;
            std::cout << "You missed!\n";
        }
        return FightActionResult::TURN_CONSUMED;
    });

    register_action('h', "Heal yourself (costs 1 mana, 50% chance)",
                    [](FightActionContext& context) {
        if (context.player->m_Mana < 1) {
            std::cout << "\n  Not enough mana.\n";
            return FightActionResult::RETRY;
        }

        if (context.roll_percent() <= 50) {
            const SpellResult result = cast_heal(context.player);
            std::cout << "\n  Heal spell succeeded: +" << result.heal
                      << " health.\n";
        } else {
            std::cout << "\n  The heal spell failed.\n";
        }
        return FightActionResult::TURN_CONSUMED;
    });

    register_action('d', "Cast additional damage (costs 2 mana, 50% chance)",
                    [](FightActionContext& context) {
        if (context.player->m_Mana < 2) {
            std::cout << "\n  Not enough mana.\n";
            return FightActionResult::RETRY;
        }

        if (context.roll_percent() <= 50) {
            const SpellResult result =
                    cast_damage_spell(context.player, context.enemy);
            std::cout << "\n  Damage spell succeeded: +" << result.damage
                  << " damage.\n";
        } else {
            std::cout << "\n  The damage spell failed.\n";
        }
        return FightActionResult::TURN_CONSUMED;
    });
}

void FightActions::register_action(char key, std::string label,
                                  FightActionHandler handler) {
    for (Action& action : m_Actions) {
        if (action.key == key) {
            action.label = std::move(label);
            action.handler = std::move(handler);
            return;
        }
    }
    m_Actions.push_back({key, std::move(label), std::move(handler)});
}

void FightActions::print_actions() const {
    for (const Action& action : m_Actions) {
        std::cout << "[" << action.key << "] " << action.label << '\n';
    }
}

FightActionResult FightActions::execute(char key,
                                       FightActionContext& context) const {
    for (const Action& action : m_Actions) {
        if (action.key == key) {
            return action.handler(context);
        }
    }
    std::cout << "\n  Unknown action.\n";
    return FightActionResult::RETRY;
}

void perform_enemy_turn(FightActionContext& context) {
    if (context.roll_percent() <= 50) {
        const std::uint64_t target = context.roll_percent();
        std::uint64_t damage = 1;
        if (target <= 33) {
            damage = perform_enemy_attack(context.enemy, context.player, 4,
                                          BodyPart::BOOTS);
        } else if (target <= 66) {
            damage = perform_enemy_attack(context.enemy, context.player, 2,
                                          BodyPart::CHEST);
        } else {
            damage = perform_enemy_attack(context.enemy, context.player, 5,
                                          BodyPart::HEAD);
        }
        std::cout << std::endl;
        std::cout << "Enemy dealt " << damage << " damage.\n";
    } else {
        std::cout << std::endl;
        std::cout << "The enemy missed.\n";
    }
}

} // namespace rpg