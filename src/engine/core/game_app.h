#pragma once
/**
 * @file game_app.h
 * @brief 定义 GameApp 类，游戏应用的核心类，负责初始化、运行和管理游戏循环。
 */

#include <memory>
#include <functional>
// Forward declarations in global namespace
struct SDL_Renderer;
struct SDL_Window;

namespace engine::resource {
    class ResourceManager;
}
namespace engine::render {
    class Renderer;
    class TextRenderer;
    class Camera;
}
namespace engine::input {
    class InputManager;
}   
namespace engine::scene
{
    class SceneManager;
}
namespace engine::physics
{
    class PhysicsEngine;
}
namespace engine::audio
{
    class IAudioPlayer;
}
namespace engine::core {
    class Time;
    class Config;
    class Context;
    class GameState;

    /**
     * @class GameApp
     * @brief 游戏应用的核心类，负责初始化所有系统、运行游戏主循环和管理游戏状态。
     * 
     * GameApp 类是整个游戏的入口点，它负责：
     * - 初始化 SDL 和其他游戏系统
     * - 创建和管理游戏资源
     * - 运行游戏主循环（事件处理、更新、渲染）
     * - 处理游戏退出和资源清理
     */
    class GameApp final{
    private:
        /// 游戏运行状态标志
        bool is_running_{false};
        /// SDL 渲染器指针
        SDL_Renderer* sdl_renderer_{nullptr};
        /// SDL 窗口指针
        SDL_Window* window_{nullptr};

        /// 时间管理器
        std::unique_ptr<engine::core::Time> time_;
        /// 资源管理器
        std::unique_ptr<engine::resource::ResourceManager> resource_manager_;
        /// 渲染器
        std::unique_ptr<engine::render::Renderer> renderer_;
        /// 文本渲染器
        std::unique_ptr<engine::render::TextRenderer> text_renderer_;
        /// 摄像机
        std::unique_ptr<engine::render::Camera> camera_;
        /// 配置管理器
        std::unique_ptr<engine::core::Config> config_;
        /// 输入管理器
        std::unique_ptr<engine::input::InputManager> input_manager_;
        /// 游戏上下文
        std::unique_ptr<engine::core::Context> context_;
        /// 场景管理器
        std::unique_ptr<engine::scene::SceneManager> scene_manager_;
        /// 物理引擎
        std::unique_ptr<engine::physics::PhysicsEngine> physics_engine_;
        /// 音频播放器
        std::unique_ptr<engine::audio::IAudioPlayer> audio_player_;
        /// 游戏状态
        std::unique_ptr<GameState> game_state_;
        /// 初始化回调函数
        std::function<void(engine::scene::SceneManager&)> on_init_;

    public:
        /**
         * @brief 构造函数，初始化游戏应用。
         */
        GameApp();
        
        /**
         * @brief 析构函数，清理游戏资源。
         */
        ~GameApp();
        
        /// 禁止拷贝构造和移动
        GameApp(const GameApp&) = delete;
        GameApp& operator=(const GameApp&) = delete;
        GameApp(GameApp&&) = delete;
        GameApp& operator=(GameApp&&) = delete;

        /**
         * @brief 启动游戏主循环。
         */
        void run();
        
        /**
         * @brief 设置初始化回调函数。
         * @param callback 初始化回调函数，接收 SceneManager 引用。
         */
        void setOnInitCallback(std::function<void(engine::scene::SceneManager&)> callback);
        
    private:
        /**
         * @brief 初始化所有游戏系统。
         * @return bool 初始化成功返回 true，否则返回 false。
         */
        [[nodiscard]] bool init();
        
        /**
         * @brief 处理 SDL 事件。
         */
        void handleEvents();
        
        /**
         * @brief 更新游戏状态。
         * @param delta_time 帧间时间间隔（秒）。
         */
        void update(float& delta_time);
        
        /**
         * @brief 渲染游戏画面。
         */
        void render();
        
        /**
         * @brief 关闭游戏，清理资源。
         */
        void close();

        /**
         * @brief 初始化配置系统。
         * @return bool 初始化成功返回 true，否则返回 false。
         */
        [[nodiscard]] bool initConfig();
        
        /**
         * @brief 初始化 SDL 系统。
         * @return bool 初始化成功返回 true，否则返回 false。
         */
        [[nodiscard]] bool initSDL();
        
        /**
         * @brief 初始化时间系统。
         * @return bool 初始化成功返回 true，否则返回 false。
         */
        [[nodiscard]] bool initTime();
        
        /**
         * @brief 初始化资源管理器。
         * @return bool 初始化成功返回 true，否则返回 false。
         */
        [[nodiscard]] bool initResourceManager();
        
        /**
         * @brief 初始化渲染器。
         * @return bool 初始化成功返回 true，否则返回 false。
         */
        [[nodiscard]] bool initRenderer();
        
        /**
         * @brief 初始化文本渲染器。
         * @return bool 初始化成功返回 true，否则返回 false。
         */
        [[nodiscard]] bool initTextRenderer();
        
        /**
         * @brief 初始化摄像机。
         * @return bool 初始化成功返回 true，否则返回 false。
         */
        [[nodiscard]] bool initCamera();
        
        /**
         * @brief 初始化输入管理器。
         * @return bool 初始化成功返回 true，否则返回 false。
         */
        [[nodiscard]] bool initInputManager();
        
        /**
         * @brief 初始化游戏上下文。
         * @return bool 初始化成功返回 true，否则返回 false。
         */
        [[nodiscard]] bool initContext();
        
        /**
         * @brief 初始化场景管理器。
         * @return bool 初始化成功返回 true，否则返回 false。
         */
        [[nodiscard]] bool initSceneManager();
        
        /**
         * @brief 初始化物理引擎。
         * @return bool 初始化成功返回 true，否则返回 false。
         */
        [[nodiscard]] bool initPhysicsEngine();
        
        /**
         * @brief 初始化音频播放器。
         * @return bool 初始化成功返回 true，否则返回 false。
         */
        [[nodiscard]] bool initAudioPlayer();
        
        /**
         * @brief 初始化游戏状态。
         * @return bool 初始化成功返回 true，否则返回 false。
         */
        [[nodiscard]] bool initGameState();

    };
}