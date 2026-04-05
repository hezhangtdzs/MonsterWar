#pragma once

#include <entt/entt.hpp>

namespace engine::core {
    class Context;
}

namespace game::system {

class SelectionSystem final {
public:
    void update(entt::registry& registry, engine::core::Context& context);

private:
    void updateHoveredUnit(entt::registry& registry, engine::core::Context& context);
    void updateSelectedUnit(entt::registry& registry, engine::core::Context& context);
};

} // namespace game::system
