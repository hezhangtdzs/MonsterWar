#include "set_target_system.h"
#include "engine/component/transform_component.h"
#include "game/component/stats_component.h"
#include "game/component/player_component.h"
#include "game/component/enemy_component.h"
#include "game/component/target_component.h"
#include "game/defs/tags.h"
#include "game/defs/constants.h"
#include "engine/utils/math.h"

namespace game::system {

using namespace engine::component;
using namespace game::component;
using namespace game::defs;
using namespace engine::utils;

void SetTargetSystem::update(entt::registry& registry) {
    updateHasTarget(registry);
    updateNoTargetPlayer(registry);
    updateNoTargetEnemy(registry);
    updateHealer(registry);
}

void SetTargetSystem::updateHasTarget(entt::registry& registry) {
    auto view = registry.view<TransformComponent, StatsComponent, TargetComponent>();

    for (auto entity : view) {
        auto& target = view.get<TargetComponent>(entity);
        
        // 1. 校验目标是否仍然有效
        if (!registry.valid(target.entity_)) {
            registry.remove<TargetComponent>(entity);
            continue;
        }

        // 2. 校验距离
        auto& self_pos = view.get<TransformComponent>(entity).position_;
        if (!registry.all_of<TransformComponent>(target.entity_)) {
             registry.remove<TargetComponent>(entity);
             continue;
        }
        auto& target_pos = registry.get<TransformComponent>(target.entity_).position_;
        auto& stats = view.get<StatsComponent>(entity);

        float range_radius = stats.range_ + UNIT_RADIUS;
        if (distanceSquared(self_pos, target_pos) > range_radius * range_radius) {
            registry.remove<TargetComponent>(entity);
        }
    }
}

void SetTargetSystem::updateNoTargetPlayer(entt::registry& registry) {
    // 玩家攻击单位：有 PlayerComponent，没有 TargetComponent，也不是治疗者
    auto view_player = registry.view<TransformComponent, StatsComponent, PlayerComponent>(
        entt::exclude<TargetComponent, HealerTag>
    );

    auto view_enemy = registry.view<TransformComponent, EnemyComponent>();

    for (auto player_entity : view_player) {
        auto& player_pos = view_player.get<TransformComponent>(player_entity).position_;
        auto& stats = view_player.get<StatsComponent>(player_entity);
        float range_radius = stats.range_ + UNIT_RADIUS;

        for (auto enemy_entity : view_enemy) {
            auto& enemy_pos = view_enemy.get<TransformComponent>(enemy_entity).position_;

            if (distanceSquared(player_pos, enemy_pos) <= range_radius * range_radius) {
                registry.emplace<TargetComponent>(player_entity, enemy_entity);
                break; // 锁定一个目标即可
            }
        }
    }
}

void SetTargetSystem::updateNoTargetEnemy(entt::registry& registry) {
    // 敌方远程单位：有 EnemyComponent, RangedUnitTag, 没有 TargetComponent，且未被阻挡（通常由近战处理）
    // 注意：这里我们只处理远程敌人寻找玩家目标。近战敌人逻辑通常在 BlockSystem 处理。
    auto view_enemy = registry.view<TransformComponent, StatsComponent, EnemyComponent, RangedUnitTag>(
        entt::exclude<TargetComponent>
    );

    auto view_player = registry.view<TransformComponent, PlayerComponent>();

    for (auto enemy_entity : view_enemy) {
        auto& enemy_pos = view_enemy.get<TransformComponent>(enemy_entity).position_;
        auto& stats = view_enemy.get<StatsComponent>(enemy_entity);
        float range_radius = stats.range_ + UNIT_RADIUS;

        for (auto player_entity : view_player) {
            auto& player_pos = view_player.get<TransformComponent>(player_entity).position_;

            if (distanceSquared(enemy_pos, player_pos) <= range_radius * range_radius) {
                registry.emplace<TargetComponent>(enemy_entity, player_entity);
                break;
            }
        }
    }
}

void SetTargetSystem::updateHealer(entt::registry& registry) {
    // 治疗者：有 HealerTag
    auto view_healer = registry.view<TransformComponent, StatsComponent, HealerTag>();
    
    // 候选目标：带 InjuredTag 的玩家单位
    auto view_injured = registry.view<TransformComponent, StatsComponent, PlayerComponent, InjuredTag>();

    for (auto healer_entity : view_healer) {
        auto& healer_pos = view_healer.get<TransformComponent>(healer_entity).position_;
        auto& stats = view_healer.get<StatsComponent>(healer_entity);
        float range_radius = stats.range_ + UNIT_RADIUS;

        entt::entity best_target = entt::null;
        float min_hp_percent = 2.0f; // 百分比范围 0-1

        for (auto target_entity : view_injured) {
            auto& target_pos = view_injured.get<TransformComponent>(target_entity).position_;
            
            if (distanceSquared(healer_pos, target_pos) <= range_radius * range_radius) {
                auto& target_stats = view_injured.get<StatsComponent>(target_entity);
                float hp_percent = target_stats.hp_ / target_stats.max_hp_;
                
                if (hp_percent < min_hp_percent) {
                    min_hp_percent = hp_percent;
                    best_target = target_entity;
                }
            }
        }

        if (best_target != entt::null) {
            registry.emplace_or_replace<TargetComponent>(healer_entity, best_target);
        } else {
            // 如果没找到受伤的，清除已有目标（以免一直对着一个满血的人）
            if (registry.all_of<TargetComponent>(healer_entity)) {
                registry.remove<TargetComponent>(healer_entity);
            }
        }
    }
}

} // namespace game::system
