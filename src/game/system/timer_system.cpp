#include "timer_system.h"

#include "../component/hero_skill_component.h"
#include "../component/stats_component.h"
#include "../defs/event.h"
#include "../defs/tags.h"
#include <algorithm>

namespace game::system {

void TimerSystem::update(entt::registry& registry, entt::dispatcher& dispatcher, float delta_time) {
    auto attack_view = registry.view<game::component::StatsComponent>(entt::exclude<game::defs::AttackReadyTag>);

    for (auto entity : attack_view) {
        auto& stats = attack_view.get<game::component::StatsComponent>(entity);
        
        stats.atk_timer_ += delta_time;
        if (stats.atk_timer_ >= stats.atk_interval_) {
            registry.emplace<game::defs::AttackReadyTag>(entity);
            // 注意：计时器不在这里归零，而是在攻击真正发起时（AttackStarterSystem）归零。
            // 这样可以确保如果单位一直找不到目标，AttackReadyTag 会一直保持，直到下一次攻击。
        }
    }

    auto skill_view = registry.view<game::component::HeroSkillComponent, game::component::StatsComponent>();
    for (auto entity : skill_view) {
        auto& skill = skill_view.get<game::component::HeroSkillComponent>(entity);

        if (skill.passive_) {
            continue;
        }

        if (skill.cooldown_timer_ > 0.0f) {
            skill.cooldown_timer_ = std::max(0.0f, skill.cooldown_timer_ - delta_time);
            if (skill.cooldown_timer_ <= 0.0f && !registry.all_of<game::defs::SkillReadyTag>(entity)) {
                registry.emplace<game::defs::SkillReadyTag>(entity);
                dispatcher.enqueue(game::defs::SkillReadyEvent{ entity });
            }
        }

        if (skill.active_ && skill.duration_timer_ > 0.0f) {
            skill.duration_timer_ = std::max(0.0f, skill.duration_timer_ - delta_time);
            if (skill.duration_timer_ <= 0.0f) {
                dispatcher.enqueue(game::defs::SkillDurationEndEvent{ entity });
            }
        }
    }
}

} // namespace game::system
