/**
 * @file log_audio_player.cpp
 * @brief LogAudioPlayer 类的实现，记录音频操作日志。
 */

#include "log_audio_player.h"
#include <spdlog/spdlog.h>

namespace engine::audio {

	LogAudioPlayer::LogAudioPlayer(std::unique_ptr<IAudioPlayer> wrapped_player) :
		wrapped_player_(std::move(wrapped_player)) {
	}

	void LogAudioPlayer::setMasterVolume(float volume) {
		spdlog::info("LogAudioPlayer: 设置主音量为 {}", volume);
		wrapped_player_->setMasterVolume(volume);
	}

	void LogAudioPlayer::setSoundVolume(float volume) {
		spdlog::info("LogAudioPlayer: 设置音效音量为 {}", volume);
		wrapped_player_->setSoundVolume(volume);
	}

	void LogAudioPlayer::setMusicVolume(float volume) {
		spdlog::info("LogAudioPlayer: 设置音乐音量为 {}", volume);
		wrapped_player_->setMusicVolume(volume);
	}

	float LogAudioPlayer::getMasterVolume() const {
		return wrapped_player_->getMasterVolume();
	}

	float LogAudioPlayer::getSoundVolume() const {
		return wrapped_player_->getSoundVolume();
	}

	float LogAudioPlayer::getMusicVolume() const {
		return wrapped_player_->getMusicVolume();
	}

	int LogAudioPlayer::playSound(const std::string& path) {
		spdlog::info("LogAudioPlayer: 播放音效 {}", path);
		return wrapped_player_->playSound(path);
	}

	int LogAudioPlayer::playSound(engine::resource::ResourceId id, std::string_view file_path) {
		spdlog::info("LogAudioPlayer: 播放音效 id={} path={}", id, file_path);
		return wrapped_player_->playSound(id, file_path);
	}

	int LogAudioPlayer::playSoundSpatial(const std::string& path, const glm::vec2& emitter_world_pos, const glm::vec2& listener_world_pos, float max_distance) {
		spdlog::info("LogAudioPlayer: 播放空间音效 {}, 位置 ({}, {}), 监听者 ({}, {}), 最大距离 {}",
			path, emitter_world_pos.x, emitter_world_pos.y, listener_world_pos.x, listener_world_pos.y, max_distance);
		return wrapped_player_->playSoundSpatial(path, emitter_world_pos, listener_world_pos, max_distance);
	}

	int LogAudioPlayer::playSoundSpatial(engine::resource::ResourceId id, std::string_view file_path, const glm::vec2& emitter_world_pos, const glm::vec2& listener_world_pos, float max_distance) {
		spdlog::info("LogAudioPlayer: 播放空间音效 id={} path={}, 位置 ({}, {}), 监听者 ({}, {}), 最大距离 {}",
			id, file_path, emitter_world_pos.x, emitter_world_pos.y, listener_world_pos.x, listener_world_pos.y, max_distance);
		return wrapped_player_->playSoundSpatial(id, file_path, emitter_world_pos, listener_world_pos, max_distance);
	}

	bool LogAudioPlayer::playMusic(const std::string& path, int loops) {
		spdlog::info("LogAudioPlayer: 播放音乐 {}, 循环次数 {}", path, loops);
		return wrapped_player_->playMusic(path, loops);
	}

	bool LogAudioPlayer::playMusic(engine::resource::ResourceId id, std::string_view file_path, int loops) {
		spdlog::info("LogAudioPlayer: 播放音乐 id={} path={}, 循环次数 {}", id, file_path, loops);
		return wrapped_player_->playMusic(id, file_path, loops);
	}

	void LogAudioPlayer::stopMusic() {
		spdlog::info("LogAudioPlayer: 停止音乐");
		wrapped_player_->stopMusic();
	}
}