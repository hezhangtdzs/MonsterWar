#include "audio_manager.h"
#include <SDL3_mixer/SDL_mixer.h>
#include <stdexcept>
#include <string>
#include <algorithm> // for std::clamp
#include <spdlog/spdlog.h>

namespace engine::resource {

    /**
     * @brief 构造函数。初始化 SDL_mixer 并打开音频设备。
     * @throws std::runtime_error 如果 SDL_mixer 初始化失败或无法打开音频设备。
     */
    AudioManager::AudioManager() {
        // 1. 初始化 (SDL3 中 MIX_Init 不需要参数)
        if (!MIX_Init()) {
            throw std::runtime_error("SDL_Mixer 初始化失败: " + std::string(SDL_GetError()));
        }

        // 2. 创建混音器设备
        // 使用默认播放设备
        SDL_AudioDeviceID device_id = SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK;
        mixer_.reset(MIX_CreateMixerDevice(device_id, NULL));

        if (!mixer_) {
            throw std::runtime_error("SDL_Mixer 打开音频失败: " + std::string(SDL_GetError()));
        }

        // 3. 创建专门播放音乐的轨道
        music_track_.reset(MIX_CreateTrack(mixer_.get()));
        if (!music_track_) {
            spdlog::error("无法创建背景音乐轨道: {}", SDL_GetError());
        }

		sound_track_.reset(MIX_CreateTrack(mixer_.get()));
		if (!sound_track_) {
			spdlog::error("无法创建音效轨道: {}", SDL_GetError());
		}

		if (music_track_) {
			MIX_TagTrack(music_track_.get(), "music");
		}
		if (sound_track_) {
			MIX_TagTrack(sound_track_.get(), "sound");
		}

        // 4. 设置主音量 (SDL3 范围是 0.0 - 1.0)
		MIX_SetMasterGain(mixer_.get(), 0.25f);
        
        spdlog::trace("AudioManager 构造成功。");
    }

    /**
     * @brief 析构函数。清理所有音频资源并关闭 SDL_mixer。
     */
    AudioManager::~AudioManager() {
        if (mixer_) {
            // 停止所有主要标签的轨道
			// 注意：这需要在播放时正确设置标签 (MIX_TagTrack)
            MIX_StopTag(mixer_.get(), "music", 0);
            MIX_StopTag(mixer_.get(), "sound", 0);

            // 清空所有资源，确保 MIX_Audio 资源在 mixer 销毁前释放
            clearAudio();

            // 重要：必须在 MIX_Quit 之前手动 reset 资源
            // 否则 unique_ptr 会在 MIX_Quit 之后析构，导致非法访问
            music_track_.reset();
			sound_track_.reset();
            mixer_.reset();
        }

        // 最后关闭 SDL_mixer
        MIX_Quit();
    }

    // --- 音效管理 (Sound Effects) ---

    /**
     * @brief 从文件路径加载音效（Sound Effect）。
     * @param file_path 音效文件的路径。
     * @return MIX_Audio* 指向加载后的音效资源的指针。如果加载失败会抛出异常。
     * @throws std::runtime_error 如果加载失败。
     */
    MIX_Audio* AudioManager::loadSound(const std::string& file_path) {
        // 1. 检查缓存
        auto it = sounds_.find(file_path);
        if (it != sounds_.end()) {
            return it->second.get();
        }

        spdlog::debug("加载音效: {}", file_path);

        // 2. 加载音效
        // 音效通常较短，使用 predecode = true (预解码) 将 PCM 数据加载到内存中，
        // 以避免播放时的解码开销，保证低延迟。
        MIX_Audio* raw_audio = MIX_LoadAudio(mixer_.get(), file_path.c_str(), true);

        if (!raw_audio) {
            throw std::runtime_error("加载音效失败: " + file_path + " 错误: " + std::string(SDL_GetError()));
        }

        // 3. 存入缓存
        sounds_.emplace(file_path, std::unique_ptr<MIX_Audio, MixAudioDeleter>(raw_audio));
        spdlog::debug("成功加载并缓存音效: {}", file_path);
        return raw_audio;
    }

    /**
     * @brief 获取音效。如果该路径的音效已加载则直接返回，否则尝试加载。
     * @param file_path 音效文件的路径。
     * @return MIX_Audio* 音效资源指针。如果加载失败返回 nullptr。
     */
    MIX_Audio* AudioManager::getSound(const std::string& file_path) {
        auto it = sounds_.find(file_path);
        if (it != sounds_.end()) {
            return it->second.get();
        }
        spdlog::warn("音效未缓存，尝试直接加载: {}", file_path);
        try {
            return loadSound(file_path);
        } catch (const std::exception& e) {
            spdlog::error("加载音效失败: {}", e.what());
            return nullptr;
        }
    }

    /**
     * @brief 从缓存中卸载并释放指定的音效资源。
     * @param file_path 要卸载的音效文件路径。
     */
    void AudioManager::unloadSound(const std::string& file_path) {
        auto it = sounds_.find(file_path);
        if (it != sounds_.end()) {
            spdlog::debug("卸载音效: {}", file_path);
            sounds_.erase(it);
        } else {
            spdlog::warn("尝试卸载未加载的音效: {}", file_path);
        }
    }

    /**
     * @brief 清空音效缓存，释放所有音效资源。
     */
    void AudioManager::clearSounds() {
        if (!sounds_.empty()) {
            spdlog::debug("正在清除所有 {} 个缓存的音效。", sounds_.size());
            sounds_.clear();
        }
    }

