# Loader 加载模块

Loader 模块负责从 Tiled 地图文件加载游戏数据，包括路径点解析和实体构建。

## 类/结构概览

| 名称 | 描述 |
|------|------|
| [EntityBuilderMW](#entitybuildermw) | 自定义实体构建器，解析路径点数据 |

---

## EntityBuilderMW

**文件**: `src/game/loader/entity_builder_mw.h`, `src/game/loader/entity_builder_mw.cpp`

自定义实体构建器，继承自引擎的 `BasicEntityBuilder`。用于在关卡加载阶段解析 Tiled 地图中的路径点对象，构建寻路图。

### 类定义

```cpp
class EntityBuilderMW final: public engine::loader::BasicEntityBuilder {
private:
    std::unordered_map<int, game::data::WaypointNode>& waypoint_nodes_;  // 路径点图
    std::vector<int>& start_points_;                                      // 起点列表
public:
    EntityBuilderMW(...);
    EntityBuilderMW* build() override;
    void buildPath();
};
```

### 工作原理

```
LevelLoader 解析地图
    ↓
遇到 gid == 0 的对象（纯形状对象）
    ↓
调用 entity_builder_->configure(&object)->build()
    ↓
EntityBuilderMW::build() 判断：
    - object_json_ 存在 && tile_info_ 为空 → 调用 buildPath()
    - 否则 → 调用 BasicEntityBuilder::build()
    ↓
buildPath() 解析路径点数据
    ↓
存储到 waypoint_nodes_ 和 start_points_
```

### buildPath() 解析逻辑

```cpp
void EntityBuilderMW::buildPath() {
    // 1. 检查是否是路径点（point对象）
    if (!object_json_->value("point", false)) return;
    
    // 2. 获取节点ID和位置
    auto id = object_json_->value("id", 0);
    glm::vec2 position(
        object_json_->value("x", 0.0f),
        object_json_->value("y", 0.0f)
    );
    
    // 3. 解析 properties 数组
    for (const auto& prop : properties) {
        // 解析 next 属性（下一节点）
        if (prop_type == "object" && prop_name.starts_with("next")) {
            node.next_node_ids_.push_back(prop.value("value", 0));
        }
        // 解析 start 属性（出怪口）
        else if (prop_name == "start" && prop.value("value", false)) {
            start_points_.push_back(id);
        }
    }
    
    // 4. 存储节点
    waypoint_nodes_[id] = std::move(node);
}
```

### Tiled 地图配置

在 Tiled 地图编辑器中配置路径点：

```json
{
    "id": 67,
    "point": true,        // 标记为点对象
    "x": 100.0,
    "y": 200.0,
    "properties": [
        {
            "name": "next",
            "type": "object",
            "value": 68        // 下一节点ID
        },
        {
            "name": "start",
            "type": "bool",
            "value": true       // 标记为起点
        }
    ]
}
```

### 使用示例

```cpp
#include "game/loader/entity_builder_mw.h"

// 在 GameScene::loadLevel() 中
engine::loader::LevelLoader level_loader;

// 创建自定义构建器
auto builder = std::make_unique<game::loader::EntityBuilderMW>(
    level_loader,
    context_,
    registry_,
    waypoint_nodes_,    // 存储解析出的路径点
    start_points_       // 存储起点列表
);

// 设置到 LevelLoader
level_loader.setEntityBuilder(std::move(builder));

// 加载关卡（会自动解析路径点）
level_loader.loadLevel("assets/maps/level1.tmj", this);
```

### 相关模块

- [WaypointNode](../data/README.md#waypointnode) - 解析出的路径点数据结构
- [FollowPathSystem](../system/README.md#followpathsystem) - 使用解析出的路径点进行寻路
