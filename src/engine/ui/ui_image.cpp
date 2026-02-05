#include "ui_image.h"
#include "../render/renderer.h"
#include "../render/image.h"
#include "../core/context.h"
#include <spdlog/spdlog.h>
#include <entt/core/hashed_string.hpp>

namespace engine::ui {

UIImage::UIImage(engine::core::Context& context,
                 std::string_view texture_path,
                 glm::vec2 position,
                 glm::vec2 size,
                 std::optional<SDL_FRect> source_rect,
                 bool is_flipped)
    : UIElement(context),
      image_(texture_path, std::move(source_rect), is_flipped)
{
    position_ = position;
    size_ = size;
    if (image_.getTextureId() == entt::null) {
        spdlog::warn("创建了一个空纹理ID的UIImage。");
    }
    spdlog::trace("UIImage 构造完成");
}

UIImage::UIImage(engine::core::Context& context,
                 entt::id_type texture_id,
                 glm::vec2 position,
                 glm::vec2 size,
                 std::optional<SDL_FRect> source_rect,
                 bool is_flipped)
    : UIElement(context),
      image_(texture_id, std::move(source_rect), is_flipped)
{
    position_ = position;
    size_ = size;
    if (image_.getTextureId() == entt::null) {
        spdlog::warn("创建了一个空纹理ID的UIImage。");
    }
    spdlog::trace("UIImage 构造完成");
}

UIImage::UIImage(engine::core::Context& context,
                 engine::render::Image image,
                 glm::vec2 position,
                 glm::vec2 size)
    : UIElement(context),
      image_(std::move(image))
{
    position_ = position;
    size_ = size;
    if (image_.getTextureId() == entt::null) {
        spdlog::warn("创建了一个空纹理ID的UIImage。");
    }
    spdlog::trace("UIImage 构造完成");
}

void UIImage::render() {
    if (!visible_ || image_.getTextureId() == entt::null) {
        return; // 如果不可见或没有分配纹理则不渲染
    }

    // 渲染自身
    auto position = getWorldPosition();
    if (size_.x == 0.0f && size_.y == 0.0f) {   // 如果尺寸为0，则使用纹理的原始尺寸
        context_.getRenderer().drawUIImage(image_, position);
    } else {
        context_.getRenderer().drawUIImage(image_, position, size_);
    }

    // 渲染子元素（调用基类方法）
    UIElement::render();
}

} // namespace engine::ui 