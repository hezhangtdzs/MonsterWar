# Defs 定义模块

Defs 模块包含游戏的核心定义，包括事件结构和标签组件。这些定义是游戏逻辑的基础构建块。

## 类/结构概览

| 名称 | 类型 | 描述 |
|------|------|------|
| [EnemyArriveHomeEvent](#enemyarrivehomeevent) | 事件 | 敌人到达基地的事件 |
| [DeadTag](#deadtag) | 标签 | 标记实体死亡，用于延迟删除 |
| [FaceLeftTag](#facelefttag) | 标签 | 标记精灵默认朝左，用于翻转渲染 |
| [MeleeUnitTag](#meleeunittag) | 标签 | 标记该实体为近战单位 |
| [RangedUnitTag](#rangedunittag) | 标签 | 标记该实体为远程单位 |

---

## DeadTag

**文件**: `src/game/defs/tags.h`

空结构体，作为 ECS 标签组件使用。

```cpp
struct DeadTag {};
struct FaceLeftTag {};
struct MeleeUnitTag {};
struct RangedUnitTag {};
```

---

## EnemyArriveHomeEvent

**文件**: `src/game/defs/event.h`

敌人到达基地时触发的事件。当敌人沿着路径移动到终点时，[FollowPathSystem](../system/README.md#followpathsystem) 会发送此事件。

### 结构定义

```cpp
struct EnemyArriveHomeEvent {};
```

### 使用场景

- 敌人到达基地时触发
- 可用于扣减玩家生命值
- 可用于统计敌人逃脱数量

### 使用示例

```cpp
#include "game/defs/event.h"

// 订阅事件
auto& dispatcher = context.getDispatcher();
dispatcher.sink<game::defs::EnemyArriveHomeEvent>().connect<&onEnemyArrive>();

// 事件处理函数
void onEnemyArrive(const game::defs::EnemyArriveHomeEvent& event) {
    // 扣减生命值
    player_health--;
    spdlog::info("敌人到达基地！剩余生命: {}", player_health);
}

// 发送事件（通常在 FollowPathSystem 中）
dispatcher.enqueue<game::defs::EnemyArriveHomeEvent>();
```

---

## DeadTag

**文件**: `src/game/defs/tags.h`

死亡标签是一个空结构体，用于标记需要被删除的实体。这是 ECS 中"延迟删除"模式的实现。

### 为什么使用延迟删除？

在 ECS 系统中，遍历组件视图时不能安全地销毁实体（会破坏迭代器）。因此采用"标记-删除"模式：

1. 先给实体添加 `DeadTag` 标记
2. 在专门的 [RemoveDeadSystem](../system/README.md#removedeadsystem) 中统一删除

### 结构定义

```cpp
struct DeadTag {};
```

### 使用场景

- 敌人到达终点需要删除
- 敌人被击败需要删除
- 子弹命中目标需要删除
- 任何需要安全删除实体的情况

### 使用示例

```cpp
#include "game/defs/tags.h"

// 标记实体死亡（在 FollowPathSystem 中）
registry.emplace<game::defs::DeadTag>(enemy_entity);

// 在 RemoveDeadSystem 中删除
auto view = registry.view<game::defs::DeadTag>();
for (auto entity : view) {
    registry.destroy(entity);
}
```

### 相关系统

- [FollowPathSystem](../system/README.md#followpathsystem) - 敌人到达终点时添加 DeadTag
- [RemoveDeadSystem](../system/README.md#removedeadsystem) - 实际删除带 DeadTag 的实体
