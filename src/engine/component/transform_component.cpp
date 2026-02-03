#include "transform_component.h"
#include "component.h"
#include <glm/glm.hpp>
#include "../object/game_object.h"
#include "sprite_component.h"

// Keep collider offsets consistent with scaling changes
#include "collider_component.h"

/**
 * @brief 设置缩放比例。
 * @param scale 新的缩放比例。
 * 
 * 设置缩放比例后，会自动更新：
 * 1. 精灵组件的偏移量
 * 2. 碰撞器组件的对齐方式
 */
void engine::component::TransformComponent::setScale(const glm::vec2& scale)
{
    scale_ = scale;
    if (owner_) {
        auto sprite_comp = owner_->getComponent<SpriteComponent>();
        if (sprite_comp) {
            sprite_comp->updateOffset();
        }
		auto collider_comp = owner_->getComponent<ColliderComponent>();
		if (collider_comp) {
			collider_comp->setAlignment(collider_comp->getAlignment());
		}
    }
}

/**
 * @brief 平移游戏对象的位置。
 * @param offset 平移的偏移量。
 * 
 * 该方法会将当前位置加上指定的偏移量，实现游戏对象的平移。
 */
void engine::component::TransformComponent::translate(const glm::vec2& offset)
{
	position_ += offset;
}
