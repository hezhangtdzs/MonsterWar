#include "sprite_component.h"
#include "transform_component.h"
#include "../object/game_object.h"
#include "../core/context.h"
#include "../render/renderer.h"
#include "../resource/resource_manager.h"
#include "../render/camera.h"
#include <stdexcept>          // 用于 std::runtime_error
#include <spdlog/spdlog.h>

namespace engine::component {

    /**
     * @brief 构造一个新的 SpriteComponent。
     * @param texture_id 纹理资源的唯一标识符。
     * @param resource_manager 资源管理器引用，用于加载和查询纹理。
     * @param alignment 精灵相对于变换位置的对齐方式，默认为 NONE。
     * @param source_rect_opt 可选的源矩形，用于指定渲染纹理的特定区域（如精灵图帧）。
     * @param is_flipped 是否水平翻转精灵。
     */
    SpriteComponent::SpriteComponent(
                const std::string& texture_id,
                engine::resource::ResourceManager& resource_manager,
                engine::utils::Alignment alignment,
                std::optional<SDL_FRect> source_rect_opt,
                bool is_flipped) 
                : resource_manager_(&resource_manager), 
                sprite_(texture_id, source_rect_opt, is_flipped),
                alignment_(alignment)
{  
    if (!resource_manager_) {
        spdlog::critical("创建 SpriteComponent 时 ResourceManager 为空！，此组件将无效。");
        // 不要在游戏主循环中使用 try...catch / throw，会极大影响性能
    }
    // offset_ 和 sprite_size_ 将在 init 中计算
    spdlog::trace("创建 SpriteComponent，纹理ID: {}", texture_id);
}

    /**
     * @brief 构造一个新的 SpriteComponent。
     * @param sprite 精灵对象的右值引用。
     * @param resource_manager 资源管理器引用，用于加载和查询纹理。
     * @param alignment 精灵相对于变换位置的对齐方式，默认为 NONE。
     */
    SpriteComponent::SpriteComponent(engine::render::Sprite&& sprite, engine::resource::ResourceManager& resource_manager, engine::utils::Alignment alignment)
        : resource_manager_(&resource_manager),
          sprite_(std::move(sprite)),
          alignment_(alignment)
    {
        if (!resource_manager_) {
            spdlog::critical("创建 SpriteComponent 时 ResourceManager 为空！");
        }
        spdlog::trace("创建 SpriteComponent，纹理ID: {}", sprite_.getTextureId());
    }

    /**
     * @brief 根据当前的对齐方式和精灵尺寸更新偏移量。
     * 
     * 偏移量计算会考虑缩放因素，确保对齐在不同缩放级别下都能正确工作。
     */
    void SpriteComponent::updateOffset()
    {
        if(sprite_size_ == glm::vec2{0.0f, 0.0f}) {
            spdlog::warn("SpriteComponent 更新偏移量警告：精灵尺寸为零，无法计算偏移量。");
            return;
        }
        auto scale =transform_-> getScale();
        switch(alignment_){
            case engine::utils::Alignment::TOP_LEFT:      offset_ = glm::vec2{0.0f, 0.0f} * scale; break;
            case engine::utils::Alignment::TOP_CENTER:    offset_ = glm::vec2{-sprite_size_.x / 2.0f, 0.0f} * scale; break;
            case engine::utils::Alignment::TOP_RIGHT:     offset_ = glm::vec2{-sprite_size_.x, 0.0f} * scale; break;
            case engine::utils::Alignment::CENTER_LEFT:   offset_ = glm::vec2{0.0f, -sprite_size_.y / 2.0f} * scale; break;
            case engine::utils::Alignment::CENTER:        offset_ = glm::vec2{-sprite_size_.x / 2.0f, -sprite_size_.y / 2.0f} * scale; break;
            case engine::utils::Alignment::CENTER_RIGHT:  offset_ = glm::vec2{-sprite_size_.x, -sprite_size_.y / 2.0f} * scale; break;
            case engine::utils::Alignment::BOTTOM_LEFT:   offset_ = glm::vec2{0.0f, -sprite_size_.y} * scale; break;
            case engine::utils::Alignment::BOTTOM_CENTER: offset_ = glm::vec2{-sprite_size_.x / 2.0f, -sprite_size_.y} * scale; break;
            case engine::utils::Alignment::BOTTOM_RIGHT:  offset_ = glm::vec2{-sprite_size_.x, -sprite_size_.y} * scale; break;
            case engine::utils::Alignment::NONE:
            default:                                      break;
        }
    }

