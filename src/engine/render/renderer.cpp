#include "renderer.h"
#include "../resource/resource_manager.h" // 确保包含完整类型声明
#include "camera.h"
#include "sprite.h"
#include <SDL3/SDL.h>
#include <stdexcept>
#include <spdlog/spdlog.h>
#include <cmath>

namespace engine::render {
    /**
     * @brief 构造一个新的 Renderer 对象。
     * 
     * @param sdl_renderer 指向 SDL 渲染上下文的指针。
     * @param resource_manager 指向资源管理器的指针，用于加载和检索纹理。
     * @throws std::runtime_error 如果任一传入指针参数为空。
     */
	Renderer::Renderer(SDL_Renderer* sdl_renderer, engine::resource::ResourceManager* resource_manager)
		: renderer_(sdl_renderer), resource_manager_(resource_manager) {
		if (!renderer_) {
			throw std::runtime_error("Renderer 初始化失败：SDL_Renderer 指针为空");
		}
		if (!resource_manager_) {
			throw std::runtime_error("Renderer 初始化失败：ResourceManager 指针为空");
		}
		spdlog::trace("Renderer 初始化成功。");
	}

    /**
     * @brief 在世界空间中绘制一个精灵。
     * 
     * 根据相机的当前位置计算屏幕位置，执行视口裁剪以优化性能，并支持缩放、旋转和水平翻转。
     * 
     * @param camera 用于坐标转换的相机。
     * @param sprite 包含纹理信息和状态的精灵对象。
     * @param position 精灵在世界空间中的左上角坐标。
     * @param scale 精灵的缩放比例。
     * @param angle 旋转角度（单位为度）。
     */
	void Renderer::drawSprite(const Camera& camera,
                              const Sprite& sprite,
                              const glm::vec2& position,
                              const glm::vec2& scale,
                              double angle) {
        auto texture = resource_manager_->getTexture(sprite.getTextureId());
        if (!texture) {
            spdlog::error("无法为 ID {} 获取纹理。", sprite.getTextureId());
            return;
        }

        auto src_rect = getSpriteSrcRect(sprite);
        if (!src_rect.has_value()) {
            spdlog::error("无法获取精灵的源矩形，ID: {}", sprite.getTextureId());
            return;
        }

        // 应用相机变换
        glm::vec2 position_screen = camera.worldToScreen(position);

        // 移除：强制取整导致的移动抖动
        /*
        #if 1
        position_screen.x = std::floor(position_screen.x);
        position_screen.y = std::floor(position_screen.y);
        #endif
        */

        // 计算目标矩形，注意 position 是精灵的左上角坐标
        float scaled_w = src_rect.value().w * scale.x;
        float scaled_h = src_rect.value().h * scale.y;
        SDL_FRect dest_rect = {
            position_screen.x,
            position_screen.y,
            scaled_w,
            scaled_h
        };

        if (!isRectInViewport(camera, dest_rect)) { // 视口裁剪：如果精灵超出视口，则不绘制
            return;
        }

        // 执行绘制(默认旋转中心为精灵的中心点)
        if (!SDL_RenderTextureRotated(renderer_, texture, &src_rect.value(), &dest_rect, angle, NULL, sprite.getIsFlipped() ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE)) {
            spdlog::error("渲染旋转纹理失败（ID: {}）：{}", sprite.getTextureId(), SDL_GetError());
        }
	}

