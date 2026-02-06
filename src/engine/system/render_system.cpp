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
#include <spdlog/spdlog.h>
#include <glm/vec2.hpp>
#include "../render/renderer.h"
#include "../render/camera.h"

namespace engine::system {

void RenderSystem::update(entt::registry& registry, render::Renderer& renderer, const render::Camera& camera) {
    // 创建组件视图：获取同时具有 TransformComponent 和 SpriteComponent 的实体
    auto view = registry.view<component::TransformComponent, component::SpriteComponent>();

    // 遍历所有可渲染实体
    for (auto entity : view) {
        // 获取变换组件（定义位置、旋转、缩放）
        const auto& transform = view.get<component::TransformComponent>(entity);

        // 获取精灵组件（定义外观、偏移、大小）
        const auto& sprite = view.get<component::SpriteComponent>(entity);

        // 计算最终渲染位置：实体位置 + 精灵偏移
        // 这样可以将精灵的锚点与实体的位置分离，实现更灵活的渲染控制
        auto position = transform.position_ + sprite.offset_;

        // 计算最终渲染大小：精灵大小 × 变换缩放
        // 允许通过 TransformComponent 的 scale_ 统一调整精灵大小
        auto size = sprite.size_ * transform.scale_;

        // 调用渲染器绘制精灵
        // 参数：摄像机、精灵数据、位置、大小、旋转角度
        renderer.drawSprite(camera, sprite.sprite_, position, size, transform.rotation_);
    }
}

} // namespace engine::system