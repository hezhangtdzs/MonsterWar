#pragma once

#include <entt/entity/entity.hpp>

namespace engine::scene {
    class Scene;
}

namespace game::ui {

class HeroInspectorUI final {
public:
    void render(engine::scene::Scene& scene);
};

} // namespace game::ui
