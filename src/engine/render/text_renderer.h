#pragma once
/**
 * @file text_renderer.h
 * @brief 定义 TextRenderer 类，用于处理 SDL3_ttf 文本渲染。
 */

#include <string>
#include <glm/vec2.hpp>
#include <SDL3_ttf/SDL_ttf.h>
#include <unordered_map>
#include <memory>
#include <cstddef>
#include <cstdint>

struct SDL_Renderer;

#include "../utils/math.h"
namespace engine::utils {
    struct FColor;
}
namespace engine::resource {
    class ResourceManager;
}

namespace engine::render {
    class Camera;

    /**
     * @class TextRenderer
     * @brief 文本渲染类，负责使用 SDL3_ttf 渲染文本到屏幕。
     * 
     * 支持两种文本渲染模式：
     * 1. UI 文本：直接使用屏幕坐标，不跟随相机移动
     * 2. 世界文本：使用世界坐标，跟随相机移动
     */
    class TextRenderer final {
    private:
        /// SDL 渲染上下文指针
        SDL_Renderer* sdl_renderer_ = nullptr;
        /// 资源管理器指针，用于获取字体
        engine::resource::ResourceManager* resource_manager_ = nullptr;
        /// SDL3_ttf 文本引擎，用于高效渲染文本
        TTF_TextEngine* text_engine_ = nullptr;
        struct TTFTextDeleter {
            void operator()(TTF_Text* text) const { if (text) TTF_DestroyText(text); }
        };
        std::unordered_map<std::uintptr_t, std::unique_ptr<TTF_Text, TTFTextDeleter>> text_cache_;
    public:
        /**
         * @brief 构造 TextRenderer 实例。
         * @param sdl_renderer SDL 渲染器指针。
         * @param resource_manager 资源管理器指针。
         */
        TextRenderer(SDL_Renderer* sdl_renderer, engine::resource::ResourceManager* resource_manager);

        /**
         * @brief 析构函数，释放资源。
         */
        ~TextRenderer();

        /**
         * @brief 在世界空间中绘制文本（跟随相机移动）。
         * @param camera 用于计算屏幕坐标的相机。
         * @param text 要绘制的文本字符串。
         * @param font_path 字体文件路径。
         * @param font_size 字体大小（点值）。
         * @param position 文本的世界空间位置。
         * @param color 文本颜色。
         */
        void drawText(const Camera& camera,
                     const std::string& text,
                     const std::string& font_path,
                     int font_size,
                     const glm::vec2& position,
                     const engine::utils::FColor& color);

        /**
         * @brief 在屏幕空间中绘制文本（UI 层，不跟随相机）。
         * @param text 要绘制的文本字符串。
         * @param font_path 字体文件路径。
         * @param font_size 字体大小（点值）。
         * @param position 文本的屏幕坐标位置。
         * @param color 文本颜色。
         */
        void drawUIText(const std::string& text,
                       const std::string& font_path,
                       int font_size,
                       const glm::vec2& position,
                        const engine::utils::FColor& color,
                        bool is_dirty = true);
        /**
         * @brief 在屏幕空间中绘制文本（UI 层，不跟随相机）。
         * @param text 要绘制的文本字符串。
         * @param font_path 字体文件路径。
         * @param font_size 字体大小（点值）。
         * @param position 文本的屏幕坐标位置。
         * @param color 文本颜色。
         */
        void drawUIText(std::string&& text,
                       const std::string& font_path,
                       int font_size,
                       const glm::vec2& position,
                       const engine::utils::FColor& color);
        /**
         * @brief 获取文本的渲染尺寸。
         * @param text 文本内容。
         * @param font_path 字体路径。
         * @param font_size 字体大小。
         * @return 文本的宽度和高度。
         */
        glm::vec2 getTextSize(const std::string& text, const std::string& font_path, int font_size);
         glm::vec2 getTextSize(const std::string& text, const std::string& font_path, int font_size, bool is_dirty);

        // 禁用拷贝和移动语义
        TextRenderer(const TextRenderer&) = delete;
        TextRenderer& operator=(const TextRenderer&) = delete;
        TextRenderer(TextRenderer&&) = delete;
        TextRenderer& operator=(TextRenderer&&) = delete;
    private:
        TTF_Text* getTTFText(const std::string& text);
        TTF_Text* createTTFText(const std::string& text, TTF_Font* font);
    };
}