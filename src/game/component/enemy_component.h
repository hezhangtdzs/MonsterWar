#pragma once
namespace game::component {
    struct EnemyComponent {
        int target_waypoint_id_;
        float speed_;
    };
}