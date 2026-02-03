#pragma once
/**
 * @file ui_element.h
 * @brief 定义UIElement类，作为所有UI控件的基类。
 */

#include <vector>
#include <memory>
#include <glm/glm.hpp>

namespace engine::core {
    class Context;
}

namespace engine::ui {

/**
 * @class UIElement
 * @brief 所有UI控件的基类，提供通用的UI元素功能。
 * 
 * UIElement类定义了所有UI控件共有的属性和方法，包括：
 * - 位置、大小和可见性管理
 * - 子元素管理（添加、移除、遍历）
 * - 通用的更新、渲染和输入处理接口
 * - 基本的交互事件处理
 */
class UIElement {
protected:
    /// 指向父元素的指针
    UIElement* parent_ = nullptr;
    /// 子元素列表
    std::vector<std::unique_ptr<UIElement>> children_;
    /// 元素位置（相对于父元素）
    glm::vec2 position_ = { 0.0f, 0.0f };
    /// 元素大小
    glm::vec2 size_ = { 0.0f, 0.0f };
    /// 可见性状态
    bool visible_ = true;
    /// 启用状态
    bool enabled_ = true;
    /// 上下文引用
    engine::core::Context& context_;

public:
    /**
     * @brief 构造函数。
     * @param context 引擎上下文引用。
     */
    explicit UIElement(engine::core::Context& context);

    /**
     * @brief 析构函数。
     */
    virtual ~UIElement();

    /**
     * @brief 更新UI元素及其子元素。
     * @param delta_time 自上一帧的时间间隔（秒）。
     */
    virtual void update(float delta_time);

    /**
     * @brief 渲染UI元素及其子元素。
     */
    virtual void render();

    /**
     * @brief 处理输入事件。
     * @return 如果事件被处理，返回true；否则返回false。
     */
    virtual bool handleInput();

    /**
     * @brief 添加子元素。
     * @param child 要添加的子元素。
     */
    virtual void addChild(std::unique_ptr<UIElement> child);

    /**
     * @brief 移除指定的子元素。
     * @param child 要移除的子元素指针。
     */
    virtual void removeChild(UIElement* child);

    /**
     * @brief 获取元素的世界坐标位置。
     * @return 元素在世界坐标系中的位置。
     */
    virtual glm::vec2 getWorldPosition() const;

    /**
     * @brief 检查点是否在元素内部。
     * @param point 要检查的点（屏幕坐标）。
     * @return 如果点在元素内部，返回true；否则返回false。
     */
    virtual bool containsPoint(const glm::vec2& point) const;

    // Getters and Setters
    /**
     * @brief 获取父元素指针。
     * @return 父元素指针。
     */
    UIElement* getParent() const { return parent_; }

    /**
     * @brief 设置父元素指针。
     * @param parent 父元素指针。
     */
    void setParent(UIElement* parent) { parent_ = parent; }

    /**
     * @brief 获取元素位置。
     * @return 元素位置（相对于父元素）。
     */
    const glm::vec2& getPosition() const { return position_; }

    /**
     * @brief 设置元素位置。
     * @param position 元素位置（相对于父元素）。
     */
    void setPosition(const glm::vec2& position) { position_ = position; }

    /**
     * @brief 获取元素大小。
     * @return 元素大小。
     */
    virtual const glm::vec2& getSize() const { return size_; }

    /**
     * @brief 设置元素大小。
     * @param size 元素大小。
     */
    virtual void setSize(const glm::vec2& size) { size_ = size; }

    /**
     * @brief 检查元素是否可见。
     * @return 如果元素可见，返回true；否则返回false。
     */
    bool isVisible() const { return visible_; }

    /**
     * @brief 设置元素可见性。
     * @param visible 可见性状态。
     */
    void setVisible(bool visible) { visible_ = visible; }

    /**
     * @brief 检查元素是否启用。
     * @return 如果元素启用，返回true；否则返回false。
     */
    bool isEnabled() const { return enabled_; }

    /**
     * @brief 设置元素启用状态。
     * @param enabled 启用状态。
     */
    void setEnabled(bool enabled) { enabled_ = enabled; }

    /**
     * @brief 清空所有子元素。
     */
    void clearChildren() {
        children_.clear();
    }
};

}
