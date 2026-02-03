#pragma once
#include "component.h"
#include <string>
#include <unordered_map>
#include <glm/vec2.hpp>
#include <cstdint>

namespace engine::core { class Context; }

namespace engine::component {

	/**
	 * @class AudioComponent
	 * @brief 轻量音频组件，用于将“动作/事件(action)”映射到一个音频资源路径，并按需播放。
	 * 
	 * 支持：
	 * - 按 action/id 播放（从内部表查找资源路径）
	 * - 3D/空间化播放（以监听者位置与最大距离进行衰减/裁剪）
	 * - 直接根据文件路径播放（绕过内部表）
	 * - 最小播放间隔节流（避免同一动作被过于频繁地触发）
	 */
	class AudioComponent : public Component {
	private:
		/**
		 * @brief action/id -> 音频文件路径的映射表
		 */
		std::unordered_map<std::string, std::string> action_sounds_;

		/**
		 * @brief action/id -> 上次播放时间戳（tick，单位由实现决定，通常为毫秒）
		 * 用于配合 `min_interval_ms_` 对播放进行节流。
		 */
		std::unordered_map<std::string, uint64_t> last_play_ticks_;

		/**
		 * @brief 同一 action/id 两次播放的最小间隔（毫秒）。
		 * 为 0 表示不做节流限制。
		 */
		uint64_t min_interval_ms_{ 0 };

	public:
		/**
		 * @brief 默认构造函数
		 */
		AudioComponent() = default;

		/**
		 * @brief 设置最小播放间隔（毫秒）。为 0 表示禁用节流。
		 * @param ms 最小间隔时间（毫秒）
		 */
		void setMinIntervalMs(uint64_t ms) { min_interval_ms_ = ms; }

		/**
		 * @brief 注册某个 action/id 对应的音频文件路径。
		 * 若 action 已存在，将覆盖原有映射。
		 * @param action 动作/事件标识符
		 * @param file_path 音频文件路径
		 */
		void registerSound(const std::string& action, const std::string& file_path) {
			action_sounds_[action] = file_path;
		}

		/**
		 * @brief 按 id 播放已注册的音频（会从 `action_sounds_` 查找路径）。
		 * 实现通常会结合 `min_interval_ms_` 与 `last_play_ticks_` 做节流。
		 * @param id 音频标识符
		 * @param context 游戏核心上下文
		 */
		void playSound(const std::string& id);


		/**
		 * @brief 空间化播放：按 id 播放，并以 `listener_world_pos` 作为监听者位置，
		 * `max_distance` 作为最大有效距离（超过后可静音/不播放）。
		 * @param id 音频标识符
		 * @param context 游戏核心上下文
		 * @param listener_world_pos 监听者世界位置
		 * @param max_distance 最大有效距离
		 */
		void playSoundSpatial(
			const std::string& id,
			const glm::vec2& listener_world_pos,
			float max_distance
		);


		/**
		 * @brief 便捷方法：以相机中心作为监听者位置进行空间化播放。
		 * @param id 音频标识符
		 * @param context 游戏核心上下文
		 * @param max_distance 最大有效距离
		 */
		void playSoundNearCamera(const std::string& id, engine::core::Context& context, float max_distance);

		/**
		 * @brief 直接播放指定文件路径的音频（不依赖 `action_sounds_`）。
		 * @param file_path 音频文件路径
		 * @param context 游戏核心上下文
		 */
		void playDirect(const std::string& file_path);

	};
}