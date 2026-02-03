#pragma once
/**
 * @file ui_manager.h
 * @brief 定义UIManager类，用于管理整个UI系统。
 */

#include <memory>
#include "ui_element.h"

namespace engine::core {
    class Context;
}

namespace engine::ui {

/**
 * @class UIManager
 * @brief UI管理器类，负责管理整个UI系统。
 * 
 * UIManager类负责：
 * - 管理UI根元素和所有UI元素
 * - 协调UI元素的更新、渲染和输入处理
 * - 处理UI元素的添加和移除
 * - 提供访问UI元素的方法
 */
class UIManager {
private:
    /// 上下文引用
    engine::core::Context& context_;
    /// UI根元素
    std::unique_ptr<UIElement> root_element_;

public:
    /**
     * @brief 构造函数。
     * @param context 引擎上下文引用。
     */
    explicit UIManager(engine::core::Context& context);

    /**
     * @brief 析构函数。
     */
    ~UIManager();

    /**
     * @brief 初始化UI管理器。
     * @param window_size 窗口逻辑尺寸。
     * @return 如果初始化成功，返回true；否则返回false。
     */
    [[nodiscard]] bool init(const glm::vec2& window_size);

    /**
     * @brief 更新所有UI元素。
     * @param delta_time 自上一帧的时间间隔（秒）。
     */
    void update(float delta_time);

    /**
     * @brief 渲染所有UI元素。
     */
    void render();

    /**
     * @brief 处理输入事件。
     * @return 如果事件被处理，返回true；否则返回false。
     */
    bool handleInput();

    /**
     * @brief 添加UI元素到根元素。
     * @param element 要添加的UI元素。
     */
    void addElement(std::unique_ptr<UIElement> element);

    /**
     * @brief 添加UI元素到指定的父元素。
     * @param parent 父元素指针。
     * @param element 要添加的UI元素。
     */
    void addElement(UIElement* parent, std::unique_ptr<UIElement> element);

    /**
     * @brief 从根元素移除指定的UI元素。
     * @param element 要移除的UI元素指针。
     */
    void removeElement(UIElement* element);

    /**
     * @brief 获取UI根元素。
     * @return UI根元素指针。
     */
    UIElement* getRootElement() const;

    /**
     * @brief 清空所有UI元素。
     */
    void clear();
};

}
