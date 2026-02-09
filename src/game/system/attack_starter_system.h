#pragma once

#include <entt/entt.hpp>

namespace game::system {

/**
 * @class AttackStarterSystem
 * @brief 攻击启动系统，负责根据锁定目标或阻挡状态触发攻击动作。
 * 
 * @details
 * 该系统负责：
 * 1. 检查近战由 BlockSystem 产生的阻挡关系，触发近战攻击。
 * 2. 检查远程单位的 TargetComponent，触发远程攻击。
 * 3. 检查治疗单位的 TargetComponent，触发治疗动作。
 * 
 * 触发攻击时，会：
 * - 播放对应的攻击动画（通过 PlayAnimationEvent）。
 * - 为实体添加 ActionLockTag（如果是需要锁定的动作）。
 * - 移除 AttackReadyTag，重置攻击冷却计时器。
 */
class AttackStarterSystem {
public:
    /**
     * @brief 检查并启动符合条件的攻击。
     * @param registry EnTT 注册表。
     * @param dispatcher 事件分发器。
     */
    void update(entt::registry& registry, entt::dispatcher& dispatcher);
};

} // namespace game::system
