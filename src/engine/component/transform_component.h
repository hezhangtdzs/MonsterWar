/**
 * @file transform_component.h
 * @brief 定义 TransformComponent 组件，用于ECS架构中的实体变换。
 *
 * @details
 * TransformComponent 是ECS架构中的核心组件之一，负责存储和管理实体在2D世界空间中的
 * 位置、旋转和缩放信息。大多数实体都需要此组件来定义其在世界中的空间状态。
 *
 * 在ECS架构中，该组件仅包含数据，不包含任何逻辑。所有变换相关的计算和更新
 * 由专门的系统（如 MovementSystem）负责处理。
 *
 * @par 使用示例
 * @code
 * // 创建实体并添加变换组件
 * auto entity = registry.create();
 * registry.emplace<TransformComponent>(entity, glm::vec2(100.0f, 200.0f));
 *
 * // 获取和修改变换数据
 * auto& transform = registry.get<TransformComponent>(entity);
 * transform.position_ += glm::vec2(10.0f, 0.0f);  // 向右移动
 * @endcode
 *
 * @see engine::system::MovementSystem 处理变换更新的系统
 */

#pragma once
#include <glm/vec2.hpp>

namespace engine::component {
	/**
	 * @struct TransformComponent
	 * @brief ECS变换组件，存储实体的空间变换数据。
	 *
	 * @details
	 * 这是ECS架构中最基础的组件之一，几乎所有需要渲染或具有物理位置的实体
	 * 都需要挂载此组件。该组件采用纯数据结构（POD）设计，便于系统批量处理。
	 *
	 * @par 与其他组件的关系
	 * - 与 SpriteComponent 配合：定义精灵的渲染位置和大小
	 * - 与 VelocityComponent 配合：定义实体的移动速度和方向
	 * - 与 AnimationComponent 配合：动画播放的位置基准
	 */
	struct TransformComponent {
		glm::vec2 position_{ 0.0f, 0.0f };  ///< 实体在世界空间中的位置坐标 (x, y)，单位为像素
		glm::vec2 scale_{ 1.0f, 1.0f };     ///< 实体的渲染缩放比例 (x, y)，1.0为原始大小
		float rotation_{ 0.0f };             ///< 实体的旋转角度（以度为单位），0表示无旋转

		/**
		 * @brief 默认构造函数，创建位于原点、无缩放、无旋转的变换组件
		 * @param position 初始位置，默认为 (0, 0)
		 * @param scale 初始缩放，默认为 (1, 1)
		 * @param rotation 初始旋转角度，默认为 0 度
		 */
		TransformComponent(glm::vec2 position = { 0.0f, 0.0f }, glm::vec2 scale = { 1.0f, 1.0f }, float rotation = 0.0f)
			: position_(position), scale_(scale), rotation_(rotation) {}
	};
}