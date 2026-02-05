#pragma once
#include "../utils/math.h"
#include <SDL3/SDL_rect.h>
#include <entt/core/hashed_string.hpp>
#include <entt/entity/entity.hpp>
#include <glm/vec2.hpp>
#include <glm/common.hpp>
#include <utility>
#include <string>


namespace engine::component {
	
	/**
	 * @struct Sprite
	 * @brief 精灵组件
	 * 
	 * 负责实体的基础渲染信息，包括纹理引用和渲染区域。
	 */
	struct Sprite{
		entt::id_type texture_id_{entt::null};  ///< 纹理资源的 ID
    	std::string texture_path_;              ///< 纹理资源的文件路径（可选，用于加载或调试）
    	engine::utils::Rect src_rect_{};        ///< 纹理中的源矩形区域（帧位置）
    	bool is_flipped_{false};                ///< 是否要在水平方向上翻转显示

    	Sprite() = default;     ///< 默认构造函数

		/**
		 * @brief 通过纹理 ID 构造精灵
		 * @param texture_id 已加载的纹理 ID
		 * @param src_rect 源矩形区域
		 * @param is_flipped 是否翻转，默认为假
		 */
		Sprite(entt::id_type texture_id,
			   const engine::utils::Rect& src_rect,
			   bool is_flipped = false)
			   : texture_id_(texture_id),
				 src_rect_(src_rect),
				 is_flipped_(is_flipped) {}

		/**
		 * @brief 通过纹理路径构造精灵
		 * @param texture_path 纹理图片文件的相对路径
		 * @param src_rect 源矩形区域
		 * @param is_flipped 是否翻转，默认为假
		 */
		Sprite(const std::string& texture_path,
			   const engine::utils::Rect& src_rect,
			   bool is_flipped = false)
			   : texture_id_(entt::hashed_string::value(texture_path.c_str())),
				 texture_path_(texture_path),
				 src_rect_(src_rect),
				 is_flipped_(is_flipped) {}
	};

	/**
	 * @struct SpriteComponent
	 * @brief 精灵组件（ECS 组件）
	 * 
	 * 挂载到实体上，用于渲染系统识别并绘制精灵。
	 * 包含精灵数据、渲染偏移和渲染大小。
	 */
	struct SpriteComponent {
		Sprite sprite_;                     ///< 基础精灵数据
		glm::vec2 offset_{ 0.0f, 0.0f };    ///< 渲染位置偏移（相对于 TransformComponent 的位置）
		glm::vec2 size_{ 0.0f, 0.0f };      ///< 渲染目标大小

		SpriteComponent() = default;

		/**
		 * @brief 通过精灵数据构造组件（自动提取大小）
		 * @param sprite 基础精灵数据
		 */
		SpriteComponent(Sprite sprite)
			: sprite_(std::move(sprite)) {
			size_ = { sprite_.src_rect_.size.x, sprite_.src_rect_.size.y };
		}

		/**
		 * @brief 构造函数
		 * @param sprite 基础精灵数据
		 * @param size 初始渲染大小
		 * @param offset 初始渲染偏移
		 */
		SpriteComponent(Sprite sprite, glm::vec2 size, glm::vec2 offset = { 0.0f, 0.0f })
			: sprite_(std::move(sprite)), size_(size), offset_(offset) {}
	};
}