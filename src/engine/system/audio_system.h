#pragma once
#include <entt/fwd.hpp>
#include "../utils/events.h"
namespace engine::system {
	/**
	 * @class AudioSystem
	 * @brief 音频系统，负责根据实体的动作状态播放对应的音效。
	 *
	 * @details
	 * AudioSystem 监听实体的动作状态变化（如攻击、受伤、死亡等），并根据实体的 AudioComponent 中
	 * 定义的动作-音效映射关系，调用音频播放器播放相应的音效。该系统与 AnimationSystem 配合使用，
	 * 确保在动画播放时同步触发正确的音效。
	 */
	class AudioSystem {
	public:
		AudioSystem(entt::registry& registry, entt::dispatcher& dispatcher);
		~AudioSystem();
	private:
		void onPlayAnimationEvent(const engine::utils::PlaySoundEvent& event);
		entt::registry& registry_;
		entt::dispatcher& dispatcher_;
	};
} // namespace engine::system