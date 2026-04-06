#pragma once

#include <entt/entt.hpp>

#include "../defs/event.h"

namespace game::system {

class GameRuleSystem final {
private:
    entt::registry& registry_;
    entt::dispatcher& dispatcher_;

public:
    GameRuleSystem(entt::registry& registry, entt::dispatcher& dispatcher);
    ~GameRuleSystem();

    void update(float delta_time);

private:
    void onEnemyArriveHome(const game::defs::EnemyArriveHomeEvent& event);
    void onUpgradeUnit(const game::defs::UpgradeUnitEvent& event);
    void onRetreat(const game::defs::RetreatEvent& event);
};

} // namespace game::system
