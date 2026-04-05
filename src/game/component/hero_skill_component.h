#pragma once

#include <entt/core/hashed_string.hpp>

namespace game::component {

struct HeroSkillComponent {
    entt::id_type skill_id_{ entt::null };
    float cooldown_{ 0.0f };
    float cooldown_timer_{ 0.0f };
    float duration_{ 0.0f };
    float duration_timer_{ 0.0f };
    bool passive_{ false };
    bool active_{ false };
    float atk_multiplier_{ 1.0f };
    float def_multiplier_{ 1.0f };
    float range_multiplier_{ 1.0f };
    float atk_interval_multiplier_{ 1.0f };
    float base_hp_{ 0.0f };
    float base_max_hp_{ 0.0f };
    float base_atk_{ 0.0f };
    float base_def_{ 0.0f };
    float base_range_{ 0.0f };
    float base_atk_interval_{ 0.0f };
};

} // namespace game::component
