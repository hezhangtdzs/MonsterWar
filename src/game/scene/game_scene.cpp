#include "game_scene.h"
#include "../component/enemy_component.h"
#include "../loader/entity_builder_mw.h"
#include "../system/followpath_system.h"
#include "../system/remove_dead_system.h"
#include "../../engine/component/transform_component.h"
#include "../../engine/component/velocity_component.h"
#include "../../engine/component/sprite_component.h"
#include "../../engine/component/render_component.h"
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

    // 初始化系统
    render_system_ = std::make_unique<engine::system::RenderSystem>();
    movement_system_ = std::make_unique<engine::system::MovementSystem>();
    animation_system_ = std::make_unique<engine::system::AnimationSystem>();
    ysort_system_ = std::make_unique<engine::system::YSortSystem>();

    follow_path_system_ = std::make_unique<game::system::FollowPathSystem>();
    remove_dead_system_ = std::make_unique<game::system::RemoveDeadSystem>();

    spdlog::info("GameScene 构造完成");
}

bool GameScene::initEntityFactory() {
    // 创建蓝图管理器并加载敌人蓝图
    blueprint_manager_ = std::make_shared<game::factory::BlueprintManager>(context_.getResourceManager());
    if (!blueprint_manager_->loadEnemyClassBlueprints("assets/data/enemy_data.json")) {
        spdlog::error("加载敌人蓝图失败");
        return false;
    }

    // 创建实体工厂
    entity_factory_ = std::make_unique<game::factory::EntityFactory>(registry_, *blueprint_manager_);
    spdlog::info("实体工厂初始化完成");
    return true;
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
    movement_system_->update(registry_, delta_time);
    animation_system_->update(registry_, delta_time);
    ysort_system_->update(registry_);   // 调用顺序要在MovementSystem之后
    Scene::update(delta_time);
}

void GameScene::render() {
    render_system_->update(registry_, context_.getRenderer(), context_.getCamera());

    Scene::render();
}

void GameScene::clean() {
    auto& dispatcher = context_.getDispatcher();
    dispatcher.disconnect(this);
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
    // 断开所有事件连接
    dispatcher.sink<game::defs::EnemyArriveHomeEvent>().connect<&GameScene::onEnemyArriveHome>(this);
    return true;
}

// --- 事件回调函数 ---
void GameScene::onEnemyArriveHome(const game::defs::EnemyArriveHomeEvent&) {
    spdlog::info("敌人到达基地");
    // TODO: 添加敌人到达基地的逻辑
}

// --- 测试函数 ---
void GameScene::createTestEnemy() {
    if (!entity_factory_) {
        spdlog::error("实体工厂未初始化");
        return;
    }

    // 每个起点创建不同类型的敌人
    for (size_t i = 0; i < start_points_.size(); ++i) {
        auto start_index = start_points_[i];
        auto position = waypoint_nodes_[start_index].position_;

    
    
                entity_factory_->createEnemyUnit("wolf"_hs, position, start_index);
 
                entity_factory_->createEnemyUnit("slime"_hs, position, start_index);
  
                entity_factory_->createEnemyUnit("goblin"_hs, position, start_index);

                entity_factory_->createEnemyUnit("dark_witch"_hs, position, start_index);
    }
    }

} // namespace game::scene