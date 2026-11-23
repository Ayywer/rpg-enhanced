#include "../location_registry.hpp"

#include "game/console.hpp"

namespace rpg {

namespace {
[[maybe_unused]] const bool registered = [] {
    RegionInteractionRegistry::register_builtin_function(
        "location", [](const RegionInteractionContext&,
                       const PlaceInteraction& interaction) {
            ui::choose_option(
                interaction.name,
                {"Leave"},
                ftxui::text(interaction.description) |
                    ftxui::color(ftxui::Color::GrayLight));
        });
    return true;
}();
} // namespace

} // namespace rpg