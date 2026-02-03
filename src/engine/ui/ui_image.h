#pragma once
/**
 * @file ui_image.h
 * @brief 定义UIImage类，用于在UI上显示图片。
 */

#include "ui_element.h"
#include "../render/sprite.h"

namespace engine::core {
    class Context;
}

namespace engine::ui {

/**
 * @class UIImage
 * @brief UI图片类，用于在UI上显示图片。
 * 
 * UIImage类主要功能包括：
 * - 显示图片内容
 * - 支持设置图片位置和大小
 * - 支持设置透明度
 */
class UIImage : public UIElement {
private:
    /// 精灵对象，用于存储纹理信息
    engine::render::Sprite sprite_;
    /// 图片透明度
    float opacity_ = 1.0f;

public:
    /**
     * @brief 构造函数。
     * @param context 引擎上下文引用。
     * @param texture_id 纹理ID。
     * @param position 图片位置。
     * @param size 图片大小（如果为0则使用纹理原始大小）。
     */
    UIImage(engine::core::Context& context, const std::string& texture_id, 
           const glm::vec2& position = {0.0f, 0.0f}, 
           const glm::vec2& size = {0.0f, 0.0f});

    /**
     * @brief 析构函数。
     */
    ~UIImage() override;

    /**
     * @brief 渲染图片。
     */
    void render() override;

    // Getters and Setters
    /**
     * @brief 获取透明度。
     * @return 透明度值（0.0f-1.0f）。
     */
    float getOpacity() const { return opacity_; }

    /**
     * @brief 设置透明度。
     * @param opacity 透明度值（0.0f-1.0f）。
     */
    void setOpacity(float opacity) { opacity_ = opacity; }

    /**
     * @brief 获取精灵对象。
     * @return 精灵对象引用。
     */
    engine::render::Sprite& getSprite() { return sprite_; }
    const engine::render::Sprite& getSprite() const { return sprite_; }
};

}