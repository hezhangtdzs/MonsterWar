#pragma once
#include "component.h"
#include <glm/vec2.hpp>
#include <spdlog/spdlog.h>

namespace engine::component {
	/**
	 * @class TransformComponent
	 * @brief 负责管理游戏对象的位置、旋转和缩放的组件。
	 *
	 * 该组件提供了对游戏对象在2D空间中的基本变换操作。
	 */
	class TransformComponent final : public Component {
		friend class engine::object::GameObject;
	private:
		glm::vec2 position_{ 0.0f, 0.0f }; ///< 位置坐标 (x, y)
		float rotation_{ 0.0f };            ///< 旋转角度（度）
		glm::vec2 scale_{ 1.0f, 1.0f };    ///< 缩放比例 (x, y)
	public:
		/**
		 * @brief 构造一个新的 TransformComponent 对象。
		 * @param position 初始位置坐标，默认为 (0, 0)。
		 * @param rotation 初始旋转角度（度），默认为 0.0f。
		 * @param scale 初始缩放比例，默认为 (1.0, 1.0)。
		 */
		TransformComponent(glm::vec2 position = { 0,0 }, float rotation = 0.0f, glm::vec2 scale = { 1.0f,1.0f })
			: position_(position), rotation_(rotation), scale_(scale) {
		}

		// 禁止拷贝和移动以确保组件生命周期管理的安全性
		TransformComponent(const TransformComponent&) = delete;
		TransformComponent& operator=(const TransformComponent&) = delete;
		TransformComponent(TransformComponent&&) = delete;
		TransformComponent& operator=(TransformComponent&&) = delete;

		/**
		 * @brief 获取位置坐标。
		 * @return glm::vec2 当前的位置坐标的常量引用。
		 */
		const glm::vec2& getPosition() const {
			return position_;
		}

		/**
		 * @brief 设置位置坐标。
		 * @param position 新的位置坐标。
		 */
		void setPosition(const glm::vec2& position) {
			position_ = position;
		}

		/**
		 * @brief 获取旋转角度。
		 * @return float 当前的旋转角度（度）。
		 */
		const float getRotation() const {
			return rotation_;
		}

		/**
		 * @brief 设置旋转角度。
		 * @param rotation 新的旋转角度（度）。
		 */
		void setRotation(float rotation) {
			rotation_ = rotation;
		}

		/**
		 * @brief 获取缩放比例。
		 * @return glm::vec2 当前的缩放比例的常量引用。
		 */
		const glm::vec2& getScale() const {
			return scale_;
		}

		/**
		 * @brief 设置缩放比例。
		 * @param scale 新的缩放比例。
		 */
		void setScale(const glm::vec2& scale);

		void translate(const glm::vec2& offset);
	};
}