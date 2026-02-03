#pragma once
/**
 * @file audio_player.h
 * @brief 定义 AudioPlayer 类，用于管理游戏中的音频播放。
 */

#include "iaudio_player.h"
#include <string>
#include <unordered_map>
#include <glm/vec2.hpp>

namespace engine::core { class Config; }

namespace engine::resource { class ResourceManager; }

namespace engine::audio {
	/**
	 * @class AudioPlayer
	 * @brief 音频播放器类，负责游戏中的音效和音乐播放。
	 * 
	 * 该类提供了音效播放、背景音乐播放和音量控制等功能，
	 * 支持3D空间化音效播放，根据发射器和听众位置计算音量衰减。
	 */
	class AudioPlayer final : public IAudioPlayer {
	private:
		/// 资源管理器引用，用于加载和管理音频资源
		engine::resource::ResourceManager& resource_manager_;
		engine::core::Config& config_;
		/// 主音量 (0.0f - 1.0f)
		float master_volume_{ 1.0f };
		/// 音效音量 (0.0f - 1.0f)
		float sound_volume_{ 1.0f };
		/// 音乐音量 (0.0f - 1.0f)
		float music_volume_{ 1.0f };
		/// 当前播放的音乐文件路径
		std::string current_music_;

	public:
		/**
		 * @brief 构造函数，创建一个新的音频播放器。
		 * @param resource_manager 资源管理器引用
		 */
		explicit AudioPlayer(engine::resource::ResourceManager& resource_manager, engine::core::Config& config);

		/// 禁止拷贝和移动语义
		AudioPlayer(const AudioPlayer&) = delete;
		AudioPlayer& operator=(const AudioPlayer&) = delete;
		AudioPlayer(AudioPlayer&&) = delete;
		AudioPlayer& operator=(AudioPlayer&&) = delete;

		/**
		 * @brief 设置主音量。
		 * @param volume 音量值 (0.0f - 1.0f)
		 */
		void setMasterVolume(float volume) override;

		/**
		 * @brief 设置音效音量。
		 * @param volume 音量值 (0.0f - 1.0f)
		 */
		void setSoundVolume(float volume) override;

		/**
		 * @brief 设置音乐音量。
		 * @param volume 音量值 (0.0f - 1.0f)
		 */
		void setMusicVolume(float volume) override;

		/**
		 * @brief 获取主音量。
		 * @return 当前主音量值
		 */
		float getMasterVolume() const override { return master_volume_; }

		/**
		 * @brief 获取音效音量。
		 * @return 当前音效音量值
		 */
		float getSoundVolume() const override { return sound_volume_; }

		/**
		 * @brief 获取音乐音量。
		 * @return 当前音乐音量值
		 */
		float getMusicVolume() const override { return music_volume_; }

		/**
		 * @brief 播放音效。
		 * @param path 音效文件路径
		 * @return 播放通道ID，失败返回-1
		 */
		int playSound(const std::string& path) override;

		/**
		 * @brief 播放空间化音效。
		 * @param path 音效文件路径
		 * @param emitter_world_pos 发射器世界位置
		 * @param listener_world_pos 听众世界位置
		 * @param max_distance 最大有效距离
		 * @return 播放通道ID，失败返回-1
		 */
		int playSoundSpatial(const std::string& path, const glm::vec2& emitter_world_pos, const glm::vec2& listener_world_pos, float max_distance) override;

		/**
		 * @brief 播放背景音乐。
		 * @param path 音乐文件路径
		 * @param loops 循环次数，-1表示无限循环
		 * @return 播放成功返回true，否则返回false
		 */
		bool playMusic(const std::string& path, int loops = -1) override;

		/**
		 * @brief 停止当前播放的背景音乐。
		 */
		void stopMusic() override;
	};
}
