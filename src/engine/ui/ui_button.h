#pragma once
/**
 * @file ui_button.h
 * @brief 定义UIButton类，作为可交互的按钮控件。
 */

#include "ui_interactive.h"
#include "ui_text.h"
#include <functional>
#include "../utils/math.h" // 用于FColor定义
#include "../render/sprite.h"

using namespace engine::utils;

namespace engine::core {
    class Context;
}

namespace engine::ui {

/**
 * @class UIButton
 * @brief UI按钮类，用于处理用户交互。
 * 
 * UIButton类主要功能包括：
 * - 支持正常、悬停、按下三种状态
 * - 支持点击事件回调
 * - 支持设置按钮文本
 * - 支持设置不同状态的精灵
 */
class UIButton : public UIInteractive {
private:
    /// 按钮文本指针（由UIElement子元素列表管理内存）
    UIText* text_element_ = nullptr;
    /// 点击事件回调函数
    std::function<void()> click_callback_;
    
    /// 正常状态背景颜色
    FColor normal_bg_color_ = { 0.2f, 0.2f, 0.2f, 1.0f };
    /// 悬停状态背景颜色
    FColor hover_bg_color_ = { 0.3f, 0.3f, 0.3f, 1.0f };
    /// 按下状态背景颜色
    FColor pressed_bg_color_ = { 0.4f, 0.4f, 0.4f, 1.0f };
    
    /// 正常状态文本颜色
    FColor normal_text_color_ = { 1.0f, 1.0f, 1.0f, 1.0f };
    /// 悬停状态文本颜色
    FColor hover_text_color_ = { 1.0f, 1.0f, 1.0f, 1.0f };
    /// 按下状态文本颜色
    FColor pressed_text_color_ = { 1.0f, 1.0f, 1.0f, 1.0f };

public:
    /**
     * @brief 构造函数（文本按钮）。
     * @param context 引擎上下文引用。
     * @param text 按钮文本。
     * @param font_path 字体文件路径。
     * @param font_size 字体大小。
     */
    UIButton(engine::core::Context& context, const std::string& text, const std::string& font_path, int font_size = 16);

    /**
     * @brief 构造函数（精灵按钮）。
     * @param context 引擎上下文引用。
     * @param normal_sprite_path 正常状态精灵路径。
     * @param hover_sprite_path 悬停状态精灵路径。
     * @param pressed_sprite_path 按下状态精灵路径。
     * @param position 按钮位置。
     * @param size 按钮大小（如果为(0,0)则使用精灵大小）。
     * @param callback 点击回调函数。
     */
    UIButton(
        engine::core::Context& context,
        const std::string& normal_sprite_path,
        const std::string& hover_sprite_path,
        const std::string& pressed_sprite_path,
        const glm::vec2& position,
        const glm::vec2& size,
        std::function<void()> callback
    );

    /**
     * @brief 析构函数。
     */
    ~UIButton() override;

    /**
     * @brief 处理输入事件。
     * @return 如果事件被处理，返回true；否则返回false。
     */
    bool handleInput() override;

    /**
     * @brief 设置元素大小。
     * @param size 元素大小。
     */
    void setSize(const glm::vec2& size) override;

    /**
     * @brief 设置点击事件回调。
     * @param callback 点击事件回调函数。
     */
    void setClickCallback(std::function<void()> callback);

    // Getters and Setters
    /**
     * @brief 获取按钮文本。
     * @return 按钮文本。
     */
    const std::string& getText() const;

    /**
     * @brief 设置按钮文本。
     * @param text 按钮文本。
     */
    void setText(const std::string& text);

    /**
     * @brief 获取正常状态背景颜色。
     * @return 正常状态背景颜色。
     */
    const FColor& getNormalBgColor() const { return normal_bg_color_; }

    /**
     * @brief 设置正常状态背景颜色。
     * @param color 正常状态背景颜色。
     */
    void setNormalBgColor(const FColor& color);

    /**
     * @brief 获取悬停状态背景颜色。
     * @return 悬停状态背景颜色。
     */
    const FColor& getHoverBgColor() const { return hover_bg_color_; }

    /**
     * @brief 设置悬停状态背景颜色。
     * @param color 悬停状态背景颜色。
     */
    void setHoverBgColor(const FColor& color);

    /**
     * @brief 获取按下状态背景颜色。
     * @return 按下状态背景颜色。
     */
    const FColor& getPressedBgColor() const { return pressed_bg_color_; }

    /**
     * @brief 设置按下状态背景颜色。
     * @param color 按下状态背景颜色。
     */
    void setPressedBgColor(const FColor& color);

    /**
     * @brief 获取正常状态文本颜色。
     * @return 正常状态文本颜色。
     */
    const FColor& getNormalTextColor() const { return normal_text_color_; }

    /**
     * @brief 设置正常状态文本颜色。
     * @param color 正常状态文本颜色。
     */
    void setNormalTextColor(const FColor& color);

    /**
     * @brief 获取悬停状态文本颜色。
     * @return 悬停状态文本颜色。
     */
    const FColor& getHoverTextColor() const { return hover_text_color_; }

    /**
     * @brief 设置悬停状态文本颜色。
     * @param color 悬停状态文本颜色。
     */
    void setHoverTextColor(const FColor& color);

    /**
     * @brief 获取按下状态文本颜色。
     * @return 按下状态文本颜色。
     */
    const FColor& getPressedTextColor() const { return pressed_text_color_; }

    /**
     * @brief 设置按下状态文本颜色。
     * @param color 按下状态文本颜色。
     */
    void setPressedTextColor(const FColor& color);

private:
    /**
     * @brief 更新文本位置，使其处于按钮中心。
     */
    void updateTextPosition();
};

}