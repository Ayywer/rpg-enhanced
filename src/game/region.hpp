#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

namespace rpg {

enum class RegionLocationType { Shop, Npc, HealingBane };

struct RegionLocation {
    std::string id;
    std::string name;
    RegionLocationType type;
    std::string description;
    std::string function_id;
};

struct FightingPath {
    std::string id;
    std::string name;
    std::uint64_t encounters;
    int level_adjustment;
};

struct RegionExit {
    std::string id;
    std::string name;
    std::string target_region_id;
};

using RegionEntryParameter =
    std::variant<bool, std::int64_t, double, std::string>;

struct RegionEntryCondition {
    std::string function_id;
    std::unordered_map<std::string, RegionEntryParameter> parameters;
};

struct NpcDialogueOption {
    std::string id;
    std::string label;
    std::string message;
};

struct Region {
    std::string id;
    std::string name;
    std::string description;
    std::uint64_t minimum_level;
    std::uint64_t maximum_level;
    std::vector<RegionLocation> locations;
    std::vector<FightingPath> fighting_paths;
    std::vector<RegionExit> exits;
    std::vector<RegionEntryCondition> entry_conditions;
};

class NpcDialogueRegistry {
public:
    static const std::vector<NpcDialogueOption>& options_for(
        const std::string& npc_id);
};

class RegionWorld {
public:
    RegionWorld();
    RegionWorld(std::vector<Region> regions, std::string starting_region_id);

    const std::vector<Region>& regions() const;
    const Region& current_region() const;
    bool travel_to(const std::string& region_id);
    std::uint64_t encounter_level(std::uint64_t player_level,
                                  const FightingPath& path) const;

private:
    std::vector<Region> m_Regions;
    std::string m_CurrentRegionId;

    const Region* find_region(const std::string& region_id) const;
    void validate() const;
};

} // namespace rpg
