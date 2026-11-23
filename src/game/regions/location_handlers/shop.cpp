#include "../location_registry.hpp"

#include "game/actions/shop/shop.hpp"

namespace rpg {

namespace {
[[maybe_unused]] const bool registered = [] {
    RegionInteractionRegistry::register_builtin_function(
        "shop", [](const RegionInteractionContext& context,
                   const PlaceInteraction& interaction) {
            Shop shop(context.player, context.item_database, interaction.id);
            shop.loop();
        });
    return true;
}();
} // namespace

} // namespace rpg