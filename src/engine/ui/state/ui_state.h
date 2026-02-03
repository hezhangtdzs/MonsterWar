#pragma once
/**
 * @file ui_state.h
 * @brief 定义UIState类，作为UI元素状态的基类。
 */

#include <memory>
#include <glm/vec2.hpp>

namespace engine::ui {
    class UIInteractive;
}

namespace engine::ui::state {

/**
 * @class UIState
 * @brief UI元素状态的基类，定义了状态的基本接口。
 * 
 * UIState类是一个抽象基类，定义了状态的四个基本方法：
 * - enter()：进入状态时调用
 * - exit()：退出状态时调用
 * - update()：状态更新时调用
 * - handleInput()：处理输入事件时调用
 * 
 * 所有具体的UI状态类都应该继承自这个类并实现这些方法。
 */
class UIState {
protected:
    engine::ui::UIInteractive* owner_; ///< 状态所属的UIInteractive对象

public:
    /**
     * @brief 构造函数。
     * @param owner 状态所属的UIInteractive对象。
     */
    UIState(engine::ui::UIInteractive* owner) : owner_(owner) {}
    
    /**
     * @brief 析构函数。
     */
    virtual ~UIState() = default;
    
    /**
     * @brief 进入状态时调用。
     * 
     * 当UI元素切换到该状态时，会调用此方法进行初始化。
     */
    virtual void enter() = 0;
    
    /**
     * @brief 退出状态时调用。
     * 
     * 当UI元素从该状态切换到其他状态时，会调用此方法进行清理。
     */
    virtual void exit() = 0;
    
    /**
     * @brief 状态更新时调用。
     * @param deltaTime 自上一帧的时间间隔（秒）。
     * 
     * 每帧更新时，会调用当前状态的此方法。
     */
    virtual void update(float deltaTime) = 0;
    
    /**
     * @brief 处理输入事件时调用。
     * @return 如果需要切换状态，返回新状态的指针；否则返回nullptr。
     * 
     * 当UI元素处理输入事件时，会调用当前状态的此方法。
     */
    virtual std::unique_ptr<UIState> handleInput() = 0;
};

}