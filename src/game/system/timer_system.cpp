#include "timer_system.h"
#include "game/component/stats_component.h"
#include "game/defs/tags.h"

namespace game::system {

void TimerSystem::update(entt::registry& registry, float delta_time) {
    auto view = registry.view<game::component::StatsComponent>(entt::exclude<game::defs::AttackReadyTag>);

    for (auto entity : view) {
        auto& stats = view.get<game::component::StatsComponent>(entity);
        
        stats.atk_timer_ += delta_time;
        if (stats.atk_timer_ >= stats.atk_interval_) {
            registry.emplace<game::defs::AttackReadyTag>(entity);
            // 注意：计时器不在这里归零，而是在攻击真正发起时（AttackStarterSystem）归零。
            // 这样可以确保如果单位一直找不到目标，AttackReadyTag 会一直保持，直到下一次攻击。
        }
    }
}

} // namespace game::system
