/**
 * @file game_state.h
 * @brief 定义 GameState 类，负责管理游戏的状态和窗口信息。
 */

#pragma once

#include <SDL3/SDL_render.h>
#include <SDL3/SDL_video.h>
#include <glm/glm.hpp>

namespace engine::core {

    /**
     * @enum GameStateType
     * @brief 游戏状态枚举类型，定义了游戏的不同状态。
     */
    enum class GameStateType {
        Title,   ///< 标题界面状态
        Playing, ///< 游戏进行中状态
        Paused,  ///< 游戏暂停状态
        GameOver,///< 游戏结束状态
    };
    
    /**
     * @class GameState
     * @brief 游戏状态管理类，负责管理游戏的状态和窗口信息。
     * 
     * GameState 类提供了对游戏状态的统一管理，包括状态转换、
     * 状态检查以及窗口大小管理等功能。
     */
    class GameState final {
    private:
        SDL_Renderer* renderer_ = nullptr; ///< SDL 渲染器指针
        GameStateType current_state_ = GameStateType::Title; ///< 当前游戏状态
        SDL_Window* window_ = nullptr; ///< SDL 窗口指针
    public:
        /**
         * @brief 构造函数，初始化游戏状态和窗口信息。
         * @param renderer SDL 渲染器指针
         * @param window SDL 窗口指针
         * @param initial_state 初始游戏状态，默认为 Title
         */
        explicit GameState(SDL_Renderer* renderer, SDL_Window* window, GameStateType initial_state = GameStateType::Title);
        
        /**
         * @brief 析构函数，清理游戏状态资源。
         */
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
         * @return 窗口大小（宽度和高度）
         */
        glm::vec2 getWindowSize() const;
        
        /**
         * @brief 设置窗口大小。
         * @param size 窗口大小（宽度和高度）
         */
        void setWindowSize(glm::vec2 size);
        
        /**
         * @brief 获取窗口逻辑大小。
         * @return 窗口逻辑大小（宽度和高度）
         */
        glm::vec2 getWindowLogicalSize() const;
        
        /**
         * @brief 设置窗口逻辑大小。
         * @param size 窗口逻辑大小（宽度和高度）
         */
        void setWindowLogicalSize(glm::vec2 size);

    };
}