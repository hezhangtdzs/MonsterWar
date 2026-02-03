#pragma once
#include <memory>       // 用于 std::unique_ptr
#include <stdexcept>    // 用于 std::runtime_error
#include <string>       // 用于 std::string
#include <unordered_map> // 用于 std::unordered_map
#include <SDL3_mixer/SDL_mixer.h>

/**
 * @namespace engine::resource
 * @brief 引擎资源管理命名空间，包含音频、纹理、字体等资源的管理类。
 */
namespace engine::resource {

	/**
	 * @class AudioManager
	 * @brief 音频管理器类，负责音频资源（音乐和音效）的加载、缓存及生命周期管理。
	 * 
	 * 该类封装了 SDL_mixer 的功能，并使用 std::unique_ptr 确保在对象销毁时正确释放音频设备和资源。
	 * 采用懒加载策略（Load-on-demand）并在内部通过哈希表缓存已加载的资源。
	 */
	class AudioManager final {
	public:
		/**
		 * @struct MixerDeleter
		 * @brief 用于 std::unique_ptr 的自定义删除器，负责销毁 MIX_Mixer 设备。
		 */
		struct MixerDeleter {
			void operator()(MIX_Mixer* m) const {
				if (m) MIX_DestroyMixer(m);
			}
		};

		/**
		 * @struct MixAudioDeleter
		 * @brief 用于 std::unique_ptr 的自定义删除器，负责释放 MIX_Audio 资源（音效或音乐）。
		 */
		struct MixAudioDeleter {
			void operator()(MIX_Audio* c) const {
				if (c) MIX_DestroyAudio(c);
			}
		};

		/**
		 * @struct TrackDeleter
		 * @brief 用于 std::unique_ptr 的自定义删除器，负责销毁 MIX_Track。
		 */
		struct TrackDeleter {
			void operator()(MIX_Track* t) const {
				if (t) MIX_DestroyTrack(t);
			}
		};

	private:
		std::unique_ptr<MIX_Mixer, MixerDeleter> mixer_; ///< SDL_mixer 混音器设备指针
		std::unique_ptr<MIX_Track, TrackDeleter> music_track_; ///< 专门播放 BGM 的轨道
		std::unique_ptr<MIX_Track, TrackDeleter> sound_track_; ///< 专门播放 SFX 的轨道
		std::unordered_map<std::string, std::unique_ptr<MIX_Audio, MixAudioDeleter>> music_; ///< 音乐资源缓存映射表 (文件路径 -> 资源指针)
		std::unordered_map<std::string, std::unique_ptr<MIX_Audio, MixAudioDeleter>> sounds_; ///< 音效资源缓存映射表 (文件路径 -> 资源指针)


	public:
		/**
		 * @brief 构造函数。初始化 SDL_mixer 并打开音频设备。
		 * @throws std::runtime_error 如果 SDL_mixer 初始化失败或无法打开音频设备。
		 */
		AudioManager();

		/**
		 * @brief 析构函数。清理所有缓存的音频资源并关闭 SDL_mixer。
		 */
		~AudioManager();

		// 禁用拷贝和移动，确保音频管理器的单一份额和所有权安全
		AudioManager(const AudioManager&) = delete;
		AudioManager& operator=(const AudioManager&) = delete;
		AudioManager(AudioManager&&) = delete;
		AudioManager& operator=(AudioManager&&) = delete;

		// 友元类声明，允许 ResourceManager 访问私有方法
		friend class ResourceManager;

private:
		/**
		 * @brief 从文件路径加载音效（Sound Effect）。
		 * @param file_path 音效文件的路径。
		 * @return MIX_Audio* 指向加载后的音效资源的指针。如果加载失败可能返回 nullptr 或抛出异常。
		 */
		MIX_Audio* loadSound(const std::string& file_path);

		/**
		 * @brief 获取音效。如果该路径的音效已加载则直接返回，否则尝试加载。
		 * @param file_path 音效文件的路径。
		 * @return MIX_Audio* 音效资源指针。
		 */
		MIX_Audio* getSound(const std::string& file_path);

		/**
		 * @brief 从缓存中卸载并释放指定的音效资源。
		 * @param file_path 要卸载的音效文件路径。
		 */
		void unloadSound(const std::string& file_path);

		/**
		 * @brief 清空音效缓存，释放所有音效资源。
		 */
		void clearSounds();

		/**
		 * @brief 播放音效（即发即弃模式）。
		 * @param file_path 音效文件路径。
		 */
		void playSound(const std::string& file_path);

		void stopSound();

		/**
		 * @brief 从文件路径加载背景音乐（Music）。
		 * @param file_path 音乐文件的路径。
		 * @return MIX_Audio* 指向加载后的音乐资源的指针。
		 */
		MIX_Audio* loadMusic(const std::string& file_path);

		/**
		 * @brief 获取音乐。如果该路径的音乐已加载则直接返回，否则尝试加载。
		 * @param file_path 音乐文件的路径。
		 * @return MIX_Audio* 音乐资源指针。
		 */
		MIX_Audio* getMusic(const std::string& file_path);

		/**
		 * @brief 从缓存中卸载并释放指定的音乐资源。
		 * @param file_path 要卸载的音乐文件路径。
		 */
		void unloadMusic(const std::string& file_path);

		/**
		 * @brief 清空音乐缓存，释放所有音乐资源。
		 */
		void clearMusic();

		/**
		 * @brief 播放背景音乐（循环播放模式）。
		 * @param file_path 音乐文件路径。
		 */
		void playMusic(const std::string& file_path);

		/**
		 * @brief 停止背景音乐播放。
		 */
		void stopMusic();

		void setMusicGain(float gain);
		void setSoundGain(float gain);
		void setMasterGain(float gain);

		/**
		 * @brief 清理所有音频资源（包括音效和音乐）。
		 */
		void clearAudio();
	};

} // namespace engine::resource