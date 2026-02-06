/**
 * @file render_system.cpp
 * @brief RenderSystem 类的实现，ECS渲染系统的具体逻辑。
 *
 * @details
 * 本文件实现了 RenderSystem::update() 方法，遍历所有具有
 * TransformComponent 和 SpriteComponent 的实体，并调用 Renderer 进行绘制。
 *
 * @par 实现细节
 * - 使用 registry.view<>() 高效查询可渲染实体
 * - 计算最终渲染参数（位置、大小、旋转）
 * - 调用 renderer.drawSprite() 提交渲染命令
 *
 * @par 坐标计算
 * - 最终位置 = transform.position_ + sprite.offset_
 * - 最终大小 = sprite.size_ * transform.scale_
 * - 旋转角度 = transform.rotation_
 */

#include "render_system.h"
#include "../component/transform_component.h"
#include "../component/sprite_component.h"
#include "../component/render_component.h"
#include <spdlog/spdlog.h>
#include <glm/vec2.hpp>
#include "../render/renderer.h"
#include "../render/camera.h"

namespace engine::system {

void RenderSystem::update(entt::registry& registry, render::Renderer& renderer, const render::Camera& camera) {
    
    registry.sort<component::RenderComponent>([](const auto& lhs, const auto& rhs) {
        return lhs < rhs; // 按深度排序，确保正确的渲染顺序
    });

    // 获取同时具有 RenderComponent, TransformComponent 和 SpriteComponent 的实体视图
    auto view = registry.view<component::RenderComponent, component::TransformComponent, component::SpriteComponent>();

    // 遍历 RenderComponent 的存储（已按深度排序），确保正确的渲染顺序
    for (auto entity : view) {
        // 仅处理视图中包含的实体
        if (!view.contains(entity)) {
            continue;
        }

        // 获取组件（此时已保证实体在该视图中）
        const auto& transform = view.get<component::TransformComponent>(entity);
        const auto& sprite = view.get<component::SpriteComponent>(entity);

        // 计算最终渲染位置：实体位置 + 精灵偏移
        auto position = transform.position_ + sprite.offset_;

        // 计算最终渲染大小：精灵大小 × 变换缩放
        auto size = sprite.size_ * transform.scale_;

        // 调用渲染器绘制
        renderer.drawSprite(camera, sprite.sprite_, position, size, transform.rotation_);
    }
}

} // namespace engine::system