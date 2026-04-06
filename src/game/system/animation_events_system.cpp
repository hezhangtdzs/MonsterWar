#include "animation_events_system.h"
#include "../component/player_component.h"
#include "../component/enemy_component.h"
#include "../component/target_component.h"
#include "../component/stats_component.h"
#include "../component/blocked_by_component.h"
#include "../defs/tags.h"
#include "../defs/event.h"
#include <entt/entt.hpp>
#include "../../engine/utils/logging.h"

using namespace entt::literals;
namespace game::system {
	AnimationEventsSystem::AnimationEventsSystem(entt::registry& registry, entt::dispatcher& dispatcher)
		: registry_(registry), dispatcher_(dispatcher) {
		dispatcher_.sink<engine::utils::AnimationEvent>().connect<&AnimationEventsSystem::onAnimationEvent>(this);
	}
	AnimationEventsSystem::~AnimationEventsSystem() {
		dispatcher_.disconnect(this);
	}

	void AnimationEventsSystem::onAnimationEvent(const engine::utils::AnimationEvent& event)
	{
		if (registry_.valid(event.entity_)) {
			if (registry_.all_of<game::defs::DeadTag>(event.entity_)) {
				return;
			}
           ENGINE_LOG_DEBUG("动画事件: entity={}, event_id={:x}, animation_id={:x}", entt::to_integral(event.entity_), event.event_id_, event.animation_id_);
		} else {
          ENGINE_LOG_WARN("无效实体: entity={}, event_id={:x}, animation_id={:x}", entt::to_integral(event.entity_), event.event_id_, event.animation_id_);
			return;
		}
      if (event.event_id_ == "hit"_hs || event.event_id_ == "emit"_hs) {
			handleHitEvent(event);
		}
	}

	void AnimationEventsSystem::handleHitEvent(const engine::utils::AnimationEvent& event)
	{
		if (registry_.all_of<game::component::EnemyComponent>(event.entity_)) {
			auto stats = registry_.try_get<game::component::StatsComponent>(event.entity_);
            ENGINE_LOG_DEBUG("处理命中事件: 实体={} 是敌人", entt::to_integral(event.entity_));
         entt::entity target = entt::null;
			if (auto blocked = registry_.try_get<game::component::BlockedByComponent>(event.entity_)) {
				target = blocked->entity_;
			} else if (auto attack_target = registry_.try_get<game::component::TargetComponent>(event.entity_)) {
				target = attack_target->entity_;
			}
			if (target != entt::null) {
               if (auto enemy = registry_.try_get<game::component::EnemyComponent>(event.entity_); enemy && enemy->projectile_id_ != entt::null) {
					dispatcher_.enqueue(game::defs::SpawnProjectileVisualEvent{ event.entity_, target, enemy->projectile_id_ });
				}
				dispatcher_.enqueue(game::defs::AttackHitEvent{ event.entity_, target, stats ? stats->atk_ : 0.0f });
				dispatcher_.enqueue(engine::utils::PlaySoundEvent{ event.entity_, event.event_id_ });
			}
		} else if (registry_.all_of<game::component::PlayerComponent>(event.entity_)) {
            ENGINE_LOG_DEBUG("处理命中事件: 实体={} 是玩家", entt::to_integral(event.entity_));
			auto stats = registry_.try_get<game::component::StatsComponent>(event.entity_);
			if (auto target = registry_.try_get<game::component::TargetComponent>(event.entity_)) {
				if (registry_.all_of<game::defs::HealerTag>(event.entity_)) {
					dispatcher_.enqueue(game::defs::HealerHitEvent{ event.entity_, target->entity_, stats ? stats->atk_ : 0.0f });
				}
				else{
                  if (auto player = registry_.try_get<game::component::PlayerComponent>(event.entity_); player && player->projectile_id_ != entt::null) {
						dispatcher_.enqueue(game::defs::SpawnProjectileVisualEvent{ event.entity_, target->entity_, player->projectile_id_ });
					}
					dispatcher_.enqueue(game::defs::AttackHitEvent{ event.entity_, target->entity_, stats ? stats->atk_ : 0.0f });
				}
              dispatcher_.enqueue(engine::utils::PlaySoundEvent{ event.entity_, event.event_id_ });
               ENGINE_LOG_DEBUG("处理命中事件完成: entity={}", entt::to_integral(event.entity_));
			}

		} else {
         ENGINE_LOG_WARN("处理命中事件: 实体={} 既不是敌人也不是玩家，未处理", entt::to_integral(event.entity_));
		}

	}

}