# 系统模块 (System Module)

> **版本**: 1.0.0  
> **最后更新**: 2026-02-15  
> **相关文档**: [组件模块](../component/README.md) | [ECS 架构](../../ECS_ARCHITECTURE.md)

系统模块是 ECS 架构中的核心部分，负责处理具有特定组件组合的实体的逻辑。每个系统都有明确的职责，通过遍历 EnTT 注册表中的实体来执行相应的操作。

---

## 目录

- [系统列表](#系统列表)
- [MovementSystem](#movementsystem)
- [RenderSystem](#rendersystem)
- [AnimationSystem](#animationsystem)
- [YSortSystem](#ysortsystem)
- [系统执行顺序](#系统执行顺序)
- [添加新系统](#添加新系统)
- [调试技巧](#调试技巧)

---

## 系统列表

| 系统 | 功能 | 处理的组件 | 更新频率 |
|------|------|------------|----------|
| [MovementSystem](#movementsystem) | 更新实体位置 | VelocityComponent + TransformComponent | 每帧 |
| [RenderSystem](#rendersystem) | 渲染实体 | TransformComponent + SpriteComponent + RenderComponent | 每帧（渲染阶段） |
| [AnimationSystem](#animationsystem) | 更新动画状态 | AnimationComponent + SpriteComponent | 每帧 |
| [YSortSystem](#ysortsystem) | Y轴排序更新 | TransformComponent + RenderComponent | 每帧 |

## MovementSystem

### 功能说明

MovementSystem 负责根据实体的速度更新其位置，实现基本的运动逻辑。

### 处理流程

```
┌─────────────────────────────────────────────────────────────┐
│                     MovementSystem                          │
├─────────────────────────────────────────────────────────────┤
│  1. 查询具有 VelocityComponent 和 TransformComponent 的实体  │
│                      ↓                                       │
│  2. 读取 VelocityComponent.velocity_                        │
│                      ↓                                       │
│  3. 更新 TransformComponent.position_                       │
│     position += velocity * delta_time                       │
└─────────────────────────────────────────────────────────────┘
```

### 代码示例

```cpp
// 系统更新
void MovementSystem::update(entt::registry& registry, float delta_time) {
    auto view = registry.view<VelocityComponent, TransformComponent>();
    
    for (auto entity : view) {
        const auto& velocity = view.get<VelocityComponent>(entity);
        auto& transform = view.get<TransformComponent>(entity);
        
        // 欧拉积分更新位置
        transform.position_ += velocity.velocity_ * delta_time;
    }
}
```

### 使用场景

- 玩家角色移动
- NPC 自动巡逻
- 子弹/投射物飞行
- 粒子效果运动

### 扩展建议

可以通过添加以下功能增强：

1. **边界检查**：防止实体移出游戏世界
```cpp
// 在 update 中添加
if (transform.position_.x < 0) transform.position_.x = 0;
if (transform.position_.x > world_width) transform.position_.x = world_width;
```

2. **加速度支持**：添加 AccelerationComponent 实现更复杂的运动
```cpp
// 新的组件
struct AccelerationComponent {
    glm::vec2 acceleration_;
};

// 在系统中先更新速度，再更新位置
velocity.velocity_ += acceleration.acceleration_ * delta_time;
transform.position_ += velocity.velocity_ * delta_time;
```

3. **摩擦力/阻力**：模拟真实物理
```cpp
// 添加阻力系数
velocity.velocity_ *= (1.0f - friction * delta_time);
```

## RenderSystem

### 功能说明

RenderSystem 负责渲染所有具有可视组件的实体，将游戏世界呈现到屏幕上。

### 处理流程

```
┌─────────────────────────────────────────────────────────────┐
│                      RenderSystem                           │
├─────────────────────────────────────────────────────────────┤
│  1. 查询具有 TransformComponent 和 SpriteComponent 的实体    │
│                      ↓                                       │
│  2. 计算渲染参数                                            │
│     - 位置: transform.position + sprite.offset              │
│     - 大小: sprite.size * transform.scale                   │
│     - 旋转: transform.rotation                              │
│                      ↓                                       │
│  3. 调用 Renderer 绘制精灵                                   │
└─────────────────────────────────────────────────────────────┘
```

### 代码示例

```cpp
void RenderSystem::update(entt::registry& registry, 
                          Renderer& renderer, 
                          const Camera& camera) {
    auto view = registry.view<TransformComponent, SpriteComponent>();
    
    for (auto entity : view) {
        const auto& transform = view.get<TransformComponent>(entity);
        const auto& sprite = view.get<SpriteComponent>(entity);
        
        // 计算最终渲染参数
        auto position = transform.position_ + sprite.offset_;
        auto size = sprite.size_ * transform.scale_;
        
        // 提交渲染
        renderer.drawSprite(camera, sprite.sprite_, 
                           position, size, transform.rotation_);
    }
}
```

### 渲染顺序

目前按实体在 EnTT 注册表中的顺序渲染。如需控制渲染层级，可考虑：

1. **添加 ZComponent 存储深度值**
```cpp
struct ZComponent {
    float z_index_;  // 深度值，越小越先渲染
};
```

2. **使用 registry.sort() 排序**
```cpp
registry.sort<SpriteComponent>([](const auto& lhs, const auto& rhs) {
    return lhs.z_index_ < rhs.z_index_;
});
```

3. **多遍渲染**
```cpp
// 第一遍：渲染背景层
renderLayer(registry, renderer, camera, Layer::BACKGROUND);

// 第二遍：渲染游戏对象
renderLayer(registry, renderer, camera, Layer::GAME_OBJECTS);

// 第三遍：渲染前景层
renderLayer(registry, renderer, camera, Layer::FOREGROUND);
```

### 性能优化

1. **视口裁剪**：只渲染在摄像机视口内的实体
2. **批量渲染**：合并使用相同纹理的渲染调用
3. **LOD（细节层次）**：远处实体使用低分辨率纹理

## AnimationSystem

### 功能说明

AnimationSystem 负责更新实体的动画状态，根据时间推进动画播放并更新精灵的源矩形。它支持循环动画和单次播放动画，并通过事件系统与其他模块交互。

**事件交互**：
- 监听 `PlayAnimationEvent` 事件来切换动画
- 当一个**非循环**动画播放结束时，系统会向 `entt::dispatcher` 发送 `AnimationFinishedEvent` 信号（包含实体 ID 和动画 ID）

### 类定义

```cpp
class AnimationSystem {
public:
    AnimationSystem(entt::registry& registry, entt::dispatcher& dispatcher);
    ~AnimationSystem();
    
    void update(float dt);

private:
    void onPlayAnimationEvent(const engine::utils::PlayAnimationEvent& event);
    
    entt::registry& registry_;
    entt::dispatcher& dispatcher_;
};
```

### 处理流程

```
┌─────────────────────────────────────────────────────────────┐
│                     AnimationSystem                         │
├─────────────────────────────────────────────────────────────┤
│  1. 查询具有 AnimationComponent 和 SpriteComponent 的实体    │
│                      ↓                                       │
│  2. 累加时间                                                │
│     current_time += delta_time * 1000 * speed               │
│                      ↓                                       │
│  3. 检查是否需要切换帧                                       │
│     if (current_time >= frame_duration)                     │
│                      ↓                                       │
│  4. 更新帧索引                                              │
│     current_frame++                                         │
│     if (current_frame >= total_frames)                      │
│         if (loop)                                           │
│             current_frame = 0                               │
│         else                                                │
│             current_frame = last_frame                      │
│             发送 AnimationFinishedEvent 事件                 │
│                      ↓                                       │
│  5. 更新 SpriteComponent 的源矩形                            │
│     sprite.src_rect = current_frame.rect                    │
└─────────────────────────────────────────────────────────────┘
```

### 代码示例

```cpp
void AnimationSystem::update(entt::registry& registry, float dt) {
    auto view = registry.view<AnimationComponent, SpriteComponent>();
    
    for (auto entity : view) {
        auto& anim = view.get<AnimationComponent>(entity);
        auto& sprite = view.get<SpriteComponent>(entity);
        
        // 获取当前动画
        auto it = anim.animations_.find(anim.current_animation_id_);
        if (it == anim.animations_.end()) continue;
        
        auto& animation = it->second;
        if (animation.frames_.empty()) continue;
        
        // 累加时间
        anim.current_time_ms_ += dt * 1000.0f * anim.speed_;
        
        // 检查是否需要切换帧
        const auto& current_frame = animation.frames_[anim.current_frame_index_];
        if (anim.current_time_ms_ >= current_frame.duration_ms_) {
            anim.current_time_ms_ -= current_frame.duration_ms_;
            anim.current_frame_index_++;
            
            // 处理循环
            if (anim.current_frame_index_ >= animation.frames_.size()) {
                if (animation.loop_) {
                    anim.current_frame_index_ = 0;
                } else {
                    anim.current_frame_index_ = animation.frames_.size() - 1;
                }
            }
        }
        
        // 更新精灵的源矩形
        const auto& frame = animation.frames_[anim.current_frame_index_];
        sprite.sprite_.src_rect_ = frame.src_rect_;
    }
}
```

### 动画状态切换

可以通过修改 AnimationComponent 来切换动画状态：

```cpp
// 切换到行走动画
auto& anim = registry.get<AnimationComponent>(entity);
anim.current_animation_id_ = "walk"_hs;
anim.current_frame_index_ = 0;
anim.current_time_ms_ = 0.0f;
```

### 动画事件

可以扩展系统支持动画事件（如攻击判定帧）：

```cpp
// 在 AnimationFrame 中添加事件标记
struct AnimationFrame {
    Rect src_rect_;
    float duration_ms_;
    bool trigger_event_;  // 是否触发事件
    std::string event_name_;  // 事件名称
};

// 在系统中触发事件
if (frame.trigger_event_) {
    dispatcher.trigger<AnimationEvent>(entity, frame.event_name_);
}
```

## 系统执行顺序

正确的系统执行顺序很重要：

```cpp
void GameScene::update(float delta_time) {
    // 1. 输入处理（可能修改 Velocity）
    input_system_->update(registry_, delta_time);
    
    // 2. AI 更新（可能修改 Velocity）
    ai_system_->update(registry_, delta_time);
    
    // 3. 移动系统（根据 Velocity 更新 Position）
    movement_system_->update(registry_, delta_time);
    
    // 4. 碰撞检测（根据 Position 检测碰撞）
    collision_system_->update(registry_, delta_time);
    
    // 5. 动画系统（更新动画帧）
    animation_system_->update(registry_, delta_time);
}

void GameScene::render() {
    // 6. 渲染系统（根据最终状态渲染）
    render_system_->update(registry_, renderer_, camera_);
}
```

## 添加新系统

要添加新系统，按照以下步骤：

### 1. 创建系统头文件

```cpp
// engine/system/my_system.h
#pragma once
#include <entt/entity/registry.hpp>

namespace engine::system {

class MySystem {
public:
    void update(entt::registry& registry, float delta_time);
};

} // namespace engine::system
```

### 2. 实现系统

```cpp
// engine/system/my_system.cpp
#include "my_system.h"
#include "../component/my_component.h"

namespace engine::system {

void MySystem::update(entt::registry& registry, float delta_time) {
    auto view = registry.view<MyComponent>();
    
    for (auto entity : view) {
        auto& component = view.get<MyComponent>(entity);
        // 处理逻辑...
    }
}

} // namespace engine::system
```

### 3. 在场景中使用

```cpp
// game/scene/my_scene.h
#include "engine/system/my_system.h"

class MyScene : public engine::scene::Scene {
private:
    std::unique_ptr<engine::system::MySystem> my_system_;
    
public:
    void init() override {
        my_system_ = std::make_unique<engine::system::MySystem>();
    }
    
    void update(float delta_time) override {
        my_system_->update(registry_, delta_time);
    }
};
```

### 4. 更新前向声明文件

```cpp
// engine/system/fwd.h
namespace engine::system {
class RenderSystem;
class AnimationSystem;
class MovementSystem;
class MySystem;  // 添加新系统的前向声明
} // namespace engine::system
```

## 调试技巧

### 1. 打印系统处理的实体数量

```cpp
void MySystem::update(entt::registry& registry, float delta_time) {
    auto view = registry.view<MyComponent>();
    spdlog::debug("MySystem 处理 {} 个实体", view.size());
    
    for (auto entity : view) {
        // 处理...
    }
}
```

### 2. 检查特定实体

```cpp
// 检查实体是否有特定组件
if (registry.all_of<TransformComponent>(entity)) {
    auto& transform = registry.get<TransformComponent>(entity);
    spdlog::debug("实体 {} 位置: ({}, {})", 
        entt::to_integral(entity), 
        transform.position_.x, 
        transform.position_.y);
}
```

### 3. 性能分析

```cpp
// 测量系统执行时间
auto start = std::chrono::high_resolution_clock::now();

movement_system_->update(registry_, delta_time);

auto end = std::chrono::high_resolution_clock::now();
auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
spdlog::debug("MovementSystem 耗时: {} μs", duration.count());
```

## 总结

系统模块是 ECS 架构的核心，每个系统都有明确的职责：

- **MovementSystem**：处理实体运动，基于速度更新位置
- **RenderSystem**：渲染游戏世界，支持层级排序
- **AnimationSystem**：管理动画播放，支持事件驱动
- **YSortSystem**：Y轴排序，实现2.5D深度效果

通过组合不同的系统，可以实现复杂的游戏逻辑，同时保持代码的清晰和可维护性。

## YSortSystem

### 功能说明

YSortSystem 负责根据实体的 Y 轴位置更新其渲染顺序，实现 2D 游戏中的 Y 轴排序效果（也称为 2.5D 效果）。这使得 Y 坐标较大的实体（在屏幕下方）会渲染在 Y 坐标较小的实体（在屏幕上方）之上。

### 类定义

```cpp
class YSortSystem {
public:
    YSortSystem() = default;
    
    void update(entt::registry& registry);
};
```

### 处理流程

```
┌─────────────────────────────────────────────────────────────┐
│                       YSortSystem                           │
├─────────────────────────────────────────────────────────────┤
│  1. 查询具有 TransformComponent 和 RenderComponent 的实体    │
│                      ↓                                       │
│  2. 读取 TransformComponent.position_.y                     │
│                      ↓                                       │
│  3. 更新 RenderComponent.y_index_                           │
│     y_index = position.y                                    │
└─────────────────────────────────────────────────────────────┘
```

### 代码示例

```cpp
void YSortSystem::update(entt::registry& registry) {
    auto view = registry.view<TransformComponent, RenderComponent>();
    
    for (auto entity : view) {
        const auto& transform = view.get<TransformComponent>(entity);
        auto& render = view.get<RenderComponent>(entity);
        
        // 更新 Y 轴索引为实体的 Y 坐标
        render.y_index_ = transform.position_.y;
    }
}
```

### 使用场景

- 2D 角色与背景的遮挡关系
- 2.5D 视角游戏中的深度排序
- 任何需要根据 Y 坐标确定渲染顺序的场景

### 与 RenderComponent 配合

```cpp
// 创建需要 Y 排序的实体
auto entity = registry.create();
registry.emplace<TransformComponent>(entity, glm::vec2(100.0f, 200.0f));
registry.emplace<SpriteComponent>(entity, sprite);
registry.emplace<RenderComponent>(entity, 1, 200.0f);  // layer=1, y_index 初始值

// YSortSystem 会自动更新 y_index
// RenderSystem 会根据 layer 和 y_index 排序后渲染
```

### 渲染排序规则

```
排序优先级：
1. layer_index_ 较小的先渲染（底层）
2. 同一 layer 内，y_index_ 较小的先渲染（上方）

示例：
┌────────────────────────────────┐
│  layer=0 (背景层)              │  ← 最先渲染
├────────────────────────────────┤
│  layer=1, y=100 (角色A)        │
│  layer=1, y=200 (角色B)        │  ← 后渲染（在上层）
│  layer=1, y=300 (角色C)        │
├────────────────────────────────┤
│  layer=100 (UI层)              │  ← 最后渲染
└────────────────────────────────┘
```
