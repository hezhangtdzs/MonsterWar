#pragma once

#include <entt/core/hashed_string.hpp>
#include <glm/vec2.hpp>

namespace game::component {

struct ProjectileVisualComponent {
    glm::vec2 start_position_{ 0.0f, 0.0f };
    glm::vec2 target_position_{ 0.0f, 0.0f };
    float arc_height_{ 0.0f };
    float flight_time_{ 0.5f };
    float elapsed_time_{ 0.0f };
    float rotation_offset_deg_{ 0.0f };
    entt::id_type effect_id_{ entt::hashed_string("skill_active") };
};

} // namespace game::component
