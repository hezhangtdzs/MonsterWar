/**
 * @file render_component.h
 * @brief 定义 RenderComponent 组件，用于ECS架构中的渲染排序。
 *
 * @details
 * RenderComponent 是ECS架构中的渲染排序组件，控制实体在渲染时的层级和顺序。
 * 该组件由 RenderSystem 使用，根据 layer_index_ 和 y_index_ 对实体进行排序后渲染。
 *
 * @par 排序规则
 * 1. 首先按 layer_index_ 排序，值较小的先渲染（在底层）
 * 2. 同一层内按 y_index_ 排序，值较小的先渲染（实现Y轴排序）
 *
 * @par 使用示例
 * @code
 * // 创建背景实体（低层级）
 * auto bg = registry.create();
 * registry.emplace<RenderComponent>(bg, 0, 0.0f);
 *
 * // 创建角色实体（高层级，Y轴排序）
 * auto player = registry.create();
 * registry.emplace<RenderComponent>(player, 1, player_transform.position_.y);
 *
 * // 创建UI实体（最高层级）
 * auto ui = registry.create();
 * registry.emplace<RenderComponent>(ui, 100, 0.0f);
 * @endcode
 *
 * @see engine::system::RenderSystem 使用此组件进行渲染排序的系统
 * @see engine::system::YSortSystem 自动更新Y轴排序的系统
 */

#pragma once

namespace engine::component {
    /**
     * @struct RenderComponent
     * @brief 渲染排序组件，控制实体的渲染层级和顺序。
     *
     * @details
     * 该组件定义了实体在渲染时的排序规则：
     * - layer_index_: 渲染层级，用于分层渲染（如背景层、游戏层、UI层）
     * - y_index_: Y轴深度索引，用于同一层内的前后关系（如2.5D效果）
     *
     * 实现了 operator< 以便在容器中进行排序。
     */
    struct RenderComponent {
        int layer_index_{0};    ///< 渲染层级索引，较小的值表示更靠后的层（先渲染）
        float y_index_{0.0f};   ///< Y轴深度索引，用于同一层内的渲染顺序控制，较小的值先渲染

        /**
         * @brief 比较运算符，用于渲染排序
         * @param other 要比较的另一个 RenderComponent
         * @return true 如果当前组件应该先于 other 渲染
         *
         * @details 首先比较 layer_index_，如果相同则比较 y_index_
         */
        bool operator<(const RenderComponent& other) const {
            if (layer_index_ != other.layer_index_) {
                return layer_index_ < other.layer_index_;
            }
            return y_index_ < other.y_index_;
        }
    };

}