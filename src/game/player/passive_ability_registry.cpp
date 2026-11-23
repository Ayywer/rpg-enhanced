#include "passive_ability_registry.hpp"

#include <stdexcept>
#include <utility>

namespace rpg {

PassiveAbilityRegistry::HandlerMap&
PassiveAbilityRegistry::builtin_handlers() {
  static HandlerMap handlers;
  return handlers;
}

void PassiveAbilityRegistry::register_builtin_function(std::string ability_id,
                                                        Handler handler) {
  if (ability_id.empty() || ability_id == "none" || !handler) {
    throw std::invalid_argument(
        "Passive ability requires a non-empty ID and handler");
  }
  if (!builtin_handlers().emplace(std::move(ability_id), std::move(handler)).second) {
    throw std::invalid_argument("Duplicate passive ability registration");
  }
}

bool PassiveAbilityRegistry::has_builtin_function(
    const std::string& ability_id) {
  return builtin_handlers().contains(ability_id);
}

std::uint64_t PassiveAbilityRegistry::apply(
    const Item& item, const Player& owner, const Enemy& opponent,
    std::uint64_t raw_damage, BodyPart body_part,
    PassiveAbilityTrigger trigger) {
  const auto handler = builtin_handlers().find(item.passive_ability);
  if (handler == builtin_handlers().end()) {
    return raw_damage;
  }
  return handler->second(
      {item, owner, opponent, raw_damage, body_part, trigger});
}

} // namespace rpg