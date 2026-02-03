/**
 * @file audio_player.cpp
 * @brief AudioPlayer 类的实现，负责游戏中的音效和音乐播放。
 */

#include "audio_player.h"
#include "../core/config.h"
#include "../resource/resource_manager.h"
#include <algorithm>
#include <cmath>

namespace engine::audio {
	/**
	 * @brief 将值限制在 0.0f 到 1.0f 之间。
	 * @param v 输入值
	 * @return 限制后的值
	 */
	static float clamp01(float v) {
		return std::clamp(v, 0.0f, 1.0f);
	}

	/**
	 * @brief 构造函数，创建一个新的音频播放器。
	 * @param resource_manager 资源管理器引用
	 */
	AudioPlayer::AudioPlayer(engine::resource::ResourceManager& resource_manager, engine::core::Config& config)
		: resource_manager_(resource_manager), config_(config) {
			setMasterVolume(config_.master_volume_);
			setSoundVolume(config_.sound_volume_);
			setMusicVolume(config_.music_volume_);
	}

	/**
	 * @brief 设置主音量。
	 * @param volume 音量值 (0.0f - 1.0f)
	 */
	void AudioPlayer::setMasterVolume(float volume) {
		master_volume_ = clamp01(volume);
		resource_manager_.setMasterGain(master_volume_);
	}

	/**
	 * @brief 设置音效音量。
	 * @param volume 音量值 (0.0f - 1.0f)
	 */
	void AudioPlayer::setSoundVolume(float volume) {
		sound_volume_ = clamp01(volume);
		resource_manager_.setSoundGain(master_volume_ * sound_volume_);
	}

	/**
	 * @brief 设置音乐音量。
	 * @param volume 音量值 (0.0f - 1.0f)
	 */
	void AudioPlayer::setMusicVolume(float volume) {
		music_volume_ = clamp01(volume);
		resource_manager_.setMusicGain(master_volume_ * music_volume_);
	}

	/**
	 * @brief 播放音效。
	 * @param path 音效文件路径
	 * @return 播放通道ID，失败返回-1
	 * 
	 * @details 当前 ResourceManager/AudioManager 不支持每个音效的增益控制。
	 * 保持分层约定；音量旋钮已存储，以备将来混音器增益连接使用。
	 */
	int AudioPlayer::playSound(const std::string& path) {
		// Current ResourceManager/AudioManager does not expose per-sound gain.
		// Keep layering contract; volume knobs are stored for future mixer gain wiring.
		resource_manager_.playSound(path);
		return 0;
	}

	/**
	 * @brief 播放空间化音效。
	 * @param path 音效文件路径
	 * @param emitter_world_pos 发射器世界位置
	 * @param listener_world_pos 听众世界位置
	 * @param max_distance 最大有效距离
	 * @return 播放通道ID，失败返回-1
	 * 
	 * @details 根据发射器和听众之间的距离计算音量衰减，实现空间化音效。
	 * 如果距离超过最大有效距离，则不播放音效。
	 */
	int AudioPlayer::playSoundSpatial(const std::string& path, const glm::vec2& emitter_world_pos, const glm::vec2& listener_world_pos, float max_distance) {
		if (max_distance <= 0.0f) {
			return playSound(path);
		}

		const glm::vec2 delta = emitter_world_pos - listener_world_pos;
		const float dist2 = (delta.x * delta.x) + (delta.y * delta.y);
		const float max2 = max_distance * max_distance;
		if (dist2 > max2) {
			return 0;
		}

		// Simple linear attenuation based on distance.
		const float dist = std::sqrt(dist2);
		const float t = 1.0f - (dist / max_distance);
		const float gain = std::clamp(t, 0.0f, 1.0f);

		// Current AudioManager exposes only tag-level gains, so approximate by temporarily scaling sound gain.
		// This is a best-effort approach until per-track/per-play gains are exposed.
		const float base_gain = master_volume_ * sound_volume_;
		resource_manager_.setSoundGain(base_gain * gain);
		resource_manager_.playSound(path);
		resource_manager_.setSoundGain(base_gain);
		return 0;
	}

	/**
	 * @brief 播放背景音乐。
	 * @param path 音乐文件路径
	 * @param loops 循环次数，-1表示无限循环
	 * @return 播放成功返回true，否则返回false
	 */
	bool AudioPlayer::playMusic(const std::string& path, int loops) {
		(void)loops;
		current_music_ = path;
		resource_manager_.playMusic(path);
		resource_manager_.setMusicGain(master_volume_ * music_volume_);
		return true;
	}

	/**
	 * @brief 停止当前播放的背景音乐。
	 */
	void AudioPlayer::stopMusic() {
		resource_manager_.stopMusic();
		current_music_.clear();
	}
}
