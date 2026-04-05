#include "hero_skill_system.h"

#include "../component/class_name_component.h"
#include "../component/hero_skill_component.h"
#include "../component/player_component.h"
#include "../component/stats_component.h"
#include "../data/game_stats.h"
#include "../factory/blueprint_manager.h"
#include "../defs/tags.h"
#include "../../engine/component/render_component.h"
#include "../../engine/utils/logging.h"
#include <memory>
#include <algorithm>

using namespace entt::literals;

namespace game::system {

HeroSkillSystem::HeroSkillSystem(entt::registry& registry, entt::dispatcher& dispatcher)
    : registry_(registry), dispatcher_(dispatcher) {
    dispatcher_.sink<game::defs::UpgradeHeroEvent>().connect<&HeroSkillSystem::onUpgradeHeroEvent>(this);
    dispatcher_.sink<game::defs::ReleaseHeroSkillEvent>().connect<&HeroSkillSystem::onReleaseHeroSkillEvent>(this);
    dispatcher_.sink<game::defs::SkillReadyEvent>().connect<&HeroSkillSystem::onSkillReadyEvent>(this);
    dispatcher_.sink<game::defs::SkillActiveEvent>().connect<&HeroSkillSystem::onSkillActiveEvent>(this);
    dispatcher_.sink<game::defs::SkillDurationEndEvent>().connect<&HeroSkillSystem::onSkillDurationEndEvent>(this);
}

HeroSkillSystem::~HeroSkillSystem() {
    dispatcher_.disconnect(this);
}

void HeroSkillSystem::update(float delta_time) {
    if (!registry_.ctx().contains<std::shared_ptr<game::factory::BlueprintManager>>() ||
        !registry_.ctx().contains<game::data::GameStats&>()) {
        return;
    }

    auto& blueprint_manager_ptr = registry_.ctx().get<std::shared_ptr<game::factory::BlueprintManager>>();
    if (!blueprint_manager_ptr) {
        return;
    }
    auto& blueprint_manager = *blueprint_manager_ptr;
    auto& game_stats = registry_.ctx().get<game::data::GameStats&>();

    auto view = registry_.view<game::component::HeroSkillComponent, game::component::StatsComponent, game::component::PlayerComponent>();
    for (auto entity : view) {
        auto& skill = view.get<game::component::HeroSkillComponent>(entity);

        if (skill.passive_) {
            if (blueprint_manager.hasSkillBlueprint(skill.skill_id_)) {
                const auto& blueprint = blueprint_manager.getSkillBlueprint(skill.skill_id_);
                if (blueprint.cost_regen_ > 0.0f) {
                    game_stats.cost_ += blueprint.cost_regen_ * delta_time;
                }
            }
        }
    }
}

void HeroSkillSystem::onUpgradeHeroEvent(const game::defs::UpgradeHeroEvent& event) {
    if (!registry_.valid(event.entity_) || !registry_.all_of<game::component::PlayerComponent, game::component::StatsComponent>(event.entity_)) {
        return;
    }

    if (!registry_.ctx().contains<game::data::GameStats&>()) {
        return;
    }
    auto& game_stats = registry_.ctx().get<game::data::GameStats&>();

    auto& player = registry_.get<game::component::PlayerComponent>(event.entity_);
    const int upgrade_cost = std::max(20, player.cost_);
    if (game_stats.cost_ < static_cast<float>(upgrade_cost)) {
        ENGINE_LOG_WARN("[HeroSkillSystem::onUpgradeHeroEvent] 升级金币不足 entity={}, current={}, need={}", entt::to_integral(event.entity_), static_cast<int>(game_stats.cost_), upgrade_cost);
        return;
    }

    game_stats.cost_ -= static_cast<float>(upgrade_cost);
    auto& stats = registry_.get<game::component::StatsComponent>(event.entity_);
    auto* skill = registry_.try_get<game::component::HeroSkillComponent>(event.entity_);
    applyUpgrade(event.entity_, skill, stats);
    ENGINE_LOG_INFO("英雄升级成功 entity={}, cost={}, level={}", entt::to_integral(event.entity_), upgrade_cost, stats.level_);
}

void HeroSkillSystem::onReleaseHeroSkillEvent(const game::defs::ReleaseHeroSkillEvent& event) {
    onSkillActiveEvent(game::defs::SkillActiveEvent{ event.entity_ });
}

void HeroSkillSystem::onSkillReadyEvent(const game::defs::SkillReadyEvent& event) {
    if (!registry_.valid(event.entity_) || !registry_.all_of<game::component::HeroSkillComponent>(event.entity_)) {
        return;
    }

    if (!registry_.all_of<game::defs::SkillReadyTag>(event.entity_)) {
        registry_.emplace<game::defs::SkillReadyTag>(event.entity_);
    }

    dispatcher_.trigger(game::defs::SpawnEffectVisualEvent{ event.entity_, entt::hashed_string("skill_ready").value() });
}

void HeroSkillSystem::onSkillActiveEvent(const game::defs::SkillActiveEvent& event) {
    if (!registry_.valid(event.entity_) || !registry_.all_of<game::component::PlayerComponent, game::component::StatsComponent>(event.entity_)) {
        return;
    }

    if (!registry_.ctx().contains<std::shared_ptr<game::factory::BlueprintManager>>()) {
        return;
    }
    auto& blueprint_manager_ptr = registry_.ctx().get<std::shared_ptr<game::factory::BlueprintManager>>();
    if (!blueprint_manager_ptr) {
        return;
    }
    auto& blueprint_manager = *blueprint_manager_ptr;

    auto* skill = registry_.try_get<game::component::HeroSkillComponent>(event.entity_);
    if (!skill || !blueprint_manager.hasSkillBlueprint(skill->skill_id_)) {
        ENGINE_LOG_WARN("[HeroSkillSystem::onReleaseHeroSkillEvent] 英雄没有可用技能 entity={}", entt::to_integral(event.entity_));
        return;
    }

    const auto& blueprint = blueprint_manager.getSkillBlueprint(skill->skill_id_);
    auto& stats = registry_.get<game::component::StatsComponent>(event.entity_);
    if (skill->passive_) {
        ENGINE_LOG_INFO("[HeroSkillSystem::onSkillActiveEvent] 被动技能无需主动释放 entity={}, skill={}", entt::to_integral(event.entity_), blueprint.name_);
        return;
    }

    if (skill->active_ || skill->cooldown_timer_ > 0.0f) {
        ENGINE_LOG_WARN("[HeroSkillSystem::onSkillActiveEvent] 技能冷却中 entity={}, skill={}, cooldown={}", entt::to_integral(event.entity_), blueprint.name_, skill->cooldown_timer_);
        return;
    }

    activateSkill(event.entity_, *skill, stats, blueprint);
    if (registry_.all_of<game::defs::SkillReadyTag>(event.entity_)) {
        registry_.remove<game::defs::SkillReadyTag>(event.entity_);
    }
    registry_.emplace_or_replace<game::defs::SkillActiveTag>(event.entity_);
    dispatcher_.trigger(game::defs::SpawnEffectVisualEvent{ event.entity_, entt::hashed_string("skill_active").value() });
    ENGINE_LOG_INFO("[HeroSkillSystem::onSkillActiveEvent] 英雄释放技能 entity={}, skill={}", entt::to_integral(event.entity_), blueprint.name_);
}

void HeroSkillSystem::onSkillDurationEndEvent(const game::defs::SkillDurationEndEvent& event) {
    if (!registry_.valid(event.entity_) || !registry_.all_of<game::component::HeroSkillComponent, game::component::StatsComponent>(event.entity_)) {
        return;
    }

    auto& skill = registry_.get<game::component::HeroSkillComponent>(event.entity_);
    auto& stats = registry_.get<game::component::StatsComponent>(event.entity_);
    if (!skill.active_) {
        return;
    }

    if (registry_.all_of<game::defs::SkillActiveTag>(event.entity_)) {
        registry_.remove<game::defs::SkillActiveTag>(event.entity_);
    }
    deactivateSkill(event.entity_, skill, stats);
    ENGINE_LOG_INFO("[HeroSkillSystem::onSkillDurationEndEvent] 技能结束 entity={}", entt::to_integral(event.entity_));
}

void HeroSkillSystem::activateSkill(entt::entity entity,
                                   game::component::HeroSkillComponent& skill,
                                   game::component::StatsComponent& stats,
                                   const game::data::SkillBlueprint& blueprint) {
    skill.base_hp_ = stats.hp_;
    skill.base_max_hp_ = stats.max_hp_;
    skill.base_atk_ = stats.atk_;
    skill.base_def_ = stats.def_;
    skill.base_range_ = stats.range_;
    skill.base_atk_interval_ = stats.atk_interval_;

    skill.atk_multiplier_ = blueprint.atk_multiplier_;
    skill.def_multiplier_ = blueprint.def_multiplier_;
    skill.range_multiplier_ = blueprint.range_multiplier_;
    skill.atk_interval_multiplier_ = blueprint.atk_interval_multiplier_;

    stats.hp_ = skill.base_hp_;
    stats.max_hp_ = skill.base_max_hp_;
    stats.atk_ = skill.base_atk_ * skill.atk_multiplier_;
    stats.def_ = skill.base_def_ * skill.def_multiplier_;
    stats.range_ = skill.base_range_ * skill.range_multiplier_;
    stats.atk_interval_ = skill.base_atk_interval_ * skill.atk_interval_multiplier_;

    skill.duration_ = blueprint.duration_;
    skill.duration_timer_ = blueprint.duration_;
    skill.cooldown_ = blueprint.cooldown_;
    skill.cooldown_timer_ = blueprint.cooldown_;
    skill.passive_ = blueprint.passive_;
    skill.active_ = true;

    if (auto* render = registry_.try_get<engine::component::RenderComponent>(entity)) {
        render->color_ = { 1.0f, 0.40f, 0.40f, 1.0f };
    }
}

void HeroSkillSystem::deactivateSkill(entt::entity entity,
                                     game::component::HeroSkillComponent& skill,
                                     game::component::StatsComponent& stats) {
    stats.hp_ = skill.base_hp_;
    stats.max_hp_ = skill.base_max_hp_;
    stats.atk_ = skill.base_atk_;
    stats.def_ = skill.base_def_;
    stats.range_ = skill.base_range_;
    stats.atk_interval_ = skill.base_atk_interval_;
    skill.active_ = false;

    if (auto* render = registry_.try_get<engine::component::RenderComponent>(entity)) {
        render->color_ = { 1.0f, 1.0f, 1.0f, 1.0f };
    }
}

void HeroSkillSystem::applyUpgrade(entt::entity,
                                  game::component::HeroSkillComponent* skill,
                                  game::component::StatsComponent& stats) {
    stats.level_ += 1;
    if (skill && skill->active_) {
        skill->base_hp_ *= 1.20f;
        skill->base_max_hp_ *= 1.20f;
        skill->base_atk_ *= 1.12f;
        skill->base_def_ *= 1.00f;
        skill->base_range_ *= 1.05f;
        skill->base_atk_interval_ *= 1.00f;
        stats.hp_ = skill->base_hp_;
        stats.max_hp_ = skill->base_max_hp_;
        stats.atk_ = skill->base_atk_ * skill->atk_multiplier_;
        stats.def_ = skill->base_def_ * skill->def_multiplier_;
        stats.range_ = skill->base_range_ * skill->range_multiplier_;
        stats.atk_interval_ = skill->base_atk_interval_ * skill->atk_interval_multiplier_;
    } else {
        stats.max_hp_ *= 1.20f;
        stats.hp_ = stats.max_hp_;
        stats.atk_ *= 1.12f;
        stats.range_ *= 1.05f;
    }
}

} // namespace game::system
