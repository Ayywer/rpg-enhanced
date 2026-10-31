#include "spell_use_registry.hpp"

#include "spells.hpp"

#include <stdexcept>
#include <utility>

namespace rpg {

SpellUseRegistry::HandlerMap& SpellUseRegistry::builtin_handlers() {
  static HandlerMap handlers;
  return handlers;
}

void SpellUseRegistry::register_builtin_function(std::string function_id,
                                                Handler handler) {
  if (function_id.empty() || !handler) {
    throw std::invalid_argument("Built-in spell handler requires an ID and function");
  }
  if (!builtin_handlers().emplace(std::move(function_id), std::move(handler)).second) {
    throw std::invalid_argument("Duplicate built-in spell handler registration");
  }
}

bool SpellUseRegistry::has_builtin_function(
    const std::string& function_id) {
  return builtin_handlers().find(function_id) != builtin_handlers().end();
}

void SpellUseRegistry::register_function(std::string function_id,
                                         Handler handler) {
  if (function_id.empty() || !handler) {
    return;
  }
  m_Handlers.insert_or_assign(std::move(function_id), std::move(handler));
}

bool SpellUseRegistry::contains(const std::string& function_id) const {
  return m_Handlers.find(function_id) != m_Handlers.end() ||
         has_builtin_function(function_id);
}

SpellEffectResult SpellUseRegistry::execute(
    const std::string& function_id, Player& player, Enemy& enemy,
    const SpellDefinition& spell) const {
  const auto custom_handler = m_Handlers.find(function_id);
  if (custom_handler != m_Handlers.end()) {
    return custom_handler->second(player, enemy, spell);
  }
  const auto builtin_handler = builtin_handlers().find(function_id);
  if (builtin_handler == builtin_handlers().end()) {
    throw std::invalid_argument("No handler is registered for spell function '" +
                                function_id + "'");
  }
  return builtin_handler->second(player, enemy, spell);
}

} // namespace rpg