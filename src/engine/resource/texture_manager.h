#pragma once
#include <memory>       // 用于 std::unique_ptr
#include <stdexcept>    // 用于 std::runtime_error
#include <string>       // 用于 std::string
#include <unordered_map> // 用于 std::unordered_map
#include <SDL3/SDL_render.h> // 用于 SDL_Texture 和 SDL_Renderer
#include <glm/glm.hpp>

namespace engine::resource {

	/**
	 * @class TextureManager
	 * @brief 负责 SDL 纹理资源的集中管理、加载与缓存。
	 * 
	 * 该类通过 std::unordered_map 提供纹理缓存功能，避免同一资源的重复加载，
	 * 并利用 std::unique_ptr 确保在对象销毁或资源卸载时自动调用 SDL_DestroyTexture。
	 */
	class TextureManager final {
	private:
		/**
		 * @struct SDLTextureDeleter
		 * @brief 用于 std::unique_ptr 的自定义删除器，专门处理 SDL_Texture 的销毁。
		 */
		struct SDLTextureDeleter {
			/**
			 * @brief 仿函数实现，销毁传入的 SDL 纹理。
			 * @param texture 指向需要销毁的 SDL_Texture 的指针。
			 */
			void operator()(SDL_Texture* texture) const {
				if (texture) {
					SDL_DestroyTexture(texture);
				}
			}
		};

		SDL_Renderer* renderer_; ///< 指向 SDL 渲染上下文的指针，用于生成纹理。
		std::unordered_map<std::string, std::unique_ptr<SDL_Texture, SDLTextureDeleter>> textures_; ///< 存储已加载纹理的映射表，键为文件路径。

	public:
		/**
		 * @brief 构造函数，初始化纹理管理器。
		 * @param renderer 指向有效的 SDL_Renderer 上下文的指针。不能为空。
		 * @throws std::runtime_error 如果 renderer 为 nullptr。
		 */
		explicit TextureManager(SDL_Renderer* renderer);

		// 禁止拷贝和移动语义以确保资源所有权的唯一性
		TextureManager(const TextureManager&) = delete;
		TextureManager& operator=(const TextureManager&) = delete;
		TextureManager(TextureManager&&) = delete;
		TextureManager& operator=(TextureManager&&) = delete;

		// 友元类声明，允许 ResourceManager 访问私有方法
		friend class ResourceManager;

private:
		/**
		 * @brief 从指定文件路径载入纹理资源并存入缓存。
		 * @param file_path 磁盘上纹理文件的完整或相对路径。
		 * @return SDL_Texture* 指向新加载纹理的原始指针。如果加载失败，返回 nullptr。
		 */
		SDL_Texture* loadTexture(const std::string& file_path);

		/**
		 * @brief 尝试获取已加载纹理的指针。
		 * @param file_path 纹理文件的路径。
		 * @return SDL_Texture* 如果命中缓存则直接返回，否则尝试实时从磁盘加载。
		 */
		SDL_Texture* getTexture(const std::string& file_path);

		/**
		 * @brief 从缓存中卸载指定的纹理资源并释放内存。
		 * @param file_path 要卸载的纹理文件的路径。
		 */
		void unloadTexture(const std::string& file_path);

		/**
		 * @brief 获取指定纹理的逻辑尺寸。
		 * @param file_path 纹理文件的路径。
		 * @return glm::vec2 包含该纹理宽度 (x) 和高度 (y) 的向量。若纹理无效则返回 {0, 0}。
		 */
		glm::vec2 getTextureSize(const std::string& file_path);

		/**
		 * @brief 清空当前所有的纹理缓存，释放所有占用的 SDL 纹理资源。
		 */
		void clearTextures();
	};
}