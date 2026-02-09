#include "animation_state_system.h"
#include "game/component/enemy_component.h"
#include "game/component/player_component.h"
#include "game/component/blocked_by_component.h"
#include "game/defs/tags.h"
#include <entt/signal/dispatcher.hpp>

namespace game::system {

using namespace game::component;
using namespace game::defs;
using namespace engine::utils;
using namespace entt::literals;

AnimationStateSystem::AnimationStateSystem(entt::registry& registry, entt::dispatcher& dispatcher)
    : registry_(registry), dispatcher_(dispatcher) {
    dispatcher_.sink<AnimationFinishedEvent>().connect<&AnimationStateSystem::onAnimationFinished>(this);
}

AnimationStateSystem::~AnimationStateSystem() {
    dispatcher_.disconnect(this);
}

void AnimationStateSystem::onAnimationFinished(const AnimationFinishedEvent& event) {
    auto entity = event.entity_;
    if (!registry_.valid(entity)) return;

    // 移除动作锁
    if (registry_.all_of<ActionLockTag>(entity)) {
        registry_.remove<ActionLockTag>(entity);
    }

    // 根据实体类型决定后续动画
    if (registry_.all_of<EnemyComponent>(entity)) {
        // 敌人：被阻挡则回 idle，否则回 walk
        if (registry_.all_of<BlockedByComponent>(entity)) {
            dispatcher_.enqueue(PlayAnimationEvent{entity, "idle"_hs, true});
        } else {
            dispatcher_.enqueue(PlayAnimationEvent{entity, "walk"_hs, true});
        }
    } else if (registry_.all_of<PlayerComponent>(entity)) {
        // 玩家：统一回 idle
        dispatcher_.enqueue(PlayAnimationEvent{entity, "idle"_hs, true});
    }
}

} // namespace game::system
