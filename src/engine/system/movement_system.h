#pragma once
#include <entt/entity/registry.hpp>
namespace engine::system {
    /**
     * @class MovementSystem
     * @brief 负责更新实体位置的移动系统
     *
     * 该系统遍历所有具有 TransformComponent 和 VelocityComponent 的实体，
     * 根据其速度和帧间隔时间更新位置，实现基本的移动逻辑。
     */
    class MovementSystem {
    public:
        /**
         * @brief 更新所有实体的位置
         * @param deltaTime 帧间隔时间（秒）
         */
        void update(entt::registry& registry, float deltaTime);
    };
} // namespace engine::system