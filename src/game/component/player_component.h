#pragma once

#include <entt/core/hashed_string.hpp>

namespace game::component {
/**
 * @struct PlayerComponent
 * @brief 玩家组件，存储玩家单位的特有属性
 */
struct PlayerComponent {
    int cost_;
    entt::id_type projectile_id_ = entt::null;
};
}