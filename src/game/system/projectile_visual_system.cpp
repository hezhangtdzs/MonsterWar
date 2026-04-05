#include "projectile_visual_system.h"
#include "../component/projectile_visual_component.h"
#include "../defs/tags.h"
#include "../../engine/component/transform_component.h"
#include <entt/entity/registry.hpp>
#include <glm/geometric.hpp>
#include <glm/common.hpp>
#include <glm/trigonometric.hpp>

namespace game::system {

void ProjectileVisualSystem::update(entt::registry& registry, float delta_time) {
    auto view = registry.view<game::component::ProjectileVisualComponent, engine::component::TransformComponent>();
    for (auto entity : view) {
        auto& projectile = view.get<game::component::ProjectileVisualComponent>(entity);
        auto& transform = view.get<engine::component::TransformComponent>(entity);

        projectile.elapsed_time_ += delta_time;
        const float flight_time = projectile.flight_time_ > 0.01f ? projectile.flight_time_ : 0.01f;
        float t = projectile.elapsed_time_ / flight_time;
        if (t > 1.0f) {
            t = 1.0f;
        }

        const glm::vec2 delta = projectile.target_position_ - projectile.start_position_;
        transform.position_ = projectile.start_position_ + delta * t;
        transform.position_.y -= projectile.arc_height_ * 4.0f * t * (1.0f - t);

        const float tangent_y = delta.y - projectile.arc_height_ * 4.0f * (1.0f - 2.0f * t);
        transform.rotation_ = glm::degrees(glm::atan(tangent_y, delta.x)) + projectile.rotation_offset_deg_;

        if (projectile.elapsed_time_ >= flight_time) {
            registry.emplace_or_replace<game::defs::DeadTag>(entity);
        }
    }
}

} // namespace game::system
