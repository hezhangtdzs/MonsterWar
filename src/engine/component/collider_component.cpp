#include "collider_component.h"
#include <spdlog/spdlog.h>
#include <glm/glm.hpp>
#include "../physics/collider.h"
#include "../object/game_object.h"
#include "transform_component.h"

/**
 * @brief 构造函数，创建一个新的碰撞体组件。
 * @param collider 碰撞体指针
 * @param alignment 碰撞体对齐方式，默认为左上角对齐
 * @param is_trigger 是否为触发器，默认为false
 * @param is_active 碰撞体是否启用，默认为true
 */
engine::component::ColliderComponent::ColliderComponent(std::unique_ptr<engine::physics::Collider> collider,
	engine::utils::Alignment alignment, bool is_trigger, bool is_active)
	: collider_(std::move(collider)),
	  alignment_(alignment),
	  is_trigger_(is_trigger),
	is_active_(is_active)
{
	if (!collider_) {
		spdlog::error("ColliderComponent 创建失败：传入的 Collider 指针为空");
	} else {
		spdlog::trace("ColliderComponent 创建完成，类型: {}", static_cast<int>(collider_->getType()));
	}
}

/**
 * @brief 获取碰撞体在世界空间中的轴对齐包围盒（AABB）。
 * @return 世界空间中的AABB
 * 
 * 计算过程包括：
 * 1. 获取变换组件的位置
 * 2. 应用偏移量
 * 3. 考虑缩放因素
 * 4. 计算最终的AABB
 */
engine::utils::Rect engine::component::ColliderComponent::getWorldAABB() const
{
	if (!transform_component_ || !collider_) {
		spdlog::error("ColliderComponent 获取世界AABB失败：缺少 TransformComponent 或 Collider");
		return engine::utils::Rect{{0,0},{0,0}};
	}
	auto top_left = transform_component_->getPosition() + offset_;
	auto scale = transform_component_->getScale();
	auto size = collider_->getAABBSize()*scale;
	return engine::utils::Rect{top_left, size};
}

/**
 * @brief 设置碰撞体对齐方式。
 * @param alignment 对齐方式
 * 
 * 设置对齐方式后，会自动更新碰撞体的偏移量。
 */
void engine::component::ColliderComponent::setAlignment(engine::utils::Alignment alignment)
{
	alignment_ = alignment;
	if(transform_component_&&collider_){
		updateOffset();
	}
}

/**
 * @brief 组件初始化，获取变换组件。
 * 
 * 初始化过程包括：
 * 1. 检查所属游戏对象是否存在
 * 2. 获取所属游戏对象的 TransformComponent
 * 3. 更新碰撞体偏移量
 */
void engine::component::ColliderComponent::init()
{
	if(owner_) {
		transform_component_ = owner_->getComponent<TransformComponent>();
		if (!transform_component_) {
			spdlog::error("ColliderComponent 初始化失败：所属对象缺少 TransformComponent 组件");
		} else {
			spdlog::trace("ColliderComponent 初始化完成");
		}
	} else {
		spdlog::error("ColliderComponent 初始化失败：所属对象为空");
	}
	updateOffset();
}

/**
 * @brief 根据对齐方式更新碰撞体偏移。
 * 
 * 偏移量计算会考虑缩放因素，确保对齐在不同缩放级别下都能正确工作。
 */
void engine::component::ColliderComponent::updateOffset()
{
	if (!transform_component_ || !collider_) {
		return;
	}
	glm::vec2 position = transform_component_->getPosition();
	glm::vec2 collider_size = collider_->getAABBSize();
	auto scale = transform_component_->getScale();
	const glm::vec2 scaled_size = collider_size * scale;
	// 根据对齐方式调整偏移量
	switch (alignment_) {
	case engine::utils::Alignment::TOP_LEFT:
		offset_ = { 0, 0 };
		break;
	case engine::utils::Alignment::TOP_CENTER:
		offset_ = glm::vec2{ -scaled_size.x / 2.0f, 0.0f };
		break;
	case engine::utils::Alignment::TOP_RIGHT:
		offset_ = glm::vec2{ -scaled_size.x, 0.0f };
		break;
	case engine::utils::Alignment::CENTER_LEFT:
		offset_ = glm::vec2{ 0.0f, -scaled_size.y / 2.0f };
		break;
	case engine::utils::Alignment::CENTER:
		offset_ = glm::vec2{ -scaled_size.x / 2.0f, -scaled_size.y / 2.0f };
		break;
	case engine::utils::Alignment::CENTER_RIGHT:
		offset_ = glm::vec2{ -scaled_size.x, -scaled_size.y / 2.0f };
		break;
	case engine::utils::Alignment::BOTTOM_LEFT:
		offset_ = glm::vec2{ 0.0f, -scaled_size.y };
		break;
	case engine::utils::Alignment::BOTTOM_CENTER:
		offset_ = glm::vec2{ -scaled_size.x / 2.0f, -scaled_size.y };
		break;
	case engine::utils::Alignment::BOTTOM_RIGHT:
		offset_ = glm::vec2{ -scaled_size.x, -scaled_size.y };
		break;
	default:
		break;
	}
	
}


