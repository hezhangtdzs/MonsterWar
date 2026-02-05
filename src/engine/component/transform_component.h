#pragma once
#include <glm/vec2.hpp>


namespace engine::component {
	/**
	 * @struct TransformComponent
	 * @brief 变换组件
	 *
	 * 负责管理实体的空间状态，包括位置、旋转和缩放。这是大多数实体的基础组件。
	 */
	struct TransformComponent {
		glm::vec2 position_{ 0.0f, 0.0f };  ///< 实体在世界空间中的位置坐标 (x, y)
		glm::vec2 scale_{ 1.0f, 1.0f };     ///< 实体的渲染缩放比例 (x, y)
		float rotation_{ 0.0f };             ///< 实体的旋转角度（以度为单位）

		/** 
		 * @brief 构造函数
		 * @param position 初始位置
		 * @param scale 初始缩放
		 * @param rotation 初始旋转角度
		 */
		TransformComponent(glm::vec2 position = { 0.0f, 0.0f }, glm::vec2 scale = { 1.0f, 1.0f }, float rotation = 0.0f)
			: position_(position), scale_(scale), rotation_(rotation) {}
	};
}