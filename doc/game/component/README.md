# Component 组件模块

> **版本**: 1.0.0  
> **最后更新**: 2026-02-15  
> **相关文档**: [系统模块](../system/README.md) | [工厂模块](../factory/README.md)

Component 模块定义了游戏特有的 ECS 组件，用于描述游戏实体的属性和状态。

---

## 目录

- [类/结构概览](#类结构概览)
- [TargetComponent](#targetcomponent)
- [PlayerComponent](#playercomponent)
- [BlockerComponent](#blockercomponent)
- [BlockedByComponent](#blockedbycomponent)
- [StatsComponent](#statscomponent)
- [ClassNameComponent](#classnamecomponent)
- [EnemyComponent](#enemycomponent)

---

## 类/结构概览

| 名称 | 描述 |
|------|------|
| [EnemyComponent](#enemycomponent) | 敌人组件，存储寻路目标和移动速度 |
| [StatsComponent](#statscomponent) | 战斗属性组件，存储生命值、攻击力等实时数值 |
| [ClassNameComponent](#classnamecomponent) | 类名组件，标识实体的类型和蓝图来源 |
| [TargetComponent](#targetcomponent) | 目标锁定组件，存储当前锁定的实体引用 |
| [PlayerComponent](#playercomponent) | 玩家组件，存储放置消耗等属性 |
| [BlockerComponent](#blockercomponent) | 阻挡者组件，存储最大阻挡数和当前阻挡数 |
| [BlockedByComponent](#blockedbycomponent) | 被阻挡组件，存储阻挡者的实体引用 |

---

## TargetComponent

**文件**: `src/game/component/target_component.h`

目标锁定组件，用于在实体之间建立“锁定”关系。它是自动寻敌、周期性攻击和定向治疗逻辑的基础。

```cpp
struct TargetComponent {
    entt::entity entity_{entt::null};
};
```

---

## PlayerComponent

**文件**: `src/game/component/player_component.h`

玩家组件，标记实体为玩家单位，存储放置消耗等属性。

```cpp
struct PlayerComponent {
    int cost_;  // 放置消耗（金币/资源）
};
```

### 使用示例

```cpp
// 创建玩家单位
auto player_unit = registry.create();
registry.emplace<game::component::PlayerComponent>(player_unit, 100);  // 消耗 100 金币
registry.emplace<game::component::StatsComponent>(player_unit, 100.0f, 100.0f, 20.0f, 5.0f, 50.0f, 1.0f, 0.0f, 1, 1);
```

---

## BlockerComponent

**文件**: `src/game/component/blocker_component.h`

阻挡者组件，用于近战单位。存储该单位可以同时阻挡的敌人数量上限和当前阻挡数量。

```cpp
struct BlockerComponent {
    int max_block_count_;      // 最大阻挡数
    std::vector<entt::entity> blocked_entities_;  // 当前阻挡的敌人列表
};
```

### 字段说明

| 字段 | 类型 | 描述 |
|------|------|------|
| `max_block_count_` | `int` | 最大可阻挡敌人数量 |
| `blocked_entities_` | `std::vector<entt::entity>` | 当前被阻挡的敌人实体列表 |

### 使用示例

```cpp
// 创建近战阻挡单位
auto blocker = registry.create();
registry.emplace<game::component::BlockerComponent>(blocker, 3);  // 最多阻挡 3 个敌人
```

### 相关系统

- [BlockSystem](../system/README.md#blocksystem) - 处理阻挡逻辑

---

## BlockedByComponent

**文件**: `src/game/component/blocked_by_component.h`

被阻挡组件，附加在被阻挡的敌人身上，记录阻挡者的引用。

```cpp
struct BlockedByComponent {
    entt::entity blocker_entity_{entt::null};  // 阻挡者实体
};
```

### 使用示例

```cpp
// 当敌人被阻挡时
registry.emplace<game::component::BlockedByComponent>(enemy, blocker_entity);
```

### 相关系统

- [BlockSystem](../system/README.md#blocksystem) - 添加此组件
- [OrientationSystem](../system/README.md#orientationsystem) - 根据此组件调整朝向

---

## StatsComponent

**文件**: `src/game/component/stats_component.h`

存储实体的战斗属性。在工厂创建时，这些属性会根据基础蓝图数值、等级和稀有度进行缩放。

```cpp
struct StatsComponent {
    float hp_, max_hp_;
    float atk_, def_;
    float range_, atk_interval_, atk_timer_;
    int level_, rarity_;
};
```

---

## ClassNameComponent

**文件**: `src/game/component/class_name_component.h`

标识实体的类型，通常用于调试或需要识别实体蓝图类型的系统逻辑。

```cpp
struct ClassNameComponent {
    entt::id_type class_id_;
    std::string class_name_;
};
```

---

## EnemyComponent

**文件**: `src/game/component/enemy_component.h`

敌人组件标记一个实体为敌人，并存储其寻路相关的数据。

### 结构定义

```cpp
struct EnemyComponent {
    int target_waypoint_id_;  // 当前目标路径点ID
    float speed_;             // 移动速度（像素/秒）
};
```

### 字段说明

| 字段 | 类型 | 描述 |
|------|------|------|
| `target_waypoint_id_` | `int` | 当前目标路径点的唯一标识符，敌人会向该节点移动 |
| `speed_` | `float` | 敌人的移动速度，单位是像素/秒 |

### 使用示例

```cpp
#include "game/component/enemy_component.h"

// 创建敌人实体
auto enemy = registry.create();

// 添加敌人组件，设置初始目标路径点和速度
registry.emplace<game::component::EnemyComponent>(
    enemy, 
    67,      // 初始目标路径点ID（从起点开始）
    100.0f   // 移动速度 100 像素/秒
);
```

### 相关系统

- [FollowPathSystem](../system/README.md#followpathsystem) - 使用此组件进行寻路移动
- [SpawnSystem](../system/README.md) - 创建敌人时初始化此组件
