#pragma once

#include <SDL3/SDL_render.h>
#include <SDL3/SDL_video.h>
#include <glm/glm.hpp>

namespace engine::core {

    enum class GameStateType {
        Title,
        Playing,
        Paused,
        GameOver,
    };
    /**
     * @class GameState
     * @brief 游戏状态管理类，负责管理游戏的状态和逻辑。
     * 
     * GameState 类提供了对游戏状态的统一管理，包括游戏循环、状态转换、
     * 事件处理和渲染等。游戏状态数据存储在 JSON 文件中，支持从文件加载和保存到文件。
     */
    class GameState final {
        SDL_Renderer* renderer_ = nullptr;
        GameStateType current_state_ = GameStateType::Title;
        SDL_Window* window_ = nullptr;
    public:
        /**
         * @brief 构造函数，指定游戏状态文件路径。
         * @param filepath 游戏状态文件路径
         */
        explicit GameState(SDL_Renderer* renderer, SDL_Window* window,GameStateType initial_state = GameStateType::Title);
        ~GameState();
        /**
         * @brief 获取当前游戏状态。
         * @return 当前游戏状态
         */
        GameStateType getState() const;
        /**
         * @brief 设置当前游戏状态。
         * @param state 新的游戏状态
         */
        void setState(GameStateType state);

        /**
         * @brief 检查游戏是否正在进行中。
         * @return 如果游戏正在进行中则返回 true，否则返回 false
         */
        bool isPlaying() const;
        /**
         * @brief 检查游戏是否暂停。
         * @return 如果游戏暂停则返回 true，否则返回 false
         */
        bool isPaused() const;
        /**
         * @brief 检查游戏是否结束。
         * @return 如果游戏结束则返回 true，否则返回 false
         */
        bool isGameOver() const;
        /**
         * @brief 获取窗口大小。
         * @return 窗口大小
         */
        glm::vec2 getWindowSize() const;
        /**
         * @brief 设置窗口大小。
         * @param size 窗口大小
         */
        void setWindowSize(glm::vec2 size);
        /**
         * @brief 获取窗口逻辑大小。
         * @return 窗口逻辑大小
         */
        glm::vec2 getWindowLogicalSize() const;
        /**
         * @brief 设置窗口逻辑大小。
         * @param size 窗口逻辑大小
         */
        void setWindowLogicalSize(glm::vec2 size);

    };
}