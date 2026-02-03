#include "ui_panel.h"
#include "../core/context.h"
#include "../render/renderer.h"

namespace engine::ui {

/**
 * @brief 构造函数。
 * @param context 引擎上下文引用。
 */
UIPanel::UIPanel(engine::core::Context& context)

    : UIElement(context) {
}

/**
 * @brief 析构函数。
 */
UIPanel::~UIPanel() = default;

/**
 * @brief 渲染面板及其子元素。
 * 
 * 渲染过程包括：
 * 1. 检查元素是否可见
 * 2. 绘制背景（如果背景颜色不透明）
 * 3. 绘制边框（如果边框宽度大于0且边框颜色不透明）
 * 4. 调用父类render方法渲染子元素
 */
void UIPanel::render() {
    if (!visible_) {
        return;
    }

    // 获取渲染器
    auto& renderer = context_.getRenderer();

    // 计算世界位置
    glm::vec2 world_pos = getWorldPosition();
    
    // 创建Rect结构
    engine::utils::Rect rect;
    rect.position = world_pos;
    rect.size = size_;

    // 绘制背景
    if (background_color_.a > 0.0f) {
        renderer.drawUIFilledRect(rect, background_color_);
    }

    // 绘制边框
    if (border_width_ > 0.0f && border_color_.a > 0.0f) {
        renderer.drawUIOutlineRect(rect, border_color_);
    }

    // 调用父类render方法渲染子元素
    UIElement::render();
}

}