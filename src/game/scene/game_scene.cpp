#include "game_scene.h"
#include "../component/enemy_component.h"
#include "../component/player_component.h"
#include "../factory/entity_factory.h"
#include "../factory/blueprint_manager.h"
#include "../loader/entity_builder_mw.h"
#include "../system/followpath_system.h"
#include "../system/remove_dead_system.h"
#include "../system/block_system.h"
#include "../system/set_target_system.h"
#include "../system/timer_system.h"
#include "../system/attack_starter_system.h"
#include "../system/animation_state_system.h"
#include "../system/orientation_system.h"
#include "../defs/tags.h"
#include "../component/stats_component.h"
#include "../../engine/component/transform_component.h"
#include "../../engine/component/velocity_component.h"
#include "../../engine/component/sprite_component.h"
#include "../../engine/component/render_component.h"
#include "../../engine/input/input_manager.h"
#include "../../engine/core/context.h"
#include "../../engine/system/render_system.h"
#include "../../engine/system/movement_system.h"
#include "../../engine/system/animation_system.h"
#include "../../engine/system/ysort_system.h"
#include "../../engine/loader/level_loader.h"
#include <entt/core/hashed_string.hpp>
#include <entt/signal/sigh.hpp>
#include <spdlog/spdlog.h>

using namespace entt::literals;

