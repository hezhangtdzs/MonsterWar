#pragma once

#include <entt/entity/entity.hpp>

namespace game::data {

struct SelectionState {
    entt::entity hovered_unit_{ entt::null };
    entt::entity selected_unit_{ entt::null };
};

} // namespace game::data
