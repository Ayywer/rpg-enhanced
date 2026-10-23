#pragma once

namespace rpg {

class ItemUseRegistry;

void register_healing_potion(ItemUseRegistry& registry);
void register_mana_potion(ItemUseRegistry& registry);
void register_fire_scroll(ItemUseRegistry& registry);
void register_lightning_scroll(ItemUseRegistry& registry);
void register_amber_relic(ItemUseRegistry& registry);

} // namespace rpg