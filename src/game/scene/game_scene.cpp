#include "game_scene.h"
#include "../component/enemy_component.h"
#include "../component/player_component.h"
#include "../factory/entity_factory.h"
#include "../factory/blueprint_manager.h"
#include "../loader/entity_builder_mw.h"
#include "./title_scene.h"
#include "./level_clear_scene.h"
#include "../ui/units_portrait_ui.h"
#include "../system/followpath_system.h"
#include "../system/remove_dead_system.h"
#include "../system/block_system.h"
#include "../system/set_target_system.h"
#include "../system/timer_system.h"
#include "../system/attack_starter_system.h"
#include "../system/projectile_visual_system.h"
#include "../system/animation_state_system.h"
#include "../system/orientation_system.h"
#include "../system/animation_events_system.h"
#include "../system/combat_resolve_system.h"
#include "../system/selection_system.h"
#include "../system/game_rule_system.h"
#include "../system/hero_skill_system.h"
#include "../system/place_unit_system.h"
#include "../system/render_range_system.h"
#include "../data/selection_state.h"
#include "../defs/tags.h"
#include "../defs/constants.h"
#include "../data/game_stats.h"
#include "../component/hero_skill_component.h"
#include "../component/stats_component.h"
#include "../../engine/component/transform_component.h"
#include "../../engine/component/velocity_component.h"
#include "../../engine/component/sprite_component.h"
#include "../../engine/component/render_component.h"
#include "../../engine/resource/resource_manager.h"
#include "../../engine/input/input_manager.h"
#include "../../engine/core/context.h"
#include "../../engine/core/game_state.h"
#include "../../engine/ui/ui_manager.h"
#include "../../engine/ui/ui_button.h"
#include "../../engine/ui/ui_panel.h"
#include "../../engine/ui/ui_image.h"
#include "../../engine/ui/ui_text.h"
#include "../../engine/ui/state/normal_state.h"
#include "../../engine/ui/state/hover_state.h"
#include "../../engine/ui/state/pressed_state.h"
#include "../../engine/system/render_system.h"
#include "../../engine/system/movement_system.h"
#include "../../engine/system/animation_system.h"
#include "../../engine/system/ysort_system.h"
#include "../../engine/system/audio_system.h"
#include "../../engine/loader/level_loader.h"
#include "../../engine/utils/logging.h"
#include "../data/session_data.h"
#include "../data/ui_config.h"
#include <fstream>
#include <algorithm>
#include <entt/core/hashed_string.hpp>
#include <entt/signal/sigh.hpp>
#include <nlohmann/json.hpp>
#include <limits>
#include <string>

using namespace entt::literals;

namespace {

void appendEnemyTypesToQueue(const nlohmann::json& wave_json, std::vector<entt::id_type>& enemy_queue) {
    if (!wave_json.contains("enemy_types") || !wave_json["enemy_types"].is_object()) {
        return;
    }

    for (auto it = wave_json["enemy_types"].begin(); it != wave_json["enemy_types"].end(); ++it) {
        if (!it.value().is_number_integer()) {
            continue;
        }

        const int count = it.value().get<int>();
        if (count <= 0) {
            continue;
        }

        const auto class_id = entt::hashed_string(it.key().c_str());
        for (int i = 0; i < count; ++i) {
            enemy_queue.push_back(class_id);
        }
    }
}

} // namespace

