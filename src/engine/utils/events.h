#pragma once
#include <memory>

namespace engine::scene {
    class Scene;
}

namespace engine::utils {
    /**
     * @file events.h
     * @brief 定义事件相关的实用工具和类型。
     */
    struct QuitEvent
    {
        /* data */
    };
    struct PopSceneEvent {};
    struct PushSceneEvent { 
        std::unique_ptr<engine::scene::Scene> scene; 
    };
    struct ReplaceSceneEvent { 
        std::unique_ptr<engine::scene::Scene> scene; 
    };
    
    // 这里可以定义一些与事件处理相关的工具函数或类型别名
} // namespace engine::utils