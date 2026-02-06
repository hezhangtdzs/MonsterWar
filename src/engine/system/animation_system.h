/**
 * @file animation_system.h
 * @brief 定义 AnimationSystem 类，ECS架构中的动画处理系统。
 *
 * @details
 * AnimationSystem 是ECS架构中的系统（System），负责更新所有具有
 * AnimationComponent 和 SpriteComponent 的实体的动画状态。
 *
 * 该系统实现了动画播放的核心逻辑：
 * - 根据时间增量推进动画播放进度
 * - 在适当的时候切换到下一帧
 * - 更新 SpriteComponent 的源矩形以显示正确的帧
 *
 * @par ECS架构角色
 * - 查询：获取具有 AnimationComponent 和 SpriteComponent 的实体
 * - 处理：更新动画时间、计算当前帧、处理循环逻辑
 * - 修改：更新 SpriteComponent 的 src_rect_ 以反映当前帧
 *
 * @par 动画播放流程
 * 1. 获取当前动画和帧数据
 * 2. 累加时间：current_time_ms += delta_time * 1000 * speed
 * 3. 检查是否需要切换到下一帧
 * 4. 更新 SpriteComponent 的源矩形为当前帧的矩形
 * 5. 处理动画循环或停止
 *
 * @see engine::component::AnimationComponent 存储动画状态和数据的组件
 * @see engine::component::SpriteComponent 被修改以显示正确帧的组件
 */

#pragma once
#include <glm/vec2.hpp>
#include <entt/entity/registry.hpp>

namespace engine::system {

    /**
     * @class AnimationSystem
     * @brief ECS动画系统，负责更新实体的动画播放状态。
     *
     * @details
     * AnimationSystem 遍历所有具有 AnimationComponent 和 SpriteComponent 的实体，
     * 根据时间增量更新动画播放进度，并修改 SpriteComponent 以显示正确的帧。
     *
     * @par 关键特性
     * - 支持多动画状态（通过 animation_id 切换）
     * - 支持播放速度调整（speed 倍率）
     * - 支持循环和非循环动画
     * - 自动更新精灵的源矩形
     *
     * @par 性能考虑
     * - 使用 EnTT view 高效遍历实体
     * - 只处理当前激活的动画
     * - 跳过没有帧的动画
     */
    class AnimationSystem {
    public:
        /**
         * @brief 更新所有实体的动画状态
         * @param registry EnTT实体注册表
         * @param deltaTime 帧间隔时间（秒），用于推进动画进度
         *
         * @note 只有同时具有 AnimationComponent 和 SpriteComponent 的实体才会被处理
         */
        void update(entt::registry& registry, float deltaTime);
    };
} // namespace engine::system