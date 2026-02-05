/**
 * @file game_scene.cpp
 * @brief GameScene 类的实现，游戏主场景的具体逻辑。
 * 
 * 实现了场景初始化、清理和输入事件处理，包含资源ID测试、
 * 音频播放测试和UI元素测试功能。
 */

#include "game_scene.h"
#include "../../engine/core/context.h"
#include "../../engine/input/input_manager.h"
#include "../../engine/audio/audio_locator.h"
#include "../../engine/resource/resource_id.h"
#include "../../engine/ui/ui_manager.h"
#include "../../engine/ui/ui_image.h"
#include "../../engine/ui/ui_text.h"
#include <entt/signal/sigh.hpp>
#include <spdlog/spdlog.h>

namespace game::scene {
 using namespace entt::literals;
GameScene::GameScene(engine::core::Context& context)
    : engine::scene::Scene("GameScene", context) {
}

GameScene::~GameScene() {
}

void GameScene::init() {
    // 测试场景编号, 每创建一个场景, 编号加1
    static int count = 0;
    scene_num_ = count++;
    spdlog::info("场景编号: {}", scene_num_);

    // --- 资源 ID 测试用例 ---
    const auto battle_bgm_id = engine::resource::toResourceId("battle_bgm");
    const auto ui_hover_id = engine::resource::toResourceId("ui_hover");

    spdlog::info("battle_bgm id = {}", battle_bgm_id);
    spdlog::info("ui_hover id = {}", ui_hover_id);

    // 测试：通过映射表使用 ID 播放
    engine::audio::AudioLocator::get().playMusic("battle_bgm"_hs);
    engine::audio::AudioLocator::get().playSound(ui_hover_id);

    // --- UI 图片/文字测试用例 ---
    if (auto* ui_manager = getUIManager()) {
        auto test_image = std::make_unique<engine::ui::UIImage>(
            context_,
            "assets/textures/UI/circle.png",
            glm::vec2{ 32.0f, 32.0f },
            glm::vec2{ 64.0f, 64.0f }
        );
        ui_manager->addElement(std::move(test_image));

        auto test_text = std::make_unique<engine::ui::UIText>(
            context_,
            "UI Test: hashed id",
            "assets/fonts/VonwaonBitmap-16px.ttf",
            16
        );
        test_text->setPosition(glm::vec2{ 120.0f, 40.0f });
        ui_manager->addElement(std::move(test_text));
    }

    // 注册输入回调事件
    auto& input_manager = context_.getInputManager();
    input_manager.onAction("jump").connect<&GameScene::onReplace>(this);       // J 键
    input_manager.onAction("mouse_left").connect<&GameScene::onPush>(this);   // 鼠标左键
    input_manager.onAction("mouse_right").connect<&GameScene::onPop>(this);   // 鼠标右键
    input_manager.onAction("pause").connect<&GameScene::onQuit>(this);        // P 键

    Scene::init();
}

void GameScene::clean() {
    // 断开输入回调事件 (谁连接，谁负责断开)
    auto& input_manager = context_.getInputManager();
    input_manager.onAction("jump").disconnect<&GameScene::onReplace>(this);
    input_manager.onAction("mouse_left").disconnect<&GameScene::onPush>(this);
    input_manager.onAction("mouse_right").disconnect<&GameScene::onPop>(this);
    input_manager.onAction("pause").disconnect<&GameScene::onQuit>(this);

    Scene::clean();
}

bool GameScene::onReplace() {
    spdlog::info("onReplace, 切换场景");
    requestReplaceScene(std::make_unique<game::scene::GameScene>(context_));
    return true;
}

bool GameScene::onPush() {
    spdlog::info("onPush, 压入场景");
    requestPushScene(std::make_unique<game::scene::GameScene>(context_));
    return true;
}

bool GameScene::onPop() {
    spdlog::info("onPop, 弹出编号为 {} 的场景", scene_num_);
    requestPopScene();
    return true;
}

bool GameScene::onQuit() {
    spdlog::info("onQuit, 退出游戏");
    quit();
    return true;
}

} // namespace game::scene