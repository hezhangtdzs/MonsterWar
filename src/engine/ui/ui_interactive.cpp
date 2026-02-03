#include "ui_interactive.h"
#include "../core/context.h"
#include "../render/renderer.h"
#include "../audio/audio_player.h"
#include "../resource/resource_manager.h"
#include "../input/input_manager.h"
#include "./state/ui_state.h"
#include "../audio/audio_locator.h"

namespace engine::ui {

/**
 * @brief 处理输入事件。
 * @return 如果事件被处理，返回true；否则返回false。
 * 
 * 处理过程包括：
 * 1. 检查元素是否可见、启用和可交互
 * 2. 处理子元素的输入
 * 3. 调用当前状态的handleInput方法
 * 4. 如果鼠标在元素范围内，拦截输入防止穿透
 */
bool UIInteractive::handleInput() {
    if (!visible_ || !enabled_ || !interactive_) {
        return false;
    }
    
    // 首先处理子元素的输入
    if (UIElement::handleInput()) {
        return true;
    }
    
    bool handled = false;
    // 调用当前状态的handleInput
    if (current_state_) {
        auto new_state = current_state_->handleInput();
        if (new_state) {
            setState(std::move(new_state));
            handled = true;
        }
    }
    
    // 如果鼠标在范围内，拦截输入防止穿透
    auto& input_manager = context_.getInputManager();
    if (containsPoint(input_manager.getLogicalMousePosition())) {
        handled = true;
    }
    
    return handled;
}

/**
 * @brief 更新UI元素及其子元素。
 * @param delta_time 自上一帧的时间间隔（秒）。
 * 
 * 更新过程包括：
 * 1. 检查元素是否可见和启用
 * 2. 调用当前状态的update方法
 * 3. 调用父类update方法更新子元素
 */
void UIInteractive::update(float delta_time) {
    if (!visible_ || !enabled_) {
        return;
    }

    if (current_state_) {
        current_state_->update(delta_time);
    }

    UIElement::update(delta_time);
}

/**
 * @brief 渲染UI元素及其子元素。
 * 
 * 渲染过程包括：
 * 1. 检查元素是否可见
 * 2. 渲染当前精灵（如果有）
 * 3. 调用父类render方法渲染子元素
 */
void UIInteractive::render() {
    if (!visible_) {
        return;
    }
    
    // 渲染当前精灵
    if (current_sprite_) {
        auto& renderer = context_.getRenderer();
        auto position = getWorldPosition();
        renderer.drawUISprite(*current_sprite_, position, size_);
    }
    
    // 渲染子元素
    UIElement::render();
}

/**
 * @brief 设置当前状态。
 * @param state 新的状态实例。
 * 
 * 设置过程包括：
 * 1. 调用当前状态的exit方法
 * 2. 设置新状态
 * 3. 调用新状态的enter方法
 */
void UIInteractive::setState(std::unique_ptr<state::UIState> state) {
    if (current_state_) {
        current_state_->exit();
    }
    current_state_ = std::move(state);
    if (current_state_) {
        current_state_->enter();
    }
}

/**
 * @brief 获取当前状态。
 * @return 当前状态指针。
 */
state::UIState* UIInteractive::getCurrentState() const {
    return current_state_.get();
}

/**
 * @brief 添加精灵。
 * @param name 精灵名称。
 * @param sprite 精灵实例。
 * 
 * 如果是第一个精灵，会自动设置为当前精灵。
 */
void UIInteractive::addSprite(const std::string& name, std::unique_ptr<engine::render::Sprite> sprite) {
    sprites_[name] = std::move(sprite);
    // 如果是第一个精灵，设置为当前精灵
    if (!current_sprite_) {
        current_sprite_ = sprites_[name].get();
    }
}

/**
 * @brief 获取精灵。
 * @param name 精灵名称。
 * @return 精灵指针，如果不存在则返回nullptr。
 */
engine::render::Sprite* UIInteractive::getSprite(const std::string& name) const {
    auto it = sprites_.find(name);
    if (it != sprites_.end()) {
        return it->second.get();
    }
    return nullptr;
}

/**
 * @brief 设置当前显示的精灵。
 * @param sprite 精灵指针。
 */
void UIInteractive::setCurrentSprite(engine::render::Sprite* sprite) {
    current_sprite_ = sprite;
}

/**
 * @brief 添加声音。
 * @param name 声音名称。
 * @param sound_file 声音文件路径。
 */
void UIInteractive::addSound(const std::string& name, const std::string& sound_file) {
    sound_[name] = sound_file;
}

/**
 * @brief 获取声音文件路径。
 * @param name 声音名称。
 * @return 声音文件路径，如果不存在则返回空字符串。
 */
std::string UIInteractive::getSound(const std::string& name) const {
    auto it = sound_.find(name);
    if (it != sound_.end()) {
        return it->second;
    }
    return "";
}

/**
 * @brief 播放声音。
 * @param name 声音名称。
 */
void UIInteractive::playSound(const std::string& name) {
    auto sound_file = getSound(name);
    if (!sound_file.empty()) {
        engine::audio::AudioLocator::get().playSound(sound_file);
    }
}

/**
 * @brief 设置是否可交互。
 * @param interactive 是否可交互。
 */
void UIInteractive::setInteractive(bool interactive) {
    interactive_ = interactive;
}

/**
 * @brief 检查是否可交互。
 * @return 如果可交互，返回true；否则返回false。
 */
bool UIInteractive::isInteractive() const {
    return interactive_;
}

/**
 * @brief 设置点击事件回调。
 * @param callback 点击事件回调函数。
 */
void UIInteractive::setClickCallback(std::function<void()> callback) {
    click_callback_ = std::move(callback);
}

/**
 * @brief 触发点击事件。
 */
void UIInteractive::triggerClick() {
    if (click_callback_) {
        click_callback_();
    }
}

/**
 * @brief 检查点是否在元素内部。
 * @param point 要检查的点（屏幕坐标）。
 * @return 如果点在元素内部，返回true；否则返回false。
 */
bool UIInteractive::containsPoint(const glm::vec2& point) const {
    auto world_pos = getWorldPosition();
    return point.x >= world_pos.x && 
           point.y >= world_pos.y && 
           point.x <= world_pos.x + size_.x && 
           point.y <= world_pos.y + size_.y;
}

} // namespace engine::ui
