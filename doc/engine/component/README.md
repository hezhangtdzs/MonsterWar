# 组件模块 (Component Module)

## 概述

组件模块是 ECS 架构中的数据层，负责定义游戏中所有可复用的数据结构。每个组件都是纯数据结构（POD），只包含数据而不包含任何逻辑。这种设计使得数据可以高效地批量处理，同时也便于序列化和网络同步。

## 组件列表

| 组件 | 功能 | 依赖 | 所在文件 |
|------|------|------|----------|
| [TransformComponent](#transformcomponent) | 位置、旋转、缩放 | 无 | `transform_component.h` |
| [VelocityComponent](#velocitycomponent) | 线速度 | 无 | `velocity_component.h` |
| [SpriteComponent](#spritecomponent) | 精灵渲染数据 | 无 | `sprite_component.h` |
| [AnimationComponent](#animationcomponent) | 动画状态和数据 | 无 | `animation_component.h` |

## 设计原则

### 1. 纯数据结构

组件只包含数据，不包含任何业务逻辑：

```cpp
// 好的做法
struct TransformComponent {
    glm::vec2 position_;
    glm::vec2 scale_;
    float rotation_;
};

// 避免：在组件中添加逻辑
struct BadComponent {
    glm::vec2 position_;
    
    void move(const glm::vec2& delta) {  // 不要这样做
        position_ += delta;
    }
};
```

### 2. 默认可构造

组件应该可以默认构造，便于 EnTT 管理：

```cpp
struct Component {
    Component() = default;  // 默认构造函数
    explicit Component(...) { ... }  // 自定义构造函数
};
```

### 3. 可拷贝和可移动

组件应该是可拷贝和可移动的：

```cpp
struct Component {
    // 隐式生成拷贝和移动操作
    // 或使用 = default 显式声明
    Component(const Component&) = default;
    Component(Component&&) = default;
    Component& operator=(const Component&) = default;
    Component& operator=(Component&&) = default;
};
```

## TransformComponent

### 功能说明

TransformComponent 是 ECS 架构中最基础的组件，定义了实体在 2D 世界空间中的位置、旋转和缩放。

### 数据结构

```cpp
struct TransformComponent {
    glm::vec2 position_{ 0.0f, 0.0f };  // 位置（像素）
    glm::vec2 scale_{ 1.0f, 1.0f };     // 缩放（1.0 为原始大小）
    float rotation_{ 0.0f };             // 旋转角度（度）
};
```

### 使用场景

几乎所有需要渲染或具有物理位置的实体都需要此组件：

- 玩家角色
- NPC 和敌人
- 道具和物品
- 粒子效果

### 使用示例

```cpp
// 创建位于 (100, 200) 的实体
auto entity = registry.create();
registry.emplace<TransformComponent>(entity, 
    glm::vec2(100.0f, 200.0f),  // 位置
    glm::vec2(2.0f, 2.0f),      // 放大 2 倍
    45.0f                       // 旋转 45 度
);

// 获取和修改
auto& transform = registry.get<TransformComponent>(entity);
transform.position_ += glm::vec2(10.0f, 0.0f);  // 向右移动
transform.rotation_ += 90.0f;                    // 旋转
```

### 与其他组件的关系

```
TransformComponent + SpriteComponent → RenderSystem 渲染
TransformComponent + VelocityComponent → MovementSystem 更新位置
```

## VelocityComponent

### 功能说明

VelocityComponent 定义了实体的线速度，用于实现运动效果。

### 数据结构

```cpp
struct VelocityComponent {
    glm::vec2 velocity_{ 0.0f, 0.0f };  // 速度向量（像素/秒）
};
```

### 使用场景

- 移动的角色和敌人
- 飞行的投射物
- 飘动的粒子
- 移动的平台的

### 使用示例

```cpp
// 创建向右移动的实体
auto entity = registry.create();
registry.emplace<TransformComponent>(entity, glm::vec2(0.0f, 0.0f));
registry.emplace<VelocityComponent>(entity, glm::vec2(100.0f, 0.0f));

// MovementSystem 会自动更新位置
// position += velocity * delta_time
```

### 速度计算

```cpp
// 设置水平速度
velocity.velocity_.x = 200.0f;  // 向右 200 像素/秒

// 设置垂直速度
velocity.velocity_.y = -100.0f;  // 向上 100 像素/秒

// 设置对角线速度
velocity.velocity_ = glm::vec2(100.0f, 100.0f);  // 右下方向

// 停止移动
velocity.velocity_ = glm::vec2(0.0f, 0.0f);
```

## SpriteComponent

### 功能说明

SpriteComponent 定义了实体的视觉表现，包含纹理引用和渲染参数。

### 数据结构

```cpp
// Sprite 结构体（嵌套在 SpriteComponent 中）
struct Sprite {
    entt::id_type texture_id_;      // 纹理资源 ID
    std::string texture_path_;      // 纹理路径（可选）
    engine::utils::Rect src_rect_;  // 源矩形（裁剪区域）
    bool is_flipped_;               // 是否水平翻转
};

// SpriteComponent
struct SpriteComponent {
    Sprite sprite_;                 // 精灵数据
    glm::vec2 offset_{ 0.0f, 0.0f };  // 渲染偏移
    glm::vec2 size_{ 0.0f, 0.0f };    // 渲染大小
};
```

### 使用场景

- 玩家和角色的视觉表现
- 道具和物品的外观
- 背景元素
- 特效和粒子

### 使用示例

```cpp
// 创建精灵
engine::component::Sprite sprite(
    "assets/player.png",                    // 纹理路径
    engine::utils::Rect(0, 0, 32, 32)       // 源矩形（精灵图集中的位置）
);

// 创建带精灵的实体
auto entity = registry.create();
registry.emplace<TransformComponent>(entity, glm::vec2(100.0f, 100.0f));
registry.emplace<SpriteComponent>(entity, sprite);

// 或者使用纹理 ID（性能更好）
entt::id_type texture_id = "player_texture"_hs;
engine::component::Sprite sprite_by_id(
    texture_id,
    engine::utils::Rect(0, 0, 32, 32)
);
```

### 精灵图集使用

```cpp
// 从精灵图集中创建动画帧
std::vector<AnimationFrame> frames = {
    AnimationFrame(Rect(0, 0, 32, 32), 100),    // 第 1 帧
    AnimationFrame(Rect(32, 0, 32, 32), 100),   // 第 2 帧
    AnimationFrame(Rect(64, 0, 32, 32), 100),   // 第 3 帧
    AnimationFrame(Rect(96, 0, 32, 32), 100),   // 第 4 帧
};
```

### 水平翻转

```cpp
// 创建面向左的精灵
engine::component::Sprite sprite_left(
    "assets/player.png",
    engine::utils::Rect(0, 0, 32, 32),
    true  // 水平翻转
);

// 或者在运行时翻转
auto& sprite = registry.get<SpriteComponent>(entity).sprite_;
sprite.is_flipped_ = true;
```

## AnimationComponent

### 功能说明

AnimationComponent 管理实体的动画状态，支持多动画状态机（如 idle、walk、attack）。

### 数据结构

```cpp
// 动画帧
struct AnimationFrame {
    engine::utils::Rect src_rect_;  // 帧的源矩形
    float duration_ms_;             // 帧持续时间（毫秒）
};

// 单个动画
struct Animation {
    std::vector<AnimationFrame> frames_;  // 帧序列
    float total_duration_ms_;             // 总时长（自动计算）
    bool loop_;                           // 是否循环
};

// 动画组件
struct AnimationComponent {
    std::unordered_map<entt::id_type, Animation> animations_;  // 动画集合
    entt::id_type current_animation_id_;                       // 当前动画 ID
    size_t current_frame_index_;                               // 当前帧索引
    float current_time_ms_;                                    // 当前时间
    float speed_{1.0f};                                        // 播放速度
};
```

### 使用场景

- 角色行走/奔跑动画
- 攻击和技能动画
- 待机和呼吸动画
- 受伤和死亡动画

### 使用示例

```cpp
// 创建动画帧
std::vector<AnimationFrame> idle_frames = {
    AnimationFrame(Rect(0, 0, 32, 32), 200),
    AnimationFrame(Rect(32, 0, 32, 32), 200),
};

std::vector<AnimationFrame> walk_frames = {
    AnimationFrame(Rect(64, 0, 32, 32), 100),
    AnimationFrame(Rect(96, 0, 32, 32), 100),
    AnimationFrame(Rect(128, 0, 32, 32), 100),
    AnimationFrame(Rect(160, 0, 32, 32), 100),
};

// 创建动画集合
std::unordered_map<entt::id_type, Animation> animations;
animations["idle"_hs] = Animation(std::move(idle_frames), true);   // 循环
animations["walk"_hs] = Animation(std::move(walk_frames), true);   // 循环

// 创建带动画的实体
auto entity = registry.create();
registry.emplace<TransformComponent>(entity, glm::vec2(100.0f, 100.0f));
registry.emplace<SpriteComponent>(entity, Sprite("player.png", Rect(0, 0, 32, 32)));
registry.emplace<AnimationComponent>(entity, std::move(animations), "idle"_hs);
```

### 切换动画状态

```cpp
// 获取动画组件
auto& anim = registry.get<AnimationComponent>(entity);

// 切换到行走动画
anim.current_animation_id_ = "walk"_hs;
anim.current_frame_index_ = 0;
anim.current_time_ms_ = 0.0f;

// 调整播放速度（2 倍速）
anim.speed_ = 2.0f;

// 暂停动画
anim.speed_ = 0.0f;
```

### 动画事件

可以扩展 AnimationFrame 支持动画事件：

```cpp
struct AnimationFrame {
    engine::utils::Rect src_rect_;
    float duration_ms_;
    bool trigger_event_ = false;     // 是否触发事件
    std::string event_name_;          // 事件名称
};

// 使用示例
std::vector<AnimationFrame> attack_frames = {
    AnimationFrame(Rect(0, 64, 32, 32), 100),                    // 起手
    AnimationFrame(Rect(32, 64, 32, 32), 50, true, "hit"),       // 攻击判定帧
    AnimationFrame(Rect(64, 64, 32, 32), 100),                   // 收手
};
```

## 组件组合示例

### 玩家角色

```cpp
auto player = registry.create();

// 基础变换
registry.emplace<TransformComponent>(player, 
    glm::vec2(100.0f, 200.0f),  // 位置
    glm::vec2(2.0f, 2.0f),      // 放大 2 倍
    0.0f                         // 无旋转
);

// 移动能力
registry.emplace<VelocityComponent>(player, glm::vec2(0.0f, 0.0f));

// 视觉表现
registry.emplace<SpriteComponent>(player, 
    Sprite("player.png", Rect(0, 0, 32, 32))
);

// 动画
std::unordered_map<entt::id_type, Animation> anims;
anims["idle"_hs] = Animation({
    AnimationFrame(Rect(0, 0, 32, 32), 200),
    AnimationFrame(Rect(32, 0, 32, 32), 200),
}, true);
anims["walk"_hs] = Animation({
    AnimationFrame(Rect(64, 0, 32, 32), 100),
    AnimationFrame(Rect(96, 0, 32, 32), 100),
    AnimationFrame(Rect(128, 0, 32, 32), 100),
    AnimationFrame(Rect(160, 0, 32, 32), 100),
}, true);

registry.emplace<AnimationComponent>(player, std::move(anims), "idle"_hs);
```

### 静态道具

```cpp
auto item = registry.create();

// 只需要变换和精灵（不需要速度和动画）
registry.emplace<TransformComponent>(item, glm::vec2(300.0f, 150.0f));
registry.emplace<SpriteComponent>(item, 
    Sprite("coin.png", Rect(0, 0, 16, 16))
);
```

### 粒子效果

```cpp
// 创建多个粒子
for (int i = 0; i < 10; ++i) {
    auto particle = registry.create();
    
    registry.emplace<TransformComponent>(particle, 
        glm::vec2(100.0f, 100.0f)
    );
    
    // 随机速度
    float angle = random(0.0f, 2.0f * M_PI);
    float speed = random(50.0f, 100.0f);
    registry.emplace<VelocityComponent>(particle, 
        glm::vec2(cos(angle) * speed, sin(angle) * speed)
    );
    
    registry.emplace<SpriteComponent>(particle, 
        Sprite("particle.png", Rect(0, 0, 4, 4))
    );
}
```

## 添加新组件

要添加新组件，按照以下步骤：

### 1. 创建组件头文件

```cpp
// engine/component/my_component.h
#pragma once

namespace engine::component {

struct MyComponent {
    int value_;
    float another_value_;
    
    MyComponent() = default;
    explicit MyComponent(int value, float another) 
        : value_(value), another_value_(another) {}
};

} // namespace engine::component
```

### 2. 在场景中使用

```cpp
// 添加组件
registry.emplace<MyComponent>(entity, 42, 3.14f);

// 获取组件
auto& my = registry.get<MyComponent>(entity);
my.value_ = 100;
```

### 3. 创建处理系统（可选）

```cpp
// engine/system/my_system.h
class MySystem {
public:
    void update(entt::registry& registry, float delta_time);
};

// engine/system/my_system.cpp
void MySystem::update(entt::registry& registry, float delta_time) {
    auto view = registry.view<MyComponent>();
    for (auto entity : view) {
        auto& my = view.get<MyComponent>(entity);
        // 处理逻辑...
    }
}
```

## 性能考虑

### 1. 组件大小

保持组件小巧：

```cpp
// 好的做法：小组件
struct SmallComponent {
    glm::vec2 position_;  // 8 字节
    float rotation_;       // 4 字节
};  // 总共 12 字节

// 避免：大组件
struct LargeComponent {
    glm::vec2 position_;
    std::vector<int> data_;  // 动态分配，避免！
    std::string name_;        // 动态分配，避免！
};
```

### 2. 组件布局

EnTT 会自动优化组件的内存布局，但应该：

- 将经常一起访问的组件放在一起
- 避免频繁添加/移除组件
- 使用 `registry.view<>` 高效遍历

### 3. 调试信息

```cpp
// 打印组件信息
spdlog::debug("TransformComponent 大小: {} 字节", sizeof(TransformComponent));

// 检查实体组件
if (registry.all_of<TransformComponent, SpriteComponent>(entity)) {
    spdlog::debug("实体 {} 可以渲染", entt::to_integral(entity));
}
```

## 总结

组件模块是 ECS 架构的数据基础：

- **TransformComponent**：定义空间属性
- **VelocityComponent**：定义运动属性
- **SpriteComponent**：定义视觉属性
- **AnimationComponent**：定义动画属性

通过组合这些基础组件，可以创建各种游戏实体，而系统则负责处理这些组件的数据。
