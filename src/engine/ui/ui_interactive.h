#pragma once
/**
 * @file ui_interactive.h
 * @brief 定义UIInteractive类，作为可交互UI元素的基类。
 */

#include "./ui_element.h"
#include <memory>
#include <unordered_map>
#include <functional>
#include "../render/sprite.h"
#include "./state/ui_state.h"
#include <glm/glm.hpp>

namespace engine::core {
    class Context;
}

namespace engine::ui {
    namespace state {
        class UIState;
    }

    /**
     * @class UIInteractive
     * @brief 可交互UI元素的基类，支持状态管理、精灵切换和声音播放。
     * 
     * UIInteractive类主要功能包括：
     * - 状态管理（正常、悬停、按下等状态）
     * - 精灵管理（不同状态的精灵切换）
     * - 声音管理（不同状态的声音播放）
     * - 点击事件回调
     * - 输入处理
     */
    class UIInteractive  : public UIElement {
        std::unique_ptr<state::UIState> current_state_; ///< 当前状态
        std::unordered_map<std::string, std::unique_ptr<engine::render::Sprite>> sprites_; ///< 精灵映射表
        std::unordered_map<std::string, std::string> sound_; ///< 声音映射表
        engine::render::Sprite* current_sprite_ = nullptr; ///< 当前显示的精灵
        bool interactive_ = true; ///< 是否可交互
        std::function<void()> click_callback_; ///< 点击回调函数

    public:
        /**
         * @brief 构造函数。
         * @param context 引擎上下文引用。
         */
        UIInteractive(engine::core::Context& context) : UIElement(context) {}
        
        /**
         * @brief 析构函数。
         */
        virtual ~UIInteractive() = default;
        
        /**
         * @brief 更新UI元素及其子元素。
         * @param delta_time 自上一帧的时间间隔（秒）。
         */
        void update(float delta_time) override;
        
        /**
         * @brief 处理输入事件。
         * @return 如果事件被处理，返回true；否则返回false。
         */
        bool handleInput() override;
        
        /**
         * @brief 渲染UI元素及其子元素。
         */
        void render() override;

        /**
         * @brief 设置当前状态。
         * @param state 新的状态实例。
         */
        void setState(std::unique_ptr<state::UIState> state);
        
        /**
         * @brief 获取当前状态。
         * @return 当前状态指针。
         */
        state::UIState* getCurrentState() const;
        
        /**
         * @brief 添加精灵。
         * @param name 精灵名称。
         * @param sprite 精灵实例。
         */
        void addSprite(const std::string& name, std::unique_ptr<engine::render::Sprite> sprite);
        
        /**
         * @brief 获取精灵。
         * @param name 精灵名称。
         * @return 精灵指针，如果不存在则返回nullptr。
         */
        engine::render::Sprite* getSprite(const std::string& name) const;
        
        /**
         * @brief 设置当前显示的精灵。
         * @param sprite 精灵指针。
         */
        void setCurrentSprite(engine::render::Sprite* sprite);
        
        /**
         * @brief 添加声音。
         * @param name 声音名称。
         * @param sound_file 声音文件路径。
         */
        void addSound(const std::string& name, const std::string& sound_file);
        
        /**
         * @brief 获取声音文件路径。
         * @param name 声音名称。
         * @return 声音文件路径，如果不存在则返回空字符串。
         */
        std::string getSound(const std::string& name) const;
        
        /**
         * @brief 播放声音。
         * @param name 声音名称。
         */
        void playSound(const std::string& name);
        
        /**
         * @brief 设置是否可交互。
         * @param interactive 是否可交互。
         */
        void setInteractive(bool interactive);
        
        /**
         * @brief 检查是否可交互。
         * @return 如果可交互，返回true；否则返回false。
         */
        bool isInteractive() const;
        
        /**
         * @brief 设置点击事件回调。
         * @param callback 点击事件回调函数。
         */
        void setClickCallback(std::function<void()> callback);
        
        /**
         * @brief 触发点击事件。
         */
        void triggerClick();
        
        /**
         * @brief 检查点是否在元素内部。
         * @param point 要检查的点（屏幕坐标）。
         * @return 如果点在元素内部，返回true；否则返回false。
         */
        bool containsPoint(const glm::vec2& point) const;
        
        /**
         * @brief 获取上下文引用。
         * @return 引擎上下文引用。
         */
        engine::core::Context& getContext() {
            return context_;
        }
    };
}