namespace game::scene {

GameScene::GameScene(engine::core::Context& context)
    : engine::scene::Scene("GameScene", context) {
    auto& dispatcher = context.getDispatcher();
    // 初始化系统
    render_system_ = std::make_unique<engine::system::RenderSystem>();
    movement_system_ = std::make_unique<engine::system::MovementSystem>();
    animation_system_ = std::make_unique<engine::system::AnimationSystem>(registry_, dispatcher);
    ysort_system_ = std::make_unique<engine::system::YSortSystem>();

    follow_path_system_ = std::make_unique<game::system::FollowPathSystem>();
    remove_dead_system_ = std::make_unique<game::system::RemoveDeadSystem>();
    block_system_ = std::make_unique<game::system::BlockSystem>();
    set_target_system_ = std::make_unique<game::system::SetTargetSystem>();
    timer_system_ = std::make_unique<game::system::TimerSystem>();
    attack_starter_system_ = std::make_unique<game::system::AttackStarterSystem>();
    animation_state_system_ = std::make_unique<game::system::AnimationStateSystem>(registry_, dispatcher);
    orientation_system_ = std::make_unique<game::system::OrientationSystem>();

    spdlog::info("GameScene 构造完成");
}

GameScene::~GameScene() {
}

void GameScene::init() {
    if (!loadLevel()) {
        spdlog::error("加载关卡失败");
        return;
    }
    if (!initEventConnections()) {
        spdlog::error("初始化事件连接失败");
        return;
    }
    if (!initInputConnections()) {
        spdlog::error("初始化输入连接失败");
        return;
    }
    if (!initEntityFactory()) {
        spdlog::error("初始化实体工厂失败");
        return;
    }
    createTestEnemy();
    Scene::init();
}

void GameScene::update(float delta_time) {
    auto& dispatcher = context_.getDispatcher();

    // 每一帧最先清理死亡实体(要在dispatcher处理完事件后再清理，因此放在下一帧开头)
    remove_dead_system_->update(registry_);

    // 注意系统更新的顺序
    follow_path_system_->update(registry_, dispatcher, waypoint_nodes_);
    block_system_->update(registry_, dispatcher);
    
    // 战斗循环
    set_target_system_->update(registry_);
    timer_system_->update(registry_, delta_time);
    attack_starter_system_->update(registry_, dispatcher);

    movement_system_->update(registry_, delta_time);
    animation_system_->update(delta_time);
    
    // 视觉修正
    orientation_system_->update(registry_);
    ysort_system_->update(registry_);   // 调用顺序要在MovementSystem之后
    Scene::update(delta_time);
}

void GameScene::render() {
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
    input_manager.onAction("pause"_hs).disconnect<&GameScene::onClearAllPlayers>(this);
    Scene::clean();
}

bool GameScene::loadLevel() {
    engine::loader::LevelLoader level_loader;
    // 设置拓展的构建器EntityBuilderMW
    level_loader.setEntityBuilder(std::make_unique<game::loader::EntityBuilderMW>(level_loader, 
        context_, 
        registry_, 
        waypoint_nodes_, 
        start_points_)
    );
    if (!level_loader.loadLevel("assets/maps/level1.tmj", this)) {
        spdlog::error("加载关卡失败");
        return false;
    }
    return true;
}

bool GameScene::initEventConnections() {
    auto& dispatcher = context_.getDispatcher();
    dispatcher.sink<game::defs::EnemyArriveHomeEvent>().connect<&GameScene::onEnemyArriveHome>(this);
    return true;
}

bool GameScene::initInputConnections() {
    auto& input_manager = context_.getInputManager();
    input_manager.onAction("mouse_right"_hs).connect<&GameScene::onCreateTestPlayerMelee>(this);
    input_manager.onAction("mouse_left"_hs).connect<&GameScene::onCreateTestPlayerRanged>(this);
    input_manager.onAction("move_left"_hs).connect<&GameScene::onCreateTestPlayerHealer>(this);
    input_manager.onAction("pause"_hs).connect<&GameScene::onClearAllPlayers>(this);
    return true;
}

bool GameScene::initEntityFactory() {
    // 如果蓝图管理器为空，则创建一个（将来可能由构造函数传入）
    if (!blueprint_manager_) {  
        blueprint_manager_ = std::make_shared<game::factory::BlueprintManager>(context_.getResourceManager());
        if (!blueprint_manager_->loadEnemyClassBlueprints("assets/data/enemy_data.json") ||
            !blueprint_manager_->loadPlayerClassBlueprints("assets/data/player_data.json")) {
            spdlog::error("加载蓝图失败");
            return false;
        }
    }
    entity_factory_ = std::make_unique<game::factory::EntityFactory>(registry_, *blueprint_manager_);
    spdlog::info("entity_factory_ 加载完成");
    return true;
}

// --- 事件回调函数 ---
void GameScene::onEnemyArriveHome(const game::defs::EnemyArriveHomeEvent&) {
    spdlog::info("敌人到达基地");
    // TODO: 添加敌人到达基地的逻辑
}

// --- 测试函数 ---
void GameScene::createTestEnemy() {
    // 每个起点创建一批敌人
    for (auto start_index : start_points_) {
        auto position = waypoint_nodes_[start_index].position_;

        entity_factory_->createEnemyUnit("wolf"_hs, position, start_index);
        entity_factory_->createEnemyUnit("slime"_hs, position, start_index);
        entity_factory_->createEnemyUnit("goblin"_hs, position, start_index);
        entity_factory_->createEnemyUnit("dark_witch"_hs, position, start_index);
    }
}

bool GameScene::onCreateTestPlayerMelee() {
    auto position = context_.getInputManager().getLogicalMousePosition();
    auto entity = entity_factory_->createPlayerUnit("warrior"_hs, position);
    
    // 测试用：设为受伤并增加受伤标签
    if (auto* stats = registry_.try_get<game::component::StatsComponent>(entity)) {
        stats->hp_ = stats->max_hp_ / 2.0f;
        registry_.emplace<game::defs::InjuredTag>(entity);
    }

    spdlog::info("创建战士: 位置: {}, {}", position.x, position.y);
    return true;
}

bool GameScene::onCreateTestPlayerRanged() {
    auto position = context_.getInputManager().getLogicalMousePosition();
    auto entity = entity_factory_->createPlayerUnit("archer"_hs, position);

    // 测试用：设为受伤并增加受伤标签
    if (auto* stats = registry_.try_get<game::component::StatsComponent>(entity)) {
        stats->hp_ = stats->max_hp_ / 2.0f;
        registry_.emplace<game::defs::InjuredTag>(entity);
    }

    spdlog::info("创建弓箭手: 位置: {}, {}", position.x, position.y);
    return true;
}

bool GameScene::onCreateTestPlayerHealer() {
    auto position = context_.getInputManager().getLogicalMousePosition();
    entity_factory_->createPlayerUnit("witch"_hs, position);
    spdlog::info("创建治疗者: 位置: {}, {}", position.x, position.y);
    return true;
}

bool GameScene::onClearAllPlayers() {
    auto view = registry_.view<game::component::PlayerComponent>();
    for (auto entity : view) {
        registry_.destroy(entity);
    }
    return true;
}



} // namespace game::scene