#pragma once
#include "game/defs/event.h"
#include <entt/entity/fwd.hpp>
#include <entt/signal/fwd.hpp>

namespace game::system {
	
	class CombatResolveSystem {
	public:
		CombatResolveSystem(entt::registry& registry, entt::dispatcher& dispatcher);
		~CombatResolveSystem();
	private:
		void onAttackEvent(const game::defs::AttackHitEvent& event);
		void onHealEvent(const game::defs::HealerHitEvent& event);
	private:
		float calculateDamage(float attack, float def);
		entt::registry& registry_;
		entt::dispatcher& dispatcher_;
	};
}