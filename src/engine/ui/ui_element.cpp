#include "ui_element.h"
#include "../core/context.h"
#include "../input/input_manager.h"
#include <glm/glm.hpp>

namespace engine::ui {

/**
 * @brief 构造函数。
 * @param context 引擎上下文引用。
 */
UIElement::UIElement(engine::core::Context& context)
    : context_(context) {
}

/**
 * @brief 析构函数。
 */
UIElement::~UIElement() {
}

/**
 * @brief 更新UI元素及其子元素。
 * @param delta_time 自上一帧的时间间隔（秒）。
 */
void UIElement::update(float delta_time) {
    if (!visible_ || !enabled_) {
        return;
    }

    // 更新所有子元素
    for (auto& child : children_) {
        child->update(delta_time);
    }
}

/**
 * @brief 渲染UI元素及其子元素。
 */
void UIElement::render() {
    if (!visible_) {
        return;
    }

    // 渲染所有子元素
    for (auto& child : children_) {
        child->render();
    }
}

/**
 * @brief 处理输入事件。
 * @return 如果事件被处理，返回true；否则返回false。
 */
bool UIElement::handleInput() {
    if (!visible_ || !enabled_) {
        return false;
    }

    // 处理所有子元素的输入，从后往前遍历（上层元素优先）
    for (auto it = children_.rbegin(); it != children_.rend(); ++it) {
        if ((*it)->handleInput()) {
            return true; // 如果子元素处理了事件，返回true
        }
    }

    return false;
}

/**
 * @brief 添加子元素。
 * @param child 要添加的子元素。
 */
void UIElement::addChild(std::unique_ptr<UIElement> child) {
    if (child) {
        child->setParent(this);
        children_.emplace_back(std::move(child));
    }
}

/**
 * @brief 移除指定的子元素。
 * @param child 要移除的子元素指针。
 */
void UIElement::removeChild(UIElement* child) {
    if (!child) {
        return;
    }

    for (auto it = children_.begin(); it != children_.end(); ++it) {
        if (it->get() == child) {
            it->release(); // 手动释放指针，避免双重删除
            children_.erase(it);
            break;
        }
    }
}

/**
 * @brief 获取元素的世界坐标位置。
 * @return 元素在世界坐标系中的位置。
 */
glm::vec2 UIElement::getWorldPosition() const {
    glm::vec2 world_pos = position_;
    const UIElement* current_parent = parent_;

    // 递归计算世界坐标
    while (current_parent) {
        world_pos += current_parent->position_;
        current_parent = current_parent->parent_;
    }

    return world_pos;
}

/**
 * @brief 检查点是否在元素内部。
 * @param point 要检查的点（屏幕坐标）。
 * @return 如果点在元素内部，返回true；否则返回false。
 */
bool UIElement::containsPoint(const glm::vec2& point) const {
    glm::vec2 world_pos = getWorldPosition();
    return point.x >= world_pos.x && point.x <= world_pos.x + size_.x &&
           point.y >= world_pos.y && point.y <= world_pos.y + size_.y;
}

}
