# Render 渲染模块

Render 模块负责处理 2D 图形渲染，包括精灵绘制、摄像机管理、动画和文本渲染。

## 架构概览

```mermaid
graph TB
    Renderer[Renderer<br/>核心渲染器]
    Camera[Camera<br/>摄像机]
    
    Renderer --> Sprite[Sprite<br/>精灵数据]
    Renderer --> TextRenderer[TextRenderer<br/>文本渲染]
    
    Camera --> Renderer
    
    Animation[Animation<br/>动画数据] --> Sprite
```

## 类概览

| 类名 | 描述 |
|------|------|
| [Renderer](#renderer) | 核心渲染类，封装 SDL 渲染操作 |
| [Camera](#camera) | 摄像机类，负责视口管理和坐标转换 |
| [Sprite](#sprite) | 精灵类，表示可渲染的 2D 图像，支持 ResourceId |
| [Animation](#animation) | 动画类，管理动画帧和播放逻辑 |
| [TextRenderer](#textrenderer) | 文本渲染类，使用 SDL3_ttf，支持 ResourceId |

---

## Renderer

**文件**: `src/engine/render/renderer.h`

核心渲染类，负责封装 SDL 渲染操作和处理场景绘制逻辑。

### 类定义

```cpp
class Renderer final {
public:
    Renderer(SDL_Renderer* sdl_renderer, 
             engine::resource::ResourceManager* resource_manager);
    ~Renderer() = default;
    
    // 世界空间绘制
    void drawSprite(const Camera& camera,
                    const Sprite& sprite, 
                    const glm::vec2& position, 
                    const glm::vec2& scale = { 1.0f, 1.0f }, 
                    double angle = 0.0f);
    
    // 视差背景绘制
    void drawParallax(const Camera& camera,
                      const Sprite& sprite, 
                      const glm::vec2& position,
                      const glm::vec2& scroll_factor,
                      const glm::bvec2& repeat = { true, true },
                      const glm::vec2& scale = { 1.0f, 1.0f });
    
    // UI 空间绘制
    void drawUISprite(const Sprite& sprite, 
                      const glm::vec2& position, 
                      const std::optional<glm::vec2>& size = std::nullopt);
    
    // UI 矩形绘制
    void drawUIFilledRect(const engine::utils::Rect& rect, 
                          const engine::utils::FColor& color);
    void drawUIOutlineRect(const engine::utils::Rect& rect, 
                           const engine::utils::FColor& color);
    
    // 帧操作
    void present();
    void clearScreen();
    
    // 渲染状态
    void setDrawColor(Uint8 r, Uint8 g, Uint8 b, Uint8 a = 255);
    
    // 获取 SDL 渲染器
    SDL_Renderer* getSDLRenderer() const;

    // 禁用拷贝和移动语义
    Renderer(const Renderer&) = delete;
    Renderer& operator=(const Renderer&) = delete;
    Renderer(Renderer&&) = delete;
    Renderer& operator=(Renderer&&) = delete;

private:
    std::optional<SDL_FRect> getSpriteSrcRect(const Sprite& sprite);
    bool isRectInViewport(const Camera& camera, const SDL_FRect& rect);
};
```

### 绘制模式

```mermaid
graph LR
    subgraph WorldSpace[世界空间]
        WS1[drawSprite]
        WS2[drawParallax]
    end
    
    subgraph UISpace[UI空间]
        UI1[drawUISprite]
        UI2[drawUIFilledRect]
        UI3[drawUIOutlineRect]
    end
    
    WorldSpace -->|跟随摄像机| Camera
    UISpace -->|屏幕坐标| Screen
```

### 使用示例

```cpp
// 世界空间绘制
renderer.drawSprite(
    camera,
    sprite,
    glm::vec2(100.0f, 200.0f),  // 位置
    glm::vec2(2.0f, 2.0f),       // 缩放
    45.0f                        // 旋转角度
);

// 视差背景
renderer.drawParallax(
    camera,
    bg_sprite,
    glm::vec2(0.0f, 0.0f),
    glm::vec2(0.5f, 0.5f),       // 视差因子
    glm::bvec2(true, false)      // 重复设置
);

// UI 绘制
renderer.drawUISprite(
    ui_sprite,
    glm::vec2(50.0f, 50.0f),     // 屏幕坐标
    glm::vec2(100.0f, 50.0f)     // 目标大小
);

// 绘制填充矩形
renderer.drawUIFilledRect(
    {{10.0f, 10.0f}, {200.0f, 50.0f}},
    {1.0f, 0.0f, 0.0f, 0.5f}     // 半透明红色
);

// 设置绘制颜色
renderer.setDrawColor(255, 0, 0, 128);  // 半透明红色
```

---

## Camera

**文件**: `src/engine/render/camera.h`

摄像机类，负责视口管理、坐标转换和边界平滑移动。

### 类定义

```cpp
class Camera final {
public:
    Camera(const glm::vec2& viewport_size,
           const glm::vec2& position = {0.0f, 0.0f},
           const std::optional<engine::utils::Rect>& limit_bounds = std::nullopt);
    ~Camera();
    
    // 更新
    void update(float delta_time);
    
    // 移动
    void move(const glm::vec2& offset);
    void setPosition(const glm::vec2& position);
    const glm::vec2& getPosition() const;
    
    // 跟随目标
    void setTarget(engine::component::TransformComponent* target);
    engine::component::TransformComponent* getTarget() const;
    void setSmoothSpeed(float speed);
    float getSmoothSpeed() const;
    
    // 视口
    glm::vec2 getViewportSize() const;
    
    // 边界限制
    void setLimitBounds(const engine::utils::Rect& bounds);
    std::optional<engine::utils::Rect> getLimitBounds() const;
    
    // 坐标转换
    glm::vec2 worldToScreen(const glm::vec2& world_pos) const;
    glm::vec2 worldToScreenWithParallax(const glm::vec2& world_pos, 
                                        float scroll_factor) const;
    glm::vec2 worldToScreenWithParallax(const glm::vec2& world_pos, 
                                        const glm::vec2& scroll_factor) const;
    glm::vec2 screenToWorld(const glm::vec2& screen_pos) const;
    
    // 像素对齐
    void setPixelSnap(bool enabled);
    bool getPixelSnap() const;

    // 禁用拷贝和移动语义
    Camera(const Camera&) = delete;
    Camera& operator=(const Camera&) = delete;
    Camera(Camera&&) = delete;
    Camera& operator=(Camera&&) = delete;

private:
    void clampPosition();
};
```

### 坐标转换

```mermaid
graph LR
    World[世界坐标<br/>World Space] -->|worldToScreen| Screen[屏幕坐标<br/>Screen Space]
    Screen -->|screenToWorld| World
    
    World -->|worldToScreenWithParallax| Parallax[视差坐标<br/>Parallax Space]
```

### 使用示例

```cpp
// 创建摄像机
camera = std::make_unique<Camera>(
    glm::vec2(1280.0f, 720.0f),  // 视口大小
    glm::vec2(0.0f, 0.0f),        // 初始位置
    engine::utils::Rect{{0, 0}, {2000, 1000}}  // 边界限制
);

// 设置跟随目标
camera->setTarget(player_transform);
camera->setSmoothSpeed(5.0f);

// 获取跟随目标和平滑速度
auto* target = camera->getTarget();
float speed = camera->getSmoothSpeed();

// 每帧更新
camera->update(delta_time);

// 坐标转换
glm::vec2 screen_pos = camera->worldToScreen(world_pos);
glm::vec2 world_pos = camera->screenToWorld(mouse_pos);

// 设置像素对齐
camera->setPixelSnap(true);
```

---

## Sprite

**文件**: `src/engine/render/sprite.h`

表示一个可渲染的精灵对象，包含纹理引用、裁剪区域及翻转状态。使用 ResourceId 进行高效的纹理标识。

### 类定义

```cpp
class Sprite final {
private:
    engine::resource::ResourceId texture_id_{ engine::resource::InvalidResourceId };
    std::optional<std::string> texture_path_;
    std::optional<SDL_FRect> source_rect_;
    bool is_flipped_{ false };

public:
    Sprite() = default;
    
    // 使用 ResourceId 构造
    explicit Sprite(engine::resource::ResourceId texture_id,
                    const std::optional<SDL_FRect>& source_rect = std::nullopt,
                    bool is_flipped = false);
    
    // 使用纹理路径构造
    explicit Sprite(std::string_view texture_path,
                    const std::optional<SDL_FRect>& source_rect = std::nullopt,
                    bool is_flipped = false);
    
    ~Sprite() = default;
    
    // Getters
    engine::resource::ResourceId getTextureId() const;
    std::string_view getTexturePath() const;
    const std::optional<SDL_FRect>& getSourceRect() const;
    bool getIsFlipped() const;
    
    // Setters
    void setTextureId(engine::resource::ResourceId texture_id);
    void setTexturePath(std::string_view texture_path);
    void setSourceRect(const std::optional<SDL_FRect>& source_rect);
    void setIsFlipped(bool is_flipped);
};
```

### 使用示例

```cpp
using namespace entt::literals;

// 使用 ResourceId 创建（推荐）
Sprite full_sprite("player_texture"_hs);

// 使用路径创建
Sprite full_sprite2("assets/player.png");

// 裁剪纹理（精灵图）
SDL_FRect src_rect = {0, 0, 32, 32};
Sprite clipped_sprite("sprite_sheet", src_rect);

// 翻转精灵
Sprite flipped_sprite("player_texture"_hs, std::nullopt, true);

// 动态修改
sprite.setTextureId("new_texture"_hs);
sprite.setTexturePath("assets/new_texture.png");
sprite.setSourceRect(SDL_FRect{32, 0, 32, 32});
sprite.setIsFlipped(true);
```

---

## Animation

**文件**: `src/engine/render/animation.h`

动画类，管理一系列动画帧和动画播放逻辑。

### 类定义

```cpp
struct AnimationFrame {
    SDL_FRect src_rect;  // 纹理源矩形
    float duration;      // 帧持续时间（秒）
};

class Animation final {
public:
    Animation(const std::string& name = "default", bool loop = true);
    ~Animation() = default;
    
    // 帧管理
    void addFrame(const SDL_FRect& source_rect, float duration);
    const AnimationFrame& getFrame(float time) const;
    
    // 查询
    const std::string& getName() const;
    float getTotalDuration() const;
    bool isLooping() const;
    bool isEmpty() const;
};
```

### 使用示例

```cpp
// 创建行走动画
auto walk_anim = std::make_unique<Animation>("walk", true);

// 添加帧
walk_anim->addFrame({0, 0, 32, 32}, 0.1f);
walk_anim->addFrame({32, 0, 32, 32}, 0.1f);
walk_anim->addFrame({64, 0, 32, 32}, 0.1f);
walk_anim->addFrame({96, 0, 32, 32}, 0.1f);

// 获取当前帧
float anim_time = fmod(current_time, walk_anim->getTotalDuration());
const AnimationFrame& frame = walk_anim->getFrame(anim_time);
```

---

## TextRenderer

**文件**: `src/engine/render/text_renderer.h`

文本渲染类，负责使用 SDL3_ttf 渲染文本到屏幕。支持 ResourceId 进行字体资源标识。

### 渲染模式

| 模式 | 描述 |
|------|------|
| 世界文本 | 使用世界坐标，跟随相机移动 |
| UI 文本 | 使用屏幕坐标，不跟随相机 |

### 类定义

```cpp
class TextRenderer final {
public:
    TextRenderer(SDL_Renderer* sdl_renderer, 
                 engine::resource::ResourceManager* resource_manager);
    ~TextRenderer();
    
    // 世界空间文本（字符串路径）
    void drawText(const Camera& camera,
                  const std::string& text,
                  const std::string& font_path,
                  int font_size,
                  const glm::vec2& position,
                  const engine::utils::FColor& color);
    
    // 世界空间文本（ResourceId）
    void drawText(const Camera& camera,
                  const std::string& text,
                  engine::resource::ResourceId font_id,
                  std::string_view font_path,
                  int font_size,
                  const glm::vec2& position,
                  const engine::utils::FColor& color);
    
    // UI 空间文本（字符串路径）
    void drawUIText(const std::string& text,
                    const std::string& font_path,
                    int font_size,
                    const glm::vec2& position,
                    const engine::utils::FColor& color,
                    bool is_dirty = true);
    
    // UI 空间文本（ResourceId）
    void drawUIText(const std::string& text,
                    engine::resource::ResourceId font_id,
                    std::string_view font_path,
                    int font_size,
                    const glm::vec2& position,
                    const engine::utils::FColor& color,
                    bool is_dirty = true);
    
    // 右值引用版本
    void drawUIText(std::string&& text,
                    const std::string& font_path,
                    int font_size,
                    const glm::vec2& position,
                    const engine::utils::FColor& color);
    
    void drawUIText(std::string&& text,
                    engine::resource::ResourceId font_id,
                    std::string_view font_path,
                    int font_size,
                    const glm::vec2& position,
                    const engine::utils::FColor& color);
    
    // 获取文本尺寸（字符串路径）
    glm::vec2 getTextSize(const std::string& text, const std::string& font_path, int font_size);
    glm::vec2 getTextSize(const std::string& text, const std::string& font_path, int font_size, bool is_dirty);
    
    // 获取文本尺寸（ResourceId）
    glm::vec2 getTextSize(const std::string& text,
                          engine::resource::ResourceId font_id,
                          std::string_view font_path,
                          int font_size);
    
    glm::vec2 getTextSize(const std::string& text,
                          engine::resource::ResourceId font_id,
                          std::string_view font_path,
                          int font_size,
                          bool is_dirty);

    // 禁用拷贝和移动语义
    TextRenderer(const TextRenderer&) = delete;
    TextRenderer& operator=(const TextRenderer&) = delete;
    TextRenderer(TextRenderer&&) = delete;
    TextRenderer& operator=(TextRenderer&&) = delete;

private:
    TTF_Text* getTTFText(const std::string& text);
    TTF_Text* createTTFText(const std::string& text, TTF_Font* font);
};
```

### 使用示例

```cpp
using namespace entt::literals;

// 世界空间文本（如漂浮伤害数字）
text_renderer.drawText(
    camera,
    "-25",
    "assets/fonts/arial.ttf",
    24,
    glm::vec2(100.0f, 200.0f),
    {1.0f, 0.0f, 0.0f, 1.0f}  // 红色
);

// 使用 ResourceId 绘制世界文本
ResourceId font_id = engine::resource::toResourceId("assets/fonts/arial.ttf");
text_renderer.drawText(
    camera,
    "Score: 1000",
    font_id,
    "assets/fonts/arial.ttf",
    32,
    glm::vec2(100.0f, 100.0f),
    {1.0f, 1.0f, 1.0f, 1.0f}
);

// UI 文本
text_renderer.drawUIText(
    "Score: 1000",
    "assets/fonts/arial.ttf",
    32,
    glm::vec2(50.0f, 50.0f),  // 屏幕坐标
    {1.0f, 1.0f, 1.0f, 1.0f}  // 白色
);

// 获取文本尺寸
glm::vec2 text_size = text_renderer.getTextSize("Hello World", "assets/fonts/arial.ttf", 24);
```

---

## 渲染流程

```mermaid
sequenceDiagram
    participant App as GameApp
    participant SM as SceneManager
    participant Scene as Scene
    participant GO as GameObject
    participant Comp as Component
    participant Renderer as Renderer
    participant SDL as SDL3

    App->>SM: render()
    SM->>Scene: render()
    Scene->>Renderer: clearScreen()
    
    loop 遍历所有 GameObject
        Scene->>GO: render(context)
        GO->>Comp: render(context)
        Comp->>Renderer: drawSprite(...)
        Renderer->>SDL: SDL_RenderTexture()
    end
    
    Scene->>Renderer: present()
    Renderer->>SDL: SDL_RenderPresent()
```

---

## 模块依赖图

```mermaid
graph TB
    Renderer --> ResourceManager
    Renderer --> Camera
    Renderer --> Sprite
    Renderer --> SDL3
    
    TextRenderer --> ResourceManager
    TextRenderer --> Camera
    TextRenderer --> SDL3_ttf
    
    Camera --> Transform
    
    Animation --> Sprite
    
    Sprite --> ResourceId
    
    style Renderer fill:#f9f,stroke:#333,stroke-width:2px
    style Camera fill:#bbf,stroke:#333,stroke-width:2px
```

## 最佳实践

1. **使用 ResourceId**: 对于频繁使用的纹理，使用 ResourceId 可以提高性能
2. **精灵图**: 使用 Animation 和 Sprite 的 source_rect 实现精灵图动画
3. **视差效果**: 使用 drawParallax 创建视差滚动背景
4. **UI 分离**: 世界空间绘制和 UI 空间绘制使用不同的方法
5. **摄像机跟随**: 使用 Camera 的 setTarget 实现平滑的摄像机跟随
6. **像素对齐**: 启用 pixel_snap 避免子像素渲染导致的模糊
