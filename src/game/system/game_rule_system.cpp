#include "game_rule_system.h"

#include "../defs/event.h"
#include "../data/game_stats.h"
#include "../../engine/utils/logging.h"

namespace game::system {

GameRuleSystem::GameRuleSystem(entt::registry& registry, entt::dispatcher& dispatcher)
    : registry_(registry), dispatcher_(dispatcher) {
    dispatcher_.sink<game::defs::EnemyArriveHomeEvent>().connect<&GameRuleSystem::onEnemyArriveHome>(this);
}

GameRuleSystem::~GameRuleSystem() {
    dispatcher_.disconnect(this);
}

void GameRuleSystem::update(float delta_time) {
    auto& game_stats = registry_.ctx().get<game::data::GameStats&>();
    game_stats.cost_ += game_stats.cost_gen_per_second_ * delta_time;
}

void GameRuleSystem::onEnemyArriveHome(const game::defs::EnemyArriveHomeEvent&) {
    auto& game_stats = registry_.ctx().get<game::data::GameStats&>();
    game_stats.enemy_arrived_count_ += 1;
    if (game_stats.home_hp_ > 0) {
        --game_stats.home_hp_;
    }
    ENGINE_LOG_WARN("敌人到达基地，home_hp={}, arrived={}, killed={}, total={}",
                    game_stats.home_hp_,
                    game_stats.enemy_arrived_count_,
                    game_stats.enemy_killed_count_,
                    game_stats.enemy_count_);
}

} // namespace game::system
