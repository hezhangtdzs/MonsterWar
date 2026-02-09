#pragma once

#include <entt/entt.hpp>

/**
 * @struct TargetComponent
 * @brief 目标组件，用于存储实体当前锁定的目标。
 * 
 * @details
 * 该组件在 ECS 架构中用于建立实体间的"锁定"关系。
 * 它是实现自动寻敌、攻击和治疗逻辑的核心组件。
 * 
 * @par 数据成员
 * - entity_: 锁定的目标实体的 ID。如果为 entt::null，表示当前没有目标。
 * 
 * @par 设计初衷
 * - 解耦搜索和动作：搜索系统（如 SetTargetSystem）负责寻找目标并添加此组件，
 *   而动作系统（如 AttackStarterSystem）只负责对目标执行操作。
 * - 简化逻辑：通过是否存在此组件，可以快速判断实体是否处于锁定状态。
 */
struct TargetComponent {
    entt::entity entity_{entt::null}; ///< 锁定的目标实体
};
