/**
 * @file ysort_system.h
 * @brief 定义 YSortSystem 类，ECS架构中的Y轴排序系统。
 * 
 * @details
 * YSortSystem 是ECS架构中的系统（System），负责根据实体的Y轴位置
 * 更新其 RenderComponent 的 y_index_ 值，实现2D游戏中的Y轴排序效果。
 * 
 * 该系统解决了2D游戏中的深度排序问题，特别是在2.5D视角的游戏中：
 * - 角色在场景中移动时，根据其Y坐标自动调整渲染顺序
 * - 确保角色在正确的背景/前景层之间显示
 * - 实现角色之间的遮挡关系
 * 
 * @par ECS架构角色
 * - 查询：获取同时具有 TransformComponent 和 RenderComponent 的实体
 * - 处理：读取实体的Y坐标位置
 * - 修改：更新 RenderComponent 的 y_index_ 字段
 * 
 * @par 排序原理
 * 1. 读取实体的 TransformComponent.position_.y
 * 2. 将该值赋给 RenderComponent.y_index_
 * 3. RenderSystem 在渲染时根据 y_index_ 排序
 * 4. Y值较小的实体先渲染（在底层），Y值较大的实体后渲染（在上层）
 * 
 * @see engine::component::TransformComponent 存储实体位置数据
 * @see engine::component::RenderComponent 存储渲染排序数据
 * @see engine::system::RenderSystem 使用排序数据进行渲染
 */

#pragma once
#include <entt/entity/registry.hpp>

namespace engine::system {
    /**
     * @class YSortSystem
     * @brief ECS Y轴排序系统，负责根据实体Y坐标更新渲染顺序。
     * 
     * @details
     * YSortSystem 实现了2D游戏中的Y轴排序效果，确保实体根据其在
     * 游戏世界中的垂直位置正确显示层级关系。
     * 
     * @par 系统执行流程
     * 1. 创建实体视图：registry.view<TransformComponent, RenderComponent>()
     * 2. 遍历视图中的所有实体
     * 3. 读取 TransformComponent 的 position_.y 值
     * 4. 更新 RenderComponent 的 y_index_ 为该Y值
     * 5. RenderSystem 会使用此值进行排序渲染
     * 
     * @par 应用场景
     * - 2D平台游戏中的角色与背景排序
     * - 2.5D视角游戏中的深度关系
     * - 任何需要根据Y坐标确定渲染顺序的场景
     */
    class YSortSystem {
    public:
        /**
         * @brief 默认构造函数
         */
        YSortSystem() = default;
        
        /**
         * @brief 默认析构函数
         */
        ~YSortSystem() = default;

        /**
         * @brief 更新所有实体的Y轴排序索引
         * @param registry EnTT实体注册表，包含所有实体和组件
         * 
         * @note 只有同时具有 TransformComponent 和 RenderComponent 的实体才会被处理
         */
        void update(entt::registry& registry);
    };
}