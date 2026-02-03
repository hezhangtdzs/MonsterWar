#include "pressed_state.h"
#include "../ui_interactive.h"
#include "normal_state.h"
#include "hover_state.h"
#include "../../core/context.h"
#include "../../input/input_manager.h"

namespace engine::ui::state {

/**
 * @brief 构造函数。
 * @param owner 状态所属的UIInteractive对象。
 */
PressedState::PressedState(engine::ui::UIInteractive* owner)
    : UIState(owner) {
}

/**
 * @brief 进入状态时调用。
 * 
 * 进入按下状态时，会：
 * 1. 尝试获取按下状态的精灵
 * 2. 如果找到精灵，设置为当前显示的精灵
 * 3. 播放按下音效
 */
void PressedState::enter() {
    if (auto interactive = dynamic_cast<engine::ui::UIInteractive*>(owner_)) {
        // 切换到按下状态的精灵
        auto pressed_sprite = interactive->getSprite("pressed");
        if (pressed_sprite) {
            interactive->setCurrentSprite(pressed_sprite);
        }
        // 播放按下音效
        interactive->playSound("pressed");
    }
}

/**
 * @brief 退出状态时调用。
 * 
 * 退出按下状态时的清理操作。
 */
void PressedState::exit() {
    // 退出按下状态时的操作
}

/**
 * @brief 状态更新时调用。
 * @param deltaTime 自上一帧的时间间隔（秒）。
 * 
 * 按下状态的更新操作，当前为空实现。
 */
void PressedState::update(float [[maybe_unused]] deltaTime) {
    // 按下状态的更新
}

/**
 * @brief 处理输入事件时调用。
 * @return 如果需要切换状态，返回新状态的指针；否则返回nullptr。
 * 
 * 处理按下状态下的输入事件：
 * 1. 获取输入管理器
 * 2. 获取鼠标位置
 * 3. 检查鼠标是否释放
 * 4. 如果鼠标释放且在元素范围内，触发点击回调并切换回HoverState
 * 5. 如果鼠标释放且不在元素范围内，切换回NormalState
 * 6. 如果鼠标未释放但离开元素范围，切换回NormalState
 * 7. 如果鼠标未释放且在元素范围内，保持按下状态
 */
std::unique_ptr<UIState> PressedState::handleInput() {
    if (auto interactive = dynamic_cast<engine::ui::UIInteractive*>(owner_)) {
        // 获取输入管理器
        auto& input_manager = interactive->getContext().getInputManager();
        
        // 获取鼠标位置
        glm::vec2 mouse_pos = input_manager.getLogicalMousePosition();
        
        // 检查鼠标是否释放
        if (input_manager.isActionReleased("MouseLeftClick")) {
            // 鼠标释放，检查是否在按钮内
            if (interactive->containsPoint(mouse_pos)) {
                // 鼠标在按钮内释放，触发点击事件
            interactive->triggerClick();
            // 然后切换到悬停状态
            return std::make_unique<HoverState>(interactive);
            } else {
                // 鼠标在按钮外释放，切换到正常状态
                return std::make_unique<NormalState>(interactive);
            }
        }
        
        // 检查鼠标是否离开按钮
        if (!interactive->containsPoint(mouse_pos)) {
            // 鼠标离开按钮，切换到正常状态
            return std::make_unique<NormalState>(interactive);
        }
    }
    return nullptr;
}

} // namespace engine::ui::state