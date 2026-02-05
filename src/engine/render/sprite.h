#pragma once

#include <string>
#include <string_view>
#include <optional>
#include <SDL3/SDL_rect.h>
#include "../resource/resource_id.h"

namespace engine::render {
	/**
	 * @class Sprite
	 * @brief 表示一个可渲染的精灵对象，包含纹理引用、裁剪区域及翻转状态。
	 */
	class Sprite final {
	private:
		/** @brief 纹理在资源管理器中的唯一标识符 */
		engine::resource::ResourceId texture_id_{ engine::resource::InvalidResourceId };

		/** @brief 可选的纹理路径（用于懒加载或调试） */
		std::optional<std::string> texture_path_;

		/** @brief 纹理的源裁剪矩形。如果为 nullopt，则渲染整个纹理。 */
		std::optional<SDL_FRect> source_rect_;

		/** @brief 渲染时是否进行水平翻转 */
		bool is_flipped_{ false };

	public:
		Sprite() = default;
		/**
		 * @brief 构造一个新的 Sprite 对象。
		 * @param texture_id 纹理的标识符。
		 * @param source_rect 纹理的源裁剪区域，默认为 nullopt。
		 * @param is_flipped 是否水平翻转，默认为 false。
		 */

		explicit Sprite(engine::resource::ResourceId texture_id,
						const std::optional<SDL_FRect>& source_rect = std::nullopt,
						bool is_flipped = false)
						: texture_id_(texture_id), source_rect_(source_rect), is_flipped_(is_flipped) {}

		explicit Sprite(std::string_view texture_path,
						const std::optional<SDL_FRect>& source_rect = std::nullopt,
						bool is_flipped = false)
						: texture_id_(engine::resource::toResourceId(texture_path)),
						  texture_path_(std::string(texture_path)),
						  source_rect_(source_rect),
						  is_flipped_(is_flipped) {}

		/** @brief 默认析构函数 */
		~Sprite() = default;

		// Getters

		/**
		 * @brief 获取纹理的标识符。
		 * @return 纹理 ID 的常量引用。
		 */
		[[nodiscard]] engine::resource::ResourceId getTextureId() const { return texture_id_; }

		[[nodiscard]] std::string_view getTexturePath() const {
			return texture_path_.has_value() ? std::string_view(*texture_path_) : std::string_view{};
		}

		/**
		 * @brief 获取纹理的源裁剪矩形。
		 * @return 可选的 SDL_Rect 引用。
		 */
		[[nodiscard]] const std::optional<SDL_FRect>& getSourceRect() const { return source_rect_; }

		/**
		 * @brief 检查精灵是否处于翻转状态。
		 * @return 如果已翻转返回 true，否则返回 false。
		 */
		[[nodiscard]] bool getIsFlipped() const { return is_flipped_; }

		// Setters

		/**
		 * @brief 设置纹理的标识符。
		 * @param texture_id 新的纹理 ID。
		 */
		void setTextureId(engine::resource::ResourceId texture_id) {
			texture_id_ = texture_id;
			texture_path_.reset();
		}

		void setTexturePath(std::string_view texture_path) {
			texture_id_ = engine::resource::toResourceId(texture_path);
			texture_path_ = std::string(texture_path);
		}

		/**
		 * @brief 设置纹理的源裁剪矩形。
		 * @param source_rect 新的裁剪区域。
		 */

		void setSourceRect(const std::optional<SDL_FRect>& source_rect) { source_rect_ = source_rect; }

		/**
		 * @brief 设置精灵的水平翻转状态。
		 * @param is_flipped 是否翻转。
		 */

		void setIsFlipped(bool is_flipped) { is_flipped_ = is_flipped; }
	};
}