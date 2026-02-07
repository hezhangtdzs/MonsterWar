# Scene 场景模块

Scene 模块包含游戏的场景实现，负责管理游戏状态、系统更新和关卡加载。

## 类/结构概览

| 名称 | 描述 |
|------|------|
| [GameScene](#gamescene) | 游戏主场景，管理塔防游戏的核心逻辑 |

---

## GameScene

**文件**: `src/game/scene/game_scene.h`, `src/game/scene/game_scene.cpp`

游戏主场景，继承自引擎的 `Scene` 类。负责初始化游戏系统、加载关卡、更新游戏逻辑。

### 类定义

```cpp
class GameScene final: public engine::scene::Scene {
private:
    // 引擎系统
    std::unique_ptr<engine::system::RenderSystem> render_system_;
    std::unique_ptr<engine::system::MovementSystem> movement_system_;
    std::unique_ptr<engine::system::AnimationSystem> animation_system_;
    std::unique_ptr<engine::system::YSortSystem> ysort_system_;
    
    // 游戏系统
    std::unique_ptr<game::system::FollowPathSystem> follow_path_system_;
    std::unique_ptr<game::system::RemoveDeadSystem> remove_dead_system_;
    
    // 路径数据
    std::unordered_map<int, game::data::WaypointNode> waypoint_nodes_;
    std::vector<int> start_points_;
    
public:
    void init() override;
    void update(float delta_time) override;
    void render() override;
    void clean() override;
};
```

### 核心数据

| 成员 | 类型 | 描述 |
|------|------|------|
| `waypoint_nodes_` | `unordered_map<int, WaypointNode>` | 路径点图，ID到节点的映射 |
| `start_points_` | `vector<int>` | 出怪口ID列表，敌人从这里生成 |
| `follow_path_system_` | `FollowPathSystem` | 寻路系统，控制敌人沿路径移动 |
| `remove_dead_system_` | `RemoveDeadSystem` | 清理系统，删除标记死亡的实体 |

### 生命周期

```
构造函数
    ↓
init()          - 初始化所有系统，加载关卡
    ↓
update()        - 每帧更新：系统更新、事件处理
    ↓
render()        - 渲染场景
    ↓
clean()         - 清理资源，断开事件连接
    ↓
析构函数
```

### 关卡加载流程

```cpp
bool GameScene::loadLevel() {
    // 1. 创建 LevelLoader
    engine::loader::LevelLoader level_loader;
    
    // 2. 设置自定义实体构建器（解析路径点）
    level_loader.setEntityBuilder(
        std::make_unique<game::loader::EntityBuilderMW>(
            level_loader, context_, registry_,
            waypoint_nodes_, start_points_
        )
    );
    
    // 3. 加载关卡（自动解析路径点）
    if (!level_loader.loadLevel("assets/maps/level1.tmj", this)) {
        return false;
    }
    
    return true;
}
```

### 系统更新顺序

```cpp
void GameScene::update(float delta_time) {
    // 1. 寻路系统（计算敌人移动方向）
    follow_path_system_->update(registry_, dispatcher, waypoint_nodes_);
    
    // 2. 移动系统（更新位置）
    movement_system_->update(registry_, delta_time);
    
    // 3. 动画系统
    animation_system_->update(registry_, delta_time);
    
    // 4. Y排序系统
    ysort_system_->update(registry_);
    
    // 5. 清理系统（删除死亡实体）
    remove_dead_system_->update(registry_);
    
    // 6. 父类更新
    Scene::update(delta_time);
}
```

### 事件处理

```cpp
// 订阅敌人到达基地事件
bool GameScene::initEventConnections() {
    auto& dispatcher = context_.getDispatcher();
    dispatcher.sink<game::defs::EnemyArriveHomeEvent>()
        .connect<&GameScene::onEnemyArriveHome>(this);
    return true;
}

// 事件回调
void GameScene::onEnemyArriveHome(const game::defs::EnemyArriveHomeEvent& event) {
    // 扣减生命值或统计逃脱敌人
    spdlog::info("敌人到达基地！");
}
```

### 相关模块

- [EntityBuilderMW](../loader/README.md#entitybuildermw) - 关卡加载时解析路径点
- [FollowPathSystem](../system/README.md#followpathsystem) - 敌人寻路系统
- [RemoveDeadSystem](../system/README.md#removedeadsystem) - 实体清理系统
