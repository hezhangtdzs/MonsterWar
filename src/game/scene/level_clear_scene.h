#pragma once

#include "../../engine/scene/scene.h"

namespace game::scene {

class LevelClearScene final : public engine::scene::Scene {
public:
    LevelClearScene(engine::core::Context& context, std::size_t current_level_index);
    ~LevelClearScene() override;

    void init() override;
    void update(float delta_time) override;
    void render() override;
    void clean() override;

private:
    std::size_t current_level_index_ = 0;
    std::size_t next_level_index_ = 0;

    void startNextLevel();
    void returnToTitle();
};

} // namespace game::scene
