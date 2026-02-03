#pragma once
#include <string>
#include <string_view>
#include <glm/vec2.hpp>

namespace engine::audio {

	/**
	 * @brief 音频播放器接口，定义了音频播放和管理的核心方法。
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
	};

} // namespace engine::audio
