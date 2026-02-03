#include "ui_image.h"
#include "../core/context.h"
#include "../render/renderer.h"
#include <spdlog/spdlog.h>

namespace engine::ui {

/**
 * @brief 构造函数。
 * @param context 引擎上下文引用。
 * @param texture_id 纹理ID。
 * @param position 图片位置。
 * @param size 图片大小（如果为0则使用默认大小32x32）。
 */
UIImage::UIImage(engine::core::Context& context, const std::string& texture_id, 
               const glm::vec2& position, const glm::vec2& size)
    : UIElement(context), sprite_(texture_id) {
    
    // 设置位置
    setPosition(position);
    
    // 设置大小
    if (size.x > 0 && size.y > 0) {
        setSize(size);
    } else {
        // 如果没有指定大小，使用默认大小
        setSize({32.0f, 32.0f});
    }
   
    spdlog::trace("UIImage 创建完成，纹理ID: {}", texture_id);
}

/**
 * @brief 析构函数。
 */
UIImage::~UIImage() = default;

/**
 * @brief 渲染图片及其子元素。
 * 
 * 渲染过程包括：
 * 1. 检查元素是否可见
 * 2. 获取渲染器
 * 3. 计算世界位置
 * 4. 使用渲染器绘制UI精灵
 * 5. 调用父类render方法渲染子元素
 */
void UIImage::render() {
    if (!visible_) {
        return;
    }
    
    // 获取渲染器
    auto& renderer = context_.getRenderer();
    
    // 获取世界位置
    glm::vec2 world_pos = getWorldPosition();
    
    // 使用渲染器绘制UI精灵
    renderer.drawUISprite(sprite_, world_pos, size_);
    
    // 调用父类render方法渲染子元素
    UIElement::render();
}

}