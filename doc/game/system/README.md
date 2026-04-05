# System 系统模块

> **相关文档**: [组件模块](../component/README.md) | [ECS 架构](../../ECS_ARCHITECTURE.md)

System 模块包含游戏的核心逻辑系统，负责处理寻路、实体清理等游戏机制。

---

## 目录

- [类/结构概览](#类结构概览)
- [BlockSystem](#blocksystem)
- [SetTargetSystem](#settargetsystem)
- [TimerSystem](#timersystem)
- [AttackStarterSystem](#attackstartersystem)
- [AnimationStateSystem](#animationstatesystem)
- [OrientationSystem](#orientationsystem)
- [FollowPathSystem](#followpathsystem)
- [RemoveDeadSystem](#removedeadsystem)
- [系统执行顺序](#系统执行顺序)

---

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

---

## 战斗系统流程

游戏中的战斗逻辑由多个系统协作完成，形成完整的战斗循环：

### 战斗系统架构

```mermaid
flowchart TB
    subgraph 目标锁定阶段
        A[SetTargetSystem] --> B{有目标?}
        B -->|是| C[添加 TargetComponent]
        B -->|否| D[无操作]
    end
    
    subgraph 冷却阶段
        E[TimerSystem] --> F{冷却结束?}
        F -->|是| G[添加 AttackReadyTag]
        F -->|否| H[继续计时]
    end
    
    subgraph 攻击触发阶段
        I[AttackStarterSystem] --> J{有目标且就绪?}
        J -->|是| K[发送攻击动画事件]
        K --> L[添加 ActionLockTag]
        L --> M[重置冷却]
    end
    
    subgraph 动画阶段
        N[AnimationSystem] --> O[播放攻击动画]
        O --> P[AnimationFinishedEvent]
    end
    
    subgraph 收尾阶段
        Q[AnimationStateSystem] --> R[移除 ActionLockTag]
        R --> S[切换回 idle/walk]
    end
    
    C --> E
    G --> I
    L --> N
    P --> Q
    S --> E
```

### 系统执行顺序

```cpp
void GameScene::update(float delta_time) {
    // 1. 目标锁定
    set_target_system_->update(registry_);
    
    // 2. 冷却计时
    timer_system_->update(registry_, delta_time);
    
    // 3. 攻击触发
    attack_starter_system_->update(registry_, dispatcher_);
    
    // 4. 阻挡处理
    block_system_->update(registry_);
    
    // 5. 寻路移动
    follow_path_system_->update(registry_, dispatcher_, waypoint_nodes_);
    
    // 6. 朝向更新
    orientation_system_->update(registry_);
    
    // 7. 动画更新
    animation_system_->update(delta_time);
    
    // 8. 基础移动
    movement_system_->update(registry_, delta_time);
    
    // 9. 清理死亡实体
    remove_dead_system_->update(registry_);
}
```

### 组件与标签协作

```mermaid
graph LR
    subgraph 攻击就绪状态
        A[AttackReadyTag] --> B[可以攻击]
    end
    
    subgraph 动作锁定状态
        C[ActionLockTag] --> D[无法移动/切换动作]
    end
    
    subgraph 目标锁定
        E[TargetComponent] --> F[有攻击目标]
    end
    
    subgraph 受伤状态
        G[InjuredTag] --> H[血量不满]
    end
    
    subgraph 死亡状态
        I[DeadTag] --> J[等待删除]
    end
```

---

## BlockSystem 详细说明

**文件**: `src/game/system/block_system.h`, `src/game/system/block_system.cpp`

### 功能说明

处理近战玩家单位（Blocker）对敌人的拦截。当敌人进入阻挡范围时建立阻挡关系。

### 阻挡流程

```mermaid
sequenceDiagram
    participant B as Blocker
    participant E as Enemy
    participant R as Registry
    
    Note over B: BlockerComponent.max_block_count > 0
    Note over E: 敌人进入阻挡范围
    
    B->>R: 检查 blocked_entities_.size() < max_block_count
    R-->>B: 可以阻挡
    B->>E: 添加 BlockedByComponent
    B->>R: 将 enemy 添加到 blocked_entities_
    E->>E: 停止移动（速度设为 0）
    
    Note over E: 敌人被阻挡，开始攻击 Blocker
```

### 类定义

```cpp
class BlockSystem {
public:
    void update(entt::registry& registry, entt::dispatcher& dispatcher);
    
private:
    static constexpr float BLOCK_RADIUS = 30.0f;  // 阻挡范围
};
```

### 核心逻辑

```cpp
void BlockSystem::update(entt::registry& registry, entt::dispatcher& dispatcher) {
    auto blockers = registry.view<BlockerComponent, TransformComponent>();
    auto enemies = registry.view<EnemyComponent, TransformComponent, VelocityComponent>();
    
    for (auto blocker : blockers) {
        auto& blocker_comp = blockers.get<BlockerComponent>(blocker);
        auto& blocker_pos = blockers.get<TransformComponent>(blocker);
        
        // 检查是否还能阻挡更多敌人
        if (blocker_comp.blocked_entities_.size() >= blocker_comp.max_block_count_) {
            continue;
        }
        
        for (auto enemy : enemies) {
            // 跳过已被阻挡的敌人
            if (registry.all_of<BlockedByComponent>(enemy)) continue;
            
            auto& enemy_pos = enemies.get<TransformComponent>(enemy);
            float distance = glm::length(enemy_pos.position_ - blocker_pos.position_);
            
            if (distance < BLOCK_RADIUS) {
                // 建立阻挡关系
                registry.emplace<BlockedByComponent>(enemy, blocker);
                blocker_comp.blocked_entities_.push_back(enemy);
                
                // 停止敌人移动
                enemies.get<VelocityComponent>(enemy).velocity_ = glm::vec2(0.0f);
            }
        }
    }
}
```

---

## SetTargetSystem 详细说明

**文件**: `src/game/system/set_target_system.h`, `src/game/system/set_target_system.cpp`

### 功能说明

统一处理所有单位的目标锁定逻辑，支持攻击目标和治疗目标两种模式。

### 类定义

```cpp
class SetTargetSystem {
public:
    void update(entt::registry& registry);

private:
    void updateHasTarget(entt::registry& registry);
    void updateNoTargetPlayer(entt::registry& registry);
    void updateNoTargetEnemy(entt::registry& registry);
    void updateHealer(entt::registry& registry);
};
```

### 私有方法说明

| 方法 | 描述 |
|------|------|
| `updateHasTarget()` | 处理已有目标的逻辑，校验距离和存活状态 |
| `updateNoTargetPlayer()` | 为没有目标的玩家单位寻找敌人 |
| `updateNoTargetEnemy()` | 为没有目标的敌方远程单位寻找射程内的目标 |
| `updateHealer()` | 为没有目标的治疗单位寻找受伤最重的友军 |

### 目标选择策略

```mermaid
flowchart TB
    A[SetTargetSystem] --> B{检查现有目标}
    B -->|目标死亡/超出射程| C[移除 TargetComponent]
    B -->|目标有效| D[保持目标]
    C --> E{单位类型}
    
    E -->|攻击单位| F[寻找最近敌方]
    E -->|治疗单位| G[寻找受伤友军]
    
    F --> H{射程内有敌人?}
    H -->|是| I[锁定最近敌人]
    H -->|否| J[无目标]
    
    G --> K{射程内有受伤友军?}
    K -->|是| L[锁定血量百分比最低的友军]
    K -->|否| J
```

### 核心逻辑

```cpp
void SetTargetSystem::update(entt::registry& registry) {
    // 处理攻击单位
    auto attackers = registry.view<StatsComponent, TransformComponent>(entt::exclude<HealerTag>);
    
    for (auto entity : attackers) {
        auto& stats = registry.get<StatsComponent>(entity);
        auto& pos = registry.get<TransformComponent>(entity);
        
        // 检查现有目标有效性
        if (registry.all_of<TargetComponent>(entity)) {
            auto& target = registry.get<TargetComponent>(entity);
            if (!isTargetValid(registry, entity, target.entity_, stats.range_)) {
                registry.remove<TargetComponent>(entity);
            }
        }
        
        // 寻找新目标
        if (!registry.all_of<TargetComponent>(entity)) {
            auto new_target = findNearestEnemy(registry, pos.position_, stats.range_);
            if (new_target != entt::null) {
                registry.emplace<TargetComponent>(entity, new_target);
            }
        }
    }
    
    // 处理治疗单位（类似逻辑）
    // ...
}
```

---

## OrientationSystem 详细说明

**文件**: `src/game/system/orientation_system.h`, `src/game/system/orientation_system.cpp`

### 功能说明

统一管理实体的翻转状态，确保实体面向正确的方向。

### 朝向判断优先级

```mermaid
flowchart TB
    A[OrientationSystem] --> B{有 TargetComponent?}
    B -->|是| C[面向目标]
    B -->|否| D{有 BlockedByComponent?}
    
    D -->|是| E[面向阻挡者]
    D -->|否| F{速度非零?}
    
    F -->|是| G[面向移动方向]
    F -->|否| H[保持当前朝向]
    
    C --> I[更新 SpriteComponent.is_flipped_]
    E --> I
    G --> I
```

### 核心逻辑

```cpp
void OrientationSystem::update(entt::registry& registry) {
    auto view = registry.view<TransformComponent, SpriteComponent>();
    
    for (auto entity : view) {
        auto& transform = view.get<TransformComponent>(entity);
        auto& sprite = view.get<SpriteComponent>(entity);
        
        bool should_flip = false;
        
        // 优先级 1: 面向目标
        if (registry.all_of<TargetComponent>(entity)) {
            auto& target = registry.get<TargetComponent>(entity);
            if (registry.valid(target.entity_)) {
                auto& target_pos = registry.get<TransformComponent>(target.entity_);
                should_flip = target_pos.position_.x < transform.position_.x;
            }
        }
        // 优先级 2: 面向阻挡者
        else if (registry.all_of<BlockedByComponent>(entity)) {
            auto& blocked = registry.get<BlockedByComponent>(entity);
            if (registry.valid(blocked.blocker_entity_)) {
                auto& blocker_pos = registry.get<TransformComponent>(blocked.blocker_entity_);
                should_flip = blocker_pos.position_.x < transform.position_.x;
            }
        }
        // 优先级 3: 面向移动方向
        else if (registry.all_of<VelocityComponent>(entity)) {
            auto& velocity = registry.get<VelocityComponent>(entity);
            if (velocity.velocity_.x != 0) {
                should_flip = velocity.velocity_.x < 0;
            }
        }
        
        sprite.sprite_.is_flipped_ = should_flip;
    }
}
```
