# Scene 场景模块

> **版本**: 1.0.0  
> **最后更新**: 2026-02-15  
> **相关文档**: [核心模块](../core/README.md) | [渲染模块](../render/README.md) | [ECS 架构](../../ECS_ARCHITECTURE.md)

Scene 模块负责管理游戏场景的生命周期、场景切换和关卡加载。支持场景栈管理，允许场景叠加（如在游戏场景上弹出菜单）。

---

## 目录

- [架构概览](#架构概览)
- [类概览](#类概览)
- [Scene](#scene)
- [SceneManager](#scenemanager)
- [LevelLoader](#levelloader)
- [场景生命周期](#场景生命周期)
- [Scene 与 ECS 集成](#scene-与-ecs-集成)
- [最佳实践](#最佳实践)

---

## 架构概览

```mermaid
graph TB
    SceneManager[SceneManager<br/>场景管理器]
    
    SceneManager --> Scene1[Scene<br/>场景基类]
    SceneManager --> Scene2[Scene]
    SceneManager --> Scene3[Scene]
    
    Scene1 --> GameObject1[GameObject]
    Scene1 --> GameObject2[GameObject]
    Scene1 --> UIManager[UIManager]
    
    LevelLoader[LevelLoader<br/>关卡加载器] --> Scene1
    LevelLoader --> TiledJSON[Tiled JSON]
```

## 类概览

| 类名 | 描述 |
|------|------|
| [Scene](#scene) | 场景基类，管理游戏对象和生命周期 |
| [SceneManager](#scenemanager) | 场景管理器，处理场景切换和栈管理 |
| [LevelLoader](#levelloader) | 关卡加载器，从 Tiled JSON 加载关卡 |

---

## Scene

**文件**: `src/engine/scene/scene.h`

场景基类，负责管理属于该场景的所有游戏对象及其生命周期。

### 类定义

```cpp
class Scene {
protected:
    std::string scene_name_;                        // 场景名称
    engine::core::Context& context_;               // 引擎上下文
    engine::scene::SceneManager& scene_manager_;    // 场景管理器引用
    bool is_initialized_ = false;                   // 初始化标记
    std::vector<std::unique_ptr<GameObject>> game_objects_;         // 活动对象
    std::vector<std::unique_ptr<GameObject>> pending_game_objects_; // 待添加对象
    std::unique_ptr<UIManager> ui_manager_;         // UI 管理器

public:
    Scene(const std::string& scene_name, 
          engine::core::Context& context, 
          engine::scene::SceneManager& scene_manager);
    virtual ~Scene();
    
    // 禁止拷贝和移动
    Scene(const Scene&) = delete;
    Scene& operator=(const Scene&) = delete;
    Scene(Scene&&) = delete;
    Scene& operator=(Scene&&) = delete;

    // 生命周期方法
    virtual void init();
    virtual void update(float delta_time);
    virtual void render();
    virtual bool handleInput();
    virtual void clean();

    // 游戏对象管理
    virtual void addGameObject(std::unique_ptr<GameObject>&& game_object);
    virtual void safeAddGameObject(std::unique_ptr<GameObject>&& game_object);
    virtual void removeGameObject(GameObject* game_object);
    virtual void safeRemoveGameObject(GameObject* game_object);
    
    // 查询
    GameObject* findGameObjectByName(const std::string& name) const;
    const std::vector<std::unique_ptr<GameObject>>& getGameObjects() const;
    UIManager* getUIManager();

    // Getters and Setters
    const std::string& getSceneName() const;
    bool isInitialized() const;
    engine::core::Context& getContext() const;
    engine::scene::SceneManager& getSceneManager() const;
};
```

### 生命周期

```mermaid
graph LR
    A[创建] --> B[init]
    B --> C[update/render<br/>循环]
    C --> D[clean]
    D --> E[销毁]
    
    C --> F[handleInput]
    F --> C
```

### 使用示例

```cpp
class GameplayScene : public engine::scene::Scene {
public:
    GameplayScene(engine::core::Context& context, 
                  engine::scene::SceneManager& scene_manager)
        : Scene("Gameplay", context, scene_manager) {}

    void init() override {
        // 加载关卡
        engine::scene::LevelLoader loader;
        loader.loadLevel("assets/levels/level1.tmj", *this);
        
        // 创建玩家
        auto player = std::make_unique<GameObject>("Player");
        player->addComponent<TransformComponent>(glm::vec2(100, 100));
        player->addComponent<SpriteComponent>("player", context_.getResourceManager());
        addGameObject(std::move(player));
    }

    void update(float delta_time) override {
        // 自定义更新逻辑
        Scene::update(delta_time);
    }

    void render() override {
        // 自定义渲染逻辑
        Scene::render();
    }
};
```

---

## SceneManager

**文件**: `src/engine/scene/scene_manager.h`

场景管理器，负责游戏场景的切换、叠加（栈管理）和生命周期调度。

### 特性

- **场景栈**: 支持场景叠加，如游戏场景上弹出暂停菜单
- **延迟处理**: 在帧末尾统一处理场景切换请求，避免遍历中修改容器
- **会话数据**: 支持跨场景共享数据

### 类定义

```cpp
class SceneManager {
public:
    explicit SceneManager(engine::core::Context& context);
    ~SceneManager();
    
    // 禁止拷贝和移动
    SceneManager(const SceneManager&) = delete;
    SceneManager& operator=(const SceneManager&) = delete;
    SceneManager(SceneManager&&) = delete;
    SceneManager& operator=(SceneManager&&) = delete;

    // 场景切换请求（异步，延迟执行）
    void requestPushScene(std::unique_ptr<Scene>&& scene);
    void requestPopScene();
    void requestReplaceScene(std::unique_ptr<Scene>&& scene);

    // 获取当前场景
    Scene* getCurrentScene() const;
    
    // 帧更新
    void update(float delta_time);
    void render();
    void handleInput();
    
    // 清理
    void close();

    // 上下文
    engine::core::Context& getContext() const;
    
    // 会话数据
    std::shared_ptr<game::data::SessionData> getSessionData();
    void setSessionData(std::shared_ptr<game::data::SessionData> session_data);

private:
    enum class PendingAction {
        None,    // 无操作
        Push,    // 压入新场景
        Pop,     // 弹出当前场景
        Replace  // 替换整个场景栈
    };
    
    void processPendingActions();
};
```

### 场景栈操作

```mermaid
graph TB
    subgraph 场景栈示例
        S1[MenuScene] 
        S2[GameplayScene]
        S3[PauseMenu]
    end
    
    Push[requestPushScene] -->|压入| 栈顶
    Pop[requestPopScene] -->|弹出| 栈顶
    Replace[requestReplaceScene] -->|清空并压入| 新栈
```

### 使用示例

```cpp
// 切换到游戏场景
scene_manager.requestReplaceScene(
    std::make_unique<GameplayScene>(context, scene_manager)
);

// 弹出暂停菜单（叠加在当前场景上）
scene_manager.requestPushScene(
    std::make_unique<PauseMenuScene>(context, scene_manager)
);

// 关闭暂停菜单，返回游戏
scene_manager.requestPopScene();

// 跨场景数据传递
auto session_data = std::make_shared<game::data::SessionData>();
session_data->player_score = 1000;
scene_manager.setSessionData(session_data);

// 在另一个场景获取数据
auto data = scene_manager.getSessionData();
int score = data->player_score;
```

---

## LevelLoader

**文件**: `src/engine/scene/level_loader.h`

负责从 Tiled JSON 文件 (.tmj) 加载关卡数据到 Scene 中。

### 支持的图层类型

| 类型 | 描述 |
|------|------|
| Image Layer | 图片图层，用于背景 |
| Tile Layer | 瓦片图层，用于地图 |
| Object Layer | 对象图层，用于实体和触发器 |

### 类定义

```cpp
struct TileData {
    engine::component::TileInfo info;
    const nlohmann::json* json_ptr = nullptr;
};

class LevelLoader final {
    friend class ObjectBuilder;

public:
    LevelLoader() = default;
    
    // 加载关卡
    bool loadLevel(const std::string& map_path, Scene& scene);

private:
    void loadImageLayer(const nlohmann::json& layer_json, Scene& scene);
    void loadTileLayer(const nlohmann::json& layer_json, Scene& scene);
    void loadObjectLayer(const nlohmann::json& layer_json, Scene& scene);
    
    TileData getTileDataByGid(int gid);
    engine::component::TileInfo getTileInfoByGid(int gid);
    const nlohmann::json* findTileset(int gid);
    void loadTileset(const std::string& tileset_path, int first_gid);
};
```

### Tiled JSON 支持

- 支持外部瓦片集 (.tsj) 链接
- 支持自定义属性
- 支持对象层中的各种形状（矩形、多边形等）

### 使用示例

```cpp
void GameplayScene::init() {
    engine::scene::LevelLoader loader;
    
    if (!loader.loadLevel("assets/levels/level1.tmj", *this)) {
        spdlog::error("Failed to load level!");
        return;
    }
    
    // 关卡已加载，所有图层和对象已添加到场景
}
```

### 瓦片类型映射

LevelLoader 将 Tiled 中的瓦片类型映射到引擎的 TileType：

| Tiled 属性 | TileType |
|------------|----------|
| 无/默认 | NORMAL |
| solid=true | SOLID |
| hazard=true | HAZARD |
| ladder=true | LADDER |
| exit=true | LEVEL_EXIT |

---

## 场景生命周期流程

```mermaid
sequenceDiagram
    participant App as GameApp
    participant SM as SceneManager
    participant Scene as Scene
    participant GO as GameObject
    participant UI as UIManager

    App->>SM: update(delta_time)
    SM->>Scene: update(delta_time)
    
    Scene->>GO: update(delta_time, context)
    Scene->>UI: update(delta_time)
    
    SM->>Scene: render()
    Scene->>GO: render(context)
    Scene->>UI: render()
    
    SM->>SM: processPendingActions()
```

---

## 模块依赖图

```mermaid
graph TB
    SceneManager --> Scene
    Scene --> GameObject
    Scene --> UIManager
    Scene --> Context
    
    LevelLoader --> Scene
    LevelLoader --> TileLayerComponent
    LevelLoader --> ObjectBuilder
    
    style SceneManager fill:#f9f,stroke:#333,stroke-width:2px
    style Scene fill:#bbf,stroke:#333,stroke-width:2px
    style LevelLoader fill:#bbf,stroke:#333,stroke-width:2px
```

## 最佳实践

1. **继承 Scene**: 为每个游戏画面创建 Scene 子类
2. **使用 safe 方法**: 在 update 循环中使用 `safeAddGameObject` 和 `safeRemoveGameObject`
3. **延迟切换**: 使用 `requestXxx` 方法进行场景切换，避免在遍历中修改场景栈
4. **关卡数据**: 使用 LevelLoader 加载 Tiled 编辑器创建的关卡
5. **会话数据**: 使用 SessionData 在场景间传递数据

## Scene 与 ECS 集成

Scene 类内置了 EnTT 注册表 (`entt::registry`)，支持完整的 ECS 架构：

### 类定义（ECS 支持）

```cpp
class Scene {
protected:
    std::string scene_name_;
    engine::core::Context& context_;
    entt::registry registry_;  // EnTT 注册表
    bool is_initialized_ = false;
    std::unique_ptr<engine::ui::UIManager> ui_manager_;

public:
    // ECS 实体管理
    entt::registry& getRegistry() { return registry_; }
    
    // 场景请求方法
    void requestPopScene();
    void requestPushScene(std::unique_ptr<engine::scene::Scene>&& scene);
    void requestReplaceScene(std::unique_ptr<engine::scene::Scene>&& scene);
    
    // 退出游戏
    void quit();
};
```

### 使用 ECS 创建实体

```cpp
class GameScene : public engine::scene::Scene {
public:
    void init() override {
        // 创建 ECS 实体
        auto entity = registry_.create();
        
        // 添加组件
        registry_.emplace<engine::component::TransformComponent>(
            entity, glm::vec2(100.0f, 200.0f)
        );
        registry_.emplace<engine::component::SpriteComponent>(
            entity, sprite_data
        );
        registry_.emplace<engine::component::VelocityComponent>(
            entity, glm::vec2(0.0f, 0.0f)
        );
    }
};
```

## 场景请求机制

SceneManager 采用"延迟处理"机制，确保场景切换安全执行：

### 延迟处理流程

```mermaid
sequenceDiagram
    participant S as Scene
    participant SM as SceneManager
    participant Stack as 场景栈

    Note over S: 场景更新中...
    S->>SM: requestPushScene(new_scene)
    SM->>SM: 记录 pending_action_ = Push
    SM->>SM: 保存 pending_scene_
    Note over SM: 等待帧结束
    SM->>SM: processPendingActions()
    SM->>Stack: 执行实际的场景压入
    Stack->>S: 调用新场景的 init()
```

### PendingAction 枚举

```cpp
enum class PendingAction {
    None,    // 无待处理操作
    Push,    // 压入新场景
    Pop,     // 弹出当前场景
    Replace  // 替换整个场景栈
};
```

### 事件驱动场景切换

Scene 通过事件系统与 SceneManager 通信：

```cpp
// 在 Scene 中请求场景切换
void Scene::requestPushScene(std::unique_ptr<Scene>&& scene) {
    context_.getDispatcher().enqueue<engine::utils::PushSceneEvent>(
        engine::utils::PushSceneEvent{std::move(scene)}
    );
}

void Scene::requestPopScene() {
    context_.getDispatcher().enqueue<engine::utils::PopSceneEvent>();
}

void Scene::quit() {
    context_.getDispatcher().enqueue<engine::utils::QuitEvent>();
}
```

## 场景栈管理示例

```mermaid
graph TB
    subgraph 正常游戏流程
        A[TitleScene] -->|开始游戏| B[GameScene]
        B -->|暂停| C[PauseScene]
        C -->|继续| B
        B -->|游戏结束| D[GameOverScene]
        D -->|重新开始| B
        D -->|返回标题| A
    end
    
    subgraph 场景栈状态
        E["TitleScene<br/>栈: [Title]"]
        F["GameScene<br/>栈: [Game]"]
        G["PauseScene<br/>栈: [Game, Pause]"]
    end
```

### 代码示例

```cpp
// TitleScene 中切换到游戏
void TitleScene::onStartButtonClicked() {
    requestReplaceScene(std::make_unique<GameScene>(context_));
}

// GameScene 中打开暂停菜单
void GameScene::onPauseButtonClicked() {
    requestPushScene(std::make_unique<PauseScene>(context_));
}

// PauseScene 中继续游戏
void PauseScene::onResumeButtonClicked() {
    requestPopScene();
}

// 退出游戏
void AnyScene::onQuitButtonClicked() {
    quit();  // 发送 QuitEvent
}
```

## 会话数据跨场景传递

```cpp
// 定义会话数据
namespace game::data {
    struct SessionData {
        int player_score = 0;
        int current_level = 1;
        std::vector<std::string> unlocked_items;
    };
}

// 在场景间传递
void TitleScene::startNewGame() {
    auto session = std::make_shared<game::data::SessionData>();
    session->current_level = 1;
    
    context_.getSceneManager().setSessionData(session);
    requestReplaceScene(std::make_unique<GameScene>(context_));
}

// 在新场景中获取
void GameScene::init() {
    auto session = context_.getSceneManager().getSessionData();
    if (session) {
        current_level_ = session->current_level;
    }
}
```
