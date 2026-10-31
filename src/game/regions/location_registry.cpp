#include "location_registry.hpp"

#include <stdexcept>
#include <utility>

namespace rpg {

RegionLocationRegistry::HandlerMap& RegionLocationRegistry::builtin_handlers() {
    static HandlerMap handlers;
    return handlers;
}

void RegionLocationRegistry::register_builtin_function(std::string function_id,
                                                       Handler handler) {
    if (function_id.empty() || !handler) {
        throw std::invalid_argument("Built-in location handler needs an ID and function");
    }
    if (!builtin_handlers().emplace(std::move(function_id), std::move(handler)).second) {
        throw std::invalid_argument("Duplicate built-in location handler registration");
    }
}

bool RegionLocationRegistry::has_builtin_function(
    const std::string& function_id) {
    return builtin_handlers().contains(function_id);
}

void RegionLocationRegistry::register_function(std::string function_id,
                                               Handler handler) {
    if (function_id.empty() || !handler) {
        throw std::invalid_argument("Location handlers need an ID and function");
    }
    m_Handlers.insert_or_assign(std::move(function_id), std::move(handler));
}

bool RegionLocationRegistry::contains(const std::string& function_id) const {
    return m_Handlers.contains(function_id) ||
           has_builtin_function(function_id);
}

void RegionLocationRegistry::execute(const std::string& function_id,
                                     const RegionLocationContext& context,
                                     const RegionLocation& location) const {
    const auto custom_handler = m_Handlers.find(function_id);
    if (custom_handler != m_Handlers.end()) {
        custom_handler->second(context, location);
        return;
    }
    const auto builtin_handler = builtin_handlers().find(function_id);
    if (builtin_handler == builtin_handlers().end()) {
        throw std::runtime_error("No handler registered for region location: " +
                                 function_id);
    }
    builtin_handler->second(context, location);
}

} // namespace rpg