#pragma once

#include <entt/entt.hpp>

#include "../component/hero_skill_component.h"
#include "../component/player_component.h"
#include "../component/stats_component.h"
#include "../data/entity_blueprint.h"
#include "../defs/event.h"

namespace game::system {

class HeroSkillSystem final {
private:
    entt::registry& registry_;
    entt::dispatcher& dispatcher_;

public:
    HeroSkillSystem(entt::registry& registry, entt::dispatcher& dispatcher);
    ~HeroSkillSystem();

    void update(float delta_time);

private:
    void onUpgradeHeroEvent(const game::defs::UpgradeHeroEvent& event);
    void onReleaseHeroSkillEvent(const game::defs::ReleaseHeroSkillEvent& event);
    void onSkillReadyEvent(const game::defs::SkillReadyEvent& event);
    void onSkillActiveEvent(const game::defs::SkillActiveEvent& event);
    void onSkillDurationEndEvent(const game::defs::SkillDurationEndEvent& event);

    void activateSkill(entt::entity entity,
                       game::component::HeroSkillComponent& skill,
                       game::component::StatsComponent& stats,
                       const game::data::SkillBlueprint& blueprint);
    void deactivateSkill(entt::entity entity,
                         game::component::HeroSkillComponent& skill,
                         game::component::StatsComponent& stats);
    void applyUpgrade(entt::entity entity,
                      game::component::HeroSkillComponent* skill,
                      game::component::StatsComponent& stats);
};

} // namespace game::system
