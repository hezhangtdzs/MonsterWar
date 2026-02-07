/**
 * @file remove_dead_system.h
 * @brief 定义 RemoveDeadSystem 类，负责清理标记为死亡的实体。
 * 
 * @details
 * RemoveDeadSystem 是游戏特定的系统，负责清理带有 DeadTag 标签的实体。
 * 该系统实现了实体的延迟删除机制，避免在游戏逻辑中直接删除实体导致的问题。
 * 
 * @par ECS架构角色
 * - 查询：获取带有 DeadTag 的实体
 * - 处理：删除标记为死亡的实体
 * - 输出：从注册表中移除实体
 * 
 * @par 延迟删除机制
 * 延迟删除机制解决了以下问题：
 * 1. 避免在遍历实体时删除实体导致的迭代器失效
 * 2. 允许实体在死亡后播放死亡动画或特效
 * 3. 统一管理实体的生命周期
 * 4. 避免在多个系统中重复处理死亡实体
 * 
 * @par 工作原理
 * 1. 其他系统或逻辑在实体应该死亡时，为其添加 DeadTag
 * 2. RemoveDeadSystem 在每帧更新时查询所有带有 DeadTag 的实体
 * 3. 从注册表中删除这些实体
 * 4. 实体的所有组件也会被自动清理
 * 
 * @see game::defs::DeadTag 死亡标签组件
 * @see entt::registry EnTT 的实体注册表
 */

#pragma once
#include <entt/entt.hpp>

namespace game::system {

/**
 * @class RemoveDeadSystem
 * @brief 死亡实体清理系统，负责删除标记为死亡的实体。
 * 
 * @details
 * RemoveDeadSystem 实现了实体的延迟删除机制，确保实体在适当的时机被清理，
 * 避免了直接删除实体可能导致的各种问题。
 * 
 * @par 系统执行流程
 * 1. 查询所有带有 DeadTag 的实体
 * 2. 遍历这些实体
 * 3. 从注册表中删除每个实体
 * 4. 实体的所有组件会被 EnTT 自动清理
 * 
 * @par 应用场景
 * - 敌人被击败后
 * - 敌人到达玩家基地后
 * - 临时实体（如特效、投射物）生命周期结束后
 * - 任何需要从游戏世界中移除的实体
 */
class RemoveDeadSystem {
public:
    /**
     * @brief 更新系统，清理标记为死亡的实体
     * @param registry EnTT 实体注册表，包含所有实体和组件
     * 
     * @details
     * 该方法实现了死亡实体的清理逻辑：
     * 1. 创建实体视图：registry.view<game::defs::DeadTag>()
     * 2. 遍历视图中的所有实体
     * 3. 从注册表中删除每个实体
     * 4. 记录删除结果
     * 
     * @par 注意事项
     * - 该系统应该在所有其他系统更新之后运行，确保死亡实体的所有逻辑都已处理完毕
     * - 删除实体时，EnTT 会自动清理实体的所有组件，无需手动处理
     */
    void update(entt::registry& registry);
};

}   // namespace game::system