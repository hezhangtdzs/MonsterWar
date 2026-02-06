#pragma once
#include "image.h"
#include <glm/glm.hpp>
#include "../utils/math.h"

struct SDL_Renderer;
namespace engine::resource {
	class ResourceManager;
}
namespace engine::component {
	struct Sprite;
}

namespace engine::render {
	class Camera;

	/**
	 * @class Renderer
	 * @brief 核心渲染类，负责封装 SDL 渲染操作和处理场景绘制逻辑。
	 */
	class Renderer final {
	private:
		/// SDL 渲染上下文指针
		SDL_Renderer* renderer_ = nullptr;
		/// 用于获取纹理和资源的管理类指针
		engine::resource::ResourceManager* resource_manager_ = nullptr;

		engine::utils::FColor back_ground_{0.0f, 0.0f, 0.0f, 0.0f}; ///< @brief 背景颜色，用于清屏或绘制背景色

	public:
		/**
		 * @brief 构造 Renderer 实例。
		 * @param sdl_renderer SDL 渲染器指针。
		 * @param resource_manager 资源管理器指针。
		 */
		Renderer(SDL_Renderer* sdl_renderer, engine::resource::ResourceManager* resource_manager);

		~Renderer() = default;

		/**
		 * @brief 在世界空间中绘制一个精灵。
		 * @param camera 用于计算屏幕坐标的相机。
		 * @param sprite 要绘制的精灵。
		 * @param position 精灵的世界空间位置。
		 * @param size 精灵的绘制大小。
		 * @param angle 旋转角度（度），默认为 0.0f。
		 */
		void drawSprite(const Camera& camera, const engine::component::Sprite& sprite, const glm::vec2& position, const glm::vec2& size, float angle = 0.0f);


		/**
		 * @brief 在屏幕空间（UI 层）中绘制一个图片。
		 * @param image 要绘制的图片。
		 * @param position 屏幕坐标位置。
		 * @param size 可选的目标绘制大小，如果不提供则使用图片原始大小。
		 */
		void drawUIImage(const Image& image, 
					  const glm::vec2& position, 
					  const std::optional<glm::vec2>& size = std::nullopt);

		/**
		 * @brief 在屏幕空间（UI 层）中绘制一个填充矩形。
		 * @param rect 矩形区域。
		 * @param color 填充颜色。
		 */
		void drawUIFilledRect(const engine::utils::Rect& rect, const engine::utils::FColor& color);

		/**
		 * @brief 在屏幕空间（UI 层）中绘制一个矩形边框。
		 * @param rect 矩形区域。
		 * @param color 边框颜色。
		 */
		void drawUIOutlineRect(const engine::utils::Rect& rect, const engine::utils::FColor& color);

		/**
		 * @brief 将当前的后备缓冲区呈现到屏幕。
		 */
		void present();

		/**
		 * @brief 清除当前屏幕缓冲区。
		 */
		void clearScreen();
		/**
		 * @brief 设置背景颜色，用于清屏或绘制背景。
		 * @param color 背景颜色（RGBA）。
		 */
		void setBackgroundColor(const engine::utils::FColor& color) { 
			back_ground_ = color;	
		}

		/**
		 * @brief 设置后续绘图操作的基础颜色（r, g, b, a）。
		 */
		void setDrawColor(Uint8 r, Uint8 g, Uint8 b, Uint8 a = 255);

		/**
		 * @brief 获取原始 SDL 渲染器指针。
		 * @return SDL_Renderer* 内部渲染器指针。
		 */
		SDL_Renderer* getSDLRenderer() const { return renderer_; }

		// 禁用拷贝和移动语义
		Renderer(const Renderer&) = delete;
		Renderer& operator=(const Renderer&) = delete;
		Renderer(Renderer&&) = delete;
		Renderer& operator=(Renderer&&) = delete;

	private:
		/**
		 * @brief 根据精灵状态（如动画帧）计算纹理的源矩形区域。
		 * @param sprite 精灵对象。
		 * @return std::optional<SDL_FRect> 源矩形，若无有效纹理则返回 nullopt。
		 */
		std::optional<SDL_FRect> getImageSrcRect(const Image& image);

		/**
		 * @brief 检查一个矩形是否在相机的可见视口内。
		 * @param camera 相机对象。
		 * @param rect 待检查的矩形区域。
		 * @return bool 如果可见则返回 true。
		 */
		bool isRectInViewport(const Camera& camera, const SDL_FRect& rect);
	};
}