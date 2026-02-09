#pragma once

#include <entt/entt.hpp>

namespace game::system {

/**
 * @class TimerSystem
 * @brief 计时器系统，处理所有带有攻击冷却计时的实体。
 * 
 * @details
 * 该系统负责维护 StatsComponent 中的 atk_timer_。
 * 当计时器达到攻击间隔（atk_interval_）时，会为实体添加 AttackReadyTag，
 * 表示该单位可以进行下一次攻击。
 */
class TimerSystem {
public:
    /**
     * @brief 更新所有实体的计时器。
     * @param registry EnTT 注册表。
     * @param delta_time 自上一帧以来的时间间隔（秒）。
     */
    void update(entt::registry& registry, float delta_time);
};

} // namespace game::system
