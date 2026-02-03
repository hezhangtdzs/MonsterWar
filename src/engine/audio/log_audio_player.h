#pragma once
#include "iaudio_player.h"

#include <memory>
namespace engine::audio {
	/**
	 * @brief 日志音频播放器，实现了 IAudioPlayer 接口但仅记录操作日志。
	 * @details 用于调试和测试，避免实际播放音频。
	 */
	class LogAudioPlayer final : public IAudioPlayer {
		std::unique_ptr<IAudioPlayer> wrapped_player_;
	public:
		explicit LogAudioPlayer(std::unique_ptr<IAudioPlayer> wrapped_player);
		void setMasterVolume(float volume) override;
		void setSoundVolume(float volume) override;
		void setMusicVolume(float volume) override;
		float getMasterVolume() const override;
		float getSoundVolume() const override;
		float getMusicVolume() const override;
		int playSound(const std::string& path) override;
		int playSoundSpatial(const std::string& path, const glm::vec2& emitter_world_pos, const glm::vec2& listener_world_pos, float max_distance) override;
		bool playMusic(const std::string& path, int loops = -1) override;
		void stopMusic() override;
	};
} // namespace engine::audio