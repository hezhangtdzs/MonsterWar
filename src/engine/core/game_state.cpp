#include "game_state.h"
#include <spdlog/spdlog.h>

/**
 * @brief 游戏状态管理类
 * 
 * 负责管理游戏的当前状态（如运行中、暂停、游戏结束）以及窗口相关的尺寸设置。
 * 提供了获取和设置游戏状态、窗口尺寸和逻辑尺寸的方法。
 */

/**
 * @brief GameState 构造函数
 * 
 * @param renderer SDL 渲染器指针
 * @param window SDL 窗口指针
 * @param initial_state 初始游戏状态
 */
engine::core::GameState::GameState(SDL_Renderer *renderer, SDL_Window *window, GameStateType initial_state)
	: renderer_(renderer), window_(window), current_state_(initial_state)
{
    if (!renderer || !window) {
        spdlog::error("GameState 初始化失败，renderer 或 window 为空！");
        return;
    }
}

engine::core::GameState::~GameState() = default;

/**
 * @brief 获取当前游戏状态
 * 
 * @return 当前游戏状态
 */
engine::core::GameStateType engine::core::GameState::getState() const
{
    return current_state_;
}

/**
 * @brief 设置当前游戏状态
 * 
 * @param state 要设置的游戏状态
 */
void engine::core::GameState::setState(GameStateType state)
{
    if (current_state_ == state) {
        spdlog::warn("GameState 状态未改变，当前状态为：{}", static_cast<int>(state));
        return;
    }
    spdlog::info("GameState 状态改变，从 {} 到 {}", static_cast<int>(current_state_), static_cast<int>(state));
    current_state_ = state;
}

/**
 * @brief 检查游戏是否处于运行状态
 * 
 * @return 如果游戏正在运行返回 true，否则返回 false
 */
bool engine::core::GameState::isPlaying() const
{
    return current_state_ == GameStateType::Playing;
}

/**
 * @brief 检查游戏是否处于暂停状态
 * 
 * @return 如果游戏已暂停返回 true，否则返回 false
 */
bool engine::core::GameState::isPaused() const
{
    return current_state_ == GameStateType::Paused;
}

/**
 * @brief 检查游戏是否结束
 * 
 * @return 如果游戏已结束返回 true，否则返回 false
 */
bool engine::core::GameState::isGameOver() const
{
    return current_state_ == GameStateType::GameOver;
}

/**
 * @brief 获取窗口尺寸
 * 
 * @return 窗口尺寸的 glm::vec2 表示
 */
glm::vec2 engine::core::GameState::getWindowSize() const
{
    int width, height;
    SDL_GetWindowSize(window_, &width, &height);
    return glm::vec2(static_cast<float>(width), static_cast<float>(height));
}

/**
 * @brief 设置窗口尺寸
 * 
 * @param size 要设置的窗口尺寸
 */
void engine::core::GameState::setWindowSize(glm::vec2 size)
{
    SDL_SetWindowSize(window_, static_cast<int>(size.x), static_cast<int>(size.y));
}

/**
 * @brief 获取窗口逻辑尺寸
 * 
 * @return 窗口逻辑尺寸的 glm::vec2 表示
 */
glm::vec2 engine::core::GameState::getWindowLogicalSize() const
{
    int width, height;
    SDL_GetRenderLogicalPresentation(renderer_, &width, &height,NULL);
    return glm::vec2(static_cast<float>(width), static_cast<float>(height));
}

/**
 * @brief 设置窗口逻辑尺寸
 * 
 * @param size 要设置的逻辑尺寸
 */
void engine::core::GameState::setWindowLogicalSize(glm::vec2 size)
{
    int width = static_cast<int>(size.x);
    int height = static_cast<int>(size.y);
    SDL_SetRenderLogicalPresentation(renderer_, width, height,SDL_LOGICAL_PRESENTATION_LETTERBOX);
}
