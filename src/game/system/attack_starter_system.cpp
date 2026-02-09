#include "attack_starter_system.h"
#include "engine/component/velocity_component.h"
#include "game/component/stats_component.h"
#include "game/component/enemy_component.h"
#include "game/component/player_component.h"
#include "game/component/target_component.h"
#include "game/component/blocked_by_component.h"
#include "game/defs/tags.h"
#include "engine/utils/events.h"

namespace game::system {

using namespace engine::component;
using namespace engine::utils;
using namespace game::component;
using namespace game::defs;
using namespace entt::literals;

void AttackStarterSystem::update(entt::registry& registry, entt::dispatcher& dispatcher) {
    // 1. 处理被阻挡的敌人（近战攻击）
    auto view_enemy_blocked = registry.view<EnemyComponent, BlockedByComponent, AttackReadyTag, StatsComponent>();
    for (auto enemy_entity : view_enemy_blocked) {
        // 标记动作锁定，移除准备就绪标签，重置计时器
        registry.emplace_or_replace<ActionLockTag>(enemy_entity);
        registry.remove<AttackReadyTag>(enemy_entity);
        registry.get<StatsComponent>(enemy_entity).atk_timer_ = 0.0f;

        // 发送播放攻击动画事件
        dispatcher.enqueue(PlayAnimationEvent{enemy_entity, "attack"_hs, false});
    }

    // 2. 处理远程敌人（在射程内有目标，且未被阻挡）
    auto view_enemy_ranged = registry.view<EnemyComponent, TargetComponent, AttackReadyTag, StatsComponent>(
        entt::exclude<BlockedByComponent>
    );
    for (auto enemy_entity : view_enemy_ranged) {
        registry.emplace_or_replace<ActionLockTag>(enemy_entity);
        registry.remove<AttackReadyTag>(enemy_entity);
        registry.get<StatsComponent>(enemy_entity).atk_timer_ = 0.0f;

        // 远程攻击时停止移动
        if (auto* vel = registry.try_get<VelocityComponent>(enemy_entity)) {
            vel->velocity_ = {0.0f, 0.0f};
        }

        dispatcher.enqueue(PlayAnimationEvent{enemy_entity, "ranged_attack"_hs, false});
    }

    // 3. 处理玩家单位（攻击或治疗）
    auto view_player = registry.view<PlayerComponent, TargetComponent, AttackReadyTag, StatsComponent>();
    for (auto player_entity : view_player) {
        registry.remove<AttackReadyTag>(player_entity);
        registry.get<StatsComponent>(player_entity).atk_timer_ = 0.0f;

        if (registry.all_of<HealerTag>(player_entity)) {
            dispatcher.enqueue(PlayAnimationEvent{player_entity, "heal"_hs, false});
        } else {
            dispatcher.enqueue(PlayAnimationEvent{player_entity, "attack"_hs, false});
        }
    }
}

} // namespace game::system
