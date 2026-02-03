#include "game_scene.h"
#include "../../engine/input/input_manager.h"
#include "../../engine/core/context.h"
#include <spdlog/spdlog.h>
namespace game::scene {

    GameScene::GameScene(engine::core::Context& context, engine::scene::SceneManager& scene_manager)
        : engine::scene::Scene("Game_Scene", context, scene_manager) {
    }

    GameScene::~GameScene(){
        clean();
    }

    void GameScene::init() {
        engine::scene::Scene::init();
        // 在此处添加游戏场景的特定初始化逻辑
        auto& input_manager = context_.getInputManager();
        // 示例：注册一个动作响应
        input_manager.onAction("jump", engine::input::ActionState::PRESSED).connect<&GameScene::handleJump>(this);
        input_manager.onAction("attack", engine::input::ActionState::RELEASED).connect<&GameScene::handleAttack>(this);
    }

    void GameScene::clean()
    {
        // 在此处添加游戏场景的特定清理逻辑

        engine::scene::Scene::clean();  
        auto& input_manager = context_.getInputManager();
        // 示例：断开动作响应
        input_manager.onAction("jump", engine::input::ActionState::PRESSED).disconnect<&GameScene::handleJump>(this);
        input_manager.onAction("attack", engine::input::ActionState::RELEASED).disconnect<&GameScene::handleAttack>(this);
    }

    void GameScene::handleJump()
    {
        spdlog::info("Jump action triggered!");
    }

    void GameScene::handleAttack()
    {
        spdlog::info("Attack action triggered!");
    }

} // namespace game::scene