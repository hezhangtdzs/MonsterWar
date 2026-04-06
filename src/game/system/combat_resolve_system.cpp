#include "combat_resolve_system.h"
#include <entt/entt.hpp>
#include "game/component/enemy_component.h"
#include "game/component/player_component.h"
#include "game/component/stats_component.h"
#include "game/component/blocker_component.h"
#include "game/component/blocked_by_component.h"
#include "game/data/game_stats.h"
#include "../defs/tags.h"
#include <algorithm>
#include "../../engine/utils/logging.h"

namespace game::system {
	CombatResolveSystem::CombatResolveSystem(entt::registry& registry, entt::dispatcher& dispatcher)
		: registry_(registry), dispatcher_(dispatcher)
	{
		dispatcher_.sink<game::defs::AttackHitEvent>().connect<&CombatResolveSystem::onAttackEvent>(this);
		dispatcher_.sink<game::defs::HealerHitEvent>().connect<&CombatResolveSystem::onHealEvent>(this);
	}
	CombatResolveSystem::~CombatResolveSystem()
	{
		dispatcher_.disconnect(this);
	}
	void CombatResolveSystem::onAttackEvent(const game::defs::AttackHitEvent& event)
	{
		if (!registry_.valid(event.target_entity_)) {
			return;
		}

		auto stats = registry_.try_get<game::component::StatsComponent>(event.target_entity_);
		if (stats) {
			const float damage = calculateDamage(event.damage_amount_, stats->def_);
			stats->hp_ = std::max(0.0f, stats->hp_ - damage);
             ENGINE_LOG_DEBUG("实体 {} 受到攻击，造成 {} 点伤害，剩余 HP: {}", entt::to_integral(event.target_entity_), damage, stats->hp_);
		}

		if (registry_.all_of<game::component::EnemyComponent>(event.target_entity_)) {
           ENGINE_LOG_DEBUG("目标实体 {} 是敌人", entt::to_integral(event.target_entity_));
			if (stats && stats->hp_ <= 0) {
              ENGINE_LOG_INFO("敌人单位 {} 已被击败", entt::to_integral(event.target_entity_));
				stats->hp_ = 0;
                dispatcher_.enqueue(game::defs::SpawnEffectVisualEvent{ event.target_entity_, entt::hashed_string("death") });
				registry_.emplace_or_replace<game::defs::DeadTag>(event.target_entity_);
				registry_.remove<game::defs::InjuredTag>(event.target_entity_);
              auto& game_stats = registry_.ctx().get<game::data::GameStats&>();
				game_stats.enemy_killed_count_ += 1;
             ENGINE_LOG_DEBUG("敌人击杀统计更新: killed={}, arrived={}, total={}", game_stats.enemy_killed_count_, game_stats.enemy_arrived_count_, game_stats.enemy_count_);
				if (registry_.all_of<game::component::BlockedByComponent>(event.target_entity_)) {
					auto blocked_by = registry_.get<game::component::BlockedByComponent>(event.target_entity_);
					if (registry_.valid(blocked_by.entity_)) {
						registry_.remove<game::component::BlockedByComponent>(event.target_entity_);
						auto& blocker = registry_.get<game::component::BlockerComponent>(blocked_by.entity_);
						blocker.current_count_--;
                     ENGINE_LOG_DEBUG("敌人 {} 被击败，解除阻挡 {} 的状态", entt::to_integral(event.target_entity_), entt::to_integral(blocked_by.entity_));
					}
				}
             if ((game_stats.enemy_killed_count_ + game_stats.enemy_arrived_count_) >= game_stats.enemy_count_) {
					ENGINE_LOG_INFO("当前关卡敌人处理完成，等待后续关卡流程");
				}
				return;
			}
         if (stats && stats->hp_ < stats->max_hp_) {
				registry_.emplace_or_replace<game::defs::InjuredTag>(event.target_entity_);
			}
		}
		else if (registry_.all_of<game::component::PlayerComponent>(event.target_entity_)) {
           ENGINE_LOG_DEBUG("目标实体 {} 是玩家", entt::to_integral(event.target_entity_));
			if (stats && stats->hp_ <= 0) {
              ENGINE_LOG_INFO("玩家单位 {} 已被击败", entt::to_integral(event.target_entity_));
				stats->hp_ = 0;
                dispatcher_.enqueue(game::defs::SpawnEffectVisualEvent{ event.target_entity_, entt::hashed_string("death") });
                dispatcher_.enqueue(game::defs::RemovePlayerUnitEvent{ event.target_entity_ });
				return;
			}
         if (stats && stats->hp_ < stats->max_hp_) {
				registry_.emplace_or_replace<game::defs::InjuredTag>(event.target_entity_);
			}
		}
		else {
            ENGINE_LOG_WARN("目标实体 {} 既不是敌人也不是玩家", entt::to_integral(event.target_entity_));
		}
	}

	void CombatResolveSystem::onHealEvent(const game::defs::HealerHitEvent& event)
	{
		if (registry_.valid(event.target_entity_)) {
			auto stats = registry_.try_get<game::component::StatsComponent>(event.target_entity_);
			if (stats) {
				stats->hp_ += event.heal_amount_;
				if (stats->hp_ > stats->max_hp_) {
					stats->hp_ = stats->max_hp_; // 确保 HP 不超过最大值
				}
             ENGINE_LOG_DEBUG("实体 {} 被治疗，恢复 {} 点 HP，当前 HP: {}", entt::to_integral(event.target_entity_), event.heal_amount_, stats->hp_);
				// 如果之前受伤，现在恢复了，移除 InjuredTag
				if (registry_.all_of<game::defs::InjuredTag>(event.target_entity_) && stats->hp_ == stats->max_hp_) {
					registry_.remove<game::defs::InjuredTag>(event.target_entity_);
                 ENGINE_LOG_DEBUG("实体 {} 已完全恢复，移除受伤状态", entt::to_integral(event.target_entity_));
				}
                dispatcher_.enqueue(game::defs::SpawnEffectVisualEvent{ event.target_entity_, entt::hashed_string("heal").value() });
			} else {
             ENGINE_LOG_WARN("治疗事件目标实体 {} 没有 StatsComponent", entt::to_integral(event.target_entity_));
			}
		} else {
            ENGINE_LOG_WARN("治疗事件目标实体 {} 无效", entt::to_integral(event.target_entity_));
		}
	}

	float CombatResolveSystem::calculateDamage(float attack, float def)
	{
		float damage = attack - def;
		damage = std::max(damage, def*0.1f); // 确保至少造成点伤害
		return damage;
	}
}
