#include "game_scene.h"

namespace game::scene {

    GameScene::GameScene(engine::core::Context& context, engine::scene::SceneManager& scene_manager)
        : engine::scene::Scene("Game_Scene", context, scene_manager) {
    }

    GameScene::~GameScene() = default;

    void GameScene::init() {
        engine::scene::Scene::init();
        // 在此处添加游戏场景的特定初始化逻辑
    }

} // namespace game::scene