    /**
     * @brief 绘制具有视差效果的背景精灵。
     * 
     * 用于多层背景渲染。支持在 X 和 Y 方向上自动平铺以覆盖整个视口。
     * 
     * @param camera 当前活动相机。
     * @param sprite 背景精灵。
     * @param position 基础层位置坐标。
     * @param scroll_factor 视差滚动因子 (0.0 表示跟随相机不动, 1.0 表示完全跟随世界移动)。
     * @param repeat 指定在各个轴上是否平铺填充。
     * @param scale 视觉缩放比例。
     */
	void Renderer::drawParallax(const Camera& camera,
                                const Sprite& sprite,
                                const glm::vec2& position,
                                const glm::vec2& scroll_factor,
                                const glm::bvec2& repeat,
                                const glm::vec2& scale) {
        auto texture = resource_manager_->getTexture(sprite.getTextureId());
		if (!texture) {
			spdlog::error("无法为 ID {} 获取纹理。", sprite.getTextureId());
			return;
		}

		auto src_rect = getSpriteSrcRect(sprite);
		if (!src_rect.has_value()) {
			spdlog::error("无法获取精灵的源矩形，ID: {}", sprite.getTextureId());
			return;
		}
		
		// 使用重载版本，支持独立的 XY 滚动系数
		glm::vec2 position_screen = camera.worldToScreenWithParallax(position, scroll_factor);
		float scaled_w = src_rect.value().w * scale.x;
		float scaled_h = src_rect.value().h * scale.y;
		glm::vec2 start, stop;
		glm::vec2 viewport_size = camera.getViewportSize();
        
        if (repeat.x && scaled_w > 0) {
            float phase = std::fmod(position_screen.x, scaled_w);
            if (phase > 0) phase -= scaled_w;
            start.x = phase;
            stop.x = viewport_size.x;
        } else {
            start.x = position_screen.x;
            stop.x = position_screen.x + scaled_w;
		}
        
        if (repeat.y && scaled_h > 0) {
            float phase = std::fmod(position_screen.y, scaled_h);
            if (phase > 0) phase -= scaled_h;
            start.y = phase;
            stop.y = viewport_size.y;
        } else {
            start.y = position_screen.y;
            stop.y = glm::min(position_screen.y + scaled_h, viewport_size.y);
        }

        for (float y = start.y; y < stop.y; y += scaled_h) {
            for (float x = start.x; x < stop.x; x += scaled_w) {
                // 移除：强制取整导致的移动抖动
                // 为防止白线，适当向上取整尺寸(ceil)或者保持浮点精度并确保覆盖
                // 这里恢复为浮点并在尺寸上增加微小重叠(epsilon)以消除缝隙
                SDL_FRect dest_rect = { x, y, scaled_w + 0.1f, scaled_h + 0.1f };
                
                if (!SDL_RenderTexture(renderer_, texture, nullptr, &dest_rect)) {
                    spdlog::error("渲染视差纹理失败（ID: {}）：{}", sprite.getTextureId(), SDL_GetError());
                    return;
                }
            }
        }
	}

    /**
     * @brief 在屏幕空间直接绘制 UI 精灵。
     * 
     * 此操作不考虑相机偏移，通常用于 HUD 或固定界面元素。
     * 
     * @param sprite UI 精灵。
     * @param position 屏幕上的坐标。
     * @param size 可选的目标绘制尺寸。
     */
    void Renderer::drawUISprite(const Sprite& sprite,
                                const glm::vec2& position,
                                const std::optional<glm::vec2>& size) {
		auto texture = resource_manager_->getTexture(sprite.getTextureId());
        if (!texture) {
            spdlog::error("无法为 ID {} 获取纹理。", sprite.getTextureId());
            return;
        }
		auto src_rect = getSpriteSrcRect(sprite);
		if (!src_rect.has_value()) {
			spdlog::error("无法获取精灵的源矩形，ID: {}", sprite.getTextureId());
			return;
            }
        float dest_w = size.has_value() ? size->x : src_rect.value().w;
        float dest_h = size.has_value() ? size->y : src_rect.value().h;
        SDL_FRect dest_rect = {
            position.x,
            position.y,
            dest_w,
            dest_h
        };
        if (!SDL_RenderTexture(renderer_, texture, &src_rect.value(), &dest_rect)) {
            spdlog::error("渲染 UI 纹理失败（ID: {}）：{}", sprite.getTextureId(), SDL_GetError());
        }
    }

    /**
     * @brief 更新屏幕，将当前缓冲区的内容呈现。
     */
    void Renderer::present() {
        SDL_RenderPresent(renderer_);
    }

    /**
     * @brief 清除当前渲染目标的所有内容。
     */
    void Renderer::clearScreen() {
        if (SDL_RenderClear(renderer_) == false) {
            spdlog::error("清除渲染器失败：{}", SDL_GetError());
        }
	}

    /**
     * @brief 设置渲染器的当前绘图颜色。
     * 
     * @param r 红色 (0-255)。
     * @param g 绿色 (0-255)。
     * @param b 蓝色 (0-255)。
     * @param a 透明度 (0-255)。
     */
    void Renderer::setDrawColor(Uint8 r, Uint8 g, Uint8 b, Uint8 a) {
        if (SDL_SetRenderDrawColor(renderer_, r, g, b, a) == false) {
            spdlog::error("设置渲染器绘图颜色失败：{}", SDL_GetError());
        }
	}

