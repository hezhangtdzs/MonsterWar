#pragma once

#include <entt/entity/entity.hpp>

namespace game::component {

struct PlaceOccupiedComponent {
	entt::entity occupied_by_{ entt::null };
};

} // namespace game::component
