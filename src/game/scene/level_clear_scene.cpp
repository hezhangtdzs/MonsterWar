#include "level_clear_scene.h"

#include "game_scene.h"
#include "title_scene.h"
#include "../../engine/core/context.h"
#include "../../engine/core/game_state.h"
#include <entt/core/hashed_string.hpp>
#include "../../engine/resource/resource_manager.h"
#include "../../engine/ui/ui_button.h"
#include "../../engine/ui/ui_manager.h"
#include "../../engine/ui/ui_panel.h"
#include "../../engine/ui/ui_text.h"
#include <glm/vec2.hpp>

namespace game::scene {

LevelClearScene::LevelClearScene(engine::core::Context& context, std::size_t current_level_index)
    : engine::scene::Scene("LevelClearScene", context), current_level_index_(current_level_index) {
    next_level_index_ = current_level_index_ == 0 ? 1 : 0;
}

LevelClearScene::~LevelClearScene() = default;

void LevelClearScene::init() {
    Scene::init();

    const auto window_size = context_.getGameState().getWindowLogicalSize();
    auto* ui_manager = getUIManager();
    if (!ui_manager) {
        return;
    }

    ui_manager->init(window_size);
    ui_manager->clear();

    context_.getResourceManager().stopMusic();
    context_.getResourceManager().playMusic(entt::hashed_string("win").value());

    auto overlay = std::unique_ptr<engine::ui::UIPanel>(new engine::ui::UIPanel(context_));
    overlay->setPosition({ 0.0f, 0.0f });
    overlay->setSize(window_size);
    overlay->setBackgroundColor({ 0.0f, 0.0f, 0.0f, 0.45f });
    auto* overlay_ptr = overlay.get();

    auto panel = std::unique_ptr<engine::ui::UIPanel>(new engine::ui::UIPanel(context_));
    panel->setPosition({ window_size.x * 0.5f - 200.0f, window_size.y * 0.5f - 150.0f });
    panel->setSize({ 400.0f, 300.0f });
    panel->setBackgroundColor({ 0.0f, 0.0f, 0.0f, 0.65f });
    panel->setBorderWidth(2.0f);
    panel->setBorderColor({ 1.0f, 1.0f, 1.0f, 0.30f });
    auto* panel_ptr = panel.get();

    auto title = std::unique_ptr<engine::ui::UIText>(new engine::ui::UIText(context_, "通关!", "assets/fonts/VonwaonBitmap-16px.ttf", 36));
    title->setAlignment(engine::ui::TextAlignment::CENTER);
    title->setPosition({ 200.0f, 54.0f });
    title->setColor({ 1.0f, 0.96f, 0.70f, 1.0f });
    panel_ptr->addChild(std::move(title));

    auto message = std::unique_ptr<engine::ui::UIText>(new engine::ui::UIText(context_, current_level_index_ == 0 ? "已完成 Level 1" : "已完成 Level 2", "assets/fonts/VonwaonBitmap-16px.ttf", 18));
    message->setAlignment(engine::ui::TextAlignment::CENTER);
    message->setPosition({ 200.0f, 100.0f });
    message->setColor({ 0.95f, 0.95f, 0.95f, 1.0f });
    panel_ptr->addChild(std::move(message));

    auto continue_button = std::unique_ptr<engine::ui::UIButton>(new engine::ui::UIButton(context_, next_level_index_ == 0 ? "返回第一关" : "下一关", "assets/fonts/VonwaonBitmap-16px.ttf", 24));
    continue_button->setPosition({ 90.0f, 150.0f });
    continue_button->setSize({ 220.0f, 40.0f });
    continue_button->setClickCallback([this]() { startNextLevel(); });
    panel_ptr->addChild(std::move(continue_button));

    auto title_button = std::unique_ptr<engine::ui::UIButton>(new engine::ui::UIButton(context_, "返回标题", "assets/fonts/VonwaonBitmap-16px.ttf", 24));
    title_button->setPosition({ 90.0f, 200.0f });
    title_button->setSize({ 220.0f, 40.0f });
    title_button->setClickCallback([this]() { returnToTitle(); });
    panel_ptr->addChild(std::move(title_button));

    overlay_ptr->addChild(std::move(panel));
    ui_manager->addElement(std::move(overlay));
}

void LevelClearScene::update(float delta_time) {
    Scene::update(delta_time);
}

void LevelClearScene::render() {
    Scene::render();
}

void LevelClearScene::clean() {
    Scene::clean();
}

void LevelClearScene::startNextLevel() {
    requestReplaceScene(std::unique_ptr<engine::scene::Scene>(new game::scene::GameScene(context_, next_level_index_)));
}

void LevelClearScene::returnToTitle() {
    requestReplaceScene(std::unique_ptr<engine::scene::Scene>(new game::scene::TitleScene(context_)));
}

} // namespace game::scene
