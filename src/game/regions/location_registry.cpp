#include "location_registry.hpp"

#include <stdexcept>
#include <utility>

namespace rpg {

RegionInteractionRegistry::HandlerMap& RegionInteractionRegistry::builtin_handlers() {
    static HandlerMap handlers;
    return handlers;
}

void RegionInteractionRegistry::register_builtin_function(std::string function_id,
                                                          Handler handler) {
    if (function_id.empty() || !handler) {
        throw std::invalid_argument("Built-in interaction handler needs an ID and function");
    }
    if (!builtin_handlers().emplace(std::move(function_id), std::move(handler)).second) {
        throw std::invalid_argument("Duplicate built-in interaction handler registration");
    }
}

bool RegionInteractionRegistry::has_builtin_function(const std::string& function_id) {
    return builtin_handlers().contains(function_id);
}

void RegionInteractionRegistry::register_function(std::string function_id,
                                                  Handler handler) {
    if (function_id.empty() || !handler) {
        throw std::invalid_argument("Interaction handlers need an ID and function");
    }
    m_Handlers.insert_or_assign(std::move(function_id), std::move(handler));
}

bool RegionInteractionRegistry::contains(const std::string& function_id) const {
    return m_Handlers.contains(function_id) || has_builtin_function(function_id);
}

void RegionInteractionRegistry::execute(const std::string& function_id,
                                        const RegionInteractionContext& context,
                                        const PlaceInteraction& interaction) const {
    const auto custom_handler = m_Handlers.find(function_id);
    if (custom_handler != m_Handlers.end()) {
        custom_handler->second(context, interaction);
        return;
    }
    const auto builtin_handler = builtin_handlers().find(function_id);
    if (builtin_handler == builtin_handlers().end()) {
        throw std::runtime_error("No handler registered for region interaction: " + function_id);
    }
    builtin_handler->second(context, interaction);
}

} // namespace rpg