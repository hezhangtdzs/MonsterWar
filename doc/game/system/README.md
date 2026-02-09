# System 系统模块

System 模块包含游戏的核心逻辑系统，负责处理寻路、实体清理等游戏机制。

## 类/结构概览

| 名称 | 描述 |
|------|------|
| [FollowPathSystem](#followpathsystem) | 寻路系统，控制敌人沿路径移动 |
| [RemoveDeadSystem](#removedeadsystem) | 清理系统，删除标记死亡的实体 |
| [BlockSystem](#blocksystem) | 阻挡系统，处理近战单位拦截敌人 |
| [SetTargetSystem](#settargetsystem) | 锁定系统，为单位寻找攻击或治疗目标 |
| [TimerSystem](#timersystem) | 计时系统，处理攻击冷却时间 |
| [AttackStarterSystem](#attackstartersystem) | 启动系统，触发攻击动画和动作硬直 |
| [AnimationStateSystem](#animationstatesystem) | 动画状态系统，处理动作结束后的逻辑转换 |
| [OrientationSystem](#orientationsystem) | 朝向系统，根据目标或速度调整面朝方向 |

---

## BlockSystem

**文件**: `src/game/system/block_system.h`, `src/game/system/block_system.cpp`

处理近战玩家单位（Blocker）对敌人的拦截。当距离小于 `BLOCK_RADIUS` 时建立阻挡关系。

---

## SetTargetSystem

**文件**: `src/game/system/set_target_system.h`, `src/game/system/set_target_system.cpp`

统一处理所有单位的目标锁定逻辑：
1. **有效性检测**: 目标死亡或超出射程时移除 `TargetComponent`。
2. **策略搜索**:
   - **普通攻击单位**: 在射程内寻找最近的敌方目标。
   - **治疗单位**: 通过 `InjuredTag` 寻找射程内血量百分比最低的友军。

---

## TimerSystem

**文件**: `src/game/system/timer_system.h`, `src/game/system/timer_system.cpp`

管理攻击冷却时间。累加 `atk_timer_`，并在计时结束后添加 `AttackReadyTag`，使实体进入待攻击状态。

---

## AttackStarterSystem

**文件**: `src/game/system/attack_starter_system.h`, `src/game/system/attack_starter_system.cpp`

游戏战斗循环的触发器。当单位具备 `AttackReadyTag` 且有有效目标（或被阻挡）时：
- 发送播放攻击/治疗动画事件。
- 为敌人添加 `ActionLockTag`（动作锁）。
- 重置攻击计时器并移除就绪标签。

---

## AnimationStateSystem

**文件**: `src/game/system/animation_state_system.h`, `src/game/system/animation_state_system.cpp`

监听 `AnimationFinishedEvent`。负责处理攻击等非循环动画播放结束后的收尾工作：
- 移除 `ActionLockTag`（解除硬直）。
- 切换回 `idle` 或 `walk` 动画。

---

## OrientationSystem

**文件**: `src/game/system/orientation_system.h`, `src/game/system/orientation_system.cpp`

统一管理实体的翻转状态。优先级顺序：
1. **锁定目标**: 面向当前攻击/治疗的目标。
2. **阻挡关系**: 被阻挡的敌人面向阻挡者。
3. **移动速度**: 面向当前移动的方向。

---

## FollowPathSystem

## FollowPathSystem

**文件**: `src/game/system/followpath_system.h`, `src/game/system/followpath_system.cpp`

寻路系统负责控制敌人沿着预定义的路径点移动。支持分叉路径，敌人到达节点时随机选择下一方向。

### 类定义

```cpp
class FollowPathSystem {
public:
    void update(
        entt::registry& registry,
        entt::dispatcher& dispatcher,
        std::unordered_map<int, game::data::WaypointNode>& waypoint_nodes
    );
};
```

### 工作原理

```
查询所有带 EnemyComponent 的实体
    ↓
获取当前目标路径点
    ↓
计算到目标点的方向向量
    ↓
距离 < 5.0f ?
    ├── 是 → 到达节点
    │           ├── 有下一节点？→ 随机选择下一个目标
    │           └── 无下一节点？→ 到达终点，发送事件并标记删除
    └── 否 → 继续移动
    ↓
更新速度向量（方向 × 速度）
```

### 核心算法

```cpp
void FollowPathSystem::update(...) {
    auto view = registry.view<
        EnemyComponent, 
        TransformComponent, 
        VelocityComponent>();
    
    for (auto entity : view) {
        auto& enemy = registry.get<EnemyComponent>(entity);
        auto& transform = registry.get<TransformComponent>(entity);
        
        // 1. 获取目标节点
        auto target_node = waypoint_nodes.at(enemy.target_waypoint_id_);
        
        // 2. 计算方向
        glm::vec2 direction = target_node.position_ - transform.position_;
        
        // 3. 到达检测（阈值 5.0f）
        if (glm::length(direction) < 5.0f) {
            // 到达终点？
            if (target_node.next_node_ids_.empty()) {
                dispatcher.enqueue<EnemyArriveHomeEvent>();
                registry.emplace<DeadTag>(entity);
                continue;
            }
            
            // 随机选择下一节点（支持分叉路径）
            int index = randomInt(0, target_node.next_node_ids_.size() - 1);
            enemy.target_waypoint_id_ = target_node.next_node_ids_[index];
            
            // 重新计算方向
            target_node = waypoint_nodes.at(enemy.target_waypoint_id_);
            direction = target_node.position_ - transform.position_;
        }
        
        // 4. 更新速度
        velocity.velocity_ = glm::normalize(direction) * enemy.speed_;
    }
}
```

### 设计要点

| 设计 | 说明 |
|------|------|
| **距离阈值** | 5.0f 像素，避免浮点误差和速度过快导致的震荡 |
| **随机选择** | 分叉路径时随机选择方向，实现敌人分流 |
| **延迟删除** | 到达终点时添加 DeadTag，由 RemoveDeadSystem 实际删除 |

### 使用示例

```cpp
#include "game/system/followpath_system.h"

// 创建系统
auto follow_path_system = std::make_unique<game::system::FollowPathSystem>();

// 每帧更新
follow_path_system->update(registry, dispatcher, waypoint_nodes);
```

### 相关模块

- [EnemyComponent](../component/README.md#enemycomponent) - 存储敌人的寻路目标
- [WaypointNode](../data/README.md#waypointnode) - 路径点数据结构
- [RemoveDeadSystem](#removedeadsystem) - 清理到达终点的敌人

---

## RemoveDeadSystem

**文件**: `src/game/system/remove_dead_system.h`, `src/game/system/remove_dead_system.cpp`

清理系统负责删除标记为死亡的实体。实现 ECS 中的"延迟删除"模式，确保在安全的时机销毁实体。

### 类定义

```cpp
class RemoveDeadSystem {
public:
    void update(entt::registry& registry);
};
```

### 为什么需要延迟删除？

在 ECS 系统中，遍历组件视图时直接销毁实体会破坏迭代器，导致未定义行为。因此采用"标记-删除"模式：

```
敌人到达终点
    ↓
添加 DeadTag 标记
    ↓
当前帧继续处理其他实体
    ↓
帧末调用 RemoveDeadSystem
    ↓
安全地销毁所有带 DeadTag 的实体
```

### 实现

```cpp
void RemoveDeadSystem::update(entt::registry& registry) {
    // 查询所有带 DeadTag 的实体
    auto view = registry.view<game::defs::DeadTag>();
    
    for (auto entity : view) {
        auto entity_id = static_cast<entt::id_type>(entity);
        registry.destroy(entity);
        spdlog::info("Entity {} destroyed", entity_id);
    }
}
```

### 使用场景

- 敌人到达终点需要删除
- 敌人被击败需要删除
- 子弹命中目标需要删除
- 任何需要安全删除实体的情况

### 使用示例

```cpp
#include "game/system/remove_dead_system.h"
#include "game/defs/tags.h"

// 创建系统
auto remove_dead_system = std::make_unique<game::system::RemoveDeadSystem>();

// 标记实体死亡（在 FollowPathSystem 中）
registry.emplace<game::defs::DeadTag>(enemy_entity);

// 帧末清理
remove_dead_system->update(registry);
```

### 相关模块

- [DeadTag](../defs/README.md#deadtag) - 死亡标记组件
- [FollowPathSystem](#followpathsystem) - 标记到达终点的敌人
