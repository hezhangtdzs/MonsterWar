/**
 * @file log_audio_player.h
 * @brief 定义 LogAudioPlayer 类，用于记录音频操作日志。
 */

#pragma once
#include "iaudio_player.h"

#include <memory>

namespace engine::audio {
	/**
	 * @class LogAudioPlayer
	 * @brief 日志音频播放器，实现了 IAudioPlayer 接口但仅记录操作日志。
	 * @details 用于调试和测试，包装另一个音频播放器并记录所有操作到日志系统。
	 * 在调试模式下使用，可以避免实际播放音频同时跟踪音频调用。
	 */
	class LogAudioPlayer final : public IAudioPlayer {
		std::unique_ptr<IAudioPlayer> wrapped_player_; ///< 被包装的实际音频播放器
	public:
		/**
		 * @brief 构造函数，包装另一个音频播放器。
		 * @param wrapped_player 要包装的实际音频播放器
		 */
		explicit LogAudioPlayer(std::unique_ptr<IAudioPlayer> wrapped_player);

		/**
		 * @brief 设置主音量，记录日志并转发给被包装对象。
		 * @param volume 音量值 (0.0f - 1.0f)
		 */
		void setMasterVolume(float volume) override;

		/**
		 * @brief 设置音效音量，记录日志并转发给被包装对象。
		 * @param volume 音量值 (0.0f - 1.0f)
		 */
		void setSoundVolume(float volume) override;

		/**
		 * @brief 设置音乐音量，记录日志并转发给被包装对象。
		 * @param volume 音量值 (0.0f - 1.0f)
		 */
		void setMusicVolume(float volume) override;

		/** @brief 获取主音量，转发给被包装对象。 */
		float getMasterVolume() const override;

		/** @brief 获取音效音量，转发给被包装对象。 */
		float getSoundVolume() const override;

		/** @brief 获取音乐音量，转发给被包装对象。 */
		float getMusicVolume() const override;

		/**
		 * @brief 播放音效，记录日志并转发给被包装对象。
		 * @param path 音效文件路径
		 * @return 播放通道ID
		 */
		int playSound(const std::string& path) override;

		/**
		 * @brief 播放空间化音效，记录日志并转发给被包装对象。
		 * @param path 音效文件路径
		 * @param emitter_world_pos 发射器世界位置
		 * @param listener_world_pos 听众世界位置
		 * @param max_distance 最大有效距离
		 * @return 播放通道ID
		 */
		int playSoundSpatial(const std::string& path, const glm::vec2& emitter_world_pos, const glm::vec2& listener_world_pos, float max_distance) override;

		/**
		 * @brief 播放背景音乐，记录日志并转发给被包装对象。
		 * @param path 音乐文件路径
		 * @param loops 循环次数，-1表示无限循环
		 * @return 播放成功返回true
		 */
		bool playMusic(const std::string& path, int loops = -1) override;

		/** @brief 停止背景音乐，记录日志并转发给被包装对象。 */
		void stopMusic() override;

		/**
		 * @brief 使用资源ID播放音效，记录日志并转发给被包装对象。
		 * @param id 音效资源ID
		 * @param file_path 可选的文件路径
		 * @return 播放通道ID
		 */
		int playSound(engine::resource::ResourceId id, std::string_view file_path = {}) override;

		/**
		 * @brief 使用资源ID播放空间化音效，记录日志并转发给被包装对象。
		 * @param id 音效资源ID
		 * @param file_path 可选的文件路径
		 * @param emitter_world_pos 发射器世界位置
		 * @param listener_world_pos 听众世界位置
		 * @param max_distance 最大有效距离
		 * @return 播放通道ID
		 */
		int playSoundSpatial(engine::resource::ResourceId id, std::string_view file_path, const glm::vec2& emitter_world_pos, const glm::vec2& listener_world_pos, float max_distance) override;

		/**
		 * @brief 使用资源ID播放背景音乐，记录日志并转发给被包装对象。
		 * @param id 音乐资源ID
		 * @param file_path 可选的文件路径
		 * @param loops 循环次数，-1表示无限循环
		 * @return 播放成功返回true
		 */
		bool playMusic(engine::resource::ResourceId id, std::string_view file_path = {}, int loops = -1) override;
	};
} // namespace engine::audio