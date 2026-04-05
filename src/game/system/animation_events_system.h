#pragma once
#include "../../engine/utils/events.h"
#include <entt/entity/fwd.hpp>
#include <entt/signal/fwd.hpp>

namespace game::system {
	/**
	 * @brief 动画事件处理系统。
	 *
	 * 负责监听并处理来自动画系统的事件，
	 * 并在需要时将事件分发到具体的游戏逻辑（例如命中判定）。
	 */
	class AnimationEventsSystem {
	public:
		/**
		 * @brief 构造动画事件处理系统并完成事件订阅初始化。
		 * @param registry EnTT 实体注册表引用，用于访问和操作实体组件。
		 * @param dispatcher EnTT 事件分发器引用，用于接收动画事件。
		 */
		AnimationEventsSystem(entt::registry& registry, entt::dispatcher& dispatcher);

		/**
		 * @brief 析构函数。
		 *
		 * 用于清理系统生命周期内建立的事件连接或相关资源。
		 */
		~AnimationEventsSystem();

	private:
		/**
		 * @brief 动画事件总入口回调。
		 * @param event 动画系统抛出的动画事件。
		 */
		void onAnimationEvent(const engine::utils::AnimationEvent& event);

		/**
		 * @brief 处理命中类型动画事件。
		 * @param event 当前命中相关的动画事件数据。
		 */
		void handleHitEvent(const engine::utils::AnimationEvent& event);

		/** @brief 实体注册表引用。 */
		entt::registry& registry_;

		/** @brief 事件分发器引用。 */
		entt::dispatcher& dispatcher_;
	};
}