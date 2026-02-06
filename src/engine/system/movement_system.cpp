/**
 * @file movement_system.cpp
 * @brief MovementSystem 类的实现，ECS移动系统的具体逻辑。
 *
 * @details
 * 本文件实现了 MovementSystem::update() 方法，使用 EnTT 的 view 功能
 * 高效遍历具有 VelocityComponent 和 TransformComponent 的实体，
 * 并根据速度和时间增量更新实体位置。
 *
 * @par 实现细节
 * - 使用 registry.view<>() 创建组件视图，只遍历符合条件的实体
 * - view.get<>() 获取特定组件的引用，避免查找开销
 * - 使用 const 引用读取速度，非const引用修改位置
 */

#include "movement_system.h"
#include "../component/velocity_component.h"
#include "../component/transform_component.h"
#include <spdlog/spdlog.h>

namespace engine::system {

void MovementSystem::update(entt::registry& registry, float delta_time) {
    spdlog::trace("MovementSystem::update");

    // 创建组件视图：只获取同时具有 VelocityComponent 和 TransformComponent 的实体
    // 这是ECS的核心优势：高效查询具有特定组件组合的实体
    auto view = registry.view<engine::component::VelocityComponent, engine::component::TransformComponent>();

    // 遍历视图中的所有实体
    for (auto entity : view) {
        // 从视图中获取组件引用
        // velocity 使用 const 引用，因为只需要读取速度值
        const auto& velocity = view.get<engine::component::VelocityComponent>(entity);

        // transform 使用非 const 引用，因为需要修改位置
        auto& transform = view.get<engine::component::TransformComponent>(entity);

        // 更新位置：新位置 = 原位置 + 速度 × 时间增量
        // 这是基本的欧拉积分，适用于简单的线性运动
        transform.position_ += velocity.velocity_ * delta_time;
    }
}

}   // namespace engine::system