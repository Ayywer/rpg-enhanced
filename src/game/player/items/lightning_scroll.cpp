#include "../item_use_registry.hpp"

#include "../player.hpp"

namespace rpg {

void register_lightning_scroll(ItemUseRegistry& registry) {
  registry.register_function(
      "lightning_scroll", [](Player& player, Enemy*, const Item& item) {
        if (item.spell_id.empty()) {
          return ItemUseResult{false, "This scroll has no spell assigned."};
        }
        if (!player.learn_spell(item.spell_id)) {
          return ItemUseResult{false, "You already know that spell."};
        }
        return ItemUseResult{true, "You learned " + item.spell_id + "!"};
      });
}

} // namespace rpg