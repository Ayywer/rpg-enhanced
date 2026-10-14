#include "fight_actions.hpp"

#include "attack.hpp"
#include "spells.hpp"

#include <iostream>

namespace rpg {

std::uint64_t FightActionContext::roll_percent() {
    std::uniform_int_distribution<std::uint64_t> distribution(1, 100);
    return distribution(random_engine);
}

FightActions::FightActions() {
    register_action('a', "Attack", [](FightActionContext& context) {
        BodyPart body_part = BodyPart::CHEST;
        std::uint64_t chance = 90;
        std::uint64_t attack_damage = 2;

        char input;
        std::cout << "Choose a hit point \n"
                  << "[b] Boots  (50% chance)\n"
                  << "[c] Chest  (90% chance)\n"
                  << "[h] Head   (20% chance)\n"
                  << "Your action: ";
        std::cin >> input;

        switch (input) {
        case 'b':
            body_part = BodyPart::BOOTS;
            chance = 50;
            attack_damage = 4;
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

        if (context.roll_percent() < chance) {
            const std::uint64_t damage = perform_attack(
                    context.player, context.enemy, attack_damage, body_part);
            std::cout << "You hit the enemy for " << damage << " damage!\n";
        } else {
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

        if (context.roll_percent() < 50) {
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

        if (context.roll_percent() < 50) {
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
    if (context.roll_percent() < 50) {
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
        std::cout << "Enemy dealt " << damage << " damage.\n";
    } else {
        std::cout << "The enemy missed.\n";
    }
}

} // namespace rpg