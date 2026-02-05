#pragma once
#include <glm/vec2.hpp>
#include <entt/entity/registry.hpp>
namespace engine::system {

    /**
     * @class AnimationSystem
     * @brief 负责更新实体动画状态的系统
     *
     * 该系统遍历所有具有 AnimationComponent 和 SpriteComponent 的实体，
     * 根据帧间隔时间更新当前动画的播放进度和帧索引，实现动画播放逻辑。
     */
    class AnimationSystem {
    public:
        /**
         * @brief 更新所有实体的动画状态
         * @param registry 实体组件注册表
         * @param deltaTime 帧间隔时间（秒）
         */
        void update(entt::registry& registry, float deltaTime);
    };
} // namespace engine::system