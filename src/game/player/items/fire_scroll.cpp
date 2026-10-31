#include "../item_use_registry.hpp"

#include "../player.hpp"

namespace rpg {

namespace {
[[maybe_unused]] const bool registered = [] {
  ItemUseRegistry::register_builtin_function(
      "fire_scroll", [](Player& player, Enemy*, const Item& item) {
        if (item.spell_id.empty()) {
          return ItemUseResult{false, "This scroll has no spell assigned."};
        }
        if (!player.learn_spell(item.spell_id)) {
          return ItemUseResult{false, "You already know that spell."};
        }
        return ItemUseResult{true, "You learned " + item.spell_id + "!"};
      });
      return true;
    }();
    } // namespace

} // namespace rpg