#pragma once

#include "../../enemy/enemy.hpp"
#include "../../player/player.hpp"

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
  FightActions();

  void register_action(char key, std::string label,
                       FightActionHandler handler);
  void print_actions() const;
  FightActionResult execute(char key, FightActionContext& context) const;

private:
  struct Action {
    char key;
    std::string label;
    FightActionHandler handler;
  };

  std::vector<Action> m_Actions;
};

void perform_enemy_turn(FightActionContext& context);

} // namespace rpg