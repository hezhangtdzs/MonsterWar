#include "ui_manager.h"
#include "../core/context.h"

namespace engine::ui {

/**
 * @brief 构造函数。
 * @param context 引擎上下文引用。
 * 
 * 构造过程中会创建一个UI根元素。
 */
UIManager::UIManager(engine::core::Context& context)
    : context_(context) {
    // 创建根元素
    root_element_ = std::make_unique<UIElement>(context_);
}

/**
 * @brief 析构函数。
 * 
 * 智能指针会自动清理资源，不需要手动释放。
 */
UIManager::~UIManager() {
    // 智能指针会自动清理资源
}

bool UIManager::init(const glm::vec2 &window_size)
{
    root_element_->setPosition(glm::vec2(0.0f));
    root_element_->setSize(window_size);
    spdlog::trace("UIManager 初始化完成，窗口逻辑尺寸: {} x {}", window_size.x, window_size.y);
    return true;
}

/**
 * @brief 更新所有UI元素。
 * @param delta_time 自上一帧的时间间隔（秒）。
 *
 * 更新过程会递归调用所有UI元素的update方法。
 */
void UIManager::update(float delta_time) {
    if (root_element_) {
        root_element_->update(delta_time);
    }
}

/**
 * @brief 渲染所有UI元素。
 * 
 * 渲染过程会递归调用所有UI元素的render方法。
 */
void UIManager::render() {
    if (root_element_) {
        root_element_->render();
    }
}

/**
 * @brief 处理输入事件。
 * @return 如果事件被处理，返回true；否则返回false。
 * 
 * 处理过程会调用根元素的handleInput方法，
 * 根元素会递归处理所有子元素的输入事件。
 */
bool UIManager::handleInput() {
    if (root_element_) {
        return root_element_->handleInput();
    }
    return false;
}

/**
 * @brief 添加UI元素到根元素。
 * @param element 要添加的UI元素。
 */
void UIManager::addElement(std::unique_ptr<UIElement> element) {
    if (root_element_ && element) {
        root_element_->addChild(std::move(element));
    }
}

/**
 * @brief 添加UI元素到指定的父元素。
 * @param parent 父元素指针。
 * @param element 要添加的UI元素。
 */
void UIManager::addElement(UIElement* parent, std::unique_ptr<UIElement> element) {
    if (parent && element) {
        parent->addChild(std::move(element));
    }
}

/**
 * @brief 从根元素移除指定的UI元素。
 * @param element 要移除的UI元素指针。
 */
void UIManager::removeElement(UIElement* element) {
    if (root_element_ && element) {
        root_element_->removeChild(element);
    }
}

/**
 * @brief 获取UI根元素。
 * @return UI根元素指针。
 */
UIElement* UIManager::getRootElement() const {
    return root_element_.get();
}

/**
 * @brief 清空所有UI元素。
 * 
 * 清空过程会调用根元素的clearChildren方法，
 * 移除所有子元素。
 */
void UIManager::clear() {
    if (root_element_) {
        // 清空根元素的所有子元素
        root_element_->clearChildren();
    }
}

}
