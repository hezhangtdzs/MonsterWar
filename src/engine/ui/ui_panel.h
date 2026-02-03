#pragma once
/**
 * @file ui_panel.h
 * @brief 定义UIPanel类，作为UI元素的容器面板。
 */

#include "ui_element.h"
#include <glm/glm.hpp>
#include "../utils/math.h" // 用于FColor定义

using namespace engine::utils;

namespace engine::core {
    class Context;
}

namespace engine::ui {

/**
 * @class UIPanel
 * @brief UI面板类，用于组织和管理其他UI元素。
 * 
 * UIPanel类是一个容器控件，主要功能包括：
 * - 作为其他UI元素的父容器
 * - 支持设置背景颜色
 * - 支持设置边框
 * - 管理子元素的布局
 */
class UIPanel : public UIElement {
private:
    /// 背景颜色
    FColor background_color_ = { 0.0f, 0.0f, 0.0f, 0.0f }; // 透明黑色
    /// 边框颜色
    FColor border_color_ = { 0.0f, 0.0f, 0.0f, 0.0f }; // 透明黑色
    /// 边框宽度
    float border_width_ = 0.0f;

public:
    /**
     * @brief 构造函数。
     * @param context 引擎上下文引用。
     */
    explicit UIPanel(engine::core::Context& context);

    /**
     * @brief 析构函数。
     */
    ~UIPanel() override;

    /**
     * @brief 渲染面板及其子元素。
     */
    void render() override;

    // Getters and Setters
    /**
     * @brief 获取背景颜色。
     * @return 背景颜色。
     */
    const FColor& getBackgroundColor() const { return background_color_; }

    /**
     * @brief 设置背景颜色。
     * @param color 背景颜色。
     */
    void setBackgroundColor(const FColor& color) { background_color_ = color; }

    /**
     * @brief 获取边框颜色。
     * @return 边框颜色。
     */
    const FColor& getBorderColor() const { return border_color_; }

    /**
     * @brief 设置边框颜色。
     * @param color 边框颜色。
     */
    void setBorderColor(const FColor& color) { border_color_ = color; }

    /**
     * @brief 获取边框宽度。
     * @return 边框宽度。
     */
    float getBorderWidth() const { return border_width_; }

    /**
     * @brief 设置边框宽度。
     * @param width 边框宽度。
     */
    void setBorderWidth(float width) { border_width_ = width; }
};

}