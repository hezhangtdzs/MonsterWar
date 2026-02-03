#pragma once
/**
 * @file collider_component.h
 * @brief 定义 ColliderComponent 类，用于管理游戏对象的碰撞体。
 */

#include "component.h"
#include <glm/vec2.hpp>
#include "../utils/alignment.h"
#include "../utils/math.h"
#include <memory>

namespace engine {
	namespace physics {
		class Collider;
	}
	namespace object {
		class GameObject;
	}
}

namespace engine::component {
	class TransformComponent;

	/**
	 * @class ColliderComponent
	 * @brief 碰撞体组件类，负责管理游戏对象的碰撞体和碰撞检测。
	 * 
	 * 该组件将物理碰撞体与游戏对象关联，并处理碰撞体的位置更新、对齐和激活状态。
	 */
	class ColliderComponent final: public Component {
		friend class engine::object::GameObject;
	private:
		/// 指向变换组件的指针，用于获取游戏对象的位置和旋转
		TransformComponent* transform_component_{ nullptr };
		/// 指向碰撞体的指针，包含碰撞检测的核心逻辑
		std::unique_ptr<engine::physics::Collider> collider_{ nullptr };

		/// 碰撞体相对于变换组件位置的偏移
		glm::vec2 offset_{ 0.0f, 0.0f };
		/// 碰撞体对齐方式
		engine::utils::Alignment alignment_{ engine::utils::Alignment::TOP_LEFT };
		/// 是否为触发器（Trigger），触发器不会产生物理响应
		bool is_trigger_{ false };
		/// 碰撞体是否启用
		bool is_active_{ true };

	public:
		/**
		 * @brief 构造函数，创建一个新的碰撞体组件。
		 * @param collider 碰撞体指针
		 * @param alignment 碰撞体对齐方式，默认为左上角对齐
		 * @param is_trigger 是否为触发器，默认为false
		 * @param is_active 碰撞体是否启用，默认为true
		 */
		explicit ColliderComponent(std::unique_ptr<engine::physics::Collider> collider, 
				engine::utils::Alignment alignment = engine::utils::Alignment::TOP_LEFT, 
				bool is_trigger = false, 
				bool is_active = true);

		/// 默认析构函数
		~ColliderComponent() override = default;

		/// 禁止拷贝和移动语义
		ColliderComponent(const ColliderComponent&) = delete;
		ColliderComponent& operator=(const ColliderComponent&) = delete;
		ColliderComponent(ColliderComponent&&) = delete;
		ColliderComponent& operator=(ColliderComponent&&) = delete;
		
		/// 设置器/获取器
		
		/**
		 * @brief 获取碰撞体在世界空间中的轴对齐包围盒（AABB）。
		 * @return 世界空间中的AABB
		 */
		engine::utils::Rect getWorldAABB() const;

		/**
		 * @brief 获取变换组件指针。
		 * @return 变换组件指针
		 */
		const TransformComponent* getTransform() const { return transform_component_; }

		/**
		 * @brief 设置碰撞体相对于变换组件位置的偏移。
		 * @param offset 偏移量
		 */
		void setOffset(const glm::vec2& offset) { offset_ = offset; }

		/**
		 * @brief 获取碰撞体相对于变换组件位置的偏移。
		 * @return 偏移量
		 */
		const glm::vec2& getOffset() const { return offset_; }

		/**
		 * @brief 设置碰撞体对齐方式。
		 * @param alignment 对齐方式
		 */
		void setAlignment(engine::utils::Alignment alignment);

		/**
		 * @brief 获取碰撞体对齐方式。
		 * @return 对齐方式
		 */
		engine::utils::Alignment getAlignment() const { return alignment_; }

		/**
		 * @brief 设置碰撞体是否为触发器。
		 * @param is_trigger 是否为触发器
		 */
		void setIsTrigger(bool is_trigger) { is_trigger_ = is_trigger; }

		/**
		 * @brief 获取碰撞体是否为触发器。
		 * @return 是否为触发器
		 */
		bool getIsTrigger() const { return is_trigger_; }

		/**
		 * @brief 设置碰撞体是否启用。
		 * @param is_active 是否启用
		 */
		void setIsActive(bool is_active) { is_active_ = is_active; }

		/**
		 * @brief 获取碰撞体是否启用。
		 * @return 是否启用
		 */
		bool getIsActive() const { return is_active_; }

		/**
		 * @brief 获取碰撞体指针。
		 * @return 碰撞体指针
		 */
		engine::physics::Collider* getCollider() const { return collider_.get(); }



	private:
		/**
		 * @brief 组件初始化，获取变换组件。
		 */
		void init() override;

		/**
		 * @brief 更新组件，碰撞体的位置更新由物理引擎处理。
		 * @param deltaTime 时间增量
		 * @param context 引擎上下文
		 */
		void update(float , engine::core::Context& ) override {}

		/**
		 * @brief 根据对齐方式更新碰撞体偏移。
		 */
		void updateOffset();
	};
}