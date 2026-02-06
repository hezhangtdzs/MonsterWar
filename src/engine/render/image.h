/**
 * @file image.h
 * @brief 定义 Image 类，用于ECS架构中的纹理渲染。
 *
 * @details
 * Image 类是ECS架构中渲染模块的基础数据结构，用于存储纹理引用和渲染参数。
 * 它与 SpriteComponent 配合使用，为渲染系统提供必要的数据。
 *
 * 设计特点：
 * - 轻量级：只存储纹理ID和渲染参数，不持有实际纹理资源
 * - 灵活性：支持通过ID或路径引用纹理
 * - 可配置：支持裁剪矩形和翻转设置
 *
 * @par 与ECS架构的关系
 * - Image 作为 Sprite 结构体的一部分存储在 SpriteComponent 中
 * - RenderSystem 读取 Image 数据并提交给 Renderer 进行绘制
 * - AnimationSystem 可以修改 Image 的 source_rect_ 实现动画效果
 *
 * @par 使用示例
 * @code
 * // 创建Image（通过纹理ID）
 * Image image("player_texture"_hs);
 *
 * // 创建Image（通过路径）
 * Image image("assets/player.png");
 *
 * // 设置裁剪区域（用于精灵图集）
 * image.setSourceRect(SDL_FRect{0, 0, 32, 32});
 *
 * // 水平翻转
 * image.setIsFlipped(true);
 * @endcode
 *
 * @see engine::component::Sprite 包含Image的结构体
 * @see engine::component::SpriteComponent 存储Sprite的ECS组件
 * @see engine::system::RenderSystem 使用Image进行渲染的系统
 */

#pragma once

#include <string>
#include <string_view>
#include <optional>
#include <SDL3/SDL_rect.h>
#include "../resource/resource_id.h"

namespace engine::render {
	/**
	 * @class Image
	 * @brief 可渲染图像类，存储纹理引用和渲染参数。
	 *
	 * @details
	 * Image 类是渲染系统的核心数据结构，用于定义要渲染的图像。
	 * 它存储纹理的引用（通过ID或路径）以及渲染参数（裁剪区域、翻转等）。
	 *
	 * @par 纹理引用方式
	 * - 通过 texture_id_：使用资源ID（推荐，性能更好）
	 * - 通过 texture_path_：使用文件路径（便于调试）
	 *
	 * @par 渲染参数
	 * - source_rect_：裁剪矩形，定义从纹理中渲染的部分
	 * - is_flipped_：水平翻转，用于实现角色面向左右
	 */
	class Image final {
	private:
		/** @brief 纹理在资源管理器中的唯一标识符（哈希值） */
		engine::resource::ResourceId texture_id_{ engine::resource::InvalidResourceId };

		/** @brief 可选的纹理路径（用于懒加载或调试） */
		std::optional<std::string> texture_path_;

		/** @brief 纹理的源裁剪矩形。如果为 nullopt，则渲染整个纹理 */
		std::optional<SDL_FRect> source_rect_;

		/** @brief 渲染时是否进行水平翻转 */
		bool is_flipped_{ false };

	public:
		/** @brief 默认构造函数，创建一个空的Image */
		Image() = default;

		/**
		 * @brief 通过纹理ID构造Image（推荐方式）
		 * @param texture_id 纹理的资源ID（使用 entt::hashed_string 生成）
		 * @param source_rect 纹理的源裁剪区域，默认为 nullopt（渲染整个纹理）
		 * @param is_flipped 是否水平翻转，默认为 false
		 *
		 * @note 使用此方法前，需确保纹理已通过 ResourceManager 加载
		 */
		explicit Image(engine::resource::ResourceId texture_id,
						const std::optional<SDL_FRect>& source_rect = std::nullopt,
						bool is_flipped = false)
						: texture_id_(texture_id), source_rect_(source_rect), is_flipped_(is_flipped) {}

		/**
		 * @brief 通过纹理路径构造Image（便于调试）
		 * @param texture_path 纹理文件的路径
		 * @param source_rect 纹理的源裁剪区域，默认为 nullopt
		 * @param is_flipped 是否水平翻转，默认为 false
		 *
		 * @note 路径会被自动哈希为 texture_id_，便于后续快速查找
		 */
		explicit Image(std::string_view texture_path,
						const std::optional<SDL_FRect>& source_rect = std::nullopt,
						bool is_flipped = false)
						: texture_id_(engine::resource::toResourceId(texture_path)),
						  texture_path_(std::string(texture_path)),
						  source_rect_(source_rect),
						  is_flipped_(is_flipped) {}

		/** @brief 默认析构函数 */
		~Image() = default;

		// Getters

		/**
		 * @brief 获取纹理的标识符。
		 * @return 纹理ID
		 */
		[[nodiscard]] engine::resource::ResourceId getTextureId() const { return texture_id_; }

		/**
		 * @brief 获取纹理路径（如果有）。
		 * @return 纹理路径，如果没有则返回空字符串视图
		 */
		[[nodiscard]] std::string_view getTexturePath() const {
			return texture_path_.has_value() ? std::string_view(*texture_path_) : std::string_view{};
		}

		/**
		 * @brief 获取纹理的源裁剪矩形。
		 * @return 可选的 SDL_FRect 引用
		 */
		[[nodiscard]] const std::optional<SDL_FRect>& getSourceRect() const { return source_rect_; }

		/**
		 * @brief 检查精灵是否处于翻转状态。
		 * @return 如果已翻转返回 true，否则返回 false
		 */
		[[nodiscard]] bool getIsFlipped() const { return is_flipped_; }

		// Setters

		/**
		 * @brief 设置纹理的标识符。
		 * @param texture_id 新的纹理ID
		 *
		 * @note 设置ID会清除 texture_path_
		 */
		void setTextureId(engine::resource::ResourceId texture_id) {
			texture_id_ = texture_id;
			texture_path_.reset();
		}

		/**
		 * @brief 设置纹理路径。
		 * @param texture_path 新的纹理路径
		 *
		 * @note 路径会被自动哈希为ID
		 */
		void setTexturePath(std::string_view texture_path) {
			texture_id_ = engine::resource::toResourceId(texture_path);
			texture_path_ = std::string(texture_path);
		}

		/**
		 * @brief 设置纹理的源裁剪矩形。
		 * @param source_rect 新的裁剪区域
		 */
		void setSourceRect(const std::optional<SDL_FRect>& source_rect) { source_rect_ = source_rect; }

		/**
		 * @brief 设置精灵的水平翻转状态。
		 * @param is_flipped 是否翻转
		 */
		void setIsFlipped(bool is_flipped) { is_flipped_ = is_flipped; }
	};
}