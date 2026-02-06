/**
 * @file movement_system.h
 * @brief 定义 MovementSystem 类，ECS架构中的移动处理系统。
 *
 * @details
 * MovementSystem 是ECS架构中的系统（System），负责处理实体的移动逻辑。
 * 它查询所有同时具有 TransformComponent 和 VelocityComponent 的实体，
 * 根据速度和时间增量更新实体的位置。
 *
 * @par ECS架构角色
 * - 查询：获取具有 TransformComponent 和 VelocityComponent 的实体视图
 * - 处理：计算新位置 = 原位置 + 速度 × 时间增量
 * - 修改：更新 TransformComponent 的 position_ 字段
 *
 * @par 使用示例
 * @code
 * // 在场景初始化时创建系统
 * auto movement_system = std::make_unique<engine::system::MovementSystem>();
 *
 * // 在游戏循环中每帧更新
 * void update(float delta_time) {
 *     movement_system->update(registry_, delta_time);
 * }
 * @endcode
 *
 * @see engine::component::TransformComponent 存储位置数据的组件
 * @see engine::component::VelocityComponent 存储速度数据的组件
 */

#pragma once
#include <entt/entity/registry.hpp>

namespace engine::system {
    /**
     * @class MovementSystem
     * @brief ECS移动系统，负责根据速度更新实体位置。
     *
     * @details
     * MovementSystem 实现了基本的欧拉积分移动逻辑：
     * position += velocity * delta_time
     *
     * 该系统使用 EnTT 的 view 功能高效遍历符合条件的实体，
     * 避免遍历所有实体，提高性能。
     *
     * @par 系统执行流程
     * 1. 创建实体视图：registry.view<VelocityComponent, TransformComponent>()
     * 2. 遍历视图中的所有实体
     * 3. 读取 VelocityComponent 的速度值
     * 4. 更新 TransformComponent 的位置值
     *
     * @par 扩展建议
     * 可以添加以下功能增强：
     * - 边界检查：防止实体移出游戏世界
     * - 碰撞检测：与其他实体碰撞时停止或反弹
     * - 加速度支持：添加 AccelerationComponent 实现更复杂的运动
     */
    class MovementSystem {
    public:
        /**
         * @brief 更新所有具有速度和变换组件的实体位置
         * @param registry EnTT实体注册表，包含所有实体和组件
         * @param deltaTime 帧间隔时间（秒），用于计算位移距离
         *
         * @note 只有同时具有 VelocityComponent 和 TransformComponent 的实体才会被处理
         */
        void update(entt::registry& registry, float deltaTime);
    };
} // namespace engine::system