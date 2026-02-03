#pragma once
#include <memory>
#include <string>
#include <glm/glm.hpp>

// Forward declarations of SDL and SDL-related types in global namespace
struct SDL_Renderer;
struct SDL_Texture;
struct MIX_Audio;
struct TTF_Font;

namespace engine::resource {
	class TextureManager;
	class FontManager;
	class AudioManager;

	/**
	 * @class ResourceManager
	 * @brief 集中式资源管理器，负责游戏所有资产（纹理、音频、字体）的生命周期管理。
	 *
	 * 该类通过封装具体的子管理器实现资源的统一加载、缓存和卸载，提供单一点进行资源存取。
	 */
	class ResourceManager {
	private:
		std::unique_ptr<TextureManager> texture_manager_; ///< 负责纹理加载与缓存的内部管理器
		std::unique_ptr<FontManager> font_manager_;       ///< 负责字体加载与缓存的内部管理器
		std::unique_ptr<AudioManager> audio_manager_;     ///< 负责音效和音乐加载与缓存的内部管理器
	public:

		/**
		 * @brief 构造函数，初始化各个子资源管理器。
		 * @param renderer SDL_Renderer 指针，用于创建纹理。
		 */
		explicit ResourceManager(SDL_Renderer* renderer);

		/**
		 * @brief 析构函数，确保所有持有的资源管理器被正确销毁。
		 */
		~ResourceManager();

		/**
		 * @brief 清空所有类型的资源缓存。
		 */
		void clear();

		// --- 统一资源访问接口 ---
	// -- Texture --
		/**
		 * @brief 从原始文件载入纹理资源。
		 * @param file_path 纹理文件的相对路径。
		 * @return 加载成功返回 SDL_Texture 指针，失败返回 nullptr。
		 */
		SDL_Texture* loadTexture(const std::string& file_path);

		/**
		 * @brief 尝试获取已加载纹理的指针，如果未加载则尝试加载该文件。
		 * @param file_path 纹理文件的相对路径。
		 * @return SDL_Texture 指针。
		 */
		SDL_Texture* getTexture(const std::string& file_path);

		/**
		 * @brief 卸载指定的纹理资源并从缓存中移除。
		 * @param file_path 要卸载的纹理文件路径。
		 */
		void unloadTexture(const std::string& file_path);

		/**
		 * @brief 获取指定纹理的逻辑尺寸。
		 * @param file_path 纹理文件路径。
		 * @return 包含宽度（x）和高度（y）的 glm::vec2。
		 */
		glm::vec2 getTextureSize(const std::string& file_path);

		/**
		 * @brief 清空所有已加载的纹理资源。
		 */
		void clearTextures();

		// -- Sound Effects (Chunks) --
		/**
		 * @brief 载入音效（Sound Effect）资源。
		 * @param file_path 音频文件的相对路径。
		 * @return MIX_Audio 指针。
		 */
		MIX_Audio* loadSound(const std::string& file_path);

		/**
		 * @brief 播放音效。
		 * @param file_path 音频文件相对路径。
		 */
		void playSound(const std::string& file_path);
		void stopSound();

		/**
		 * @brief 尝试获取已加载音效的指针，如果未加载则尝试从文件加载。
		 * @param file_path 音频文件的相对路径。
		 * @return MIX_Audio 指针。
		 */
		MIX_Audio* getSound(const std::string& file_path);

		/**
		 * @brief 卸载指定的音效资源。
		 * @param file_path 音频文件路径。
		 */
		void unloadSound(const std::string& file_path);

		/**
		 * @brief 清空所有已加载的音效资源。
		 */
		void clearSounds();

		// -- Music --
		/**
		 * @brief 载入音乐（Music）资源，通常用于背景音乐。
		 * @param file_path 音乐文件的相对路径。
		 * @return MIX_Audio 指针。
		 */
		MIX_Audio* loadMusic(const std::string& file_path);

		/**
		 * @brief 播放背景音乐。
		 * @param file_path 音乐文件路径。
		 */
		void playMusic(const std::string& file_path);

		/**
		 * @brief 停止背景音乐播放。
		 */
		void stopMusic();

		void setMasterGain(float gain);
		void setSoundGain(float gain);
		void setMusicGain(float gain);

		/**
		 * @brief 获取已加载音乐的指针，如果未加载则尝试加载。
		 * @param file_path 音乐文件的相对路径。
		 * @return MIX_Audio 指针。
		 */
		MIX_Audio* getMusic(const std::string& file_path);

		/**
		 * @brief 卸载指定的音乐资源。
		 * @param file_path 音乐文件路径。
		 */
		void unloadMusic(const std::string& file_path);

		/**
		 * @brief 清空所有已加载的音乐资源。
		 */
		void clearMusic();

		/**
		 * @brief 清理所有音频资源（包括音效和音乐）。
		 */
		void cleanAudio();

		// -- Fonts --
		/**
		 * @brief 载入特定点大小的字体资源。
		 * @param file_path 字体文件的相对路径。
		 * @param point_size 字体的大小（号）。
		 * @return TTF_Font 指针。
		 */
		TTF_Font* loadFont(const std::string& file_path, int point_size);

		/**
		 * @brief 尝试获取已加载的字体指针，如果未加载则尝试加载。
		 * @param file_path 字体文件的相对路径。
		 * @param point_size 字体的大小（号）。
		 * @return TTF_Font 指针。
		 */
		TTF_Font* getFont(const std::string& file_path, int point_size);

		/**
		 * @brief 卸载指定路径和大小的字体资源。
		 * @param file_path 字体文件路径。
		 * @param point_size 字体的大小。
		 */
		void unloadFont(const std::string& file_path, int point_size);

		/**
		 * @brief 清空所有已加载的字体资源。
		 */
		void clearFonts();
	};
}