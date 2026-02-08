/**
 * @file events.h
 * @brief 定义游戏引擎中使用的各种事件类型。
 * 
 * 该文件包含了场景管理、游戏生命周期等相关的事件定义，
 * 用于在引擎各系统之间传递消息和状态变化。
 */

#pragma once
#include <memory>
#include <entt/entity/entity.hpp>

namespace engine::scene {
    class Scene;
}

namespace engine::utils {
    /**
     * @struct QuitEvent
     * @brief 退出游戏事件。
     * 
     * 当需要退出游戏时触发此事件，由 GameApp 处理并终止游戏主循环。
     */
    struct QuitEvent
    {
        /* data */
    };

    /**
     * @struct PopSceneEvent
     * @brief 弹出当前场景事件。
     * 
     * 当需要从场景栈中移除当前场景时触发此事件。
     */
    struct PopSceneEvent {};

    /**
     * @struct PushSceneEvent
     * @brief 压入新场景事件。
     * 
     * 当需要将新场景压入场景栈时触发此事件。
     * @var scene 要压入的新场景实例
     */
    struct PushSceneEvent {
        std::unique_ptr<engine::scene::Scene> scene;
    };

    /**
     * @struct ReplaceSceneEvent
     * @brief 替换当前场景事件。
     * 
     * 当需要替换整个场景栈时触发此事件。
     * @var scene 用于替换的新场景实例
     */
    struct ReplaceSceneEvent {
        std::unique_ptr<engine::scene::Scene> scene;
    };
    /// @brief 播放动画事件
    struct PlayAnimationEvent {
        entt::entity entity_{entt::null};           ///< @brief 目标实体
        entt::id_type animation_id_{entt::null};    ///< @brief 动画ID
        bool loop_{true};                           ///< @brief 是否循环
    };

    // 这里可以定义一些与事件处理相关的工具函数或类型别名
} // namespace engine::utils