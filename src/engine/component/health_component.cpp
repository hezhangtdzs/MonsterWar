#include "health_component.h"
#include "../object/game_object.h"
#include "../interface/observer.h"

#include<spdlog/spdlog.h>

namespace engine::component {

/**
 * @brief 接收伤害
 * 
 * @param damage 伤害值
 * @return bool 如果成功造成伤害返回true，否则返回false（如处于无敌状态）
 * @details 处理游戏对象受到的伤害，减少生命值并在受伤后进入无敌状态
 */
bool HealthComponent::takeDamage(int damage)
{
	if (damage <= 0 || !isAlive() || isInvincible()) return false;
	currentHealth_ -= damage;
	currentHealth_ = glm::max(0, currentHealth_);
	if (isAlive() && invincibility_duration_ > 0.0f)
	{
		invincibility_timer_ = invincibility_duration_;
	}
	spdlog::info("GameObject [{}] took {} damage, current health: {}/{}", owner_->getName(), damage, currentHealth_, maxHealth_);
	// 通知观察者生命值变化
	notifyObservers(engine::interface::EventType::HEALTH_CHANGED, currentHealth_);
	return true;

}

/**
 * @brief 恢复生命值
 * 
 * @param amount 恢复的生命值
 * @details 增加游戏对象的生命值，不会超过最大生命值
 */
void HealthComponent::heal(int amount)
{
	if (amount <= 0 || !isAlive()) return;
	currentHealth_ += amount;
	currentHealth_ = glm::min(currentHealth_, maxHealth_);
	spdlog::info("GameObject [{}] healed {} health, current health: {}/{}", owner_->getName(), amount, currentHealth_, maxHealth_);
	// 通知观察者生命值变化
	notifyObservers(engine::interface::EventType::HEALTH_CHANGED, currentHealth_);
}

/**
 * @brief 更新组件逻辑
 * 
 * @param deltaTime 时间增量（秒）
 * @param context 引擎上下文
 * @details 处理无敌状态计时器的更新，减少计时器值直到为0
 */
void HealthComponent::update(float deltaTime, engine::core::Context& context)
{
	if (invincibility_timer_ > 0.0f) {
		invincibility_timer_ -= deltaTime;
		if (invincibility_timer_ < 0.0f) {
			invincibility_timer_ = 0.0f;
		}
	}
}

}  // namespace engine::component
