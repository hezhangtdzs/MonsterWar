#pragma once
#include "../data/waypoint_node.h"
#include "../defs/event.h"
#include "../system/fwd.h"
#include "../factory/blueprint_manager.h"
#include "../factory/entity_factory.h"
#include "../../engine/scene/scene.h"
#include "../../engine/system/fwd.h"
#include <memory>
#include <unordered_map>
#include <vector>

namespace game::scene {

class GameScene final: public engine::scene::Scene {
private:
    std::unique_ptr<engine::system::RenderSystem> render_system_;
    std::unique_ptr<engine::system::MovementSystem> movement_system_;
    std::unique_ptr<engine::system::AnimationSystem> animation_system_;
    std::unique_ptr<engine::system::YSortSystem> ysort_system_;

    std::unique_ptr<game::system::FollowPathSystem> follow_path_system_;
    std::unique_ptr<game::system::RemoveDeadSystem> remove_dead_system_;

    std::unordered_map<int, game::data::WaypointNode> waypoint_nodes_;  // 路径节点ID到节点数据的映射
    std::vector<int> start_points_;                                     // 起点ID列表

    // 蓝图管理器和实体工厂
    std::shared_ptr<game::factory::BlueprintManager> blueprint_manager_;
    std::unique_ptr<game::factory::EntityFactory> entity_factory_;

public:
    GameScene(engine::core::Context& context);
    ~GameScene();

    void init() override;
    void update(float delta_time) override;
    void render() override;
    void clean() override;

private:
    [[nodiscard]] bool loadLevel();
    [[nodiscard]] bool initEventConnections();
    [[nodiscard]] bool initEntityFactory();

    // 事件回调函数
    void onEnemyArriveHome(const game::defs::EnemyArriveHomeEvent& event);

    // 测试函数
    void createTestEnemy();

};

} // namespace game::scene