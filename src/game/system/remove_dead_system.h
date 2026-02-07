#pragma once
#include <entt/entt.hpp>
namespace game::system {

class RemoveDeadSystem {
public:
    void update(entt::registry& registry);
};

}   // namespace game::system