#pragma once

#include "../../engine/scene/scene.h"

namespace game::scene {

class TitleScene final : public engine::scene::Scene {
public:
    explicit TitleScene(engine::core::Context& context);
    ~TitleScene() override;

    void init() override;
    void update(float delta_time) override;
    void render() override;
    void clean() override;

private:
    void startGame();
};

} // namespace game::scene
