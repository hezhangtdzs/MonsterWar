#include "text_renderer.h"
#include "camera.h"
#include <stdexcept>
#include <string>
#include <glm/glm.hpp>
#include "../utils/math.h"

#include "../resource/resource_manager.h"
#include "../resource/font_manager.h"

namespace engine::render {

    // 静态变量，用于跟踪TTF库的初始化状态
    static bool ttf_initialized = false;
    static int text_renderer_instances = 0;

    /**
     * @brief 构造 TextRenderer 实例。
     * @param sdl_renderer SDL 渲染器指针
     * @param resource_manager 资源管理器指针
     * @throw std::runtime_error 如果初始化失败
     */
    TextRenderer::TextRenderer(SDL_Renderer* sdl_renderer, engine::resource::ResourceManager* resource_manager)
        : sdl_renderer_(sdl_renderer), resource_manager_(resource_manager) {

        ++text_renderer_instances;
        
        // 初始化 SDL3_ttf 库
        if (!ttf_initialized) {
            if (!TTF_Init()) {
                throw std::runtime_error("Failed to initialize SDL3_ttf: " + std::string(SDL_GetError()));
            }
            ttf_initialized = true;
        }
        
        // 初始化 TTF_TextEngine
        text_engine_ = TTF_CreateRendererTextEngine(sdl_renderer_);
        if (!text_engine_) {
            throw std::runtime_error("Failed to create TTF_TextEngine: " + std::string(SDL_GetError()));
        }
    }
    
    /**
     * @brief 析构函数，释放资源。
     */
    TextRenderer::~TextRenderer() {
        // 必须先销毁缓存的 TTF_Text（它们依赖 text_engine_ / SDL_ttf 全局状态）
        text_cache_.clear();

        if (text_engine_) {
            TTF_DestroyRendererTextEngine(text_engine_);
            text_engine_ = nullptr;
        }
        
        // 关闭 SDL3_ttf 库（仅当所有 TextRenderer 实例都被销毁时）
        --text_renderer_instances;
        if (ttf_initialized && text_renderer_instances <= 0) {
            TTF_Quit();
            ttf_initialized = false;
            text_renderer_instances = 0;
        }
    }
    
    /**
     * @brief 在世界空间中绘制文本（跟随相机移动）。
     * @param camera 用于计算屏幕坐标的相机
     * @param text 要绘制的文本字符串
     * @param font_path 字体文件路径
     * @param font_size 字体大小（点值）
     * @param position 文本的世界空间位置
     * @param color 文本颜色
     */
    void TextRenderer::drawText(const Camera& camera,
                               const std::string& text,
                               const std::string& font_path,
                               int font_size,
                               const glm::vec2& position,
                               const engine::utils::FColor& color) {
        // 获取字体
        TTF_Font* font = resource_manager_->getFont(font_path, font_size);
        if (!font) {
            return;
        }
        
        // 将世界坐标转换为屏幕坐标
        glm::vec2 screen_pos = camera.worldToScreen(position);
        
        // 创建文本对象
        TTF_Text* ttf_text = TTF_CreateText(text_engine_, font, text.c_str(), 0);
        if (!ttf_text) {
            return;
        }
        TTF_SetTextColorFloat(ttf_text, 0.0f, 0.0f, 0.0f, 1.0f);
        TTF_DrawRendererText(ttf_text, position.x + 2, position.y + 2);
        
        TTF_SetTextColorFloat(ttf_text, color.r, color.g, color.b, color.a);
        
        // 绘制文本
        TTF_DrawRendererText(ttf_text, screen_pos.x, screen_pos.y);
        
        // 销毁文本对象
        TTF_DestroyText(ttf_text);
    }
    
