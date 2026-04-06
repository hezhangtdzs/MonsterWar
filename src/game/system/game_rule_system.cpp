#include "game_rule_system.h"

#include "../component/class_name_component.h"
#include "../component/player_component.h"
#include "../component/stats_component.h"
#include "../defs/event.h"
#include "../factory/blueprint_manager.h"
#include "../data/game_stats.h"
#include "../../engine/utils/math.h"
#include "../../engine/audio/audio_locator.h"
#include "../../engine/utils/logging.h"
#include <algorithm>
#include <memory>

namespace game::system {

GameRuleSystem::GameRuleSystem(entt::registry& registry, entt::dispatcher& dispatcher)
    : registry_(registry), dispatcher_(dispatcher) {
    dispatcher_.sink<game::defs::EnemyArriveHomeEvent>().connect<&GameRuleSystem::onEnemyArriveHome>(this);
    dispatcher_.sink<game::defs::UpgradeUnitEvent>().connect<&GameRuleSystem::onUpgradeUnit>(this);
    dispatcher_.sink<game::defs::RetreatEvent>().connect<&GameRuleSystem::onRetreat>(this);
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

void GameRuleSystem::onUpgradeUnit(const game::defs::UpgradeUnitEvent& event) {
    auto& game_stats = registry_.ctx().get<game::data::GameStats&>();
    if (event.entity_ == entt::null || !registry_.valid(event.entity_) || !registry_.all_of<game::component::ClassNameComponent, game::component::StatsComponent, game::component::PlayerComponent>(event.entity_)) {
        return;
    }

    if (game_stats.cost_ < static_cast<float>(event.cost_)) {
        ENGINE_LOG_WARN("升级失败，金币不足: current={}, need={}", static_cast<int>(game_stats.cost_), event.cost_);
        return;
    }

    auto& stats = registry_.get<game::component::StatsComponent>(event.entity_);
    const auto& class_name = registry_.get<game::component::ClassNameComponent>(event.entity_);
    const auto& blueprints = registry_.ctx().get<std::shared_ptr<game::factory::BlueprintManager>>();
    if (!blueprints || !blueprints->hasPlayerClassBlueprint(class_name.class_id_)) {
        ENGINE_LOG_WARN("升级失败，缺少玩家蓝图 entity={}", entt::to_integral(event.entity_));
        return;
    }

    game_stats.cost_ -= static_cast<float>(event.cost_);
    ++stats.level_;
    stats.rarity_ = std::max(1, stats.rarity_);

    const auto& blueprint = blueprints->getPlayerClassBlueprint(class_name.class_id_);
    stats.max_hp_ = engine::utils::statModify(blueprint.stats_.hp_, stats.level_, stats.rarity_);
    stats.atk_ = engine::utils::statModify(blueprint.stats_.atk_, stats.level_, stats.rarity_);
    stats.def_ = engine::utils::statModify(blueprint.stats_.def_, stats.level_, stats.rarity_);
    stats.range_ = engine::utils::statModify(blueprint.stats_.range_, stats.level_, stats.rarity_);
    stats.atk_interval_ = std::max(0.1f, engine::utils::statModify(blueprint.stats_.atk_interval_, stats.level_, stats.rarity_));
    stats.hp_ = stats.max_hp_;

    dispatcher_.trigger(game::defs::SpawnEffectVisualEvent{ event.entity_, entt::hashed_string("level_up") });
    engine::audio::AudioLocator::get().playSound(entt::hashed_string("unit_upgrade").value());
    ENGINE_LOG_INFO("升级单位成功 entity={}, level={}, gold={}", entt::to_integral(event.entity_), stats.level_, static_cast<int>(game_stats.cost_));
}

void GameRuleSystem::onRetreat(const game::defs::RetreatEvent& event) {
    auto& game_stats = registry_.ctx().get<game::data::GameStats&>();
    if (event.entity_ == entt::null || !registry_.valid(event.entity_) || !registry_.all_of<game::component::PlayerComponent>(event.entity_)) {
        return;
    }

    const int refund = std::max(1, event.cost_ / 2);
    game_stats.cost_ += static_cast<float>(refund);
    dispatcher_.enqueue(game::defs::RemovePlayerUnitEvent{ event.entity_ });
    ENGINE_LOG_INFO("撤退单位成功 entity={}, refund={}, gold={}", entt::to_integral(event.entity_), refund, static_cast<int>(game_stats.cost_));
}

} // namespace game::system
