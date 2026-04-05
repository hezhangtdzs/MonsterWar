#pragma once

#include <entt/core/hashed_string.hpp>
#include "../defs/constants.h"

namespace game::component {

struct UnitPrepComponent {
	entt::id_type name_id_{ 0 };
	entt::id_type class_id_{ 0 };
	game::defs::PlayerType type_{ game::defs::PlayerType::UNKNOWN };
	int cost_{ 0 };
	float range_{ 0.0f };
	int level_{ 1 };
	int rarity_{ 1 };
};

} // namespace game::component
