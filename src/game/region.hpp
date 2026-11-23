#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

namespace rpg {

enum class PlaceInteractionType { Shop, Npc, HealingBane, Location };

struct PlaceInteraction {
    std::string id;
    std::string name;
    PlaceInteractionType type;
    std::string description;
    std::string function_id;
};

struct FightingPath {
    std::string id;
    std::string name;
    std::uint64_t encounters;
    int level_adjustment;
};

struct PlaceExit {
    std::string id;
    std::string name;
    std::string target_place_id;
};

struct RegionBridge {
    std::string place_id;
    std::string target_region_id;
};

using RegionEntryParameter =
    std::variant<bool, std::int64_t, double, std::string>;

struct RegionEntryCondition {
    std::string function_id;
    std::unordered_map<std::string, RegionEntryParameter> parameters;
};

struct Place {
    std::string id;
    std::string name;
    std::string description;
    std::vector<PlaceInteraction> interactions;
    std::vector<FightingPath> fighting_paths;
    std::vector<PlaceExit> exits;
    std::vector<RegionEntryCondition> entry_conditions;
};

struct NpcDialogueOption {
    std::string id;
    std::string label;
    std::string message;
};

struct Region {
    std::string id;
    std::string name;
    std::vector<Place> places;
    std::vector<RegionBridge> bridges;
};

class NpcDialogueRegistry {
public:
    static const std::vector<NpcDialogueOption>& options_for(
        const std::string& npc_id);
};

class RegionWorld {
public:
    RegionWorld();
    RegionWorld(std::vector<Region> regions, std::string starting_region_id,
                std::string starting_place_id = {});

    const std::vector<Region>& regions() const;
    const Region& current_region() const;
    const Place* current_place() const;
    bool enter_place(const std::string& place_id);
    bool travel_to_place(const std::string& place_id);
    void leave_place();
    std::uint64_t encounter_level(std::uint64_t player_level,
                                  const FightingPath& path) const;

private:
    std::vector<Region> m_Regions;
    std::string m_CurrentRegionId;
    std::string m_CurrentPlaceId;

    const Region* find_region(const std::string& region_id) const;
    const Place* find_place(const std::string& place_id) const;
    bool switch_region_from_place(const std::string& place_id);
    void validate() const;
};

} // namespace rpg
