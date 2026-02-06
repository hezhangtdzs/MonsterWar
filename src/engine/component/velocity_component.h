/**
 * @file velocity_component.h
 * @brief 定义 VelocityComponent 组件，用于ECS架构中的实体运动。
 *
 * @details
 * VelocityComponent 是ECS架构中的运动相关组件，负责存储实体的线速度向量。
 * 该组件与 TransformComponent 配合使用，由 MovementSystem 每帧更新实体的位置。
 *
 * 速度向量的单位是 像素/秒，表示实体每秒移动的距离和方向。
 *
 * @par 使用示例
 * @code
 * // 创建具有速度的实体
 * auto entity = registry.create();
 * registry.emplace<TransformComponent>(entity, glm::vec2(0.0f, 0.0f));
 * registry.emplace<VelocityComponent>(entity, glm::vec2(100.0f, 0.0f));  // 向右移动
 *
 * // MovementSystem 会自动更新位置：
 * // position += velocity * delta_time
 * @endcode
 *
 * @see engine::component::TransformComponent 存储位置信息的组件
 * @see engine::system::MovementSystem 处理速度并更新位置的系统
 */

#pragma once
#include <glm/vec2.hpp>

namespace engine::component {
    /**
     * @struct VelocityComponent
     * @brief ECS速度组件，存储实体的线速度向量。
     *
     * @details
     * 该组件采用纯数据结构（POD）设计，仅包含速度数据，不包含任何运动逻辑。
     * 所有运动计算由 MovementSystem 在更新阶段统一处理。
     *
     * @par 在ECS架构中的作用
     * - 作为 MovementSystem 的输入数据
     * - 与 TransformComponent 配合实现实体运动
     * - 可被其他系统（如AI系统）修改以改变实体运动状态
     */
    struct VelocityComponent {
        glm::vec2 velocity_{ 0.0f, 0.0f }; ///< 当前移动速度向量 (像素/秒)，x和y分量分别表示水平和垂直速度

        /**
         * @brief 构造函数，创建具有指定速度的组件
         * @param velocity 初始速度向量，默认为 (0, 0) 表示静止
         */
        explicit VelocityComponent(const glm::vec2& velocity = { 0.0f, 0.0f })
            : velocity_(velocity) {}
    };
}