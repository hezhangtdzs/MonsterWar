#pragma once
#include <entt/entity/registry.hpp>
namespace engine::render {
    class Renderer;
    class Camera;
}

namespace engine::system {
   class RenderSystem {
   public:
         RenderSystem() = default;
         ~RenderSystem() = default;

         void  update(entt::registry& registry,
                engine::render::Renderer& renderer,
                const engine::render::Camera& camera);
    };
}