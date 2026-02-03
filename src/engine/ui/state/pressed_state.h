#pragma once
/**
 * @file pressed_state.h
 * @brief 定义PressedState类，表示UI元素的按下状态。
 */

#include "ui_state.h"

namespace engine::ui::state {

/**
 * @class PressedState
 * @brief UI元素的按下状态，表示鼠标在元素上按下的状态。
 * 
 * PressedState类实现了UIState接口，处理UI元素在按下状态下的行为：
 * - 进入状态时设置按下状态的精灵
 * - 处理输入事件，当鼠标松开时触发点击回调并切换回HoverState
 * - 处理输入事件，当鼠标离开时切换回NormalState
 */
class PressedState : public UIState {
public:
    /**
     * @brief 构造函数。
     * @param owner 状态所属的UIInteractive对象。
     */
    PressedState(engine::ui::UIInteractive* owner);
    
    /**
     * @brief 析构函数。
     */
    ~PressedState() override = default;
    
    /**
     * @brief 进入状态时调用。
     * 
     * 进入按下状态时，会设置按下状态的精灵并播放按下音效。
     */
    void enter() override;
    
    /**
     * @brief 退出状态时调用。
     * 
     * 退出按下状态时的清理操作。
     */
    void exit() override;
    
    /**
     * @brief 状态更新时调用。
     * @param deltaTime 自上一帧的时间间隔（秒）。
     * 
     * 按下状态的更新操作。
     */
    void update(float deltaTime) override;
    
    /**
     * @brief 处理输入事件时调用。
     * @return 如果需要切换状态，返回新状态的指针；否则返回nullptr。
     * 
     * 处理按下状态下的输入事件：
     * - 当鼠标松开时，触发点击回调并切换回HoverState
     * - 当鼠标离开时，切换回NormalState
     */
    std::unique_ptr<UIState> handleInput() override;
};

} // namespace engine::ui::state
