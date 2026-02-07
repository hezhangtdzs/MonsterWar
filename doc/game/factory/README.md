# Factory 工厂模块

Factory 模块负责实体的创建和管理，通过数据驱动的方式将实体的配置（蓝图）与创建逻辑（工厂）分离。

## 类/结构概览

| 名称 | 描述 |
|------|------|
| [BlueprintManager](#blueprintmanager) | 蓝图管理器，从 JSON 加载并缓存实体配置数据 |
| [EntityFactory](#entityfactory) | 实体工厂，根据蓝图数据装配 ECS 组件并创建实体 |

---

## BlueprintManager

**文件**: `src/game/factory/blueprint_manager.h`, `src/game/factory/blueprint_manager.cpp`

蓝图管理器负责读取 `assets/data/enemy_data.json` 等配置文件，将其解析为内存中的蓝图结构体。它使用 `entt::hashed_string` 生成的 `id_type` 作为键来存储蓝图。

### 主要功能

- **加载与解析**：支持加载敌人、单位等配置。
- **资源预加载**：在加载蓝图时自动预加载关联的音效资源。
- **快速查询**：提供哈希后的 ID 进行 $O(1)$ 级别的配置查找。

### 核心接口

```cpp
bool loadEnemyClassBlueprints(std::string_view enemy_json_path);
const data::EnemyClassBlueprint& getEnemyClassBlueprint(entt::id_type id) const;
```

---

## EntityFactory

**文件**: `src/game/factory/entity_factory.h`, `src/game/factory/entity_factory.cpp`

实体工厂是创建实体的核心。它从蓝图管理器获取数据，并按照预设的步骤将各种 ECS 组件（Transform, Sprite, Animation, Stats 等）装配到实体上。

### 工作流程

1. **获取数据**：根据传入的 `class_id` 从 `BlueprintManager` 查找蓝图。
2. **创建实体**：使用 `registry_.create()` 生成新的实体句柄。
3. **分步装配**：
   - 添加基础组件（Transform, Velocity, Render）。
   - 添加表现组件（Sprite, Animation）。
   - 添加战斗组件（Stats, Enemy, ClassName）。
   - 添加逻辑标签（FaceLeftTag, MeleeUnitTag 等）。
4. **属性计算**：根据等级（Level）和稀有度（Rarity）动态计算最终战斗属性。

### 核心接口

```cpp
entt::entity createEnemyUnit(
    entt::id_type class_id, 
    const glm::vec2& position, 
    int target_waypoint_id, 
    int level = 1, 
    int rarity = 1
);
```
