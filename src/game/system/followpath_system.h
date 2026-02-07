#pragma once
#include <entt/entt.hpp>
#include <unordered_map>
#include "game/data/waypoint_node.h"
namespace game::system {
    class FollowPathSystem {
    public:
        void update(entt::registry& registry,entt::dispatcher& dispatcher, 
        std::unordered_map<int, game::data::WaypointNode>& waypoint_nodes);
    };
}