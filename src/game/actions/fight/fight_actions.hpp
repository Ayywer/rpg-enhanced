#pragma once

#include "../../enemy/enemy.hpp"
#include "../../player/player.hpp"
#include "spells.hpp"

#include <cstddef>
#include <cstdint>
#include <functional>
#include <random>
#include <string>
#include <vector>

namespace rpg {

struct FightActionContext {
  std::shared_ptr<Player> player;
  std::shared_ptr<Enemy> enemy;
  std::mt19937& random_engine;
  std::string message;

  std::uint64_t roll_percent();
};

enum class FightActionResult {
  TURN_CONSUMED,
  RETRY
};

using FightActionHandler =
    std::function<FightActionResult(FightActionContext&)>;

class FightActions {
public:
  explicit FightActions(const SpellDatabase& spell_database);

  void register_action(char key, std::string label,
                       FightActionHandler handler);
  std::vector<std::string> labels(const Player& player) const;
  std::vector<char> shortcuts() const;
  std::vector<std::string> learn_available(Player& player) const;
  FightActionResult execute(std::size_t index,
                            FightActionContext& context) const;

private:
  struct Action {
    char key;
    std::string label;
    FightActionHandler handler;
  };

  std::vector<Action> m_Actions;
  const SpellDatabase& m_SpellDatabase;
};

void perform_enemy_turn(FightActionContext& context);

} // namespace rpg