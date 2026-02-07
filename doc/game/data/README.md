# Data 数据模块

Data 模块定义了游戏中使用的核心数据结构，包括路径节点、配置数据等。

## 类/结构概览

| 名称 | 描述 |
|------|------|
| [WaypointNode](#waypointnode) | 路径点节点，用于构建寻路图 |
| [EntityBlueprint](#entityblueprint) | 实体蓝图配置，包含属性、精灵、动画等数据 |

---

## EntityBlueprint

**文件**: `src/game/data/entity_blueprint.h`

蓝图是一组用于定义实体初始状态的纯数据结构。它将 JSON 配置映射到 C++ 结构体中。

### 子蓝图结构

- **StatsBlueprint**: 基础战斗属性（HP, ATK, DEF, Range等）。
- **SpriteBlueprint**: 纹理路径、裁剪区域、渲染偏移等。
- **AnimationBlueprint**: 帧序列及每帧持续时间。
- **SoundBlueprint**: 动作到音效资源的哈希映射。
- **EnemyBlueprint**: 敌人特有属性（移动速度、是否远程等）。

### 核心蓝图定义

```cpp
struct EnemyClassBlueprint {
    entt::id_type class_id_;
    std::string class_name_;
    StatsBlueprint stats_;
    EnemyBlueprint enemy_;
    SoundBlueprint sounds_;
    SpriteBlueprint sprite_;
    DisplayInfoBlueprint display_info_;
    std::unordered_map<entt::id_type, AnimationBlueprint> animations_;
};
```

---

## WaypointNode

**文件**: `src/game/data/waypoint_node.h`

路径点节点表示寻路图中的一个节点，包含位置信息和可到达的下一节点列表。支持分叉路径设计。

### 结构定义

```cpp
struct WaypointNode {
    int id_;                          // 路径点的唯一标识符
    glm::vec2 position_;              // 路径点的位置（像素坐标）
    std::vector<int> next_node_ids_;  // 可到达的下一节点ID列表
};
```

### 字段说明

| 字段 | 类型 | 描述 |
|------|------|------|
| `id_` | `int` | 节点的唯一标识符，对应 Tiled 地图中对象的 ID |
| `position_` | `glm::vec2` | 节点在地图上的位置坐标（像素） |
| `next_node_ids_` | `std::vector<int>` | 可到达的下一节点ID列表，支持分叉路径 |

### 路径图结构

WaypointNode 通过 `next_node_ids_` 构建有向图结构：

```
单一路径：
    节点67 → 节点68 → 节点69 → 节点70
    
分叉路径：
    节点79 ─┬─→ 节点86
            └─→ 节点80 ─→ 节点81
```

### 使用示例

```cpp
#include "game/data/waypoint_node.h"

// 创建路径点
std::unordered_map<int, game::data::WaypointNode> waypoint_nodes;

// 添加节点
waypoint_nodes[67] = game::data::WaypointNode{
    67,                           // id
    glm::vec2(100.0f, 200.0f),   // position
    {68}                          // next_node_ids（指向节点68）
};

waypoint_nodes[68] = game::data::WaypointNode{
    68,
    glm::vec2(200.0f, 200.0f),
    {69, 70}  // 分叉：可以走到69或70
};

// 获取下一节点
int current_id = 68;
auto& node = waypoint_nodes.at(current_id);
if (!node.next_node_ids_.empty()) {
    int next_id = node.next_node_ids_[0];  // 或随机选择
}
```

### 相关模块

- [EntityBuilderMW](../loader/README.md#entitybuildermw) - 从 Tiled 地图解析并创建路径点
- [FollowPathSystem](../system/README.md#followpathsystem) - 使用路径点进行寻路
