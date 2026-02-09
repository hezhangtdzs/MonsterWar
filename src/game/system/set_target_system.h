#pragma once

#include <entt/entt.hpp>

namespace game::system {

/**
 * @class SetTargetSystem
 * @brief 目标锁定系统，负责为单位寻找和管理攻击/治疗目标。
 * 
 * @details
 * 该系统每帧运行，处理不同类型单位的目标逻辑：
 * 1. 验证已有目标的有效性（是否存在、是否超出射程）。
 * 2. 为没有目标的攻击单位寻找射程内的敌人。
 * 3. 为治疗单位（Healer）寻找受伤的友方单位（InjuredTag）。
 */
class SetTargetSystem {
public:
    /**
     * @brief 更新所有实体的目标锁定状态。
     * @param registry EnTT 注册表。
     */
    void update(entt::registry& registry);

private:
    /**
     * @brief 处理已有目标的逻辑（距离校验、存活校验）。
     */
    void updateHasTarget(entt::registry& registry);

    /**
     * @brief 为没有目标的玩家单位寻找敌人。
     */
    void updateNoTargetPlayer(entt::registry& registry);

    /**
     * @brief 为没有目标的敌方远程单位寻找射程内的目标。
     */
    void updateNoTargetEnemy(entt::registry& registry);

    /**
     * @brief 为没有目标的治疗单位寻找受伤最重的友军。
     */
    void updateHealer(entt::registry& registry);
};

} // namespace game::system
