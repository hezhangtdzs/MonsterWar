#include "ui_text.h"
#include "../core/context.h"
#include "../render/camera.h"
#include <glm/glm.hpp>

namespace engine::ui {

/**
 * @brief 构造函数。
 * @param context 引擎上下文引用。
 * @param text 初始文本内容。
 * @param font_path 字体文件路径。
 * @param font_size 字体大小。
 */
UIText::UIText(engine::core::Context& context, const std::string& text, const std::string& font_path, int font_size)
    : UIElement(context),
      text_(text),
      font_path_(font_path),
      font_size_(font_size) {
    is_dirty_ = true;
}

/**
 * @brief 析构函数。
 */
UIText::~UIText() = default;

const glm::vec2& UIText::getSize() const
{
    ensureUpToDate();
    return size_;
}

/**
 * @brief 渲染文本及其子元素。
 * 
 * 渲染过程包括：
 * 1. 检查元素是否可见
 * 2. 获取文本渲染器
 * 3. 计算世界位置（UI世界位置即为屏幕位置）
 * 4. 根据对齐方式调整文本位置
 * 5. 使用TextRenderer绘制UI文本
 * 6. 调用父类render方法渲染子元素
 */
void UIText::render() {
    if (!visible_) {
        return;
    }

    ensureUpToDate();

    // 获取文本渲染器
    auto& text_renderer = context_.getTextRenderer();
    
    // 计算世界位置（在此系统中，UI 世界位置即为屏幕位置）
    glm::vec2 world_pos = getWorldPosition();
    
    // 根据对齐方式调整文本位置
    glm::vec2 render_pos = world_pos;
    
    if (alignment_ != TextAlignment::LEFT) {
        const glm::vec2& text_size = size_;
        if (alignment_ == TextAlignment::CENTER) {
            render_pos.x -= text_size.x * 0.5f;
            render_pos.y -= text_size.y * 0.5f; // 同时进行水平和垂直居中
        } else if (alignment_ == TextAlignment::RIGHT) {
            render_pos.x -= text_size.x;
        }
    }
    
    // 使用TextRenderer绘制UI文本（屏幕空间）
    text_renderer.drawUIText(text_,
                           font_path_,
                           font_size_,
                           render_pos,
                           color_,
                           is_dirty_);

    is_dirty_ = false;
    
    // 调用父类render方法渲染子元素
    UIElement::render();
}

void UIText::onNotify(engine::interface::EventType event_type, const std::any &data)
{
    if (event_type == engine::interface::EventType::SCORE_CHANGED) {
        // 安全地从 std::any 中提取数据
        if (const int* score = std::any_cast<int>(&data)) {
            setText("Score: " + std::to_string(*score));
        }
    }
}

void UIText::updateSize()
{
    size_ = context_.getTextRenderer().getTextSize(text_, font_path_, font_size_, is_dirty_);
}

void UIText::ensureUpToDate() const
{
    if (!is_dirty_) {
        return;
    }

    const_cast<UIText*>(this)->updateSize();
}
}
