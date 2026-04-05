#include "render_range_system.h"

#include "../component/player_component.h"
#include "../component/stats_component.h"
#include "../component/unit_prep_component.h"
#include "../defs/tags.h"
#include "../../engine/component/transform_component.h"
#include "../../engine/render/camera.h"
#include "../../engine/render/renderer.h"

namespace game::system {

void RenderRangeSystem::update(entt::registry& registry, engine::render::Renderer& renderer, const engine::render::Camera& camera) {
    auto prep_view = registry.view<game::defs::ShowRangeTag, engine::component::TransformComponent, game::component::UnitPrepComponent>();
    for (auto entity : prep_view) {
        const auto& transform = prep_view.get<engine::component::TransformComponent>(entity);
        const auto& prep = prep_view.get<game::component::UnitPrepComponent>(entity);
        if (prep.range_ <= 0.0f) {
            continue;
        }
        renderer.drawFilledCircle(camera, transform.position_, prep.range_, game::defs::RANGE_COLOR);
    }

    auto selected_view = registry.view<game::defs::ShowRangeTag, engine::component::TransformComponent, game::component::StatsComponent, game::component::PlayerComponent>();
    for (auto entity : selected_view) {
        const auto& transform = selected_view.get<engine::component::TransformComponent>(entity);
        const auto& stats = selected_view.get<game::component::StatsComponent>(entity);
        if (stats.range_ <= 0.0f) {
            continue;
        }
        renderer.drawFilledCircle(camera, transform.position_, stats.range_, game::defs::RANGE_COLOR);
    }
}

} // namespace game::system
