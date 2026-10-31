#pragma once

#include <cstdint>
#include <functional>
#include <string>
#include <unordered_map>

namespace rpg {

class Enemy;
class Player;
struct SpellDefinition;

struct SpellEffectResult {
  std::uint64_t heal = 0;
  std::uint64_t damage = 0;
};

class SpellUseRegistry {
public:
  using Handler = std::function<SpellEffectResult(
      Player&, Enemy&, const SpellDefinition&)>;

  static void register_builtin_function(std::string function_id,
                                        Handler handler);
  static bool has_builtin_function(const std::string& function_id);
  void register_function(std::string function_id, Handler handler);
  bool contains(const std::string& function_id) const;
  SpellEffectResult execute(const std::string& function_id, Player& player,
                            Enemy& enemy,
                            const SpellDefinition& spell) const;

private:
  using HandlerMap = std::unordered_map<std::string, Handler>;
  static HandlerMap& builtin_handlers();

  std::unordered_map<std::string, Handler> m_Handlers;
};

} // namespace rpg