#pragma once
/**
 * @file physics_component.h
 * @brief 定义 PhysicsComponent 类，用于管理游戏对象的物理属性和运动。
 */

#include "component.h"
#include <glm/vec2.hpp>
#include <algorithm>

namespace engine {
	namespace object {
		class GameObject;
	}
}

namespace engine::physics {
	class PhysicsEngine;
}

namespace engine::component {
	class TransformComponent;

	/**
	 * @class PhysicsComponent
	 * @brief 物理组件类，负责处理游戏对象的物理属性和运动。
	 * 
	 * 该组件管理物体的速度、力、质量等物理属性，并与物理引擎交互，
	 * 处理物体的运动更新和碰撞检测结果。
	 */
	class PhysicsComponent : public Component {
		friend class engine::object::GameObject;
	public:
		/// 物体的速度向量（单位：单位/秒）
		glm::vec2 velocity_{ 0.0f, 0.0f };
	private:
		/// 指向物理引擎的指针
		engine::physics::PhysicsEngine* physics_engine_{ nullptr };
		/// 指向变换组件的指针
		TransformComponent* transform_component_{ nullptr };

		/// 作用在物体上的总力（单位：单位·质量/秒²）
		glm::vec2 force_{ 0.0f, 0.0f };
		/// 物体的质量（单位：质量单位）
		float mass_{ 1.0f };
		/// 是否受重力影响
		bool use_gravity_{ true };
		/// 物理模拟是否启用
		bool enable_{ true };

		/// 下方碰撞状态
		bool collided_below_ = false;
		/// 上方碰撞状态
		bool collided_above_ = false;
		/// 左侧碰撞状态
		bool collided_left_ = false;
		/// 右侧碰撞状态
		bool collided_right_ = false;
		/// 是否正在攀爬
		bool is_climbing_ = false;
		/// 抑制捕捉计时器（秒）
		float suppress_snap_timer_ = 0.0f;

	public:
		/**
		 * @brief 构造函数，创建一个新的物理组件。
		 * @param physics_engine 指向物理引擎的指针
		 * @param use_gravity 是否受重力影响，默认为true
		 * @param mass 物体质量，默认为1.0
		 */
		PhysicsComponent(engine::physics::PhysicsEngine* physics_engine, bool use_gravity = true, float mass = 1.0f);

		/// 析构函数
		~PhysicsComponent();

		/// 禁止拷贝和移动语义
		PhysicsComponent(const PhysicsComponent&) = delete;
		PhysicsComponent& operator=(const PhysicsComponent&) = delete;
		PhysicsComponent(PhysicsComponent&&) = delete;
		PhysicsComponent& operator=(PhysicsComponent&&) = delete;

		// PhysicsEngine使用的物理方法
		void addForce(const glm::vec2& force) { if (enable_) force_ += force; }    ///< @brief 添加力
		void clearForce() { force_ = { 0.0f, 0.0f }; }                                ///< @brief 清除力
		const glm::vec2& getForce() const { return force_; }                        ///< @brief 获取当前力
		float getMass() const { return mass_; }                                     ///< @brief 获取质量
		bool isEnabled() const { return enable_; }                                 ///< @brief 获取组件是否启用
		bool isUseGravity() const { return use_gravity_; }                          ///< @brief 获取组件是否受重力影响

		// 设置器/获取器
		
		/**
		 * @brief 设置组件是否启用。
		 * @param enabled 是否启用
		 */
		void setEnabled(bool enabled) { enable_ = enabled; }

		/**
		 * @brief 设置物体质量，质量必须为正。
		 * @param mass 质量值
		 */
		void setMass(float mass) { mass_ = (mass > 0.0f) ? mass : 1.0f; }

		/**
		 * @brief 设置组件是否受重力影响。
		 * @param use_gravity 是否受重力影响
		 */
		void setUseGravity(bool use_gravity) { use_gravity_ = use_gravity; }

		/**
		 * @brief 设置是否正在攀爬。
		 * @param is_climbing 是否正在攀爬
		 */
		void setClimbing(bool is_climbing) { is_climbing_ = is_climbing; }

		/**
		 * @brief 获取是否正在攀爬。
		 * @return 正在攀爬返回true，否则返回false
		 */
		bool isClimbing() const { return is_climbing_; }

		/**
		 * @brief 设置抑制捕捉的持续时间。
		 * @param seconds 抑制时间（秒）
		 */
		void suppressSnapFor(float seconds) { suppress_snap_timer_ = (seconds > 0.0f) ? seconds : 0.0f; }

		/**
		 * @brief 检查是否处于抑制捕捉状态。
		 * @return 处于抑制状态返回true，否则返回false
		 */
		bool isSnapSuppressed() const { return suppress_snap_timer_ > 0.0f; }

		/**
		 * @brief 更新抑制捕捉计时器。
		 * @param dt 时间增量（秒）
		 */
		void tickSnapSuppression(float dt) { if (suppress_snap_timer_ > 0.0f) suppress_snap_timer_ = std::max(0.0f, suppress_snap_timer_ - dt); }

		/**
		 * @brief 设置物体的速度。
		 * @param velocity 速度向量
		 */
		void setVelocity(const glm::vec2& velocity) { velocity_ = velocity; }

		/**
		 * @brief 获取物体的当前速度。
		 * @return 速度向量
		 */
		const glm::vec2& getVelocity() const { return velocity_; }

		/**
		 * @brief 获取变换组件指针。
		 * @return 变换组件指针
		 */
		TransformComponent* getTransform() const { return transform_component_; }

		/**
		 * @brief 重置所有碰撞标志。
		 */
		void resetCollisionFlags() {
			collided_below_ = false;
			collided_above_ = false;
			collided_left_ = false;
			collided_right_ = false;
		}

		/**
		 * @brief 检查是否与下方物体碰撞。
		 * @return 碰撞返回true，否则返回false
		 */
		bool hasCollidedBelow() const { return collided_below_; }

		/**
		 * @brief 检查是否与上方物体碰撞。
		 * @return 碰撞返回true，否则返回false
		 */
		bool hasCollidedAbove() const { return collided_above_; }

		/**
		 * @brief 检查是否与左侧物体碰撞。
		 * @return 碰撞返回true，否则返回false
		 */
		bool hasCollidedLeft() const { return collided_left_; }

		/**
		 * @brief 检查是否与右侧物体碰撞。
		 * @return 碰撞返回true，否则返回false
		 */
		bool hasCollidedRight() const { return collided_right_; }

		/**
		 * @brief 设置下方碰撞状态。
		 * @param collided 是否碰撞
		 */
		void setCollidedBelow(bool collided) { collided_below_ = collided; }

		/**
		 * @brief 设置上方碰撞状态。
		 * @param collided 是否碰撞
		 */
		void setCollidedAbove(bool collided) { collided_above_ = collided; }

		/**
		 * @brief 设置左侧碰撞状态。
		 * @param collided 是否碰撞
		 */
		void setCollidedLeft(bool collided) { collided_left_ = collided; }

		/**
		 * @brief 设置右侧碰撞状态。
		 * @param collided 是否碰撞
		 */
		void setCollidedRight(bool collided) { collided_right_ = collided; }

	private:
		/**
		 * @brief 组件初始化，获取变换组件并注册到物理引擎。
		 */
		void init() override;

		/**
		 * @brief 更新组件，处理物理模拟和位置更新。
		 * @param deltaTime 时间增量（秒）
		 * @param context 引擎上下文
		 */
		void update(float deltaTime, engine::core::Context& context) override;

		/**
		 * @brief 组件清理，从物理引擎中注销。
		 */
		void clean() override;
	};
}  // namespace engine::component