    /**
     * @brief 播放音效（即发即弃模式）。
     * @param file_path 音效文件路径。
     */
    void AudioManager::playSound(const std::string& file_path) {
		if (!sound_track_) return;

		MIX_Audio* audio = getSound(file_path);
		if (!audio) return;

		MIX_SetTrackAudio(sound_track_.get(), audio);
		if (!MIX_PlayTrack(sound_track_.get(), 0)) {
			spdlog::error("播放音效失败: {} - {}", file_path, SDL_GetError());
		}
    }

    /**
     * @brief 停止当前播放的音效。
     */
	void AudioManager::stopSound() {
		if (sound_track_) {
			MIX_StopTrack(sound_track_.get(), 0);
		}
	}

    // --- 音乐管理 (Music) ---

    /**
     * @brief 从文件路径加载背景音乐（Music）。
     * @param file_path 音乐文件的路径。
     * @return MIX_Audio* 指向加载后的音乐资源的指针。如果加载失败会抛出异常。
     * @throws std::runtime_error 如果加载失败。
     */
    MIX_Audio* AudioManager::loadMusic(const std::string& file_path) {
        auto it = music_.find(file_path);
        if (it != music_.end()) {
            return it->second.get();
        }

        spdlog::debug("加载音乐: {}", file_path);

        // 2. 加载音乐
        // 关键优化：音乐文件通常较大（如 BGM），将 predecode 设置为 false。
        // 这样会保留源格式（如 mp3/ogg），在播放时流式解码，大幅减少内存占用。
        MIX_Audio* raw_audio = MIX_LoadAudio(mixer_.get(), file_path.c_str(), false);

        if (!raw_audio) {
            throw std::runtime_error("加载音乐失败: " + file_path + " 错误: " + std::string(SDL_GetError()));
        }

        music_.emplace(file_path, std::unique_ptr<MIX_Audio, MixAudioDeleter>(raw_audio));
        return raw_audio;
    }

    /**
     * @brief 获取音乐。如果该路径的音乐已加载则直接返回，否则尝试加载。
     * @param file_path 音乐文件的路径。
     * @return MIX_Audio* 音乐资源指针。如果加载失败返回 nullptr。
     */
    MIX_Audio* AudioManager::getMusic(const std::string& file_path) {
        auto it = music_.find(file_path);
        if (it != music_.end()) {
            return it->second.get();
        }
        spdlog::warn("音乐未缓存，尝试直接加载: {}", file_path);
        try {
            return loadMusic(file_path);
        } catch (const std::exception& e) {
            spdlog::error("加载音乐失败: {}", e.what());
            return nullptr;
        }
    }

    /**
     * @brief 从缓存中卸载并释放指定的音乐资源。
     * @param file_path 要卸载的音乐文件路径。
     */
    void AudioManager::unloadMusic(const std::string& file_path) {
        auto it = music_.find(file_path);
        if (it != music_.end()) {
            spdlog::debug("卸载音乐: {}", file_path);
            music_.erase(it);
        } else {
            spdlog::warn("尝试卸载未加载的音乐: {}", file_path);
        }
    }

    /**
     * @brief 清空音乐缓存，释放所有音乐资源。
     */
    void AudioManager::clearMusic() {
        if (!music_.empty()) {
            spdlog::debug("正在清除所有 {} 个缓存的音乐。", music_.size());
            music_.clear();
        }
    }

    /**
     * @brief 播放背景音乐（循环播放模式）。
     * @param file_path 音乐文件路径。
     */
    void AudioManager::playMusic(const std::string& file_path) {
        if (!music_track_) return;

        MIX_Audio* music = getMusic(file_path);
        if (music) {
            // 1. 设置到音乐轨道
            MIX_SetTrackAudio(music_track_.get(), music);

            // 2. 准备播放选项：循环播放 (-1 为无限循环)
            SDL_PropertiesID props = SDL_CreateProperties();
            SDL_SetNumberProperty(props, MIX_PROP_PLAY_LOOPS_NUMBER, -1);
            
            // 3. 开始播放
            if (!MIX_PlayTrack(music_track_.get(), props)) {
                spdlog::error("播放音乐失败: {} - {}", file_path, SDL_GetError());
            }

            // 4. 清理属性
            SDL_DestroyProperties(props);
            
            spdlog::debug("正在播放音乐: {}", file_path);
        }
    }

    /**
     * @brief 停止背景音乐播放。
     */
    void AudioManager::stopMusic() {
        if (music_track_) {
            MIX_StopTrack(music_track_.get(), 0);
        }
    }

    /**
     * @brief 设置音乐轨道的音量增益。
     * @param gain 增益值（0.0f-1.0f）。
     */
	void AudioManager::setMusicGain(float gain) {
		if (music_track_) {
			MIX_SetTrackGain(music_track_.get(), std::max(0.0f, gain));
		}
	}

    /**
     * @brief 设置音效轨道的音量增益。
     * @param gain 增益值（0.0f-1.0f）。
     */
	void AudioManager::setSoundGain(float gain) {
		if (sound_track_) {
			MIX_SetTrackGain(sound_track_.get(), std::max(0.0f, gain));
		}
	}

    /**
     * @brief 设置主音量增益。
     * @param gain 增益值（0.0f-1.0f）。
     */
	void AudioManager::setMasterGain(float gain) {
		if (mixer_) {
			MIX_SetMasterGain(mixer_.get(), std::max(0.0f, gain));
		}
	}

    // --- 统一管理 ---

    /**
     * @brief 清理所有音频资源（包括音效和音乐）。
     */
    void AudioManager::clearAudio() {
        clearMusic();
        clearSounds();
    }

    
} // namespace engine::resource