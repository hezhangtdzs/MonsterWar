#include "normal_state.h"
#include "../ui_interactive.h"
#include "hover_state.h"
#include "../ui_element.h"
#include "../../core/context.h"
#include "../../input/input_manager.h"

namespace engine::ui::state {

/**
 * @brief 构造函数。
 * @param owner 状态所属的UIInteractive对象。
 */
NormalState::NormalState(engine::ui::UIInteractive* owner)
    : UIState(owner) {
}

/**
 * @brief 进入状态时调用。
 * 
 * 进入正常状态时，会：
 * 1. 尝试获取正常状态的精灵
 * 2. 如果找到精灵，设置为当前显示的精灵
 * 3. 可以在这里播放进入正常状态的音效
 */
void NormalState::enter() {
    if (auto interactive = dynamic_cast<engine::ui::UIInteractive*>(owner_)) {
        // 切换到正常状态的精灵
        auto normal_sprite = interactive->getSprite("normal");
        if (normal_sprite) {
            interactive->setCurrentSprite(normal_sprite);
        }
        // 可以在这里播放进入正常状态的音效
    }
}

/**
 * @brief 退出状态时调用。
 * 
 * 退出正常状态时的清理操作。
 */
void NormalState::exit() {
    // 退出正常状态时的操作
}

/**
 * @brief 状态更新时调用。
 * @param deltaTime 自上一帧的时间间隔（秒）。
 * 
 * 正常状态的更新操作，当前为空实现。
 */
void NormalState::update(float [[maybe_unused]] deltaTime) {
    // 正常状态的更新
}

/**
 * @brief 处理输入事件时调用。
 * @return 如果需要切换状态，返回新状态的指针；否则返回nullptr。
 * 
 * 处理正常状态下的输入事件：
 * 1. 获取输入管理器
 * 2. 获取鼠标位置
 * 3. 检查鼠标是否在元素范围内
 * 4. 如果鼠标在范围内，播放悬停音效并切换到HoverState
 * 5. 如果鼠标不在范围内，保持正常状态
 */
std::unique_ptr<UIState> NormalState::handleInput() {
    if (auto interactive = dynamic_cast<engine::ui::UIInteractive*>(owner_)) {
        // 获取输入管理器
        auto& input_manager = interactive->getContext().getInputManager();
        
        // 获取鼠标位置
        glm::vec2 mouse_pos = input_manager.getLogicalMousePosition();
        
        // 检查鼠标是否在按钮内
        if (interactive->containsPoint(mouse_pos)) {
            // 鼠标进入按钮，切换到悬停状态
            interactive->playSound("hover");
            return std::make_unique<HoverState>(interactive);
        }
    }
    // 鼠标不在按钮内，保持正常状态
    return nullptr;
}

} // namespace engine::ui::state
