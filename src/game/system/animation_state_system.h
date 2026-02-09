#pragma once

#include <entt/entt.hpp>
#include "engine/utils/events.h"

namespace game::system {

/**
 * @class AnimationStateSystem
 * @brief 动画状态系统，处理动画结束后的逻辑切换。
 * 
 * @details
 * 该系统监听 AnimationFinishedEvent 事件。当非循环动画（如攻击、治疗）播放结束时：
 * 1. 移除 ActionLockTag。
 * 2. 根据单位当前状态（是否被阻挡等）切换回 idle 或 walk 动画。
 */
class AnimationStateSystem {
public:
    AnimationStateSystem(entt::registry& registry, entt::dispatcher& dispatcher);
    ~AnimationStateSystem();

private:
    /**
     * @brief 处理动画结束事件的回调。
     */
    void onAnimationFinished(const engine::utils::AnimationFinishedEvent& event);

    entt::registry& registry_;
    entt::dispatcher& dispatcher_;
};

} // namespace game::system
