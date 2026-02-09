#pragma once

#include <entt/entt.hpp>

namespace game::system {

/**
 * @class OrientationSystem
 * @brief 朝向系统，负责根据目标位置或移动方向调整实体的翻转状态。
 * 
 * @details
 * 优先级处理规则：
 * 1. 如果有锁定目标（TargetComponent），面朝目标。
 * 2. 如果被阻挡（BlockedByComponent），面朝阻挡者。
 * 3. 否则，如果是移动中的敌人，面朝移动方向。
 * 
 * 同时考虑 FaceLeftTag，用于修正某些素材默认朝向不同的情况。
 */
class OrientationSystem {
public:
    /**
     * @brief 更新所有实体的朝向。
     * @param registry EnTT 注册表。
     */
    void update(entt::registry& registry);
};

} // namespace game::system
