/**
 * @file render_system.h
 * @brief 定义 RenderSystem 类，ECS架构中的渲染处理系统。
 *
 * @details
 * RenderSystem 是ECS架构中的系统（System），负责渲染所有具有
 * TransformComponent 和 SpriteComponent 的实体。
 *
 * 该系统实现了数据（组件）与逻辑（系统）的分离：
 * - 组件只存储渲染所需的数据（位置、精灵等）
 * - 系统负责实际的渲染逻辑和调用
 *
 * @par ECS架构角色
 * - 查询：获取具有 TransformComponent 和 SpriteComponent 的实体
 * - 处理：计算最终渲染位置、大小和旋转
 * - 输出：调用 Renderer 绘制精灵
 *
 * @par 渲染流程
 * 1. 查询所有可渲染实体（具有 Transform + Sprite）
 * 2. 计算世界坐标：transform.position + sprite.offset
 * 3. 应用缩放：sprite.size * transform.scale
 * 4. 调用 renderer.drawSprite() 提交渲染
 *
 * @see engine::component::TransformComponent 定义渲染位置
 * @see engine::component::SpriteComponent 定义渲染外观
 * @see engine::render::Renderer 实际执行渲染的类
 */

#pragma once
#include <entt/entity/registry.hpp>

namespace engine::render {
    class Renderer;  ///< 前向声明：渲染器类
    class Camera;    ///< 前向声明：摄像机类
}

namespace engine::system {
    /**
     * @class RenderSystem
     * @brief ECS渲染系统，负责绘制场景中的所有实体。
     *
     * @details
     * RenderSystem 遍历所有具有 TransformComponent 和 SpriteComponent 的实体，
     * 根据组件数据计算渲染参数，并调用 Renderer 进行绘制。
     *
     * @par 渲染顺序
     * 目前按实体在 EnTT registry 中的顺序渲染。
     * 如需控制渲染顺序（如实现层级），可考虑：
     * - 添加 ZComponent 存储深度值
     * - 使用 registry.sort<>() 对实体排序
     * - 实现多遍渲染（先背景后前景）
     */
    class RenderSystem {
    public:
        /** @brief 默认构造函数 */
        RenderSystem() = default;

        /** @brief 默认析构函数 */
        ~RenderSystem() = default;

        /**
         * @brief 更新（渲染）所有符合条件的实体
         * @param registry EnTT实体注册表
         * @param renderer 渲染器引用，用于实际绘制
         * @param camera 摄像机引用，用于坐标转换
         *
         * @note 只有同时具有 TransformComponent 和 SpriteComponent 的实体才会被渲染
         */
        void update(entt::registry& registry,
                    engine::render::Renderer& renderer,
                    const engine::render::Camera& camera);
    };
} // namespace engine::system