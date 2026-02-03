#include "audio_component.h"
#include "../core/context.h"
#include "../audio/audio_locator.h" // Added AudioLocator
#include "transform_component.h"
#include "../object/game_object.h"
#include "../render/camera.h" // Added Camera for playSoundNearCamera
#include <spdlog/spdlog.h>
#include <SDL3/SDL.h>
#include <cmath>

namespace engine::component {
	/**
	 * @brief 检查是否可以播放指定ID的音频
	 * 
	 * @param id 音频标识符
	 * @param min_interval_ms 最小播放间隔（毫秒）
	 * @param last_play_ticks 上次播放时间戳映射表
	 * @return bool 是否可以播放
	 * @details 根据最小播放间隔检查是否可以播放音频，若可以则更新时间戳
	 */
	static bool canPlay(const std::string& id, uint64_t min_interval_ms, std::unordered_map<std::string, uint64_t>& last_play_ticks) {
		if (min_interval_ms == 0) return true;
		const uint64_t now = SDL_GetTicks();
		auto it = last_play_ticks.find(id);
		if (it != last_play_ticks.end() && (now - it->second) < min_interval_ms) {
			return false;
		}
		last_play_ticks[id] = now;
		return true;
	}

	/**
	 * @brief 按 id 播放已注册的音频
	 * 
	 * @param id 音频标识符
	 * @details 从内部映射表查找音频路径并播放，应用播放间隔节流
	 */
	void AudioComponent::playSound(const std::string& id) {
		auto it = action_sounds_.find(id);
		if (it == action_sounds_.end()) {
			spdlog::debug("音效ID '{}' 没有关联的资源路径。", id);
			return;
		}
		if (!canPlay(id, min_interval_ms_, last_play_ticks_)) return;
		engine::audio::AudioLocator::get().playSound(it->second);
	}

	/**
	 * @brief 空间化播放音频
	 * 
	 * @param id 音频标识符
	 * @param listener_world_pos 监听者世界位置
	 * @param max_distance 最大有效距离
	 * @details 计算音频发射器与监听者的距离，应用空间化效果并播放
	 */
	void AudioComponent::playSoundSpatial(const std::string& id, const glm::vec2& listener_world_pos, float max_distance) {
		auto it = action_sounds_.find(id);
		if (it == action_sounds_.end()) {
			spdlog::debug("音效ID '{}' 没有关联的资源路径。", id);
			return;
		}
		if (!canPlay(id, min_interval_ms_, last_play_ticks_)) return;

		glm::vec2 emitter = listener_world_pos;
		if (owner_) {
			if (auto* tc = owner_->getComponent<engine::component::TransformComponent>()) {
				emitter = tc->getPosition();
			}
		}
		const glm::vec2 delta = emitter - listener_world_pos;
		const float dist2 = (delta.x * delta.x) + (delta.y * delta.y);
		const float dist = std::sqrt(dist2);

		spdlog::trace(
			"AudioComponent: spatial play id='{}' path='{}' emitter=({}, {}) listener=({}, {}) dist={} max={}",
			id,
			it->second,
			emitter.x,
			emitter.y,
			listener_world_pos.x,
			listener_world_pos.y,
			dist,
			max_distance
		);
		engine::audio::AudioLocator::get().playSoundSpatial(it->second, emitter, listener_world_pos, max_distance);
	}

	/**
	 * @brief 以相机中心作为监听者位置进行空间化播放
	 * 
	 * @param id 音频标识符
	 * @param context 游戏核心上下文
	 * @param max_distance 最大有效距离
	 * @details 计算相机中心位置并调用空间化播放方法
	 */
	void AudioComponent::playSoundNearCamera(const std::string& id, engine::core::Context& context, float max_distance) {
		const auto listener = context.getCamera().getPosition() + context.getCamera().getViewportSize() * 0.5f;
		playSoundSpatial(id, listener, max_distance);
	}

	/**
	 * @brief 直接播放指定文件路径的音频
	 * 
	 * @param file_path 音频文件路径
	 * @details 绕过内部映射表，直接根据文件路径播放音频
	 */
	void AudioComponent::playDirect(const std::string& file_path) {
		engine::audio::AudioLocator::get().playSound(file_path);
	}
}
