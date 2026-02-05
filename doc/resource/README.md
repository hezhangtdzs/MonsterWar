# Resource 资源管理模块

Resource 模块负责游戏所有资产（纹理、音频、字体）的生命周期管理，提供统一的资源加载、缓存和卸载接口。使用 ResourceId 系统基于字符串哈希实现高效的资源标识。

## 架构概览

```mermaid
graph TB
    ResourceManager[ResourceManager<br/>资源管理器]
    
    ResourceManager --> TextureManager[TextureManager<br/>纹理管理]
    ResourceManager --> AudioManager[AudioManager<br/>音频管理]
    ResourceManager --> FontManager[FontManager<br/>字体管理]
    
    TextureManager --> SDL3[SDL3 Texture]
    AudioManager --> SDL3_mixer[SDL3_mixer]
    FontManager --> SDL3_ttf[SDL3_ttf]
    
    ResourceId[ResourceId<br/>资源标识] --> ResourceManager
```

## 类概览

| 类名 | 描述 |
|------|------|
| [ResourceId](#resourceid) | 资源标识符类型，基于 entt::id_type |
| [ResourceManager](#resourcemanager) | 集中式资源管理器，统一接口 |
| [TextureManager](#texturemanager) | 纹理资源管理 |
| [AudioManager](#audiomanager) | 音频资源管理 |
| [FontManager](#fontmanager) | 字体资源管理 |

---

## ResourceId

**文件**: `src/engine/resource/resource_id.h`

资源标识符类型，使用 `entt::hashed_string` 对字符串进行哈希，提供高效的资源查找。

```cpp
using ResourceId = entt::id_type;

constexpr ResourceId InvalidResourceId = ResourceId{};

// 从字符串创建 ResourceId
constexpr ResourceId toResourceId(std::string_view key) noexcept {
    return entt::hashed_string{ key.data(), key.size() }.value();
}

// 获取类型ID
template<typename T>
constexpr ResourceId typeId() noexcept {
    return entt::type_id<T>().hash();
}
```

### 使用示例

```cpp
// 创建 ResourceId
ResourceId tex_id = engine::resource::toResourceId("assets/player.png");

// 使用 entt::hashed_string 字面量
using namespace entt::literals;
ResourceId id = "assets/enemy.png"_hs;

// 获取纹理（支持 ResourceId 和字符串）
SDL_Texture* tex1 = resource_manager->getTexture(tex_id);
SDL_Texture* tex2 = resource_manager->getTexture("assets/player.png");
```

---

## ResourceManager

**文件**: `src/engine/resource/resource_manager.h`

集中式资源管理器，通过封装具体的子管理器实现资源的统一加载、缓存和卸载。

### 类定义

```cpp
class ResourceManager {
public:
    explicit ResourceManager(SDL_Renderer* renderer);
    ~ResourceManager();
    
    // 清空所有资源
    void clear();
    
    // 从配置文件加载资源映射
    bool loadResources(std::string_view mapping_path, bool preload = true);
    
    // ========== 纹理接口 ==========
    SDL_Texture* loadTexture(ResourceId id, std::string_view file_path);
    SDL_Texture* loadTexture(const std::string& file_path);
    SDL_Texture* loadTexture(entt::hashed_string str_hs);
    
    SDL_Texture* getTexture(ResourceId id, std::string_view file_path = {});
    SDL_Texture* getTexture(const std::string& file_path);
    SDL_Texture* getTexture(entt::hashed_string str_hs);
    
    void unloadTexture(ResourceId id);
    void unloadTexture(const std::string& file_path);
    void unloadTexture(entt::hashed_string str_hs);
    
    glm::vec2 getTextureSize(ResourceId id, std::string_view file_path = {});
    glm::vec2 getTextureSize(const std::string& file_path);
    glm::vec2 getTextureSize(entt::hashed_string str_hs);
    
    void clearTextures();
    
    // ========== 音效接口 ==========
    MIX_Audio* loadSound(ResourceId id, std::string_view file_path);
    MIX_Audio* loadSound(const std::string& file_path);
    MIX_Audio* loadSound(entt::hashed_string str_hs);
    
    void playSound(ResourceId id, std::string_view file_path = {});
    void playSound(const std::string& file_path);
    void playSound(entt::hashed_string str_hs);
    void stopSound();
    
    MIX_Audio* getSound(ResourceId id, std::string_view file_path = {});
    MIX_Audio* getSound(const std::string& file_path);
    MIX_Audio* getSound(entt::hashed_string str_hs);
    
    void unloadSound(ResourceId id);
    void unloadSound(const std::string& file_path);
    void unloadSound(entt::hashed_string str_hs);
    
    void clearSounds();
    
    // ========== 音乐接口 ==========
    MIX_Audio* loadMusic(ResourceId id, std::string_view file_path);
    MIX_Audio* loadMusic(const std::string& file_path);
    MIX_Audio* loadMusic(entt::hashed_string str_hs);
    
    void playMusic(ResourceId id, std::string_view file_path = {});
    void playMusic(const std::string& file_path);
    void playMusic(entt::hashed_string str_hs);
    void stopMusic();
    
    MIX_Audio* getMusic(ResourceId id, std::string_view file_path = {});
    MIX_Audio* getMusic(const std::string& file_path);
    MIX_Audio* getMusic(entt::hashed_string str_hs);
    
    void unloadMusic(ResourceId id);
    void unloadMusic(const std::string& file_path);
    void unloadMusic(entt::hashed_string str_hs);
    
    void clearMusic();
    void cleanAudio();
    
    // ========== 音量控制 ==========
    void setMasterGain(float gain);
    void setSoundGain(float gain);
    void setMusicGain(float gain);
    
    // ========== 字体接口 ==========
    TTF_Font* loadFont(ResourceId id, std::string_view file_path, int point_size);
    TTF_Font* loadFont(const std::string& file_path, int point_size);
    TTF_Font* loadFont(entt::hashed_string str_hs, int point_size);
    
    TTF_Font* getFont(ResourceId id, std::string_view file_path, int point_size);
    TTF_Font* getFont(const std::string& file_path, int point_size);
    TTF_Font* getFont(entt::hashed_string str_hs, int point_size);
    
    void unloadFont(ResourceId id, int point_size);
    void unloadFont(const std::string& file_path, int point_size);
    void unloadFont(entt::hashed_string str_hs, int point_size);
    
    void clearFonts();
};
```

### 使用示例

```cpp
// 创建资源管理器
auto resource_manager = std::make_unique<ResourceManager>(sdl_renderer);

// 从配置文件加载资源映射
resource_manager->loadResources("assets/resources.json", true);

// 加载纹理（多种方式）
SDL_Texture* player_tex = resource_manager->loadTexture("assets/player.png");
SDL_Texture* enemy_tex = resource_manager->loadTexture(
    engine::resource::toResourceId("assets/enemy.png"), 
    "assets/enemy.png"
);

// 获取纹理尺寸
glm::vec2 size = resource_manager->getTextureSize("assets/player.png");

// 加载并播放音效
resource_manager->loadSound("assets/jump.wav");
resource_manager->playSound("assets/jump.wav");

// 使用 ResourceId 播放音效
ResourceId jump_id = engine::resource::toResourceId("assets/jump.wav");
resource_manager->playSound(jump_id);

// 播放音乐
resource_manager->playMusic("assets/bgm.mp3");  // -1 表示循环播放

// 设置音量
resource_manager->setMasterGain(0.8f);
resource_manager->setSoundGain(1.0f);
resource_manager->setMusicGain(0.5f);

// 加载字体
TTF_Font* font = resource_manager->loadFont("assets/arial.ttf", 24);

// 清空所有资源
resource_manager->clear();
```

---

## TextureManager

**文件**: `src/engine/resource/texture_manager.h`

负责 SDL 纹理资源的集中管理、加载与缓存。

### 类定义

```cpp
class TextureManager final {
public:
    explicit TextureManager(SDL_Renderer* renderer);
    ~TextureManager() = default;
    
    // 禁止拷贝和移动
    TextureManager(const TextureManager&) = delete;
    TextureManager& operator=(const TextureManager&) = delete;
    TextureManager(TextureManager&&) = delete;
    TextureManager& operator=(TextureManager&&) = delete;

private:
    SDL_Texture* loadTexture(ResourceId id, std::string_view file_path);
    SDL_Texture* loadTexture(const std::string& file_path);
    
    SDL_Texture* getTexture(ResourceId id, std::string_view file_path = {});
    SDL_Texture* getTexture(const std::string& file_path);
    
    void unloadTexture(ResourceId id);
    void unloadTexture(const std::string& file_path);
    
    glm::vec2 getTextureSize(ResourceId id, std::string_view file_path = {});
    glm::vec2 getTextureSize(const std::string& file_path);
    
    void clearTextures();
    
    friend class ResourceManager;
};
```

### 特性

- **自动缓存**: 相同路径的纹理只会加载一次
- **RAII 管理**: 使用 `std::unique_ptr` 自动管理纹理生命周期
- **延迟加载**: `getTexture` 会在纹理未加载时自动加载
- **ResourceId 支持**: 支持使用哈希 ID 快速访问资源

### 自定义删除器

```cpp
struct SDLTextureDeleter {
    void operator()(SDL_Texture* texture) const {
        if (texture) {
            SDL_DestroyTexture(texture);
        }
    }
};
```

---

## AudioManager

**文件**: `src/engine/resource/audio_manager.h`

音频管理器类，负责音频资源（音乐和音效）的加载、缓存及生命周期管理。

### 类定义

```cpp
class AudioManager final {
public:
    AudioManager();
    ~AudioManager();
    
    // 禁止拷贝和移动
    AudioManager(const AudioManager&) = delete;
    AudioManager& operator=(const AudioManager&) = delete;
    AudioManager(AudioManager&&) = delete;
    AudioManager& operator=(AudioManager&&) = delete;

private:
    // 音效
    MIX_Audio* loadSound(ResourceId id, std::string_view file_path);
    MIX_Audio* loadSound(const std::string& file_path);
    
    MIX_Audio* getSound(ResourceId id, std::string_view file_path = {});
    MIX_Audio* getSound(const std::string& file_path);
    
    void unloadSound(ResourceId id);
    void unloadSound(const std::string& file_path);
    void clearSounds();
    
    void playSound(ResourceId id, std::string_view file_path = {});
    void playSound(const std::string& file_path);
    void stopSound();
    
    // 音乐
    MIX_Audio* loadMusic(ResourceId id, std::string_view file_path);
    MIX_Audio* loadMusic(const std::string& file_path);
    
    MIX_Audio* getMusic(ResourceId id, std::string_view file_path = {});
    MIX_Audio* getMusic(const std::string& file_path);
    
    void unloadMusic(ResourceId id);
    void unloadMusic(const std::string& file_path);
    void clearMusic();
    
    void playMusic(ResourceId id, std::string_view file_path = {});
    void playMusic(const std::string& file_path);
    void stopMusic();
    
    void setMusicGain(float gain);
    void setSoundGain(float gain);
    void setMasterGain(float gain);
    
    void clearAudio();
    
    friend class ResourceManager;
};
```

### 自定义删除器

```cpp
struct MixerDeleter {
    void operator()(MIX_Mixer* m) const {
        if (m) MIX_DestroyMixer(m);
    }
};

struct MixAudioDeleter {
    void operator()(MIX_Audio* c) const {
        if (c) MIX_DestroyAudio(c);
    }
};

struct TrackDeleter {
    void operator()(MIX_Track* t) const {
        if (t) MIX_DestroyTrack(t);
    }
};
```

### 音频轨道

AudioManager 使用独立的轨道分别播放音乐和音效：

| 轨道 | 用途 |
|------|------|
| music_track_ | 背景音乐 (BGM) |
| sound_track_ | 音效 (SFX) |

---

## FontManager

**文件**: `src/engine/resource/font_manager.h`

字体资源管理器，负责 SDL3_ttf 的初始化与字体资源的集中管理。

### 类定义

```cpp
class FontManager final {
public:
    FontManager();
    ~FontManager();
    
    // 禁止拷贝和移动
    FontManager(const FontManager&) = delete;
    FontManager& operator=(const FontManager&) = delete;
    FontManager(FontManager&&) = delete;
    FontManager& operator=(FontManager&&) = delete;

private:
    TTF_Font* loadFont(ResourceId id, std::string_view file_path, int point_size);
    TTF_Font* loadFont(const std::string& file_path, int point_size);
    
    TTF_Font* getFont(ResourceId id, std::string_view file_path, int point_size);
    TTF_Font* getFont(const std::string& file_path, int point_size);
    
    void unloadFont(ResourceId id, int point_size);
    void unloadFont(const std::string& file_path, int point_size);
    void clearFonts();
    
    friend class ResourceManager;
};
```

### FontKey 结构

字体缓存使用组合键（资源ID + 字体大小）来唯一标识字体：

```cpp
struct FontKey {
    ResourceId id;      // 字体资源ID
    int point_size;     // 字体大小
    
    bool operator==(const FontKey& other) const {
        return id == other.id && point_size == other.point_size;
    }
};

struct FontKeyHasher {
    std::size_t operator()(const FontKey& key) const {
        return std::hash<ResourceId>()(key.id) ^ 
               std::hash<int>()(key.point_size);
    }
};
```

### 使用示例

```cpp
// 加载不同大小的同一字体
TTF_Font* font_small = resource_manager->loadFont("assets/arial.ttf", 16);
TTF_Font* font_medium = resource_manager->loadFont("assets/arial.ttf", 24);
TTF_Font* font_large = resource_manager->loadFont("assets/arial.ttf", 32);

// 使用 ResourceId 加载字体
ResourceId font_id = engine::resource::toResourceId("assets/arial.ttf");
TTF_Font* font = resource_manager->loadFont(font_id, "assets/arial.ttf", 24);

// 这三个字体会被分别缓存
```

---

## 资源加载流程

```mermaid
sequenceDiagram
    participant User as 调用者
    participant RM as ResourceManager
    participant TM as TextureManager
    participant Cache as 缓存
    participant Disk as 磁盘

    User->>RM: getTexture("player.png")
    RM->>TM: getTexture(id, "player.png")
    TM->>Cache: 查找缓存 (ResourceId)
    
    alt 缓存命中
        Cache-->>TM: 返回纹理指针
        TM-->>RM: 返回纹理指针
        RM-->>User: 返回纹理指针
    else 缓存未命中
        Cache-->>TM: 未找到
        TM->>Disk: 加载文件
        Disk-->>TM: 文件数据
        TM->>TM: 创建 SDL_Texture
        TM->>Cache: 存入缓存 (ResourceId -> Texture)
        TM-->>RM: 返回纹理指针
        RM-->>User: 返回纹理指针
    end
```

---

## 模块依赖图

```mermaid
graph TB
    ResourceManager --> TextureManager
    ResourceManager --> AudioManager
    ResourceManager --> FontManager
    
    TextureManager --> SDL3[SDL3]
    AudioManager --> SDL3_mixer[SDL3_mixer]
    FontManager --> SDL3_ttf[SDL3_ttf]
    
    ResourceId --> ResourceManager
    ResourceId --> TextureManager
    ResourceId --> AudioManager
    ResourceId --> FontManager
    
    style ResourceManager fill:#f9f,stroke:#333,stroke-width:2px
    style TextureManager fill:#bbf,stroke:#333,stroke-width:2px
    style AudioManager fill:#bbf,stroke:#333,stroke-width:2px
    style FontManager fill:#bbf,stroke:#333,stroke-width:2px
    style ResourceId fill:#bfb,stroke:#333,stroke-width:2px
```

## 最佳实践

1. **使用 ResourceId**: 对于频繁访问的资源，预先计算 ResourceId 可以提高性能
2. **配置驱动**: 使用 `loadResources()` 从 JSON 配置文件加载资源映射，便于管理
3. **统一路径**: 使用相对路径，以项目根目录为基准
4. **及时清理**: 场景切换时调用 `clear()` 释放不需要的资源
5. **预加载**: 在场景初始化时预加载所需资源，避免运行时卡顿
6. **音量控制**: 使用 `setMasterGain/setSoundGain/setMusicGain` 进行音量管理