    /**
     * @brief 获取精灵在纹理中的源矩形。
     * 
     * 如果精灵没有自定义裁剪区域，则返回纹理的完整尺寸。
     * 
     * @param sprite 精灵对象。
     * @return std::optional<SDL_FRect> 成功返回矩形区域，失败返回 nullopt。
     */
    std::optional<SDL_FRect> Renderer::getSpriteSrcRect(const Sprite& sprite) {
        auto texture = resource_manager_->getTexture(sprite.getTextureId());
        if (!texture) {
            spdlog::error("无法为 ID {} 获取纹理。", sprite.getTextureId());
            return std::nullopt;
        }
        auto src_rect = sprite.getSourceRect();
        if (src_rect.has_value()) {
            if (src_rect.value().w <= 0 || src_rect.value().h <= 0) {
                spdlog::error("源矩形尺寸无效，ID: {}", sprite.getTextureId());
                return std::nullopt;
            }
            return src_rect;
        }
        else {
            SDL_FRect result = { 0, 0, 0, 0 };
            if (!SDL_GetTextureSize(texture, &result.w, &result.h)) {
                spdlog::error("无法获取纹理尺寸，ID: {}", sprite.getTextureId());
                return std::nullopt;
            }
            return result;
        }
    }

    /**
     * @brief 判定矩形是否与当前相机视口相交。
     * 
     * 用于剔除视口外的多余渲染绘制，提高帧率。
     * 
     * @param camera 指向视口的相机。
     * @param rect 屏幕空间的矩形区域。
     * @return true 如果矩形可见。
     * @return false 如果矩形完全在视口外。
     */
    bool Renderer::isRectInViewport(const Camera& camera, const SDL_FRect& rect) {
        glm::vec2 viewport_size = camera.getViewportSize();
        
        // 扩大裁剪边距以解决：
        // 1. 旋转精灵的包围盒比原始矩形大，可能导致边缘处被错误裁剪
        // 2. 负缩放导致的负宽高矩形在简单比较下判定错误
        // 3. 浮点误差
        constexpr float cull_margin = 128.0f;
        SDL_FRect viewport_rect = {
            -cull_margin,
            -cull_margin,
            viewport_size.x + cull_margin * 2.0f,
            viewport_size.y + cull_margin * 2.0f
        };

        // 归一化矩形（处理负宽高的情况）
        float r_x = rect.x;
        float r_y = rect.y;
        float r_w = rect.w;
        float r_h = rect.h;

        if (r_w < 0) {
            r_x += r_w;
            r_w = -r_w;
        }
        if (r_h < 0) {
            r_y += r_h;
            r_h = -r_h;
        }

        // 执行 AABB 相交检测
        bool no_overlap = r_x + r_w <= viewport_rect.x ||
                          r_x >= viewport_rect.x + viewport_rect.w ||
                          r_y + r_h <= viewport_rect.y ||
                          r_y >= viewport_rect.y + viewport_rect.h;
        return !no_overlap;
	}

    /**
     * @brief 在屏幕空间（UI 层）中绘制一个填充矩形。
     * @param rect 矩形区域。
     * @param color 填充颜色。
     */
    void Renderer::drawUIFilledRect(const engine::utils::Rect& rect, const engine::utils::FColor& color) {
        if (color.a <= 0.0f) {
            return;
        }

        // 创建SDL_FRect结构
        SDL_FRect sdl_rect = {
            rect.position.x,
            rect.position.y,
            rect.size.x,
            rect.size.y
        };

        // 设置绘制颜色
        SDL_SetRenderDrawColor(renderer_,
                               static_cast<Uint8>(color.r * 255),
                               static_cast<Uint8>(color.g * 255),
                               static_cast<Uint8>(color.b * 255),
                               static_cast<Uint8>(color.a * 255));

        // 填充矩形
        if (!SDL_RenderFillRect(renderer_, &sdl_rect)) {
            spdlog::error("渲染填充矩形失败：{}", SDL_GetError());
        }
    }

    /**
     * @brief 在屏幕空间（UI 层）中绘制一个矩形边框。
     * @param rect 矩形区域。
     * @param color 边框颜色。
     */
    void Renderer::drawUIOutlineRect(const engine::utils::Rect& rect, const engine::utils::FColor& color) {
        if (color.a <= 0.0f) {
            return;
        }

        // 创建SDL_FRect结构
        SDL_FRect sdl_rect = {
            rect.position.x,
            rect.position.y,
            rect.size.x,
            rect.size.y
        };

        // 设置绘制颜色
        SDL_SetRenderDrawColor(renderer_,
                               static_cast<Uint8>(color.r * 255),
                               static_cast<Uint8>(color.g * 255),
                               static_cast<Uint8>(color.b * 255),
                               static_cast<Uint8>(color.a * 255));

        // 绘制矩形边框
        if (!SDL_RenderRect(renderer_, &sdl_rect)) {
            spdlog::error("渲染矩形边框失败：{}", SDL_GetError());
        }
    }
}
