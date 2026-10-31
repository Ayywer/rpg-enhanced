#include "region.hpp"

#include "regions/region_database.hpp"

#include <algorithm>
#include <stdexcept>
#include <unordered_set>
#include <utility>

namespace rpg {

RegionWorld::RegionWorld()
        : m_Regions(region_data::database().regions),
            m_CurrentRegionId(region_data::database().starting_region_id) {
    validate();
}

RegionWorld::RegionWorld(std::vector<Region> regions,
                         std::string starting_region_id)
    : m_Regions(std::move(regions)),
      m_CurrentRegionId(std::move(starting_region_id)) {
    validate();
}

const std::vector<Region>& RegionWorld::regions() const { return m_Regions; }

const Region& RegionWorld::current_region() const {
    const Region* region = find_region(m_CurrentRegionId);
    if (region == nullptr) {
        throw std::logic_error("Current region is not in the world");
    }
    return *region;
}

bool RegionWorld::travel_to(const std::string& region_id) {
    const Region& current = current_region();
    const bool is_connected = std::any_of(
        current.exits.begin(), current.exits.end(),
        [&region_id](const RegionExit& exit) {
            return exit.target_region_id == region_id;
        });
    if (!is_connected) {
        return false;
    }
    m_CurrentRegionId = region_id;
    return true;
}

std::uint64_t RegionWorld::encounter_level(
    std::uint64_t player_level, const FightingPath& path) const {
    const Region& region = current_region();
    const std::uint64_t base_level = std::clamp(
        player_level, region.minimum_level, region.maximum_level);
    if (path.level_adjustment < 0) {
        const auto adjustment = static_cast<std::uint64_t>(
            -(path.level_adjustment + 1)) + 1;
        return adjustment >= base_level - region.minimum_level
                   ? region.minimum_level
                   : base_level - adjustment;
    }
    const auto adjustment = static_cast<std::uint64_t>(path.level_adjustment);
    return adjustment >= region.maximum_level - base_level
               ? region.maximum_level
               : base_level + adjustment;
}

const Region* RegionWorld::find_region(const std::string& region_id) const {
    const auto region = std::find_if(
        m_Regions.begin(), m_Regions.end(),
        [&region_id](const Region& candidate) {
            return candidate.id == region_id;
        });
    return region == m_Regions.end() ? nullptr : &*region;
}

void RegionWorld::validate() const {
    if (m_Regions.empty() || find_region(m_CurrentRegionId) == nullptr) {
        throw std::invalid_argument(
            "A region world needs regions and a valid starting region");
    }

    std::unordered_set<std::string> region_ids;
    for (const Region& region : m_Regions) {
        if (region.id.empty() || region.name.empty() ||
            region.minimum_level == 0 ||
            region.minimum_level > region.maximum_level ||
            !region_ids.insert(region.id).second) {
            throw std::invalid_argument("Invalid or duplicate region");
        }
        std::unordered_set<std::string> location_ids;
        for (const RegionLocation& location : region.locations) {
            if (location.id.empty() || location.name.empty() ||
                location.function_id.empty() ||
                !location_ids.insert(location.id).second) {
                throw std::invalid_argument("Invalid or duplicate location in " +
                                            region.id);
            }
        }
        for (const RegionEntryCondition& condition : region.entry_conditions) {
            if (condition.function_id.empty()) {
                throw std::invalid_argument(
                    "Invalid entry condition in " + region.id);
            }
        }
        std::unordered_set<std::string> path_ids;
        for (const FightingPath& path : region.fighting_paths) {
            if (path.id.empty() || path.name.empty() || path.encounters == 0 ||
                !path_ids.insert(path.id).second) {
                throw std::invalid_argument("Invalid fighting path in " +
                                            region.id);
            }
        }
        for (const RegionExit& exit : region.exits) {
            if (exit.id.empty() || exit.name.empty() ||
                exit.target_region_id == region.id ||
                find_region(exit.target_region_id) == nullptr) {
                throw std::invalid_argument("Invalid region exit in " +
                                            region.id);
            }
        }
    }
}

} // namespace rpg