namespace game::scene {

GameScene::GameScene(engine::core::Context& context, std::size_t level_index)
    : engine::scene::Scene("GameScene", context), selected_level_index_(level_index) {
    auto& dispatcher = context.getDispatcher();
    // 初始化系统
    render_system_ = std::make_unique<engine::system::RenderSystem>();
    movement_system_ = std::make_unique<engine::system::MovementSystem>();
    animation_system_ = std::make_unique<engine::system::AnimationSystem>(registry_, dispatcher);
    ysort_system_ = std::make_unique<engine::system::YSortSystem>();
	audio_system_ = std::make_unique<engine::system::AudioSystem>(registry_, dispatcher);

    follow_path_system_ = std::make_unique<game::system::FollowPathSystem>();
    remove_dead_system_ = std::make_unique<game::system::RemoveDeadSystem>();
    block_system_ = std::make_unique<game::system::BlockSystem>();
    set_target_system_ = std::make_unique<game::system::SetTargetSystem>();
    timer_system_ = std::make_unique<game::system::TimerSystem>();
    attack_starter_system_ = std::make_unique<game::system::AttackStarterSystem>();
    projectile_visual_system_ = std::make_unique<game::system::ProjectileVisualSystem>();
    animation_state_system_ = std::make_unique<game::system::AnimationStateSystem>(registry_, dispatcher);
    orientation_system_ = std::make_unique<game::system::OrientationSystem>();
	animation_events_system_ = std::make_unique<game::system::AnimationEventsSystem>(registry_, dispatcher);
	combat_resolve_system_ = std::make_unique<game::system::CombatResolveSystem>(registry_, dispatcher);

    ENGINE_LOG_INFO("GameScene 构造完成");
}

GameScene::~GameScene() {
}

void GameScene::init() {
    ENGINE_LOG_INFO("GameScene 初始化开始");

    auto fail_and_clean = [this](const char* message) {
        ENGINE_LOG_ERROR("{}", message);
        clean();
    };

    if (!loadLevel()) {
        fail_and_clean("加载关卡失败");
        return;
    }
    if (!initSessionData()) {
        fail_and_clean("初始化 SessionData 失败");
        return;
    }
    if (!initUIConfig()) {
        fail_and_clean("初始化 UIConfig 失败");
        return;
    }
    if (!initEventConnections()) {
        fail_and_clean("初始化事件连接失败");
        return;
    }
    if (!initInputConnections()) {
        fail_and_clean("初始化输入连接失败");
        return;
    }
    if (!initEntityFactory()) {
        fail_and_clean("初始化实体工厂失败");
        return;
    }
    if (!initRegistryContext()) {
        fail_and_clean("初始化注册表上下文失败");
        return;
    }
    if (!initSelectionSystem()) {
        fail_and_clean("初始化选择系统失败");
        return;
    }
    if (!initGameRuleSystem()) {
        fail_and_clean("初始化关卡规则系统失败");
        return;
    }
    if (!initHeroSkillSystem()) {
        fail_and_clean("初始化英雄技能系统失败");
        return;
    }
    if (!initPlacementSystem()) {
        fail_and_clean("初始化出击系统失败");
        return;
    }
    Scene::init();
    context_.getGameState().setState(engine::core::GameStateType::Playing);
    if (!initUI()) {
        fail_and_clean("初始化 UI 失败");
        return;
    }
    context_.getResourceManager().stopMusic();
    context_.getResourceManager().playMusic(entt::hashed_string("battle_bgm").value());
    base_hp_ = game::defs::INITIAL_BASE_HP;
    game_stats_.home_hp_ = base_hp_;
    game_stats_.cost_ = static_cast<float>(game::defs::INITIAL_GOLD);
    game_stats_.cost_gen_per_second_ = 1.0f;
    last_unit_panel_cost_ = static_cast<int>(game_stats_.cost_);
    current_wave_ = 0;
    wave_running_ = false;
    selected_unit_id_ = 0;
    selected_unit_name_.clear();
    health_bar_widgets_.clear();
    wave_spawn_timer_ = 0.0f;
    wave_break_timer_ = 0.0f;
    wave_break_duration_ = level_prep_time_;
    ENGINE_LOG_INFO("GameScene 初始化完成");
}

void GameScene::update(float delta_time) {
    if (context_.getGameState().isGameOver()) {
        updateUi(delta_time);
        Scene::update(delta_time);
        return;
    }

    auto& dispatcher = context_.getDispatcher();

    if (selection_system_) {
        selection_system_->update(registry_, context_);
    }

    dispatcher.update();

    if (!context_.getGameState().isPaused()) {
        if (game_rule_system_) {
            game_rule_system_->update(delta_time);
        }
        if (hero_skill_system_) {
            hero_skill_system_->update(delta_time);
        }
        if (game_stats_.home_hp_ <= 0) {
            context_.getGameState().setState(engine::core::GameStateType::GameOver);
        }
        if (place_unit_system_) {
            place_unit_system_->update(delta_time);
        }
        updateWaveFlow(delta_time);

        // 每一帧最先清理死亡实体(要在dispatcher处理完事件后再清理，因此放在下一帧开头)
        remove_dead_system_->update(registry_);

        // 注意系统更新的顺序
        follow_path_system_->update(registry_, dispatcher, waypoint_nodes_);
        block_system_->update(registry_, dispatcher);

        // 战斗循环
        set_target_system_->update(registry_);
        timer_system_->update(registry_, dispatcher, delta_time);
        attack_starter_system_->update(registry_, dispatcher);
        projectile_visual_system_->update(registry_, delta_time);

        movement_system_->update(registry_, delta_time);
        animation_system_->update(delta_time);

        // 视觉修正
        orientation_system_->update(registry_);
        ysort_system_->update(registry_);   // 调用顺序要在MovementSystem之后
    }

    updateUi(delta_time);
    Scene::update(delta_time);

    ENGINE_LOG_DEBUG("alive={}", registry_.storage<entt::entity>().size());
}

void GameScene::render() {
    if (render_range_system_) {
        render_range_system_->update(registry_, context_.getRenderer(), context_.getCamera());
    }
    render_system_->update(registry_, context_.getRenderer(), context_.getCamera());

    Scene::render();
}

void GameScene::clean() {
    auto& dispatcher = context_.getDispatcher();
    auto& input_manager = context_.getInputManager();
    // 断开所有事件连接
    dispatcher.disconnect(this);
    // 断开输入信号连接
    input_manager.onAction("mouse_right"_hs).disconnect<&GameScene::onCreateTestPlayerMelee>(this);
    input_manager.onAction("mouse_left"_hs).disconnect<&GameScene::onCreateTestPlayerRanged>(this);
    input_manager.onAction("move_left"_hs).disconnect<&GameScene::onCreateTestPlayerHealer>(this);
    input_manager.onAction("release_skill"_hs).disconnect<&GameScene::onReleaseSelectedHeroSkill>(this);
    input_manager.onAction("pause"_hs).disconnect<&GameScene::togglePause>(this);
    health_bar_widgets_.clear();
    hidden_unit_portrait_ids_.clear();
    health_bar_layer_ = nullptr;
    unit_panel_ = nullptr;
    pause_overlay_ = nullptr;
    selection_system_.reset();
    game_rule_system_.reset();
    hero_skill_system_.reset();
    place_unit_system_.reset();
    render_range_system_.reset();
    Scene::clean();
}

bool GameScene::loadLevel() {
    if (!loadLevelConfig()) {
        return false;
    }

    engine::loader::LevelLoader level_loader;
    // 设置拓展的构建器EntityBuilderMW
    level_loader.setEntityBuilder(std::make_unique<game::loader::EntityBuilderMW>(level_loader, 
        context_, 
        registry_, 
        waypoint_nodes_, 
        start_points_)
    );
    if (!level_loader.loadLevel(current_map_path_, this)) {
        ENGINE_LOG_ERROR("加载关卡失败: {}", current_map_path_);
        return false;
    }
    return true;
}

bool GameScene::initSessionData()
{
    session_data_ = std::make_unique<game::data::SessionData>();
    if (!session_data_->load("assets/data/default_session_data.json")) {
        ENGINE_LOG_ERROR("加载默认存档失败");
        return false;
    }
    return true;
}

bool GameScene::initUIConfig()
{
    ui_config_ = std::make_unique<game::data::UIConfig>();
    if (!ui_config_->load("assets/data/ui_config.json")) {
        ENGINE_LOG_ERROR("加载 UI 配置失败");
        return false;
    }
    return true;
}

bool GameScene::loadLevelConfig() {
    std::ifstream file(level_config_path_);
    if (!file.is_open()) {
        ENGINE_LOG_ERROR("无法打开关卡配置文件: {}", level_config_path_);
        return false;
    }

    nlohmann::json config_json;
    try {
        file >> config_json;
    }
    catch (const std::exception& e) {
        ENGINE_LOG_ERROR("解析关卡配置失败: {}", e.what());
        return false;
    }

    if (!config_json.is_array() || config_json.empty()) {
        ENGINE_LOG_ERROR("关卡配置文件为空或格式无效: {}", level_config_path_);
        return false;
    }

    if (selected_level_index_ >= config_json.size()) {
        selected_level_index_ = 0;
    }

    const auto& level_json = config_json.at(selected_level_index_);
    current_level_name_ = level_json.value("name", current_level_name_);
    current_map_path_ = level_json.value("map_path", current_map_path_);
    level_prep_time_ = level_json.value("prep_time", level_prep_time_);
    enemy_level_ = level_json.value("enemy_level", enemy_level_);
    enemy_rarity_ = level_json.value("enemy_rarity", enemy_rarity_);

    level_waves_.clear();
    if (level_json.contains("waves") && level_json["waves"].is_array()) {
        for (const auto& wave_json : level_json["waves"]) {
            WaveConfig wave;
            wave.spawn_interval_ = wave_json.value("spawn_interval", game::defs::WAVE_SPAWN_INTERVAL);
            wave.next_wave_interval_ = wave_json.value("next_wave_interval", game::defs::WAVE_BREAK_DURATION);
            appendEnemyTypesToQueue(wave_json, wave.enemy_queue_);

            if (!wave.enemy_queue_.empty()) {
                level_waves_.push_back(std::move(wave));
            }
        }
    }

    ENGINE_LOG_INFO("关卡配置加载完成: {} -> {}，波次数: {}", current_level_name_, current_map_path_, level_waves_.size());
    return true;
}

bool GameScene::initEventConnections() {
    auto& dispatcher = context_.getDispatcher();
    dispatcher.sink<game::defs::SpawnProjectileVisualEvent>().connect<&GameScene::onSpawnProjectileVisual>(this);
    dispatcher.sink<game::defs::SpawnEffectVisualEvent>().connect<&GameScene::onSpawnEffectVisual>(this);
    dispatcher.sink<game::defs::RemoveUIPortraitEvent>().connect<&GameScene::onRemoveUIPortrait>(this);
    dispatcher.sink<game::defs::UIPortraitHoverEnterEvent>().connect<&GameScene::onUIPortraitHoverEnter>(this);
    dispatcher.sink<game::defs::UIPortraitHoverLeaveEvent>().connect<&GameScene::onUIPortraitHoverLeave>(this);
    dispatcher.sink<game::defs::RestartEvent>().connect<&GameScene::onRestartRequested>(this);
    dispatcher.sink<game::defs::BackToTitleEvent>().connect<&GameScene::onBackToTitleRequested>(this);
    dispatcher.sink<game::defs::SaveEvent>().connect<&GameScene::onSaveRequested>(this);
    dispatcher.sink<game::defs::LevelClearEvent>().connect<&GameScene::onLevelClearRequested>(this);
    return true;
}

bool GameScene::initInputConnections() {
    auto& input_manager = context_.getInputManager();
    input_manager.onAction("pause"_hs).connect<&GameScene::togglePause>(this);
    return true;
}

bool GameScene::initEntityFactory() {
    if (!blueprint_manager_) {
        blueprint_manager_ = std::make_shared<game::factory::BlueprintManager>(context_.getResourceManager());
        if (!blueprint_manager_->loadEnemyClassBlueprints("assets/data/enemy_data.json") ||
            !blueprint_manager_->loadPlayerClassBlueprints("assets/data/player_data.json") ||
            !blueprint_manager_->loadSkillBlueprints("assets/data/skill_data.json") ||
            !blueprint_manager_->loadProjectileBlueprints("assets/data/projectile_data.json") ||
            !blueprint_manager_->loadEffectBlueprints("assets/data/effect_data.json")) {
            ENGINE_LOG_ERROR("蓝图资源加载失败");
            return false;
        }
    }

    entity_factory_ = std::make_unique<game::factory::EntityFactory>(registry_, *blueprint_manager_);
    ENGINE_LOG_INFO("entity_factory_ 加载完成");
    return true;
}

bool GameScene::initRegistryContext() {
    try {
        registry_.ctx().emplace<std::shared_ptr<game::factory::BlueprintManager>>(std::shared_ptr<game::factory::BlueprintManager>(blueprint_manager_.get(), [](game::factory::BlueprintManager*) {}));
        registry_.ctx().emplace<std::shared_ptr<game::data::SessionData>>(std::shared_ptr<game::data::SessionData>(session_data_.get(), [](game::data::SessionData*) {}));
        registry_.ctx().emplace<std::shared_ptr<game::data::UIConfig>>(std::shared_ptr<game::data::UIConfig>(ui_config_.get(), [](game::data::UIConfig*) {}));
        registry_.ctx().emplace<game::data::GameStats&>(game_stats_);
        registry_.ctx().emplace<game::data::SelectionState>();
    } catch (const std::exception& e) {
        ENGINE_LOG_ERROR("初始化注册表上下文失败: {}", e.what());
        return false;
    }
    return true;
}

bool GameScene::initSelectionSystem() {
    try {
        selection_system_ = std::make_unique<game::system::SelectionSystem>();
    } catch (const std::exception& e) {
        ENGINE_LOG_ERROR("初始化选择系统失败: {}", e.what());
        return false;
    }
    return true;
}

bool GameScene::initHeroSkillSystem() {
    try {
        hero_skill_system_ = std::make_unique<game::system::HeroSkillSystem>(registry_, context_.getDispatcher());
    } catch (const std::exception& e) {
        ENGINE_LOG_ERROR("初始化英雄技能系统失败: {}", e.what());
        return false;
    }
    return true;
}

bool GameScene::initGameRuleSystem() {
    try {
        game_rule_system_ = std::make_unique<game::system::GameRuleSystem>(registry_, context_.getDispatcher());
    } catch (const std::exception& e) {
        ENGINE_LOG_ERROR("初始化关卡规则系统失败: {}", e.what());
        return false;
    }
    return true;
}

bool GameScene::initPlacementSystem() {
    try {
        place_unit_system_ = std::make_unique<game::system::PlaceUnitSystem>(registry_, context_.getDispatcher(), context_, *entity_factory_);
        render_range_system_ = std::make_unique<game::system::RenderRangeSystem>();
    } catch (const std::exception& e) {
        ENGINE_LOG_ERROR("初始化出击系统失败: {}", e.what());
        return false;
    }
    return true;
}

bool GameScene::initUI() {
    auto* ui_manager = getUIManager();
    if (!ui_manager) {
        ENGINE_LOG_ERROR("UIManager 为空，无法初始化 UI");
        return false;
    }

    const auto window_size = context_.getGameState().getWindowLogicalSize();
    if (!ui_manager->init(window_size)) {
        ENGINE_LOG_ERROR("UIManager 初始化失败");
        return false;
    }

    auto hud_panel = std::make_unique<engine::ui::UIPanel>(context_);
    hud_panel->setPosition({ window_size.x * 0.5f - 220.0f, 12.0f });
    hud_panel->setSize({ 440.0f, 72.0f });
    hud_panel->setBackgroundColor({ 0.0f, 0.0f, 0.0f, 0.0f });
    hud_panel->setBorderColor({ 0.0f, 0.0f, 0.0f, 0.0f });
    hud_panel->setBorderWidth(0.0f);
    auto* hud_panel_ptr = hud_panel.get();

    auto hp_icon = std::make_unique<engine::ui::UIImage>(context_, "ui_circle", glm::vec2{ 14.0f, 14.0f }, glm::vec2{ 24.0f, 24.0f });
    hud_panel_ptr->addChild(std::move(hp_icon));

    auto gold_icon = std::make_unique<engine::ui::UIImage>(context_, "ui_weapon_icon", glm::vec2{ 14.0f, 50.0f }, glm::vec2{ 24.0f, 24.0f });
    hud_panel_ptr->addChild(std::move(gold_icon));

    auto health_bar_layer = std::make_unique<engine::ui::UIPanel>(context_);
    health_bar_layer->setPosition({ 0.0f, 0.0f });
    health_bar_layer->setSize(window_size);
    health_bar_layer->setBackgroundColor({ 0.0f, 0.0f, 0.0f, 0.0f });
    health_bar_layer_ = health_bar_layer.get();
    ui_manager->addElement(std::move(health_bar_layer));

    auto pause_button = std::make_unique<engine::ui::UIButton>(context_, "Pause", font_path_, 24);
    pause_button->setPosition({ window_size.x - 160.0f, 18.0f });
    pause_button->setSize({ 140.0f, 40.0f });
    pause_button->setClickCallback([this]() { togglePause(); });
    pause_button_ = pause_button.get();
    ui_manager->addElement(std::move(pause_button));

    auto hud_text = std::make_unique<engine::ui::UIText>(context_, "", font_path_, 20);
    hud_text->setPosition({ 12.0f, 6.0f });
    hud_text_ = hud_text.get();
    hud_text_->setColor({ 1.0f, 1.0f, 1.0f, 1.0f });
    hud_panel_ptr->addChild(std::move(hud_text));

    auto gold_text = std::make_unique<engine::ui::UIText>(context_, "", font_path_, 20);
    gold_text->setPosition({ 12.0f, 34.0f });
    gold_text_ = gold_text.get();
    gold_text_->setColor({ 1.0f, 0.92f, 0.45f, 1.0f });
    hud_panel_ptr->addChild(std::move(gold_text));

    ui_manager->addElement(std::move(hud_panel));

    createUnitsPortraitUI();

    auto wave_banner = std::make_unique<engine::ui::UIText>(context_, "", font_path_, 26);
    wave_banner->setAlignment(engine::ui::TextAlignment::CENTER);
    wave_banner->setPosition({ window_size.x * 0.5f, 72.0f });
    wave_banner_text_ = wave_banner.get();
    wave_banner_text_->setColor({ 1.0f, 0.95f, 0.65f, 1.0f });
    wave_banner_text_->setVisible(false);
    ui_manager->addElement(std::move(wave_banner));

    auto pause_overlay = std::make_unique<engine::ui::UIPanel>(context_);
    pause_overlay->setPosition({ window_size.x * 0.5f - 150.0f, window_size.y * 0.5f - 90.0f });
    pause_overlay->setSize({ 300.0f, 180.0f });
    pause_overlay->setBackgroundColor({ 0.0f, 0.0f, 0.0f, 0.0f });
    pause_overlay->setVisible(false);
    pause_overlay_ = pause_overlay.get();
    ui_manager->addElement(std::move(pause_overlay));

    auto pause_frame = std::make_unique<engine::ui::UIImage>(context_, "ui_frame", glm::vec2{ 0.0f, 0.0f }, glm::vec2{ 300.0f, 180.0f });
    pause_overlay_->addChild(std::move(pause_frame));

    auto pause_banner = std::make_unique<engine::ui::UIImage>(context_, "ui_title", glm::vec2{ 22.0f, 10.0f }, glm::vec2{ 256.0f, 40.0f });
    pause_overlay_->addChild(std::move(pause_banner));

    auto pause_title = std::make_unique<engine::ui::UIText>(context_, "Paused", font_path_, 32);
    pause_title->setAlignment(engine::ui::TextAlignment::CENTER);
    pause_title->setPosition({ 150.0f, 28.0f });
    pause_title_text_ = pause_title.get();
    pause_title_text_->setColor({ 1.0f, 1.0f, 1.0f, 1.0f });
    pause_overlay_->addChild(std::move(pause_title));

    auto resume_button = std::make_unique<engine::ui::UIButton>(context_, "Resume", font_path_, 24);
    resume_button->setPosition({ 70.0f, 74.0f });
    resume_button->setSize({ 160.0f, 36.0f });
    resume_button->setClickCallback([this]() { togglePause(); });
    resume_button_ = resume_button.get();
    pause_overlay_->addChild(std::move(resume_button));

    auto quit_button = std::make_unique<engine::ui::UIButton>(context_, "Quit", font_path_, 24);
    quit_button->setPosition({ 70.0f, 116.0f });
    quit_button->setSize({ 160.0f, 36.0f });
    quit_button->setClickCallback([this]() { quit(); });
    quit_button_ = quit_button.get();
    pause_overlay_->addChild(std::move(quit_button));

    refreshHudText();
    return true;
}

void GameScene::createUnitsPortraitUI()
{
    if (!session_data_ || !ui_config_ || !entity_factory_) {
        return;
    }

    const auto& layout = ui_config_->getUnitPanelLayout();
    const auto window_size = context_.getGameState().getWindowLogicalSize();

    if (registry_.ctx().contains<game::data::SelectionState>()) {
        registry_.ctx().get<game::data::SelectionState>().hovered_portrait_name_id_ = 0;
    }

    if (!unit_panel_) {
        auto unit_panel = std::make_unique<engine::ui::UIPanel>(context_);
        unit_panel->setBackgroundColor({ 0.0f, 0.0f, 0.0f, 0.0f });
        unit_panel_ = unit_panel.get();
        if (auto* ui_manager = getUIManager()) {
            ui_manager->addElement(std::move(unit_panel));
        }
    }
    const float previous_scroll_x = unit_panel_scroll_x_;
    unit_panel_content_width_ = 0.0f;
    game::ui::rebuildUnitsPortraitUI(
        *unit_panel_,
        context_,
        *session_data_,
        *ui_config_,
        *entity_factory_,
        game_stats_,
        hidden_unit_portrait_ids_,
        unit_panel_scroll_x_,
        &unit_panel_content_width_,
        [this](const game::data::UnitData& unit, int cost) {
            selected_unit_id_ = unit.name_id_;
            selected_unit_name_ = unit.name_;
            ENGINE_LOG_INFO("选择角色肖像: {} ({})", unit.name_, unit.class_name_);
            context_.getDispatcher().enqueue(game::defs::PrepUnitEvent{ unit.name_id_, unit.class_id_, cost, unit.level_, unit.rarity_ });
        });

    unit_panel_max_scroll_x_ = std::max(0.0f, unit_panel_content_width_ - (window_size.x - static_cast<float>(layout.padding_) * 2.0f));
    unit_panel_scroll_x_ = std::clamp(unit_panel_scroll_x_, 0.0f, unit_panel_max_scroll_x_);
    if (unit_panel_scroll_x_ != previous_scroll_x) {
        unit_panel_content_width_ = 0.0f;
        game::ui::rebuildUnitsPortraitUI(
            *unit_panel_,
            context_,
            *session_data_,
            *ui_config_,
            *entity_factory_,
            game_stats_,
            hidden_unit_portrait_ids_,
            unit_panel_scroll_x_,
            &unit_panel_content_width_,
            [this](const game::data::UnitData& unit, int cost) {
                selected_unit_id_ = unit.name_id_;
                selected_unit_name_ = unit.name_;
                ENGINE_LOG_INFO("选择角色肖像: {} ({})", unit.name_, unit.class_name_);
                context_.getDispatcher().enqueue(game::defs::PrepUnitEvent{ unit.name_id_, unit.class_id_, cost, unit.level_, unit.rarity_ });
            });
    }
}

void GameScene::updateHealthBars()
{
    if (!health_bar_layer_) {
        return;
    }

    std::vector<entt::id_type> alive_entities;
    alive_entities.reserve(registry_.storage<entt::entity>().size());

    registry_.view<game::component::StatsComponent, engine::component::TransformComponent>().each([&](auto entity, const auto& stats, const auto& transform) {
        const auto entity_id = static_cast<entt::id_type>(entity);
        alive_entities.push_back(entity_id);

        auto [it, inserted] = health_bar_widgets_.try_emplace(entity_id);
        auto& widget = it->second;
        if (inserted || !widget.container_) {
            auto container = std::unique_ptr<engine::ui::UIPanel>(new engine::ui::UIPanel(context_));
            container->setId(entity_id);
            container->setSize({ 48.0f, 6.0f });
            container->setBackgroundColor({ 0.08f, 0.08f, 0.08f, 0.95f });
            container->setBorderColor({ 0.0f, 0.0f, 0.0f, 0.85f });
            container->setBorderWidth(1.0f);

            auto fill = std::unique_ptr<engine::ui::UIPanel>(new engine::ui::UIPanel(context_));
            fill->setPosition({ 1.0f, 1.0f });
            fill->setSize({ 46.0f, 4.0f });
            fill->setBackgroundColor({ 0.2f, 0.8f, 0.2f, 1.0f });
            widget.fill_ = fill.get();
            container->addChild(std::move(fill));

            widget.container_ = container.get();
            health_bar_layer_->addChild(std::move(container));
        }

        const float hp_ratio = stats.max_hp_ > 0.0f ? std::clamp(stats.hp_ / stats.max_hp_, 0.0f, 1.0f) : 0.0f;
        const glm::vec2 screen_pos = context_.getCamera().worldToScreen(transform.position_ + glm::vec2(-24.0f, 28.0f));
        widget.container_->setPosition(screen_pos);

        if (widget.fill_) {
            widget.fill_->setSize({ 46.0f * hp_ratio, 4.0f });
            if (hp_ratio > 0.66f) {
                widget.fill_->setBackgroundColor({ 0.18f, 0.78f, 0.18f, 1.0f });
            } else if (hp_ratio > 0.33f) {
                widget.fill_->setBackgroundColor({ 0.92f, 0.72f, 0.12f, 1.0f });
            } else {
                widget.fill_->setBackgroundColor({ 0.92f, 0.18f, 0.18f, 1.0f });
            }
        }

        widget.container_->setVisible(true);
    });

    std::vector<entt::id_type> to_remove;
    for (const auto& [entity_id, widget] : health_bar_widgets_) {
        if (std::find(alive_entities.begin(), alive_entities.end(), entity_id) == alive_entities.end()) {
            if (health_bar_layer_) {
                health_bar_layer_->removeChildById(entity_id);
            }
            to_remove.push_back(entity_id);
        }
    }

    for (auto entity_id : to_remove) {
        health_bar_widgets_.erase(entity_id);
    }
}

void GameScene::updateUi(float delta_time) {
    updateHealthBars();

    const auto wheel_delta = context_.getInputManager().getMouseWheelDelta();
    if (unit_panel_ && wheel_delta.y != 0.0f && unit_panel_->containsPoint(context_.getInputManager().getLogicalMousePosition())) {
        const float next_scroll = std::clamp(unit_panel_scroll_x_ - wheel_delta.y * 28.0f, 0.0f, unit_panel_max_scroll_x_);
        if (next_scroll != unit_panel_scroll_x_) {
            unit_panel_scroll_x_ = next_scroll;
            createUnitsPortraitUI();
        }
    }

    const int current_unit_panel_cost = static_cast<int>(game_stats_.cost_);
    if (unit_panel_ && current_unit_panel_cost != last_unit_panel_cost_) {
        last_unit_panel_cost_ = current_unit_panel_cost;
        createUnitsPortraitUI();
    }

    refreshHudText();

    if (wave_banner_text_ && wave_banner_text_->isVisible()) {
        if (wave_banner_timer_ > 0.0f) {
            wave_banner_timer_ -= delta_time;
        }
        if (wave_banner_timer_ <= 0.0f) {
            wave_banner_text_->setVisible(false);
        }
    }
}

void GameScene::refreshHudText() {
    if (!hud_text_) {
        return;
    }

    std::string state_text = "Playing";
    if (context_.getGameState().isPaused()) {
        state_text = "Paused";
    } else if (context_.getGameState().isGameOver()) {
        state_text = "Game Over";
    }

    hud_text_->setText(
        current_level_name_ +
        " | " +
        "Wave " + std::to_string(current_wave_) +
        " | HP " + std::to_string(game_stats_.home_hp_) +
        " | " + state_text +
        (selected_unit_name_.empty() ? std::string{} : (" | Selected " + selected_unit_name_))
    );

    if (gold_text_) {
        gold_text_->setText("Gold " + std::to_string(static_cast<int>(game_stats_.cost_)));
    }
}

void GameScene::setPauseOverlayVisible(bool visible) {
    if (pause_overlay_) {
        pause_overlay_->setVisible(visible);
    }
    if (pause_button_) {
        pause_button_->setVisible(!visible);
    }
}

bool GameScene::togglePause() {
    if (context_.getGameState().isGameOver()) {
        return false;
    }

    const bool should_pause = !context_.getGameState().isPaused();
    context_.getGameState().setState(should_pause ? engine::core::GameStateType::Paused
                                                   : engine::core::GameStateType::Playing);
    setPauseOverlayVisible(should_pause);
    ENGINE_LOG_INFO("切换游戏状态为 {}", should_pause ? "Paused" : "Playing");
    return true;
}

// --- 事件回调函数 ---
void GameScene::onEnemyArriveHome(const game::defs::EnemyArriveHomeEvent&) {
    if (base_hp_ <= 0) {
        return;
    }

    --base_hp_;
    ENGINE_LOG_WARN("敌人到达基地，基地生命值: {}", base_hp_);

    if (base_hp_ <= 0) {
        context_.getResourceManager().stopMusic();
        context_.getResourceManager().playMusic(entt::hashed_string("lose").value());
        context_.getGameState().setState(engine::core::GameStateType::GameOver);
        ENGINE_LOG_WARN("基地生命耗尽，游戏结束");
    }
}

void GameScene::onSpawnProjectileVisual(const game::defs::SpawnProjectileVisualEvent& event) {
    if (!entity_factory_ || !registry_.valid(event.source_entity_) || !registry_.valid(event.target_entity_)) {
        return;
    }

    const auto* source_transform = registry_.try_get<engine::component::TransformComponent>(event.source_entity_);
    const auto* target_transform = registry_.try_get<engine::component::TransformComponent>(event.target_entity_);
    if (!source_transform || !target_transform) {
        return;
    }

    const auto* target_velocity = registry_.try_get<engine::component::VelocityComponent>(event.target_entity_);
    const glm::vec2 target_velocity_value = target_velocity ? target_velocity->velocity_ : glm::vec2{ 0.0f, 0.0f };
    entity_factory_->createProjectileVisual(event.projectile_id_, source_transform->position_, target_transform->position_, target_velocity_value);
}

void GameScene::onSpawnEffectVisual(const game::defs::SpawnEffectVisualEvent& event) {
    if (!entity_factory_ || !registry_.valid(event.target_entity_)) {
        return;
    }

    const auto* target_transform = registry_.try_get<engine::component::TransformComponent>(event.target_entity_);
    if (!target_transform) {
        return;
    }

    entity_factory_->createEffectVisual(event.effect_id_, target_transform->position_);
}

void GameScene::onRemoveUIPortrait(const game::defs::RemoveUIPortraitEvent& event) {
    if (std::find(hidden_unit_portrait_ids_.begin(), hidden_unit_portrait_ids_.end(), event.name_id_) == hidden_unit_portrait_ids_.end()) {
        hidden_unit_portrait_ids_.push_back(event.name_id_);
    }
    selected_unit_id_ = 0;
    selected_unit_name_.clear();
    if (unit_panel_) {
        createUnitsPortraitUI();
    }
}

void GameScene::onUIPortraitHoverEnter(const game::defs::UIPortraitHoverEnterEvent& event) {
    if (registry_.ctx().contains<game::data::SelectionState>()) {
        registry_.ctx().get<game::data::SelectionState>().hovered_portrait_name_id_ = event.name_id_;
    }
}

void GameScene::onUIPortraitHoverLeave(const game::defs::UIPortraitHoverLeaveEvent&) {
    if (registry_.ctx().contains<game::data::SelectionState>()) {
        registry_.ctx().get<game::data::SelectionState>().hovered_portrait_name_id_ = 0;
    }
}

void GameScene::onRestartRequested(const game::defs::RestartEvent&) {
    ENGINE_LOG_INFO("收到重开请求，重新构建 GameScene");
    requestReplaceScene(std::unique_ptr<engine::scene::Scene>(new game::scene::GameScene(context_, selected_level_index_)));
}

void GameScene::onBackToTitleRequested(const game::defs::BackToTitleEvent&) {
    ENGINE_LOG_INFO("收到回标题请求");
    requestReplaceScene(std::unique_ptr<engine::scene::Scene>(new game::scene::TitleScene(context_)));
}

void GameScene::onSaveRequested(const game::defs::SaveEvent&) {
    if (!session_data_) {
        return;
    }

    nlohmann::json json_data;
    json_data["level"] = static_cast<int>(selected_level_index_) + 1;
    json_data["point"] = static_cast<int>(game_stats_.cost_);
    json_data["level_clear"] = false;

    auto& unit_json = json_data["unit"];
    for (const auto& [name_id, unit] : session_data_->getUnitMap()) {
        unit_json[unit.name_]["class"] = unit.class_name_;
        unit_json[unit.name_]["level"] = unit.level_;
        unit_json[unit.name_]["rarity"] = unit.rarity_;
    }

    const std::string save_path = "assets/save/SLOT_1.json";
    std::ofstream out_file(save_path);
    if (!out_file.is_open()) {
        ENGINE_LOG_ERROR("保存失败，无法写入文件: {}", save_path);
        return;
    }

    out_file << json_data.dump(4);
    ENGINE_LOG_INFO("保存完成: {}", save_path);
}

void GameScene::onLevelClearRequested(const game::defs::LevelClearEvent&) {
    ENGINE_LOG_INFO("收到通关请求，推入通关覆盖层: level={}", selected_level_index_ + 1);
    requestPushScene(std::unique_ptr<engine::scene::Scene>(new game::scene::LevelClearScene(context_, selected_level_index_)));
}

// --- 测试函数 ---
void GameScene::createTestEnemy() {
    startNextWave();
}

bool GameScene::onCreateTestPlayerMelee() {
    return tryCreateTestPlayerUnit("warrior"_hs, "战士", true);
}

bool GameScene::onCreateTestPlayerRanged() {
    return tryCreateTestPlayerUnit("archer"_hs, "弓箭手", true);
}

bool GameScene::onCreateTestPlayerHealer() {
    return tryCreateTestPlayerUnit("witch"_hs, "治疗者", false);
}

bool GameScene::tryCreateTestPlayerUnit(entt::id_type class_id, const char* log_name, bool injured) {
    if (!context_.getGameState().isPlaying() || !entity_factory_) {
        return false;
    }

    auto position = context_.getInputManager().getLogicalMousePosition();
    if (pause_button_ && pause_button_->containsPoint(position)) {
        return false;
    }

    const auto cost = getUnitCost(class_id);
    if (!trySpendGold(cost)) {
        return false;
    }

    auto entity = entity_factory_->createPlayerUnit(class_id, position);
    if (entity == entt::null) {
        game_stats_.cost_ += static_cast<float>(cost);
        ENGINE_LOG_ERROR("创建{}失败，已回退金币: {}", log_name, cost);
        return false;
    }

    if (injured) {
        if (auto* stats = registry_.try_get<game::component::StatsComponent>(entity)) {
            stats->hp_ = stats->max_hp_ / 2.0f;
            registry_.emplace<game::defs::InjuredTag>(entity);
        }
    }

    ENGINE_LOG_INFO("创建{}: 位置: {}, {}", log_name, position.x, position.y);
    return true;
}

bool GameScene::onUpgradeClosestPlayer() {
    if (!context_.getGameState().isPlaying()) {
        return false;
    }

    const auto mouse_position = context_.getInputManager().getLogicalMousePosition();

    entt::entity target = entt::null;
    float best_distance_sq = std::numeric_limits<float>::max();
    auto view = registry_.view<game::component::PlayerComponent>();
    for (auto entity : view) {
        const auto* transform = registry_.try_get<engine::component::TransformComponent>(entity);
        const auto* stats_component = registry_.try_get<game::component::StatsComponent>(entity);
        if (!transform || !stats_component) {
            continue;
        }
        const auto delta = transform->position_ - mouse_position;
        const float distance_sq = delta.x * delta.x + delta.y * delta.y;
        if (distance_sq < best_distance_sq) {
            best_distance_sq = distance_sq;
            target = entity;
        }
    }

    if (target == entt::null || best_distance_sq > game::defs::UNIT_RADIUS * game::defs::UNIT_RADIUS * 9.0f) {
        ENGINE_LOG_WARN("未找到可升级单位，鼠标位置: {}, {}", mouse_position.x, mouse_position.y);
        return false;
    }

    const auto& player = registry_.get<game::component::PlayerComponent>(target);
    context_.getDispatcher().enqueue(game::defs::UpgradeUnitEvent{ target, player.cost_ });
    return true;
}

bool GameScene::onSellClosestPlayer() {
    if (!context_.getGameState().isPlaying()) {
        return false;
    }

    const auto mouse_position = context_.getInputManager().getLogicalMousePosition();

    entt::entity target = entt::null;
    float best_distance_sq = std::numeric_limits<float>::max();
    auto view = registry_.view<engine::component::TransformComponent, game::component::PlayerComponent>();
    view.each([&](auto entity, const auto& transform, const auto&) {
        const auto delta = transform.position_ - mouse_position;
        const float distance_sq = delta.x * delta.x + delta.y * delta.y;
        if (distance_sq < best_distance_sq) {
            best_distance_sq = distance_sq;
            target = entity;
        }
    });

    if (target == entt::null || best_distance_sq > game::defs::UNIT_RADIUS * game::defs::UNIT_RADIUS * 9.0f) {
        ENGINE_LOG_WARN("未找到可出售单位，鼠标位置: {}, {}", mouse_position.x, mouse_position.y);
        return false;
    }

    const auto& player = registry_.get<game::component::PlayerComponent>(target);
    context_.getDispatcher().enqueue(game::defs::RetreatEvent{ target, player.cost_ });

    ENGINE_LOG_INFO("撤退单位已触发: entity={}, cost={}", entt::to_integral(target), player.cost_);
    return true;
}

bool GameScene::onReleaseSelectedHeroSkill() {
    if (!context_.getGameState().isPlaying()) {
        return false;
    }

    if (!registry_.ctx().contains<game::data::SelectionState>()) {
        ENGINE_LOG_WARN("技能释放热键触发失败: 选择状态不存在");
        return false;
    }

    const auto& selection = registry_.ctx().get<game::data::SelectionState>();
    const auto selected_unit = selection.selected_unit_;
    if (selected_unit == entt::null || !registry_.valid(selected_unit)) {
        ENGINE_LOG_WARN("技能释放热键触发失败: 当前没有有效选中单位");
        return false;
    }

    if (!registry_.all_of<game::component::HeroSkillComponent>(selected_unit)) {
        ENGINE_LOG_WARN("技能释放热键触发失败: 选中单位没有英雄技能组件 entity={}", entt::to_integral(selected_unit));
        return false;
    }

    context_.getDispatcher().trigger(game::defs::ReleaseHeroSkillEvent{ selected_unit });
    ENGINE_LOG_INFO("技能释放热键已触发 entity={}", entt::to_integral(selected_unit));
    return true;
}

bool GameScene::onClearAllPlayers() {
    std::vector<entt::entity> entities;
    auto view = registry_.view<game::component::PlayerComponent>();
    for (auto entity : view) {
        entities.push_back(entity);
    }

    for (auto entity : entities) {
        context_.getDispatcher().enqueue(game::defs::RemovePlayerUnitEvent{ entity });
    }
    return true;
}

void GameScene::startNextWave() {
    ++current_wave_;
    pending_wave_enemies_.clear();
    pending_wave_index_ = 0;
    wave_spawn_timer_ = 0.0f;
    wave_break_timer_ = 0.0f;
    wave_running_ = true;

    if (level_waves_.empty()) {
        wave_running_ = false;
        ENGINE_LOG_ERROR("关卡 {} 未配置 waves，无法开始第 {} 波", current_level_name_, current_wave_);
        return;
    }

    const auto wave_index = (current_wave_ - 1) % level_waves_.size();
    const auto& wave = level_waves_[wave_index];
    pending_wave_enemies_ = wave.enemy_queue_;
    wave_spawn_interval_ = std::max(0.1f, wave.spawn_interval_);
    wave_break_duration_ = std::max(0.1f, wave.next_wave_interval_);

    game_stats_.enemy_count_ += static_cast<int>(pending_wave_enemies_.size());

    ENGINE_LOG_INFO("{} 第 {} 波开始，共 {} 只敌人", current_level_name_, current_wave_, pending_wave_enemies_.size());
    if (wave_banner_text_) {
        wave_banner_text_->setText("Wave " + std::to_string(current_wave_));
        wave_banner_text_->setVisible(true);
        wave_banner_timer_ = 2.0f;
    }
    refreshHudText();
}

void GameScene::updateWaveFlow(float delta_time) {
    if (start_points_.empty() || !entity_factory_) {
        return;
    }

    if (level_waves_.empty()) {
        return;
    }

    if (!wave_running_) {
        wave_break_timer_ += delta_time;
        if (wave_break_timer_ >= wave_break_duration_ ) {
            startNextWave();
        }
        return;
    }

    if (pending_wave_index_ < pending_wave_enemies_.size()) {
        wave_spawn_timer_ += delta_time;
        if (wave_spawn_timer_ >= wave_spawn_interval_) {
            wave_spawn_timer_ = 0.0f;
            spawnNextEnemy();
        }
        return;
    }

    const auto enemy_count = registry_.view<game::component::EnemyComponent>().size();
    if (enemy_count == 0) {
        wave_running_ = false;
        wave_break_timer_ = 0.0f;
        ENGINE_LOG_INFO("第 {} 波已清空，准备下一波", current_wave_);
    }
}

void GameScene::spawnNextEnemy() {
    if (pending_wave_index_ >= pending_wave_enemies_.size() || start_points_.empty()) {
        return;
    }

    const auto class_id = pending_wave_enemies_[pending_wave_index_++];
    const auto start_index = start_points_[(current_wave_ + pending_wave_index_) % start_points_.size()];
    const auto position = waypoint_nodes_.at(start_index).position_;

    auto entity = entity_factory_->createEnemyUnit(class_id, position, start_index, enemy_level_, enemy_rarity_);
    if (entity == entt::null) {
        ENGINE_LOG_ERROR("敌人生成失败: {}", class_id);
    }
}

int GameScene::getUnitCost(entt::id_type class_id, int rarity) const {
    if (!entity_factory_) {
        return 0;
    }
    return entity_factory_->getPlayerUnitCost(class_id, rarity);
}

bool GameScene::trySpendGold(int amount) {
    if (amount <= 0) {
        return true;
    }
    if (game_stats_.cost_ < static_cast<float>(amount)) {
        ENGINE_LOG_WARN("金币不足，当前金币: {}, 需要: {}", static_cast<int>(game_stats_.cost_), amount);
        return false;
    }
    game_stats_.cost_ -= static_cast<float>(amount);
    ENGINE_LOG_INFO("消耗金币 {}，当前金币: {}", amount, static_cast<int>(game_stats_.cost_));
    return true;
}



} // namespace game::scene