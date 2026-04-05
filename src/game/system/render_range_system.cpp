#include "render_range_system.h"

#include "../component/unit_prep_component.h"
#include "../defs/tags.h"
#include "../../engine/component/transform_component.h"
#include "../../engine/render/camera.h"
#include "../../engine/render/renderer.h"

namespace game::system {

void RenderRangeSystem::update(entt::registry& registry, engine::render::Renderer& renderer, const engine::render::Camera& camera) {
    auto view = registry.view<game::defs::ShowRangeTag, engine::component::TransformComponent, game::component::UnitPrepComponent>();
    for (auto entity : view) {
        const auto& transform = view.get<engine::component::TransformComponent>(entity);
        const auto& prep = view.get<game::component::UnitPrepComponent>(entity);
        if (prep.range_ <= 0.0f) {
            continue;
        }
        renderer.drawFilledCircle(camera, transform.position_, prep.range_, game::defs::RANGE_COLOR);
    }
}

} // namespace game::system
