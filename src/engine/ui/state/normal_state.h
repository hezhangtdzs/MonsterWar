#pragma once
/**
 * @file normal_state.h
 * @brief 定义NormalState类，表示UI元素的正常状态。
 */

#include "ui_state.h"

namespace engine::ui::state {

/**
 * @class NormalState
 * @brief UI元素的正常状态，表示元素未被鼠标悬停或点击的状态。
 * 
 * NormalState类实现了UIState接口，处理UI元素在正常状态下的行为：
 * - 进入状态时设置正常状态的精灵
 * - 处理输入事件，当鼠标悬停时切换到HoverState
 * - 处理输入事件，当鼠标按下时切换到PressedState
 */
class NormalState : public UIState {
public:
    /**
     * @brief 构造函数。
     * @param owner 状态所属的UIInteractive对象。
     */
    NormalState(engine::ui::UIInteractive* owner);
    
    /**
     * @brief 析构函数。
     */
    ~NormalState() override = default;
    
    /**
     * @brief 进入状态时调用。
     * 
     * 进入正常状态时，会设置正常状态的精灵。
     */
    void enter() override;
    
    /**
     * @brief 退出状态时调用。
     * 
     * 退出正常状态时的清理操作。
     */
    void exit() override;
    
    /**
     * @brief 状态更新时调用。
     * @param deltaTime 自上一帧的时间间隔（秒）。
     * 
     * 正常状态的更新操作。
     */
    void update(float deltaTime) override;
    
    /**
     * @brief 处理输入事件时调用。
     * @return 如果需要切换状态，返回新状态的指针；否则返回nullptr。
     * 
     * 处理正常状态下的输入事件：
     * - 当鼠标悬停时，切换到HoverState
     * - 当鼠标按下时，切换到PressedState
     */
    std::unique_ptr<UIState> handleInput() override;
};

} // namespace engine::ui::state
