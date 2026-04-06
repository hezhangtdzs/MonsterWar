#include "title_scene.h"

#include "game_scene.h"
#include "../defs/event.h"
#include "../../engine/core/context.h"
#include "../../engine/core/game_state.h"
#include "../../engine/resource/resource_manager.h"
#include "../../engine/ui/ui_image.h"
#include "../../engine/ui/ui_manager.h"
#include "../../engine/ui/ui_button.h"
#include "../../engine/ui/ui_panel.h"
#include "../../engine/ui/ui_text.h"
#include <glm/vec2.hpp>

using namespace entt::literals;

namespace game::scene {

TitleScene::TitleScene(engine::core::Context& context)
    : engine::scene::Scene("TitleScene", context) {
}

TitleScene::~TitleScene() = default;

void TitleScene::init() {
    Scene::init();

    const auto window_size = context_.getGameState().getWindowLogicalSize();
    auto* ui_manager = getUIManager();
    if (!ui_manager) {
        return;
    }

    ui_manager->init(window_size);
    ui_manager->clear();

    auto background = std::unique_ptr<engine::ui::UIImage>(new engine::ui::UIImage(context_, "assets/textures/ref.png", glm::vec2{ 0.0f, 0.0f }, window_size));
    ui_manager->addElement(std::move(background));

    auto panel = std::unique_ptr<engine::ui::UIPanel>(new engine::ui::UIPanel(context_));
    panel->setPosition({ (window_size.x - 280.0f) * 0.5f, (window_size.y - 160.0f) * 0.5f });
    panel->setSize({ 280.0f, 160.0f });
    panel->setBackgroundColor({ 0.0f, 0.0f, 0.0f, 0.45f });
    panel->setBorderWidth(2.0f);
    panel->setBorderColor({ 1.0f, 1.0f, 1.0f, 0.25f });
    auto* panel_ptr = panel.get();

    auto title = std::unique_ptr<engine::ui::UIText>(new engine::ui::UIText(context_, "Monster War", "assets/fonts/VonwaonBitmap-16px.ttf", 36));
    title->setAlignment(engine::ui::TextAlignment::CENTER);
    title->setPosition({ 140.0f, 34.0f });
    title->setColor({ 1.0f, 0.96f, 0.72f, 1.0f });
    panel_ptr->addChild(std::move(title));

    auto start_button = std::unique_ptr<engine::ui::UIButton>(new engine::ui::UIButton(context_, "开始游戏", "assets/fonts/VonwaonBitmap-16px.ttf", 24));
    start_button->setPosition({ 40.0f, 72.0f });
    start_button->setSize({ 200.0f, 32.0f });
    start_button->setClickCallback([this]() { startGame(); });
    panel_ptr->addChild(std::move(start_button));

    auto quit_button = std::unique_ptr<engine::ui::UIButton>(new engine::ui::UIButton(context_, "退出", "assets/fonts/VonwaonBitmap-16px.ttf", 24));
    quit_button->setPosition({ 40.0f, 112.0f });
    quit_button->setSize({ 200.0f, 32.0f });
    quit_button->setClickCallback([this]() { quit(); });
    panel_ptr->addChild(std::move(quit_button));

    ui_manager->addElement(std::move(panel));

    context_.getResourceManager().stopMusic();
    context_.getResourceManager().playMusic(entt::hashed_string("title_bgm").value());
}

void TitleScene::update(float delta_time) {
    Scene::update(delta_time);
}

void TitleScene::render() {
    Scene::render();
}

void TitleScene::clean() {
    Scene::clean();
}

void TitleScene::startGame() {
    requestReplaceScene(std::unique_ptr<engine::scene::Scene>(new game::scene::GameScene(context_, 0)));
}

} // namespace game::scene
