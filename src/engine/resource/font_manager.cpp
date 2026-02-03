#include "font_manager.h"
#include <spdlog/spdlog.h>

namespace engine::resource {

/**
 * @brief 构造函数。初始化 SDL_ttf。
 * @attention 如果 TTF_Init() 失败，将抛出异常。
 * @throws std::runtime_error 如果 SDL_ttf 初始化失败。
 */
FontManager::FontManager()
{
	if (!TTF_WasInit() && !TTF_Init()) {
		throw std::runtime_error("FontManager 错误: TTF_Init 失败：" + std::string(SDL_GetError()));
	}
	spdlog::trace("FontManager 构造成功，SDL_ttf 初始化完成。");
}

/**
 * @brief 析构函数。清空所有字体缓存并关闭 SDL_ttf。
 */
FontManager::~FontManager()
{
	if (!fonts_.empty()) {
		spdlog::debug("FontManager 不为空，调用 clearFonts 处理清理逻辑。");
		clearFonts();       // 调用 clearFonts 处理清理逻辑
	}
	TTF_Quit();
	spdlog::trace("FontManager 析构成功。");
}

/**
 * @brief 从文件路径加载指定点大小的字体。
 * @param file_path 字体文件的完整路径。
 * @param point_size 字体的大小（磅值）。
 * @return 返回指向 TTF_Font 的裸指针。如果加载失败返回 nullptr。
 */
TTF_Font* FontManager::loadFont(const std::string& file_path, int point_size)
{
    // 检查点大小是否有效
    if (point_size <= 0) {
        spdlog::error("无法加载字体 '{}'：无效的点大小 {}。", file_path, point_size);
        return nullptr;
    }

    // 创建映射表的键
    FontKey key = { file_path, point_size };

    // 首先检查缓存
    auto it = fonts_.find(key);
    if (it != fonts_.end()) {
        return it->second.get();
    }

    // 缓存中不存在，则加载字体
    spdlog::debug("正在加载字体：{} ({}pt)", file_path, point_size);
    TTF_Font* raw_font = TTF_OpenFont(file_path.c_str(), static_cast<float>(point_size));
    if (!raw_font) {
        spdlog::error("加载字体 '{}' ({}pt) 失败：{}", file_path, point_size, SDL_GetError());
        return nullptr;
    }

    // 使用 unique_ptr 存储到缓存中
    fonts_.emplace(key, std::unique_ptr<TTF_Font, SDLFontDeleter>(raw_font));
    spdlog::debug("成功加载并缓存字体：{} ({}pt)", file_path, point_size);
    return raw_font;
}

/**
 * @brief 尝试获取已加载字体的指针，如果未加载则执行加载逻辑。
 * @param file_path 字体文件的路径。
 * @param point_size 字体的大小。
 * @return 返回对应的 TTF_Font 指针。如果加载失败返回 nullptr。
 */
TTF_Font* FontManager::getFont(const std::string& file_path, int point_size)
{
    FontKey key = { file_path, point_size };
    auto it = fonts_.find(key);
    if (it != fonts_.end()) {
        return it->second.get();
    }

    spdlog::warn("字体 '{}' ({}pt) 不在缓存中，尝试加载。", file_path, point_size);
    return loadFont(file_path, point_size);
}

/**
 * @brief 卸载特定字体并从缓存中移除。
 * @param file_path 字体文件的路径。
 * @param point_size 字体的大小。
 */
void FontManager::unloadFont(const std::string& file_path, int point_size)
{
    FontKey key = { file_path, point_size };
    auto it = fonts_.find(key);
    if (it != fonts_.end()) {
        spdlog::debug("卸载字体：{} ({}pt)", file_path, point_size);
        fonts_.erase(it);       // unique_ptr 会处理 TTF_CloseFont
    }
    else {
        spdlog::warn("尝试卸载不存在的字体：{} ({}pt)", file_path, point_size);
    }
}

/**
 * @brief 清空所有缓存的字体并释放相关资源。
 */
void FontManager::clearFonts()
{
    if (!fonts_.empty()) {
        spdlog::debug("正在清理所有 {} 个缓存的字体。", fonts_.size());
        fonts_.clear();         // unique_ptr 会处理删除
    }
}

} // namespace engine::resource
