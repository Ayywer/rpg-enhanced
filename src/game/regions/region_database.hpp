#pragma once

#include "game/region.hpp"

namespace rpg::region_data {

struct WorldData {
	std::vector<Region> regions;
	std::string starting_region_id;
};

const WorldData& database();

} // namespace rpg::region_data