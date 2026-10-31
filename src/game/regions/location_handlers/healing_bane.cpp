#include "../location_registry.hpp"

#include "game/console.hpp"
#include "game/player/player.hpp"

namespace rpg {

namespace {
[[maybe_unused]] const bool registered = [] {
    RegionLocationRegistry::register_builtin_function(
        "healing_bane", [](const RegionLocationContext& context,
                           const RegionLocation& location) {
            context.player->m_Health = context.player->m_MaxHealth;
            context.player->m_Mana = context.player->m_MaxMana;
            ui::notify(location.name,
                       location.description + " Health and mana restored.");
        });
    return true;
}();
} // namespace

} // namespace rpg