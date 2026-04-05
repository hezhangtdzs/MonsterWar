#include "animation_state_system.h"
#include "game/component/enemy_component.h"
#include "game/component/player_component.h"
#include "game/component/blocked_by_component.h"
#include "game/defs/tags.h"
#include "../../engine/component/animation_component.h"
#include "../../engine/component/transform_component.h"
#include "../../engine/component/sprite_component.h"
#include "../../engine/component/velocity_component.h"
#include "../../engine/component/render_component.h"
#include <entt/signal/dispatcher.hpp>
#include "../../engine/utils/logging.h"

namespace game::system {

using namespace game::component;
using namespace game::defs;
using namespace engine::utils;
using namespace engine::component;
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

    if (registry_.all_of<VisualEffectTag>(entity)) {
        if (registry_.all_of<TransformComponent>(entity)) {
            registry_.remove<TransformComponent>(entity);
        }
        if (registry_.all_of<AnimationComponent>(entity)) {
            registry_.remove<AnimationComponent>(entity);
        }
        if (registry_.all_of<SpriteComponent>(entity)) {
            registry_.remove<SpriteComponent>(entity);
        }
        if (registry_.all_of<VelocityComponent>(entity)) {
            registry_.remove<VelocityComponent>(entity);
        }
        if (registry_.all_of<RenderComponent>(entity)) {
            registry_.remove<RenderComponent>(entity);
        }
        if (registry_.all_of<InactiveVisualTag>(entity)) {
            return;
        }
        registry_.emplace<InactiveVisualTag>(entity);
        ENGINE_LOG_DEBUG("视觉实体动画结束并回收: {}", entt::to_integral(entity));
        return;
    }

    // 根据实体类型决定后续动画
    if (registry_.all_of<EnemyComponent>(entity)) {
        // 敌人：被阻挡则回 idle，否则回 walk
        if (registry_.all_of<BlockedByComponent>(entity)) {
            ENGINE_LOG_DEBUG("动画结束后回切敌方受阻实体: {}", entt::to_integral(entity));
            dispatcher_.enqueue(PlayAnimationEvent{entity, "idle"_hs, true});
        } else {
            ENGINE_LOG_DEBUG("动画结束后回切敌方移动实体: {}", entt::to_integral(entity));
            dispatcher_.enqueue(PlayAnimationEvent{entity, "walk"_hs, true});
        }
    } else if (registry_.all_of<PlayerComponent>(entity)) {
        // 玩家：统一回 idle
        ENGINE_LOG_DEBUG("动画结束后回切玩家实体: {}", entt::to_integral(entity));
        dispatcher_.enqueue(PlayAnimationEvent{entity, "idle"_hs, true});
    }
}

} // namespace game::system