    /**
     * @brief 通过纹理 ID 更改精灵的纹理。
     * @param texture_id 新的纹理 ID。
     * @param source_rect_opt 新的可选源矩形。
     * 
     * 此方法会自动更新精灵尺寸和偏移量。
     */
    void SpriteComponent::setSpriteById(const std::string &texture_id, const std::optional<SDL_FRect> &source_rect_opt)
    {
        sprite_.setTextureId(texture_id);
        sprite_.setSourceRect(source_rect_opt);
        updateSpriteSize();
        updateOffset();
    }

    /**
     * @brief 设置精灵的源矩形。
     * @param source_rect_opt 源矩形，为 std::nullopt 时渲染整个纹理。
     * 
     * 此方法会自动更新精灵尺寸和偏移量。
     */
    void SpriteComponent::setSourceRect(const std::optional<SDL_FRect> &source_rect_opt)
    {
        sprite_.setSourceRect(source_rect_opt);
        updateSpriteSize();
        updateOffset();
    }

    /**
     * @brief 设置精灵的对齐方式并更新偏移。
     * @param anchor 新的对齐锚点。
     */
    void SpriteComponent::setAlignment(engine::utils::Alignment anchor)
    {
        alignment_ = anchor;
        updateOffset();
    }

    /**
     * @brief 更新精灵的逻辑尺寸属性。
     * 
     * 如果设置了源矩形，尺寸将基于源矩形；否则，尺寸将基于整个纹理。
     */
    void SpriteComponent::updateSpriteSize()
    {
        if(!resource_manager_) {
            spdlog::critical("SpriteComponent 更新精灵尺寸失败：ResourceManager 为空！");
            return;
        }
        if(sprite_.getSourceRect().has_value()) {
            const SDL_FRect& src_rect = sprite_.getSourceRect().value();
            sprite_size_.x = src_rect.w;
            sprite_size_.y = src_rect.h;
        } else {
            sprite_size_ = resource_manager_->getTextureSize(sprite_.getTextureId());
        }
    }

    /**
     * @brief 组件初始化。
     * 
     * 初始化过程包括：
     * 1. 获取所属游戏对象的 TransformComponent 引用
     * 2. 更新精灵尺寸
     * 3. 更新偏移量
     */
    void SpriteComponent::init()
    {
        if (!owner_) {
            spdlog::critical("SpriteComponent 初始化失败：所属的 GameObject 为空！");
            return;
        }
        transform_ = owner_->getComponent<TransformComponent>();
        if (!transform_) {
            spdlog::warn("SpriteComponent 初始化警告：所属的 GameObject '{}' 没有 TransformComponent，位置和对齐可能无法正确计算。", owner_->getName());
        }
        updateSpriteSize();
        updateOffset();
    }

    /**
     * @brief 每帧更新逻辑。
     * @param deltaTime 自上一帧的时间间隔（秒）。
     * @param context 引擎上下文环境。
     * 
     * SpriteComponent 的 update 方法为空实现，因为它不需要每帧更新逻辑。
     */
    void SpriteComponent::update(float /*deltaTime*/, engine::core::Context &/*context*/)
    {
    }

    /**
     * @brief 渲染精灵。
     * @param context 引擎上下文环境。
     * 
     * 渲染过程包括：
     * 1. 检查是否隐藏或缺少必要组件
     * 2. 计算最终渲染位置（考虑偏移量）
     * 3. 调用渲染器绘制精灵
     */
    void SpriteComponent::render(engine::core::Context &context)
    {
        if(is_hidden_ || !transform_ || !resource_manager_) {
            return;
        }
        const glm::vec2& position = transform_->getPosition() + offset_;
        const glm::vec2& scale = transform_->getScale();
        float rotation = transform_->getRotation();
        context.getRenderer().drawSprite(context.getCamera(),sprite_, position, scale, rotation);
    }

} // namespace engine::component