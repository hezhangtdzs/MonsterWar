# Component 组件模块

Component 模块定义了游戏特有的 ECS 组件，用于描述游戏实体的属性和状态。

## 类/结构概览

| 名称 | 描述 |
|------|------|
| [EnemyComponent](#enemycomponent) | 敌人组件，存储寻路目标和移动速度 |
| [StatsComponent](#statscomponent) | 战斗属性组件，存储生命值、攻击力等实时数值 |
| [ClassNameComponent](#classnamecomponent) | 类名组件，标识实体的类型和蓝图来源 |

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
