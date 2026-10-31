#include "../location_registry.hpp"

#include "game/console.hpp"

#include <string>
#include <vector>

namespace rpg {

namespace {
[[maybe_unused]] const bool registered = [] {
    RegionLocationRegistry::register_builtin_function(
        "npc_dialogue", [](const RegionLocationContext&,
                           const RegionLocation& location) {
            const std::vector<NpcDialogueOption>& options =
                NpcDialogueRegistry::options_for(location.id);
            if (options.empty()) {
                ui::notify(location.name, location.description);
                return;
            }

            std::vector<std::string> choices;
            choices.reserve(options.size() + 1);
            for (const NpcDialogueOption& option : options) {
                choices.push_back(option.label);
            }
            choices.push_back("Leave");

            const int selected = ui::choose_option(
                location.name, choices,
                ftxui::text(location.description) |
                    ftxui::color(ftxui::Color::GrayLight));
            if (selected < 0 || selected >= static_cast<int>(options.size())) {
                return;
            }

            const NpcDialogueOption& chosen =
                options[static_cast<std::size_t>(selected)];
            ui::notify(chosen.label, chosen.message);
        });
    return true;
}();
} // namespace

} // namespace rpg