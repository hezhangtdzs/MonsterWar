#pragma once
/**
 * @file health_component.h
 * @brief 定义 HealthComponent 类，用于管理游戏对象的生命值和无敌状态。
 */

#include "component.h"
#include <glm/glm.hpp>
#include "../../engine/interface/subject.h"
#include "../../engine/interface/observer.h"

namespace engine::component {
	/**
	 * @class HealthComponent
	 * @brief 生命值组件类，负责管理游戏对象的生命值、伤害处理和无敌状态。
	 * 
	 * 该组件提供了生命值管理、伤害接收、治疗和无敌状态控制等功能，
	 * 用于实现游戏中角色和敌人的生命值系统。
	 */
	class HealthComponent final :public Component, public engine::interface::Subject {
		friend class engine::object::GameObject;
	private:
		/// 最大生命值
		int maxHealth_;
		/// 当前生命值
		int currentHealth_;
		/// 无敌状态持续时间（秒）
		float invincibility_duration_ = 1.5f;
		/// 当前无敌状态计时器（秒）
		float invincibility_timer_ = 0.0f;
	public:
		/**
		 * @brief 构造函数，创建一个新的生命值组件。
		 * @param maxHealth 最大生命值
		 * @param invincibility_duration 无敌状态持续时间，默认为1.5秒
		 */
		HealthComponent(int maxHealth, float invincibility_duration = 1.5f)
			: maxHealth_(maxHealth), currentHealth_(maxHealth), invincibility_duration_(invincibility_duration) {}

		/**
		 * @brief 设置最大生命值。
		 * @param maxHealth 新的最大生命值
		 */
		void setMaxHealth(int maxHealth) {
			maxHealth_ = maxHealth;
			if (currentHealth_ > maxHealth_) {
				currentHealth_ = maxHealth_;
			}
			// 通知观察者最大生命值变化
			notifyObservers(engine::interface::EventType::MAX_HEALTH_CHANGED, maxHealth_);
		}

		/**
		 * @brief 设置当前生命值。
		 * @param currentHealth 新的当前生命值，会被限制在0到最大生命值之间
		 */
		void setCurrentHealth(int currentHealth) {
			int oldHealth = currentHealth_;
			currentHealth_ = glm::clamp(currentHealth, 0, maxHealth_);
			// 只在生命值实际变化时通知
			if (currentHealth_ != oldHealth) {
				notifyObservers(engine::interface::EventType::HEALTH_CHANGED, currentHealth_);
			}
		}

		/**
		 * @brief 设置无敌状态持续时间。
		 * @param duration 无敌状态持续时间（秒）
		 */
		void setInvincibilityDuration(float duration) {
			invincibility_duration_ = duration;
		}

		/**
		 * @brief 设置无敌状态。
		 * @param duration 无敌状态持续时间（秒）
		 */
		void setInvincible(float duration) {
			invincibility_timer_ = duration;
		}

		//getters
		
		/**
		 * @brief 获取当前生命值。
		 * @return 当前生命值
		 */
		int getCurrentHealth() const { return currentHealth_; }

		/**
		 * @brief 获取最大生命值。
		 * @return 最大生命值
		 */
		int getMaxHealth() const { return maxHealth_; }

		/**
		 * @brief 检查是否处于无敌状态。
		 * @return 处于无敌状态返回true，否则返回false
		 */
		bool isInvincible() const { return invincibility_timer_ > 0.0f; }

		/**
		 * @brief 获取无敌状态持续时间。
		 * @return 无敌状态持续时间（秒）
		 */
		float getInvincibilityDuration() const { return invincibility_duration_; }

		/**
		 * @brief 获取当前无敌状态计时器值。
		 * @return 当前无敌状态计时器值（秒）
		 */
		float getInvincibilityTimer() const { return invincibility_timer_; }

		/**
		 * @brief 接收伤害。
		 * @param damage 伤害值
		 * @return 如果成功造成伤害返回true，否则返回false（如处于无敌状态）
		 */
		bool takeDamage(int damage);

		/**
		 * @brief 恢复生命值。
		 * @param amount 恢复的生命值
		 */
		void heal(int amount);

		/**
		 * @brief 检查是否存活。
		 * @return 当前生命值大于0返回true，否则返回false
		 */
		bool isAlive() const {
			return currentHealth_ > 0;
		}
	protected:
		/**
		 * @brief 更新组件，处理无敌状态计时器。
		 * @param deltaTime 时间增量（秒）
		 * @param context 引擎上下文
		 */
		void update(float deltaTime, engine::core::Context& context) override;
	};

}