    /**
     * @brief 在屏幕空间中绘制文本（UI 层，不跟随相机）。
     * @param text 要绘制的文本字符串
     * @param font_path 字体文件路径
     * @param font_size 字体大小（点值）
     * @param position 文本的屏幕坐标位置
     * @param color 文本颜色
     */
    void TextRenderer::drawUIText(const std::string& text,
                                 const std::string& font_path,
                                 int font_size,
                                 const glm::vec2& position,
                                 const engine::utils::FColor& color,
                                 bool is_dirty) {
        TTF_Text* ttf_text = nullptr;

        if (is_dirty) {
            // 获取字体
            TTF_Font* font = resource_manager_->getFont(font_path, font_size);
            if (!font) {
                return;
            }

            // 从缓存获取 / 创建文本对象
            ttf_text = getTTFText(text);
            if (!ttf_text) {
                ttf_text = createTTFText(text, font);
                if (!ttf_text) {
                    return;
                }
            }

            // 仅在脏时同步状态（昂贵）
            TTF_SetTextFont(ttf_text, font);
            TTF_SetTextString(ttf_text, text.c_str(), 0);
        } else {
            // 干净时直接复用缓存
            ttf_text = getTTFText(text);
            if (!ttf_text) {
                // 理论上不应发生：干净意味着之前已创建并同步
                return;
            }
        }

        TTF_SetTextColorFloat(ttf_text, 0.0f, 0.0f, 0.0f, 1.0f);
        TTF_DrawRendererText(ttf_text, position.x + 2, position.y + 2);

        TTF_SetTextColorFloat(ttf_text, color.r, color.g, color.b, color.a);
        
        // 直接使用屏幕坐标绘制文本
        TTF_DrawRendererText(ttf_text, position.x, position.y);
    }

    void TextRenderer::drawUIText(std::string &&text, const std::string &font_path, int font_size, const glm::vec2 &position, const engine::utils::FColor &color)
    {
        // 获取字体
        TTF_Font* font = resource_manager_->getFont(font_path, font_size);
        if (!font) {
            return;
        }
        
        // 创建文本对象
        TTF_Text* ttf_text = TTF_CreateText(text_engine_, font, text.c_str(), 0);
        if (!ttf_text) {
            return;
        }
        
        TTF_SetTextColorFloat(ttf_text, 0.0f, 0.0f, 0.0f, 1.0f);
        TTF_DrawRendererText(ttf_text, position.x + 2, position.y + 2); 

        TTF_SetTextColorFloat(ttf_text, color.r, color.g, color.b, color.a);
        
        // 直接使用屏幕坐标绘制文本
        TTF_DrawRendererText(ttf_text, position.x, position.y);
        
        // 销毁文本对象
        TTF_DestroyText(ttf_text);
    }

    /**
     * @brief 获取文本的渲染尺寸。
     * @param text 文本内容
     * @param font_path 字体文件路径
     * @param font_size 字体大小（点值）
     * @return 文本的宽度和高度
     */
    glm::vec2 TextRenderer::getTextSize(const std::string& text, const std::string& font_path, int font_size) {
        return getTextSize(text, font_path, font_size, true);
    }

    glm::vec2 TextRenderer::getTextSize(const std::string& text, const std::string& font_path, int font_size, bool is_dirty) {
        TTF_Font* font = resource_manager_->getFont(font_path, font_size);
        if (!font) {
            return { 0.0f, 0.0f };
        }

        TTF_Text* ttf_text = nullptr;
        if (is_dirty) {
            ttf_text = getTTFText(text);
            if (!ttf_text) {
                ttf_text = createTTFText(text, font);
                if (!ttf_text) {
                    return { 0.0f, 0.0f };
                }
            }

            TTF_SetTextFont(ttf_text, font);
            TTF_SetTextString(ttf_text, text.c_str(), 0);
        } else {
            ttf_text = getTTFText(text);
            if (!ttf_text) {
                return { 0.0f, 0.0f };
            }
        }

        int w = 0, h = 0;
        TTF_GetTextSize(ttf_text, &w, &h);
        return { static_cast<float>(w), static_cast<float>(h) };
    }
    TTF_Text *TextRenderer::getTTFText(const std::string &text)
    {
        const auto cache_key = reinterpret_cast<std::uintptr_t>(&text);
        auto it = text_cache_.find(cache_key);
        if (it == text_cache_.end()) {
            return nullptr;
        }
        return it->second.get();
    }
    TTF_Text *TextRenderer::createTTFText(const std::string &text, TTF_Font *font)
    {
        const auto cache_key = reinterpret_cast<std::uintptr_t>(&text);
        auto& slot = text_cache_[cache_key];
        if (!slot) {
            slot = std::unique_ptr<TTF_Text, TTFTextDeleter>(TTF_CreateText(text_engine_, font, text.c_str(), 0));
        }
        return slot.get();
    }
}