/**
 * @file ysort_system.cpp
 * @brief YSortSystem 类的实现，负责根据实体Y坐标更新渲染排序。
 * 
 * @details
 * 本文件实现了 YSortSystem::update() 方法，遍历所有具有
 * TransformComponent、RenderComponent 和 SpriteComponent 的实体，
 * 根据实体的Y坐标位置更新其渲染排序索引。
 * 
 * @par 实现细节
 * - 使用 registry.view<>() 高效查询符合条件的实体
 * - 计算排序索引：transform.position_.y + sprite.size_.y
 * - 这样可以确保实体的底部边缘对齐排序，产生更自然的视觉效果
 */

#include "ysort_system.h"
#include <entt/entity/registry.hpp>
#include "../component/render_component.h"
#include "../component/transform_component.h"
#include "../component/sprite_component.h"

namespace engine::system {
    /**
     * @brief 更新所有实体的Y轴排序索引
     * @param registry EnTT实体注册表，包含所有实体和组件
     * 
     * @details
     * 该方法实现了Y轴排序的核心逻辑：
     * 1. 创建实体视图：获取同时具有 RenderComponent、TransformComponent 和 SpriteComponent 的实体
     * 2. 遍历视图中的所有实体
     * 3. 计算排序索引：transform.position_.y + sprite.size_.y
     * 4. 更新 RenderComponent 的 y_index_ 为计算值
     * 
     * @par 排序计算说明
     * 使用 transform.position_.y + sprite.size_.y 而不是简单的 transform.position_.y，
     * 这样可以确保实体的底部边缘对齐排序，产生更自然的视觉效果。
     * 例如，较高的角色会从其底部开始与其他角色比较，而不是从顶部。
     * 
     * @par 性能考虑
     * - 使用 EnTT 的 view 功能，避免遍历所有实体
     * - 只查询必要的组件，减少内存访问
     * - 计算简单，每帧执行成本低
     */
    void YSortSystem::update(entt::registry& registry) {
        // 获取同时具有 RenderComponent、TransformComponent 和 SpriteComponent 的实体视图
        auto view = registry.view<component::RenderComponent, const component::TransformComponent, const component::SpriteComponent>();
        
        // 遍历视图中的所有实体
        for (auto entity : view) {
            // 获取组件引用
            auto& render = view.get<component::RenderComponent>(entity);
            const auto& transform = view.get<const component::TransformComponent>(entity);
            const auto& sprite = view.get<const component::SpriteComponent>(entity);
            
            // 计算排序索引：使用实体底部的Y坐标
            // 这样可以确保实体的底部边缘对齐排序，产生更自然的视觉效果
            render.y_index_ = transform.position_.y + sprite.size_.y;
        }
    }
}
