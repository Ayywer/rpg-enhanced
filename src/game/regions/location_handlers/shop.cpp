#include "../location_registry.hpp"

#include "game/actions/shop/shop.hpp"

namespace rpg {

namespace {
[[maybe_unused]] const bool registered = [] {
    RegionLocationRegistry::register_builtin_function(
        "shop", [](const RegionLocationContext& context,
                   const RegionLocation&) {
            Shop shop(context.player, context.item_database);
            shop.loop();
        });
    return true;
}();
} // namespace

} // namespace rpg