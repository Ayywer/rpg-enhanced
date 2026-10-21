#include "leveling.hpp"

namespace rpg {

void apply_level_progress(std::shared_ptr<Player> player, std::uint64_t exp_gain) {
  player->m_Exp += exp_gain;

  while (player->m_Exp >= player->m_MaxExp) {
    player->m_Exp -= player->m_MaxExp;
    ++player->m_Level;
    player->m_MaxExp *= 2;

  }
}

} // namespace rpg
