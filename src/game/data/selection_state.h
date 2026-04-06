#pragma once

#include <entt/entity/entity.hpp>

namespace game::data {

struct SelectionState {
    entt::entity hovered_unit_{ entt::null };
    entt::entity selected_unit_{ entt::null };
    entt::id_type hovered_portrait_name_id_{ 0 };
};

} // namespace game::data
