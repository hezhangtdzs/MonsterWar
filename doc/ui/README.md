# UI 用户界面模块

UI 模块提供了一套完整的用户界面系统，支持按钮、面板、文本、图片等控件，采用树形结构组织，支持事件处理和状态管理。支持使用 ResourceId 进行高效的资源标识。

## 架构概览

```mermaid
graph TB
    UIManager[UIManager<br/>UI管理器]
    
    UIManager --> Root[UIElement<br/>根元素]
    
    Root --> Panel[UIPanel<br/>面板]
    Root --> Button[UIButton<br/>按钮]
    Root --> Text[UIText<br/>文本]
    Root --> Image[UIImage<br/>图片]
    
    Panel --> Button2[UIButton]
    Panel --> Text2[UIText]
    
    UIInteractive[UIInteractive<br/>交互基类] --> Button
    UIInteractive --> UIInteractive2[...]
    
    UIState[UIState<br/>状态基类] --> Normal[NormalState]
    UIState --> Hover[HoverState]
    UIState --> Pressed[PressedState]
    
    ResourceId[ResourceId<br/>资源标识] --> UIInteractive
    ResourceId --> UIImage
    ResourceId --> UIText
```

## 类概览

| 类名 | 描述 |
|------|------|
| [UIManager](#uimanager) | UI 管理器，管理整个 UI 系统 |
| [UIElement](#uielement) | 所有 UI 控件的基类 |
| [UIInteractive](#uiinteractive) | 可交互 UI 元素的基类，支持 ResourceId |
| [UIButton](#uibutton) | 按钮控件 |
| [UIPanel](#uipanel) | 面板容器 |
| [UIText](#uitext) | 文本显示，支持 ResourceId |
| [UIImage](#uiimage) | 图片显示，支持 ResourceId |
| [UIState](#uistate) | UI 状态基类 |

---

## UIManager

**文件**: `src/engine/ui/ui_manager.h`

UI 管理器类，负责管理整个 UI 系统。

### 类定义

```cpp
class UIManager {
public:
    explicit UIManager(engine::core::Context& context);
    ~UIManager();
    
    // 初始化
    [[nodiscard]] bool init(const glm::vec2& window_size);
    
    // 生命周期
    void update(float delta_time);
    void render();
    bool handleInput();
    
    // 元素管理
    void addElement(std::unique_ptr<UIElement> element);
    void addElement(UIElement* parent, std::unique_ptr<UIElement> element);
    void removeElement(UIElement* element);
    
    // 查询
    UIElement* getRootElement() const;
    
    // 清理
    void clear();
};
```

### 使用示例

```cpp
// 在场景中初始化 UI
void MenuScene::init() {
    ui_manager_ = std::make_unique<UIManager>(context_);
    ui_manager_->init(glm::vec2(1280, 720));
    
    // 创建开始按钮
    auto start_btn = std::make_unique<UIButton>(
        context_, "Start Game", "assets/fonts/arial.ttf", 24
    );
    start_btn->setPosition(glm::vec2(540, 300));
    start_btn->setSize(glm::vec2(200, 50));
    start_btn->setClickCallback([this]() {
        scene_manager_.requestReplaceScene(
            std::make_unique<GameplayScene>(context_, scene_manager_)
        );
    });
    
    ui_manager_->addElement(std::move(start_btn));
}
```

---

## UIElement

**文件**: `src/engine/ui/ui_element.h`

所有 UI 控件的基类，提供通用的 UI 元素功能。

### 类定义

```cpp
class UIElement {
protected:
    UIElement* parent_ = nullptr;
    std::vector<std::unique_ptr<UIElement>> children_;
    glm::vec2 position_ = { 0.0f, 0.0f };
    glm::vec2 size_ = { 0.0f, 0.0f };
    bool visible_ = true;
    bool enabled_ = true;
    engine::core::Context& context_;

public:
    explicit UIElement(engine::core::Context& context);
    virtual ~UIElement();
    
    // 生命周期
    virtual void update(float delta_time);
    virtual void render();
    virtual bool handleInput();
    
    // 层级管理
    virtual void addChild(std::unique_ptr<UIElement> child);
    virtual void removeChild(UIElement* child);
    void clearChildren();  // 清空所有子元素
    
    // 坐标转换
    virtual glm::vec2 getWorldPosition() const;
    virtual bool containsPoint(const glm::vec2& point) const;
    
    // Getters and Setters
    UIElement* getParent() const;
    void setParent(UIElement* parent);
    const std::vector<std::unique_ptr<UIElement>>& getChildren() const;
    
    const glm::vec2& getPosition() const;
    void setPosition(const glm::vec2& position);
    
    virtual const glm::vec2& getSize() const;  // 改为 virtual
    virtual void setSize(const glm::vec2& size);
    
    bool isVisible() const;
    void setVisible(bool visible);
    bool isEnabled() const;
    void setEnabled(bool enabled);
};
```

### 层级结构

```mermaid
graph TB
    Root[Root] --> Panel1[Panel]
    Root --> Panel2[Panel]
    
    Panel1 --> Btn1[Button]
    Panel1 --> Text1[Text]
    
    Panel2 --> Img1[Image]
    Panel2 --> Btn2[Button]
    
    style Root fill:#f9f,stroke:#333,stroke-width:2px
```

---

## UIInteractive

**文件**: `src/engine/ui/ui_interactive.h`

可交互 UI 元素的基类，支持状态管理、精灵切换和声音播放。使用 ResourceId 进行高效的资源标识。

### 类定义

```cpp
class UIInteractive : public UIElement {
    std::unique_ptr<state::UIState> current_state_; 
    std::unordered_map<engine::resource::ResourceId, std::unique_ptr<engine::render::Sprite>> sprites_;
    std::unordered_map<engine::resource::ResourceId, engine::resource::ResourceId> sound_;
    engine::render::Sprite* current_sprite_ = nullptr;
    bool interactive_ = true;
    std::function<void()> click_callback_;

public:
    UIInteractive(engine::core::Context& context);
    virtual ~UIInteractive() = default;
    
    // 生命周期
    void update(float delta_time) override;
    bool handleInput() override;
    void render() override;
    
    // 状态管理
    void setState(std::unique_ptr<state::UIState> state);
    state::UIState* getCurrentState() const;
    
    // 精灵管理（ResourceId 版本）
    void addSprite(engine::resource::ResourceId name, std::unique_ptr<engine::render::Sprite> sprite);
    engine::render::Sprite* getSprite(engine::resource::ResourceId name) const;
    
    // 精灵管理（字符串版本）
    void addSprite(std::string_view name, std::unique_ptr<engine::render::Sprite> sprite);
    engine::render::Sprite* getSprite(std::string_view name) const;
    
    void setCurrentSprite(engine::render::Sprite* sprite);
    engine::render::Sprite* getCurrentSprite() const;
    
    // 声音管理（ResourceId 版本）
    void addSound(engine::resource::ResourceId name, engine::resource::ResourceId sound_id);
    engine::resource::ResourceId getSound(engine::resource::ResourceId name) const;
    void playSound(engine::resource::ResourceId name);
    
    // 声音管理（字符串版本）
    void addSound(engine::resource::ResourceId name, std::string_view sound_key_or_path);
    void addSound(std::string_view name, std::string_view sound_key_or_path);
    engine::resource::ResourceId getSound(std::string_view name) const;
    void playSound(std::string_view name);
    
    // 交互设置
    void setInteractive(bool interactive);
    bool isInteractive() const;
    void setClickCallback(std::function<void()> callback);
    void triggerClick();  // 改名为 triggerClick
    
    // 检查点是否在元素内部
    bool containsPoint(const glm::vec2& point) const;
    
    // 获取上下文引用
    engine::core::Context& getContext();
};
```

### 状态机

```mermaid
stateDiagram-v2
    [*] --> Normal
    Normal --> Hover: 鼠标进入
    Hover --> Normal: 鼠标离开
    Hover --> Pressed: 鼠标按下
    Pressed --> Hover: 鼠标释放(在区域内)
    Pressed --> Normal: 鼠标释放(在区域外)
```

---

## UIButton

**文件**: `src/engine/ui/ui_button.h`

UI 按钮类，用于处理用户交互。

### 类定义

```cpp
class UIButton : public UIInteractive {
private:
    UIText* text_element_ = nullptr;
    std::function<void()> click_callback_;
    
    FColor normal_bg_color_ = { 0.2f, 0.2f, 0.2f, 1.0f };
    FColor hover_bg_color_ = { 0.3f, 0.3f, 0.3f, 1.0f };
    FColor pressed_bg_color_ = { 0.4f, 0.4f, 0.4f, 1.0f };
    
    FColor normal_text_color_ = { 1.0f, 1.0f, 1.0f, 1.0f };
    FColor hover_text_color_ = { 1.0f, 1.0f, 1.0f, 1.0f };
    FColor pressed_text_color_ = { 1.0f, 1.0f, 1.0f, 1.0f };

public:
    // 文本按钮
    UIButton(engine::core::Context& context, 
             const std::string& text, 
             const std::string& font_path, 
             int font_size = 16);
    
    // 精灵按钮
    UIButton(
        engine::core::Context& context,
        const std::string& normal_sprite_path,
        const std::string& hover_sprite_path,
        const std::string& pressed_sprite_path,
        const glm::vec2& position,
        const glm::vec2& size,
        std::function<void()> callback
    );
    
    ~UIButton() override;

    bool handleInput() override;
    void setSize(const glm::vec2& size) override;
    
    // 回调设置
    void setClickCallback(std::function<void()> callback);
    
    // 文本设置
    const std::string& getText() const;
    void setText(const std::string& text);
    
    // 背景颜色 Getter/Setter
    const FColor& getNormalBgColor() const;
    void setNormalBgColor(const FColor& color);
    const FColor& getHoverBgColor() const;
    void setHoverBgColor(const FColor& color);
    const FColor& getPressedBgColor() const;
    void setPressedBgColor(const FColor& color);
    
    // 文本颜色 Getter/Setter
    const FColor& getNormalTextColor() const;
    void setNormalTextColor(const FColor& color);
    const FColor& getHoverTextColor() const;
    void setHoverTextColor(const FColor& color);
    const FColor& getPressedTextColor() const;
    void setPressedTextColor(const FColor& color);

private:
    void updateTextPosition();
};
```

### 使用示例

```cpp
using namespace entt::literals;

// 文本按钮
auto btn = std::make_unique<UIButton>(
    context_, 
    "Click Me", 
    "assets/fonts/arial.ttf", 
    24
);
btn->setPosition(glm::vec2(100, 100));
btn->setSize(glm::vec2(200, 50));
btn->setClickCallback([]() {
    spdlog::info("Button clicked!");
});

// 设置颜色
btn->setNormalBgColor({0.2f, 0.2f, 0.2f, 1.0f});
btn->setHoverBgColor({0.3f, 0.3f, 0.3f, 1.0f});
btn->setPressedBgColor({0.4f, 0.4f, 0.4f, 1.0f});

// 获取颜色设置
auto bg_color = btn->getNormalBgColor();
auto text_color = btn->getNormalTextColor();

// 使用 ResourceId 添加精灵
btn->addSprite("normal"_hs, std::make_unique<engine::render::Sprite>("btn_normal"_hs));
btn->addSprite("hover"_hs, std::make_unique<engine::render::Sprite>("btn_hover"_hs));

// 使用 ResourceId 添加声音
btn->addSound("click"_hs, "click_sound"_hs);
btn->playSound("click"_hs);
```

---

## UIPanel

**文件**: `src/engine/ui/ui_panel.h`

UI 面板类，用于组织和管理其他 UI 元素。

### 类定义

```cpp
class UIPanel : public UIElement {
private:
    FColor background_color_ = { 0.0f, 0.0f, 0.0f, 0.0f };
    FColor border_color_ = { 0.0f, 0.0f, 0.0f, 0.0f };
    float border_width_ = 0.0f;

public:
    explicit UIPanel(engine::core::Context& context);
    ~UIPanel() override;
    
    void render() override;
    
    // 样式设置
    const FColor& getBackgroundColor() const;
    void setBackgroundColor(const FColor& color);
    const FColor& getBorderColor() const;
    void setBorderColor(const FColor& color);
    float getBorderWidth() const;
    void setBorderWidth(float width);
};
```

### 使用示例

```cpp
auto panel = std::make_unique<UIPanel>(context_);
panel->setPosition(glm::vec2(50, 50));
panel->setSize(glm::vec2(300, 400));
panel->setBackgroundColor({0.1f, 0.1f, 0.1f, 0.9f});
panel->setBorderColor({0.5f, 0.5f, 0.5f, 1.0f});
panel->setBorderWidth(2.0f);

// 向面板添加子元素
auto btn = std::make_unique<UIButton>(context_, "OK", "assets/fonts/arial.ttf");
btn->setPosition(glm::vec2(100, 300));
panel->addChild(std::move(btn));

ui_manager_->addElement(std::move(panel));
```

---

## UIText

**文件**: `src/engine/ui/ui_text.h`

UI 文本类，用于显示文本内容。继承自 Observer，支持响应数据变化。支持使用 ResourceId 进行字体资源标识。

### 类定义

```cpp
enum class TextAlignment {
    LEFT,
    CENTER,
    RIGHT
};

class UIText : public UIElement, public engine::interface::Observer {
private:
    std::string text_;
    std::string font_path_;
    engine::resource::ResourceId font_id_{ engine::resource::InvalidResourceId };
    int font_size_ = 16;
    FColor color_ = { 1.0f, 1.0f, 1.0f, 1.0f };
    TextAlignment alignment_ = TextAlignment::LEFT;
    mutable bool is_dirty_ = true;

public:
    // 字符串路径构造函数
    UIText(engine::core::Context& context, 
           const std::string& text, 
           const std::string& font_path, 
           int font_size = 16);
    
    // ResourceId 构造函数
    UIText(engine::core::Context& context, 
           const std::string& text, 
           engine::resource::ResourceId font_id, 
           int font_size = 16);
    
    ~UIText() override;
    
    void render() override;
    const glm::vec2& getSize() const override;
    void ensureUpToDate() const;
    
    // 文本设置
    const std::string& getText() const;
    void setText(const std::string& text);
    
    // 字体路径设置
    const std::string& getFontPath() const;
    void setFontPath(const std::string& font_path);
    
    // ResourceId 字体设置
    engine::resource::ResourceId getFontId() const;
    void setFontId(engine::resource::ResourceId font_id);
    
    // 字体大小设置
    int getFontSize() const;
    void setFontSize(int font_size);
    
    // 样式设置
    const FColor& getColor() const;
    void setColor(const FColor& color);
    TextAlignment getAlignment() const;
    void setAlignment(TextAlignment alignment);
    
    // 观察者接口
    void onNotify(engine::interface::EventType event_type, const std::any& data) override;

private:
    void updateSize();
};
```

### 使用示例

```cpp
using namespace entt::literals;

// 字符串路径创建
auto score_text = std::make_unique<UIText>(
    context_, 
    "Score: 0", 
    "assets/fonts/arial.ttf", 
    32
);

// ResourceId 创建
ResourceId font_id = engine::resource::toResourceId("assets/fonts/arial.ttf");
auto score_text2 = std::make_unique<UIText>(
    context_,
    "Score: 0",
    font_id,
    32
);

score_text->setPosition(glm::vec2(50, 50));
score_text->setColor({1.0f, 1.0f, 0.0f, 1.0f});  // 黄色
score_text->setAlignment(TextAlignment::LEFT);

// 获取/设置字体 ID
auto current_font_id = score_text->getFontId();
score_text->setFontId(new_font_id);

// 订阅分数变化
score_component->addObserver(score_text.get());

// 在 onNotify 中更新文本
void ScoreUI::onNotify(const EventType event, const std::any& data) {
    if (event == EventType::SCORE_CHANGED) {
        int score = std::any_cast<int>(data);
        setText("Score: " + std::to_string(score));
    }
}
```

---

## UIImage

**文件**: `src/engine/ui/ui_image.h`

UI 图片类，用于在 UI 上显示图片。支持使用 ResourceId 进行纹理资源标识。

### 类定义

```cpp
class UIImage : public UIElement {
private:
    engine::render::Sprite sprite_;
    float opacity_ = 1.0f;

public:
    // 字符串路径构造函数
    UIImage(engine::core::Context& context, 
            const std::string& texture_id, 
            const glm::vec2& position = {0.0f, 0.0f}, 
            const glm::vec2& size = {0.0f, 0.0f});
    
    // ResourceId 构造函数
    UIImage(engine::core::Context& context, 
            engine::resource::ResourceId texture_id,
            const glm::vec2& position = {0.0f, 0.0f},
            const glm::vec2& size = {0.0f, 0.0f});
    
    ~UIImage() override;
    
    void render() override;
    
    // 透明度
    float getOpacity() const;
    void setOpacity(float opacity);
    
    // 精灵访问
    engine::render::Sprite& getSprite();
    const engine::render::Sprite& getSprite() const;
};
```

### 使用示例

```cpp
using namespace entt::literals;

// 字符串路径创建
auto icon = std::make_unique<UIImage>(
    context_, 
    "heart_icon", 
    glm::vec2(50, 50), 
    glm::vec2(32, 32)
);

// ResourceId 创建
ResourceId icon_id = engine::resource::toResourceId("heart_icon");
auto icon2 = std::make_unique<UIImage>(
    context_,
    icon_id,
    glm::vec2(50, 50),
    glm::vec2(32, 32)
);

icon->setOpacity(0.8f);
```

---

## UIState

**文件**: `src/engine/ui/state/ui_state.h`

UI 状态基类，用于实现状态模式。接口已重构，使用 owner 模式管理状态所属对象。

### 类定义

```cpp
namespace state {
    class UIState {
    protected:
        engine::ui::UIInteractive* owner_;  // 状态所属的 UIInteractive 对象

    public:
        UIState(engine::ui::UIInteractive* owner) : owner_(owner) {}
        virtual ~UIState() = default;
        
        virtual void enter() = 0;           // 进入状态时调用
        virtual void exit() = 0;            // 退出状态时调用
        virtual void update(float deltaTime) = 0;  // 状态更新时调用
        virtual std::unique_ptr<UIState> handleInput() = 0;  // 处理输入，返回新状态或 nullptr
    };
}
```

### 内置状态

| 状态 | 描述 |
|------|------|
| NormalState | 正常状态 |
| HoverState | 鼠标悬停状态 |
| PressedState | 按下状态 |

### 自定义状态示例

```cpp
class CustomState : public engine::ui::state::UIState {
public:
    CustomState(engine::ui::UIInteractive* owner) : UIState(owner) {}
    
    void enter() override {
        // 进入状态时的初始化
        if (auto* sprite = owner_->getSprite("custom"_hs)) {
            owner_->setCurrentSprite(sprite);
        }
    }
    
    void exit() override {
        // 退出状态时的清理
    }
    
    void update(float deltaTime) override {
        // 每帧更新逻辑
    }
    
    std::unique_ptr<UIState> handleInput() override {
        // 处理输入，根据需要返回新状态
        // 返回 nullptr 表示保持当前状态
        return nullptr;
    }
};

// 使用自定义状态
button->setState(std::make_unique<CustomState>(button.get()));
```

---

## 模块依赖图

```mermaid
graph TB
    UIManager --> UIElement
    UIElement --> UIInteractive
    UIInteractive --> UIButton
    UIElement --> UIPanel
    UIElement --> UIText
    UIElement --> UIImage
    
    UIInteractive --> UIState
    UIState --> NormalState
    UIState --> HoverState
    UIState --> PressedState
    
    UIText --> Observer
    
    ResourceId --> UIInteractive
    ResourceId --> UIText
    ResourceId --> UIImage
    
    style UIManager fill:#f9f,stroke:#333,stroke-width:2px
    style UIElement fill:#bbf,stroke:#333,stroke-width:2px
    style UIInteractive fill:#bbf,stroke:#333,stroke-width:2px
    style ResourceId fill:#bfb,stroke:#333,stroke-width:2px
```

## 最佳实践

1. **使用 UIManager**: 每个 Scene 应该有自己的 UIManager
2. **层级组织**: 使用 UIPanel 组织相关控件
3. **使用 ResourceId**: 对于频繁访问的精灵和声音资源，使用 ResourceId 可以提高性能
4. **事件处理**: 使用回调函数处理按钮点击
5. **数据绑定**: 使用 Observer 模式绑定 UI 和数据
6. **屏幕坐标**: UI 元素使用屏幕坐标，不受摄像机影响
7. **状态管理**: 使用 UIState 模式管理复杂的交互状态
8. **资源管理**: UI 资源（精灵、字体、声音）通过 ResourceManager 统一管理
