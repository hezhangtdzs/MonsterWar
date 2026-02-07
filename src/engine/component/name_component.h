/**
 * @file name_component.h
 * @brief 定义 NameComponent 组件，用于ECS架构中的实体命名。
 *
 * @details
 * NameComponent 是ECS架构中的元数据组件，为实体提供可读名称和哈希ID。
 * 该组件主要用于调试、日志记录和场景编辑器中识别实体。
 *
 * @par 使用场景
 * - 调试时识别实体
 * - 日志中记录实体操作
 * - 场景编辑器中显示实体名称
 * - 通过名称查找实体（配合哈希ID）
 *
 * @par 使用示例
 * @code
 * // 为实体添加名称
 * auto entity = registry.create();
 * registry.emplace<NameComponent>(entity, "player"_hs, "Player Hero");
 *
 * // 通过名称查找实体
 * auto view = registry.view<NameComponent>();
 * for (auto e : view) {
 *     if (view.get<NameComponent>(e).name_id_ == "player"_hs) {
 *         // 找到玩家实体
 *     }
 * }
 * @endcode
 */

#pragma once

#include <string>
#include <entt/entity/entity.hpp>

namespace engine::component {

/**
 * @struct NameComponent
 * @brief 名称组件，用于标记和识别实体。
 *
 * @details
 * 提供两种名称表示方式：
 * - name_id_: 哈希ID，用于快速比较和查找
 * - name_: 可读字符串，用于显示和调试
 */
struct NameComponent {
    entt::id_type name_id_{entt::null};   ///< 名称的哈希ID，用于快速查找和比较
    std::string name_;                    ///< 实体的可读名称，用于调试和显示
};

}