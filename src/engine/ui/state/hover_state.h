#pragma once
/**
 * @file hover_state.h
 * @brief 定义HoverState类，表示UI元素的悬停状态。
 */

#include "ui_state.h"

namespace engine::ui::state {

/**
 * @class HoverState
 * @brief UI元素的悬停状态，表示鼠标悬停在元素上的状态。
 * 
 * HoverState类实现了UIState接口，处理UI元素在悬停状态下的行为：
 * - 进入状态时设置悬停状态的精灵
 * - 处理输入事件，当鼠标离开时切换回NormalState
 * - 处理输入事件，当鼠标按下时切换到PressedState
 */
class HoverState : public UIState {
public:
    /**
     * @brief 构造函数。
     * @param owner 状态所属的UIInteractive对象。
     */
    HoverState(engine::ui::UIInteractive* owner);
    
    /**
     * @brief 析构函数。
     */
    ~HoverState() override = default;
    
    /**
     * @brief 进入状态时调用。
     * 
     * 进入悬停状态时，会设置悬停状态的精灵。
     */
    void enter() override;
    
    /**
     * @brief 退出状态时调用。
     * 
     * 退出悬停状态时的清理操作。
     */
    void exit() override;
    
    /**
     * @brief 状态更新时调用。
     * @param deltaTime 自上一帧的时间间隔（秒）。
     * 
     * 悬停状态的更新操作。
     */
    void update(float deltaTime) override;
    
    /**
     * @brief 处理输入事件时调用。
     * @return 如果需要切换状态，返回新状态的指针；否则返回nullptr。
     * 
     * 处理悬停状态下的输入事件：
     * - 当鼠标离开时，切换回NormalState
     * - 当鼠标按下时，切换到PressedState
     */
    std::unique_ptr<UIState> handleInput() override;
};

} // namespace engine::ui::state
