# Game 游戏逻辑模块

Game 模块包含了 MonsterWar 游戏的具体业务逻辑。它建立在引擎层（Engine）之上，使用组件系统（ECS）来驱动游戏世界。

## 目录结构

| 目录 | 描述 |
|------|------|
| [component/](component/README.md) | 游戏特有的 ECS 组件（如 Stats, Enemy 等） |
| [data/](data/README.md) | 游戏数据结构和蓝图定义 |
| [defs/](defs/README.md) | 事件定义和标签组件 |
| [factory/](factory/README.md) | **实体工厂与蓝图管理器（核心）**：负责数据驱动的实体生成 |
| [loader/](loader/README.md) | 关卡加载器和实体构建逻辑 |
| [scene/](scene/README.md) | 具体的游戏场景实现（如 GameScene） |
| [system/](system/README.md) | 游戏逻辑系统（如寻路、状态更新等） |

---

## 核心设计：数据驱动的实体生成

MonsterWar 采用“蓝图 -> 工厂”的模式来管理复杂游戏实体的创建。

### 1. 蓝图 (Blueprints)
蓝图是存储在 JSON 里的纯数据。它定义了一个单位“应该是什么样子”。
- **配置文件**：`assets/data/enemy_data.json`
- **解析器**：`BlueprintManager`

### 2. 工厂 (EntityFactory)
工厂负责将蓝图转化为活生生的实体。它负责：
- 在 EnTT 注册表中 `registry.create()`。
- 根据蓝图内容挂载各种组件。
- 执行初始化的属性计算（如根据等级缩放 HP）。

### 3. 工作流 (Workflow)

1. **地图加载期**：`LevelLoader` 解析地图上的起点坐标。
2. **逻辑初始化**：`GameScene` 初始化 `BlueprintManager` 并预加载所有敌人配置。
3. **实体生成**：外部逻辑（如刷怪系统）调用 `EntityFactory::createEnemyUnit("wolf"_hs, position, ...)`。
4. **ECS 运作**：创建出的实体具备 `Velocity` 和 `EnemyComponent`，随后被 `MovementSystem` 和 `FollowPathSystem` 自动接管开始移动。
