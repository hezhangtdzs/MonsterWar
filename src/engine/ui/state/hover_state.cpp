#include "hover_state.h"
#include "../ui_interactive.h"
#include "normal_state.h"
#include "pressed_state.h"
#include "../../core/context.h"
#include "../../input/input_manager.h"

namespace engine::ui::state {

/**
 * @brief 构造函数。
 * @param owner 状态所属的UIInteractive对象。
 */
HoverState::HoverState(engine::ui::UIInteractive* owner)
    : UIState(owner) {
}

/**
 * @brief 进入状态时调用。
 * 
 * 进入悬停状态时，会：
 * 1. 尝试获取悬停状态的精灵
 * 2. 如果找到精灵，设置为当前显示的精灵
 */
void HoverState::enter() {
    if (auto interactive = dynamic_cast<engine::ui::UIInteractive*>(owner_)) {
        // 切换到悬停状态的精灵
        auto hover_sprite = interactive->getSprite("hover");
        if (hover_sprite) {
            interactive->setCurrentSprite(hover_sprite);
        }
		
        
    }
}

/**
 * @brief 退出状态时调用。
 * 
 * 退出悬停状态时的清理操作。
 */
void HoverState::exit() {
    // 退出悬停状态时的操作
}

/**
 * @brief 状态更新时调用。
 * @param deltaTime 自上一帧的时间间隔（秒）。
 * 
 * 悬停状态的更新操作，当前为空实现。
 */
void HoverState::update(float [[maybe_unused]] deltaTime) {
    // 悬停状态的更新
}

/**
 * @brief 处理输入事件时调用。
 * @return 如果需要切换状态，返回新状态的指针；否则返回nullptr。
 * 
 * 处理悬停状态下的输入事件：
 * 1. 获取输入管理器
 * 2. 获取鼠标位置
 * 3. 检查鼠标是否在元素范围内
 * 4. 如果鼠标不在范围内，切换回NormalState
 * 5. 如果鼠标在范围内，检查鼠标是否按下
 * 6. 如果鼠标按下，切换到PressedState
 * 7. 如果鼠标在范围内且未按下，保持悬停状态
 */
std::unique_ptr<UIState> HoverState::handleInput() {
    if (auto interactive = dynamic_cast<engine::ui::UIInteractive*>(owner_)) {
        // 获取输入管理器
        auto& input_manager = interactive->getContext().getInputManager();
        
        // 获取鼠标位置
        glm::vec2 mouse_pos = input_manager.getLogicalMousePosition();
        
        // 检查鼠标是否在按钮内
        if (!interactive->containsPoint(mouse_pos)) {
            // 鼠标离开按钮，切换到正常状态
            return std::make_unique<NormalState>(interactive);
        }
     
        // 检查鼠标是否按下
        if (input_manager.isActionDown("MouseLeftClick")) {
            // 鼠标按下，切换到按下状态
            return std::make_unique<PressedState>(interactive);
        }
    }
    // 鼠标在按钮内但未按下，保持悬停状态
    return nullptr;
}

} // namespace engine::ui::state
