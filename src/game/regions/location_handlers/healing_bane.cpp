#include "../location_registry.hpp"

#include "game/console.hpp"
#include "game/player/player.hpp"

namespace rpg {

namespace {
[[maybe_unused]] const bool registered = [] {
    RegionInteractionRegistry::register_builtin_function(
        "healing_bane", [](const RegionInteractionContext& context,
                           const PlaceInteraction& interaction) {
            context.player->m_Health = context.player->m_MaxHealth;
            context.player->m_Mana = context.player->m_MaxMana;
            ui::notify(interaction.name,
                       interaction.description + " Health and mana restored.");
        });
    return true;
}();
} // namespace

} // namespace rpg