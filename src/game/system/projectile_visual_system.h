#pragma once

#include <entt/entity/fwd.hpp>

namespace game::system {

class ProjectileVisualSystem {
public:
    void update(entt::registry& registry, float delta_time);
};

} // namespace game::system
