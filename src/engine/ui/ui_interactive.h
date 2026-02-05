#pragma once
/**
 * @file ui_interactive.h
 * @brief 定义UIInteractive类，作为可交互UI元素的基类。
 */

#include "./ui_element.h"
#include <memory>
#include <unordered_map>
#include <functional>
#include <string_view>
#include "../render/image.h"
#include "./state/ui_state.h"
#include <glm/glm.hpp>
#include "../resource/resource_id.h"

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
        std::unordered_map<engine::resource::ResourceId, std::unique_ptr<engine::render::Image>> images_; ///< 精灵映射表
        std::unordered_map<engine::resource::ResourceId, engine::resource::ResourceId> sound_; ///< 声音映射表
        engine::render::Image* current_image_ = nullptr; ///< 当前显示的精灵
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
        void addImage(engine::resource::ResourceId name, std::unique_ptr<engine::render::Image> image);
        void addImage(std::string_view name, std::unique_ptr<engine::render::Image> image);
        
        /**
         * @brief 获取精灵。
         * @param name 精灵名称。
         * @return 精灵指针，如果不存在则返回nullptr。
         */
        engine::render::Image* getImage(engine::resource::ResourceId name) const;
        engine::render::Image* getImage(std::string_view name) const;
        
        /**
         * @brief 设置当前显示的图片。
         * @param image 图片指针。
         */
        void setCurrentImage(engine::render::Image* image);
        
        /**
         * @brief 添加声音。
         * @param name 声音名称。
         * @param sound_id 声音资源ID。
         */
        void addSound(engine::resource::ResourceId name, engine::resource::ResourceId sound_id);
        void addSound(engine::resource::ResourceId name, std::string_view sound_key_or_path);
        void addSound(std::string_view name, std::string_view sound_key_or_path);
        
        /**
         * @brief 获取声音文件路径。
         * @param name 声音名称。
         * @return 声音文件路径，如果不存在则返回空字符串。
         */
        engine::resource::ResourceId getSound(engine::resource::ResourceId name) const;
        engine::resource::ResourceId getSound(std::string_view name) const;
        
        /**
         * @brief 播放声音。
         * @param name 声音名称。
         */
        void playSound(engine::resource::ResourceId name);
        void playSound(std::string_view name);
        
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