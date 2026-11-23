#include "region.hpp"

#include "regions/region_database.hpp"

#include <algorithm>
#include <stdexcept>
#include <unordered_set>
#include <utility>

namespace rpg {

RegionWorld::RegionWorld()
    : m_Regions(region_data::database().regions),
      m_CurrentRegionId(region_data::database().starting_region_id),
      m_CurrentPlaceId(region_data::database().starting_place_id) {
    validate();
}

RegionWorld::RegionWorld(std::vector<Region> regions,
                         std::string starting_region_id,
                         std::string starting_place_id)
    : m_Regions(std::move(regions)),
      m_CurrentRegionId(std::move(starting_region_id)),
      m_CurrentPlaceId(std::move(starting_place_id)) {
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

const Place* RegionWorld::current_place() const {
    return m_CurrentPlaceId.empty() ? nullptr : find_place(m_CurrentPlaceId);
}

bool RegionWorld::enter_place(const std::string& place_id) {
    if (current_place() != nullptr) {
        return false;
    }
    const auto place = std::find_if(
        current_region().places.begin(), current_region().places.end(),
        [&place_id](const Place& candidate) { return candidate.id == place_id; });
    if (place == current_region().places.end()) {
        return false;
    }
    m_CurrentPlaceId = place_id;
    switch_region_from_place(place_id);
    return true;
}

bool RegionWorld::travel_to_place(const std::string& place_id) {
    const Place* place = current_place();
    if (place == nullptr) {
        return false;
    }
    const bool is_connected = std::any_of(
        place->exits.begin(), place->exits.end(),
        [&place_id](const PlaceExit& exit) {
            return exit.target_place_id == place_id;
        });
    if (!is_connected || find_place(place_id) == nullptr) {
        return false;
    }
    m_CurrentPlaceId = place_id;
    switch_region_from_place(place_id);
    return true;
}

void RegionWorld::leave_place() { m_CurrentPlaceId.clear(); }

std::uint64_t RegionWorld::encounter_level(
    std::uint64_t player_level, const FightingPath& path) const {
    if (path.level_adjustment < 0) {
        const auto adjustment = static_cast<std::uint64_t>(-(path.level_adjustment + 1)) + 1;
        return adjustment >= player_level ? 1 : player_level - adjustment;
    }
    return player_level + static_cast<std::uint64_t>(path.level_adjustment);
}

const Region* RegionWorld::find_region(const std::string& region_id) const {
    const auto region = std::find_if(
        m_Regions.begin(), m_Regions.end(),
        [&region_id](const Region& candidate) { return candidate.id == region_id; });
    return region == m_Regions.end() ? nullptr : &*region;
}

const Place* RegionWorld::find_place(const std::string& place_id) const {
    const Region& region = current_region();
    const auto place = std::find_if(
        region.places.begin(), region.places.end(),
        [&place_id](const Place& candidate) { return candidate.id == place_id; });
    return place == region.places.end() ? nullptr : &*place;
}

bool RegionWorld::switch_region_from_place(const std::string& place_id) {
    const Region& source = current_region();
    const auto bridge = std::find_if(
        source.bridges.begin(), source.bridges.end(),
        [&place_id](const RegionBridge& candidate) {
            return candidate.place_id == place_id;
        });
    if (bridge == source.bridges.end()) {
        return false;
    }
    const Region* destination = find_region(bridge->target_region_id);
    if (destination == nullptr) {
        throw std::logic_error("Region bridge targets an unknown region");
    }
    m_CurrentRegionId = destination->id;
    m_CurrentPlaceId.clear();
    return true;
}

void RegionWorld::validate() const {
    if (m_Regions.empty() || find_region(m_CurrentRegionId) == nullptr) {
        throw std::invalid_argument("A region world needs regions and a valid starting region");
    }
    if (!m_CurrentPlaceId.empty() && find_place(m_CurrentPlaceId) == nullptr) {
        throw std::invalid_argument("The starting place must belong to the starting region");
    }

    std::unordered_set<std::string> region_ids;
    for (const Region& region : m_Regions) {
        if (region.id.empty() || region.name.empty() ||
            !region_ids.insert(region.id).second) {
            throw std::invalid_argument("Invalid or duplicate region");
        }
        std::unordered_set<std::string> place_ids;
        for (const Place& place : region.places) {
            if (place.id.empty() || place.name.empty() ||
                !place_ids.insert(place.id).second) {
                throw std::invalid_argument("Invalid or duplicate place in " + region.id);
            }
            std::unordered_set<std::string> interaction_ids;
            for (const PlaceInteraction& interaction : place.interactions) {
                if (interaction.id.empty() || interaction.name.empty() ||
                    interaction.function_id.empty() ||
                    !interaction_ids.insert(interaction.id).second) {
                    throw std::invalid_argument("Invalid or duplicate interaction in place " + place.id);
                }
            }
            for (const RegionEntryCondition& condition : place.entry_conditions) {
                if (condition.function_id.empty()) {
                    throw std::invalid_argument("Invalid entry condition in place " + place.id);
                }
            }
            std::unordered_set<std::string> place_path_ids;
            for (const FightingPath& path : place.fighting_paths) {
                if (path.id.empty() || path.name.empty() || path.encounters == 0 ||
                    !place_path_ids.insert(path.id).second) {
                    throw std::invalid_argument("Invalid fighting path in place " + place.id);
                }
            }
            std::unordered_set<std::string> place_exit_ids;
            for (const PlaceExit& exit : place.exits) {
                const bool target_exists = std::any_of(
                    region.places.begin(), region.places.end(),
                    [&exit](const Place& candidate) {
                        return candidate.id == exit.target_place_id;
                    });
                if (exit.id.empty() || exit.name.empty() ||
                    exit.target_place_id == place.id || !target_exists ||
                    !place_exit_ids.insert(exit.id).second) {
                    throw std::invalid_argument("Invalid place exit in " + place.id);
                }
            }
        }
        for (const RegionBridge& bridge : region.bridges) {
            const bool place_exists = std::any_of(
                region.places.begin(), region.places.end(),
                [&bridge](const Place& place) { return place.id == bridge.place_id; });
            if (!place_exists || find_region(bridge.target_region_id) == nullptr) {
                throw std::invalid_argument("Invalid region bridge in " + region.id);
            }
        }
    }
}

} // namespace rpg
