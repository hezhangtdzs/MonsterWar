#include "ui_button.h"
#include "../core/context.h"
#include "../input/input_manager.h"
#include "../render/sprite.h"
#include "state/normal_state.h"
#include <glm/glm.hpp>

namespace engine::ui {

/**
 * @brief 构造函数（文本按钮）。
 * @param context 引擎上下文引用。
 * @param text 按钮文本。
 * @param font_path 字体文件路径。
 * @param font_size 字体大小。
 */
UIButton::UIButton(engine::core::Context& context, const std::string& text, const std::string& font_path, int font_size)
    : UIInteractive(context) {
    // 创建文本元素
    auto text_element = std::make_unique<UIText>(context, text, font_path, font_size);
    
    // 保存文本元素指针
    text_element_ = text_element.get();
    
    // 设置文本元素的位置居中对齐
    text_element->setAlignment(TextAlignment::CENTER);
    addSound("hover", "assets/audio/button_hover.wav");
    addSound("pressed", "assets/audio/button_click.wav");
    // 添加文本元素作为子元素
    addChild(std::move(text_element));
    
    // 设置初始按钮大小并自动更新文本位置
    setSize({ 200.0f, 50.0f });
    
    // 设置初始状态为正常状态
    setState(std::make_unique<state::NormalState>(this));
}

/**
 * @brief 构造函数（精灵按钮）。
 * @param context 引擎上下文引用。
 * @param normal_sprite_path 正常状态精灵路径。
 * @param hover_sprite_path 悬停状态精灵路径。
 * @param pressed_sprite_path 按下状态精灵路径。
 * @param position 按钮位置。
 * @param size 按钮大小（如果为(0,0)则使用默认大小100x50）。
 * @param callback 点击回调函数。
 */
UIButton::UIButton(
    engine::core::Context& context,
    const std::string& normal_sprite_path,
    const std::string& hover_sprite_path,
    const std::string& pressed_sprite_path,
    const glm::vec2& position,
    const glm::vec2& size,
    std::function<void()> callback
) : UIInteractive(context) {
    // 设置按钮位置
    setPosition(position);
    
    // 创建并添加精灵
    auto normal_sprite = std::make_unique<render::Sprite>(normal_sprite_path);
    auto hover_sprite = std::make_unique<render::Sprite>(hover_sprite_path);
    auto pressed_sprite = std::make_unique<render::Sprite>(pressed_sprite_path);
    
    addSprite("normal", std::move(normal_sprite));
    addSprite("hover", std::move(hover_sprite));
    addSprite("pressed", std::move(pressed_sprite));
    addSound("hover", "assets/audio/button_hover.wav");
    addSound("pressed", "assets/audio/button_click.wav");
    // 设置按钮大小
    if (size.x > 0 && size.y > 0) {
        setSize(size);
    } else {
        // 如果没有指定大小，使用精灵大小
        auto normal_sprite_ptr = getSprite("normal");
        if (normal_sprite_ptr) {
            // 这里需要获取精灵的大小，假设Sprite类有getSize方法
            // setSize(normal_sprite_ptr->getSize());
            // 暂时使用默认大小
            setSize({ 100.0f, 50.0f });
        }
    }
    
    // 设置点击回调
    setClickCallback(std::move(callback));
    
    // 设置初始状态为正常状态
    setState(std::make_unique<state::NormalState>(this));
}

/**
 * @brief 析构函数。
 */
UIButton::~UIButton() = default;

/**
 * @brief 处理输入事件。
 * @return 如果事件被处理，返回true；否则返回false。
 */
bool UIButton::handleInput() {
    return UIInteractive::handleInput();
}

/**
 * @brief 设置元素大小。
 * @param size 元素大小。
 */
void UIButton::setSize(const glm::vec2& size) {
    UIInteractive::setSize(size);
    updateTextPosition();
}

/**
 * @brief 更新文本位置，使其处于按钮中心。
 */
void UIButton::updateTextPosition() {
    if (text_element_) {
        // 设置文本位置为按钮的中心点
        // 由于设置了 TextAlignment::CENTER，UIText 会自动以此位置为中心进行渲染
        text_element_->setPosition(size_ * 0.5f);
    }
}

/**
 * @brief 设置点击事件回调。
 * @param callback 点击事件回调函数。
 */
void UIButton::setClickCallback(std::function<void()> callback) {
    click_callback_ = std::move(callback);
    // 设置UIInteractive的点击回调
    UIInteractive::setClickCallback([this]() {
        if (click_callback_) {
            click_callback_();
        }
    });
}

/**
 * @brief 获取按钮文本。
 * @return 按钮文本。
 */
const std::string& UIButton::getText() const {
    return text_element_->getText();
}

/**
 * @brief 设置按钮文本。
 * @param text 按钮文本。
 */
void UIButton::setText(const std::string& text) {
    text_element_->setText(text);
}

/**
 * @brief 设置正常状态背景颜色。
 * @param color 正常状态背景颜色。
 */
void UIButton::setNormalBgColor(const FColor& color) {
    normal_bg_color_ = color;
}

/**
 * @brief 设置悬停状态背景颜色。
 * @param color 悬停状态背景颜色。
 */
void UIButton::setHoverBgColor(const FColor& color) {
    hover_bg_color_ = color;
}

/**
 * @brief 设置按下状态背景颜色。
 * @param color 按下状态背景颜色。
 */
void UIButton::setPressedBgColor(const FColor& color) {
    pressed_bg_color_ = color;
}

/**
 * @brief 设置正常状态文本颜色。
 * @param color 正常状态文本颜色。
 */
void UIButton::setNormalTextColor(const FColor& color) {
    normal_text_color_ = color;
    if (text_element_) {
        text_element_->setColor(color);
    }
}

/**
 * @brief 设置悬停状态文本颜色。
 * @param color 悬停状态文本颜色。
 */
void UIButton::setHoverTextColor(const FColor& color) {
    hover_text_color_ = color;
}

/**
 * @brief 设置按下状态文本颜色。
 * @param color 按下状态文本颜色。
 */
void UIButton::setPressedTextColor(const FColor& color) {
    pressed_text_color_ = color;
}

} // namespace engine::ui