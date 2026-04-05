#pragma once

#include <entt/entt.hpp>

namespace engine::render {
    class Renderer;
    class Camera;
}

namespace game::system {

class RenderRangeSystem final {
public:
    RenderRangeSystem() = default;
    ~RenderRangeSystem() = default;

    void update(entt::registry& registry, engine::render::Renderer& renderer, const engine::render::Camera& camera);
};

} // namespace game::system
