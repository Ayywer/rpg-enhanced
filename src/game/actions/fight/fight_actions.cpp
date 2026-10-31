#include "fight_actions.hpp"

#include "../../console.hpp"
#include "attack.hpp"
#include <optional>
#include <utility>

namespace rpg {
namespace {

struct AttackChoice {
    BodyPart body_part;
    std::uint64_t hit_chance;
    std::uint64_t damage_multiplier;
};

std::uint64_t precision_scaled_chance(std::uint64_t base_chance,
                                      std::uint64_t precision) {
    return base_chance * precision / 100;
}

std::optional<AttackChoice> choose_attack() {
    const int choice = ui::choose_option(
        "Choose a target",
        {"Head      / 20% chance / 5x damage",
         "Chest     / 90% chance",
         "Boots     / 50% chance / 3x damage",
         "Back to actions"});

    switch (choice) {
    case 0:
        return AttackChoice{BodyPart::HEAD, 20, 5};
    case 1:
        return AttackChoice{BodyPart::CHEST, 90, 1};
    case 2:
        return AttackChoice{BodyPart::BOOTS, 50, 3};
    default:
        return std::nullopt;
    }
}

} // namespace

std::uint64_t FightActionContext::roll_percent() {
    std::uniform_int_distribution<std::uint64_t> distribution(1, 100);
    return distribution(random_engine);
}

FightActions::FightActions(const SpellDatabase& spell_database,
                                                     const ItemUseRegistry& item_use_registry)
        : m_SpellDatabase(spell_database),
            m_ItemUseRegistry(item_use_registry) {
    register_action("Attack", [](FightActionContext& context) {
        const std::optional<AttackChoice> choice = choose_attack();
        if (!choice) {
            context.message = "You returned to the action menu.";
            return FightActionResult::RETRY;
        }
        const std::uint64_t hit_chance = precision_scaled_chance(
            choice->hit_chance, context.player->m_Precision);
        if (context.roll_percent() <= hit_chance) {
            const std::uint64_t damage = perform_attack(
                    context.player, context.enemy, choice->damage_multiplier,
                    choice->body_part);
            context.message = "You hit " + context.enemy->m_Name + " for " +
                              std::to_string(damage) + " damage.";
        } else {
            context.message = "Your attack missed.";
        }
        return FightActionResult::TURN_CONSUMED;
    });
    register_action("Use item", [this](FightActionContext& context) {
        std::vector<const ItemStack*> usable_items;
        std::vector<std::string> choices;
        for (const ItemStack& stack : context.player->m_Inventory) {
            if (stack.item.type == ItemType::EQUIPMENT ||
                stack.item.type == ItemType::RELIC || stack.quantity == 0) {
                continue;
            }
            usable_items.push_back(&stack);
            choices.push_back(stack.item.name + "  x" +
                              std::to_string(stack.quantity));
        }
        if (choices.empty()) {
            context.message = "You have no usable items.";
            return FightActionResult::RETRY;
        }
        choices.push_back("Back to actions");
        const int selected = ui::choose_option("Choose an item", choices);
        if (selected < 0 || selected >= static_cast<int>(usable_items.size())) {
            context.message = "You returned to the action menu.";
            return FightActionResult::RETRY;
        }
        const ItemStack& stack = *usable_items[static_cast<std::size_t>(selected)];
        const Item item = stack.item;
        const ItemUseResult result = m_ItemUseRegistry.use_item(
            *context.player, item.id, context.enemy.get());
        if (!result.succeeded) {
            context.message = result.message;
            return FightActionResult::RETRY;
        }
        context.message = result.message;
        return FightActionResult::TURN_CONSUMED;
    });
}

void FightActions::register_action(std::string label,
                                  FightActionHandler handler) {
    m_Actions.push_back({std::move(label), std::move(handler)});
}

std::vector<std::string> FightActions::labels(const Player& player) const {
    std::vector<std::string> result;
    const std::vector<const SpellDefinition*> spells =
        m_SpellDatabase.known_spells(player);
    result.reserve(m_Actions.size() + spells.size());
    for (const Action& action : m_Actions) {
        result.push_back(action.label);
    }
    for (const SpellDefinition* spell : spells) {
        result.push_back(spell->name + "  |  " +
                         std::to_string(spell->mana_cost) + " mana  |  " +
                         std::to_string(spell->success_chance) + "%");
    }
    return result;
}

std::vector<std::string> FightActions::learn_available(Player& player) const {
    return m_SpellDatabase.learn_available(player);
}

FightActionResult FightActions::execute(std::size_t index,
                                       FightActionContext& context) const {
    if (index < m_Actions.size()) {
        return m_Actions[index].handler(context);
    }

    const std::vector<const SpellDefinition*> spells =
        m_SpellDatabase.known_spells(*context.player);
    const std::size_t spell_index = index - m_Actions.size();
    if (spell_index >= spells.size()) {
        context.message = "That action is not available.";
        return FightActionResult::RETRY;
    }

    const SpellDefinition& spell = *spells[spell_index];
    if (context.player->m_Mana < spell.mana_cost) {
        context.message = "Not enough mana to cast " + spell.name + ".";
        return FightActionResult::RETRY;
    }

    const std::uint64_t success_chance = precision_scaled_chance(
        spell.success_chance, context.player->m_Precision);
    const bool succeeds = context.roll_percent() <= success_chance;
    const std::uint64_t enemy_health_before = static_cast<std::uint64_t>(
        std::max<std::int64_t>(0, context.enemy->m_Health));
    const SpellResult result = m_SpellDatabase.cast(
        *context.player, *context.enemy, spell, succeeds);
    if (!result.succeeded) {
        context.message = spell.name + " failed.";
        } else if (result.heal > 0) {
        context.message = spell.name + " restored " +
                          std::to_string(result.heal) + " health and spent " +
                          std::to_string(result.mana_spent) + " mana.";
        } else if (result.damage > 0) {
        context.message = spell.name + " dealt " +
                          std::to_string(result.damage) + " damage and spent " +
                          std::to_string(result.mana_spent) + " mana.";
        if (result.damage > 0 && result.damage == enemy_health_before) {
            context.message += " The enemy had " +
                               std::to_string(enemy_health_before) +
                               " health remaining.";
        }
    } else {
        context.message = spell.name + " took effect and spent " +
                          std::to_string(result.mana_spent) + " mana.";
    }
    return FightActionResult::TURN_CONSUMED;
}

void perform_enemy_turn(FightActionContext& context) {
    const std::uint64_t hit_chance =
        precision_scaled_chance(70, context.enemy->m_Precision);
    if (context.roll_percent() <= hit_chance) {
        const std::uint64_t target = context.roll_percent();
        std::uint64_t damage = 1;
        if (target <= 33) {
            damage = perform_enemy_attack(context.enemy, context.player, 4, BodyPart::BOOTS);
        } else if (target <= 66) {
            damage = perform_enemy_attack(context.enemy, context.player, 2, BodyPart::CHEST);
        } else {
            damage = perform_enemy_attack(context.enemy, context.player, 5, BodyPart::HEAD);
        }
        context.message = context.enemy->m_Name + " dealt " + std::to_string(damage) + " damage.";
    } else {
        context.message = context.enemy->m_Name + " missed.";
    }
}

} // namespace rpg