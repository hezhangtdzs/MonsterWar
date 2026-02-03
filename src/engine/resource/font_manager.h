#pragma once
 /**
  * @file font_manager.h
  * @brief 定义 FontManager 类，用于管理 SDL3_ttf 字体的加载、缓存和生命周期。
  */

#include <memory>       // 用于 std::unique_ptr
#include <stdexcept>    // 用于 std::runtime_error
#include <string>       // 用于 std::string
#include <unordered_map> // 用于 std::unordered_map
#include <utility>      // 用于 std::pair
#include <functional>   // 用于 std::hash

#include <SDL3_ttf/SDL_ttf.h> // SDL_ttf 主头文件

namespace engine::resource {
	/**
	 * @class FontManager
	 * @brief 字体资源管理器，负责 SDL3_ttf 的初始化与字体资源的集中管理。
	 * 
	 * 该类通过 RAII 机制确保 SDL3_ttf 正确初始化与退出，并利用内部缓存避免重复加载相同路径和大小的字体。
	 */
	class FontManager final {
	private:
		/**
		 * @struct FontKey
		 * @brief 字体缓存映射的唯一键，结合了文件路径和字体磅值。
		 */
		struct FontKey {
			std::string file_path; ///< 字体文件的相对或绝对磁盘路径
			int point_size;        ///< 字体大小（点/磅值）

			/**
			 * @brief 比较两个 FontKey 是否相等。
			 * @param other 另一个 FontKey 实例。
			 * @return true 如果路径和大小均相等，否则返回 false。
			 */
			bool operator==(const FontKey& other) const {
				return file_path == other.file_path && point_size == other.point_size;
			}
		};

		/**
		 * @struct FontKeyHasher
		 * @brief FontKey 的哈希函数对象，用于 std::unordered_map。
		 */
		struct FontKeyHasher {
			/**
			 * @brief 为 FontKey 生成哈希值。
			 * @param key 需要哈希的 FontKey。
			 * @return 生成的 size_t 类型哈希值。
			 */
			std::size_t operator()(const FontKey& key) const {
				return std::hash<std::string>()(key.file_path) ^ std::hash<int>()(key.point_size);
			}
		};

		/**
		 * @struct SDLFontDeleter
		 * @brief 自定义删除器，用于在 std::unique_ptr 销毁时安全调用 TTF_CloseFont。
		 */
		struct SDLFontDeleter {
			/**
			 * @brief 释放指向 TTF_Font 的内存并关闭 SDL 字体资源。
			 * @param font 指向要关闭的字体的指针。
			 */
			void operator()(TTF_Font* font) const {
				if (font) {
					TTF_CloseFont(font);
				}
			}
		};

		/**
		 * @brief 已加载字体的内部映射表，使用 std::unique_ptr 管理 TTF_Font 的生命周期。
		 */
		std::unordered_map<FontKey, std::unique_ptr<TTF_Font, SDLFontDeleter>, FontKeyHasher> fonts_;

	public:

		/**
		 * @brief 构造函数。初始化 SDL_ttf。
		 * @attention 如果 TTF_Init() 失败，将抛出异常。
		 * @throws std::runtime_error 如果 SDL_ttf 初始化失败。
		 */
		FontManager();

		/**
		 * @brief 析构函数。清空所有字体缓存并关闭 SDL_ttf。
		 */
		~FontManager();

		// 禁用拷贝与移动构造及赋值运算，防止管理器实例被意外复制
		FontManager(const FontManager&) = delete;
		FontManager& operator=(const FontManager&) = delete;
		FontManager(FontManager&&) = delete;
		FontManager& operator=(FontManager&&) = delete;

		// 友元类声明，允许 ResourceManager 访问私有方法
		friend class ResourceManager;

private:
		/**
		 * @brief 从文件路径加载指定点大小的字体。
		 * @param file_path 字体文件的完整路径。
		 * @param point_size 字体的大小（磅值）。
		 * @return 返回指向 TTF_Font 的裸指针。
		 * @throws std::runtime_error 如果 TTF_OpenFont 失败。
		 */
		TTF_Font* loadFont(const std::string& file_path, int point_size);

		/**
		 * @brief 尝试获取已加载字体的指针，如果未加载则执行加载逻辑。
		 * @param file_path 字体文件的路径。
		 * @param point_size 字体的大小。
		 * @return 返回对应的 TTF_Font 指针。
		 */
		TTF_Font* getFont(const std::string& file_path, int point_size);

		/**
		 * @brief 卸载特定字体并从缓存中移除。
		 * @param file_path 字体文件的路径。
		 * @param point_size 字体的大小。
		 */
		void unloadFont(const std::string& file_path, int point_size);

		/**
		 * @brief 清空所有缓存的字体并释放相关资源。
		 */
		void clearFonts();
	
	};
}