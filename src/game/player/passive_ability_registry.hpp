#pragma once

#include "equipment.hpp"
#include "item.hpp"

#include <cstdint>
#include <functional>
#include <string>
#include <unordered_map>

namespace rpg {

class Enemy;
class Player;

enum class PassiveAbilityTrigger {
  ATTACK,
  DEFENSE,
};

struct PassiveAbilityContext {
  const Item& item;
  const Player& owner;
  const Enemy& opponent;
  std::uint64_t raw_damage;
  BodyPart body_part;
  PassiveAbilityTrigger trigger;
};

class PassiveAbilityRegistry {
public:
  using Handler = std::function<std::uint64_t(const PassiveAbilityContext&)>;

  static void register_builtin_function(std::string ability_id,
                                        Handler handler);
  static bool has_builtin_function(const std::string& ability_id);
  static std::uint64_t apply(const Item& item, const Player& owner,
                             const Enemy& opponent, std::uint64_t raw_damage,
                             BodyPart body_part,
                             PassiveAbilityTrigger trigger);

private:
  using HandlerMap = std::unordered_map<std::string, Handler>;
  static HandlerMap& builtin_handlers();
};

} // namespace rpg