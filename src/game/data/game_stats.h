#pragma once

namespace game::data {

struct GameStats {
    float cost_{ 10.0f };
    float cost_gen_per_second_{ 1.0f };
    int home_hp_{ 5 };
    int enemy_count_{ 0 };
    int enemy_arrived_count_{ 0 };
    int enemy_killed_count_{ 0 };
};

} // namespace game::data
