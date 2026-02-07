/**
 * @file iaudio_player.h
 * @brief 定义音频播放器接口 IAudioPlayer 和空实现 NullAudioPlayer。
 *
 * @details
 * 该文件定义了音频系统的核心接口，采用策略模式允许不同的音频后端实现。
 * IAudioPlayer 接口提供了音量控制、音效播放、音乐播放等功能。
 * NullAudioPlayer 是一个空对象模式实现，用于避免空指针检查。
 *
 * @see engine::audio::AudioPlayer 具体实现类
 * @see engine::audio::AudioLocator 服务定位器
 */

#pragma once
#include <string>
#include <string_view>
#include <glm/vec2.hpp>
#include "../resource/resource_id.h"

namespace engine::audio {

	/**
	 * @class IAudioPlayer
	 * @brief 音频播放器接口，定义了音频播放和管理的核心方法。
	 *
	 * @details
	 * 该接口定义了所有音频播放器必须实现的功能，包括：
	 * - 音量控制（主音量、音效音量、音乐音量）
	 * - 音效播放（普通和空间化）
	 * - 背景音乐播放
	 *
	 * 使用资源ID的版本支持资源管理系统的集成。
	 */
	class IAudioPlayer {
	public:
		virtual ~IAudioPlayer() = default;

		virtual void setMasterVolume(float volume) = 0;
		virtual void setSoundVolume(float volume) = 0;
		virtual void setMusicVolume(float volume) = 0;

		virtual float getMasterVolume() const = 0;
		virtual float getSoundVolume() const = 0;
		virtual float getMusicVolume() const = 0;

		virtual int playSound(const std::string& path) = 0;
		virtual int playSoundSpatial(const std::string& path, const glm::vec2& emitter_world_pos, const glm::vec2& listener_world_pos, float max_distance) = 0;
		virtual bool playMusic(const std::string& path, int loops = -1) = 0;
		virtual void stopMusic() = 0;

		virtual int playSound(engine::resource::ResourceId id, std::string_view file_path = {}) = 0;
		virtual int playSoundSpatial(engine::resource::ResourceId id, std::string_view file_path, const glm::vec2& emitter_world_pos, const glm::vec2& listener_world_pos, float max_distance) = 0;
		virtual bool playMusic(engine::resource::ResourceId id, std::string_view file_path = {}, int loops = -1) = 0;
	};

	/**
	 * @brief 空音频播放器，实现了 IAudioPlayer 接口但没有任何操作。
	 * @details 用于服务定位器模式的默认实现，避免空指针检查。
	 */
	class NullAudioPlayer final : public IAudioPlayer {
	public:
		void setMasterVolume(float /*volume*/) override {}
		void setSoundVolume(float /*volume*/) override {}
		void setMusicVolume(float /*volume*/) override {}

		float getMasterVolume() const override { return 0.0f; }
		float getSoundVolume() const override { return 0.0f; }
		float getMusicVolume() const override { return 0.0f; }

		int playSound(const std::string& /*path*/) override { return -1; }
		int playSoundSpatial(const std::string& /*path*/, const glm::vec2& /*emitter_world_pos*/, const glm::vec2& /*listener_world_pos*/, float /*max_distance*/) override { return -1; }
		bool playMusic(const std::string& /*path*/, int /*loops*/ = -1) override { return false; }
		void stopMusic() override {}

		int playSound(engine::resource::ResourceId /*id*/, std::string_view /*file_path*/ = {}) override { return -1; }
		int playSoundSpatial(engine::resource::ResourceId /*id*/, std::string_view /*file_path*/, const glm::vec2& /*emitter_world_pos*/, const glm::vec2& /*listener_world_pos*/, float /*max_distance*/) override { return -1; }
		bool playMusic(engine::resource::ResourceId /*id*/, std::string_view /*file_path*/ = {}, int /*loops*/ = -1) override { return false; }
	};

} // namespace engine::audio
