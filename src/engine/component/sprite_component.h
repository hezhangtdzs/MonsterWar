/**
 * @file sprite_component.h
 * @brief 定义 Sprite 和 SpriteComponent 组件，用于ECS架构中的实体渲染。
 *
 * @details
 * SpriteComponent 是ECS架构中的渲染相关组件，负责存储实体的视觉表现数据。
 * 该组件与 TransformComponent 配合使用，由 RenderSystem 每帧渲染到屏幕上。
 *
 * 组件采用分离设计：
 * - Sprite 结构体：存储纹理引用和裁剪信息（可复用的资源数据）
 * - SpriteComponent：存储渲染实例数据（偏移、大小等）
 *
 * @par 使用示例
 * @code
 * // 创建可渲染的实体
 * auto entity = registry.create();
 * registry.emplace<TransformComponent>(entity, glm::vec2(100.0f, 100.0f));
 *
 * // 创建精灵数据
 * engine::component::Sprite sprite("assets/player.png",
 *                                   engine::utils::Rect(0, 0, 32, 32));
 *
 * // 添加精灵组件
 * registry.emplace<SpriteComponent>(entity, sprite);
 * @endcode
 *
 * @see engine::component::TransformComponent 定义渲染位置的组件
 * @see engine::system::RenderSystem 负责渲染精灵的系统
 * @see engine::component::AnimationComponent 可修改精灵的动画组件
 */

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
	 * @brief 精灵数据结构，存储纹理引用和渲染区域信息。
	 *
	 * @details
	 * Sprite 是一个轻量级的数据结构，用于定义图像资源在纹理图集中的位置。
	 * 它可以被多个 SpriteComponent 共享使用，实现资源复用。
	 *
	 * 支持两种纹理引用方式：
	 * - 通过 texture_id_：使用已加载的纹理资源ID（推荐，性能更好）
	 * - 通过 texture_path_：使用文件路径（便于调试和懒加载）
	 */
	struct Sprite{
		entt::id_type texture_id_{entt::null};  ///< 纹理资源的哈希ID，用于快速查找纹理
    	std::string texture_path_;              ///< 纹理资源的文件路径（可选，用于加载或调试）
    	engine::utils::Rect src_rect_{};        ///< 纹理中的源矩形区域（裁剪矩形），定义要渲染的图像部分
    	bool is_flipped_{false};                ///< 是否水平翻转显示，用于实现面向左右的角色

    	Sprite() = default;     ///< 默认构造函数，创建一个空的精灵

		/**
		 * @brief 通过纹理ID构造精灵（推荐方式）
		 * @param texture_id 已加载的纹理资源ID（使用 entt::hashed_string 生成）
		 * @param src_rect 源矩形区域，定义从纹理中裁剪的部分
		 * @param is_flipped 是否水平翻转，默认为false
		 *
		 * @note 使用此方法前，需确保纹理已通过 ResourceManager 加载
		 */
		Sprite(entt::id_type texture_id,
			   const engine::utils::Rect& src_rect,
			   bool is_flipped = false)
			   : texture_id_(texture_id),
				 src_rect_(src_rect),
				 is_flipped_(is_flipped) {}

		/**
		 * @brief 通过纹理路径构造精灵（便于调试）
		 * @param texture_path 纹理图片文件的相对路径
		 * @param src_rect 源矩形区域，定义从纹理中裁剪的部分
		 * @param is_flipped 是否水平翻转，默认为false
		 *
		 * @note 路径会被自动哈希为 texture_id_，便于后续快速查找
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
	 * @brief ECS精灵组件，挂载到实体上用于渲染。
	 *
	 * @details
	 * SpriteComponent 是ECS架构中的渲染组件，包含 Sprite 数据和实例特定的渲染参数。
	 * 该组件由 RenderSystem 查询并渲染，支持偏移和自定义大小。
	 *
	 * @par 渲染流程
	 * 1. RenderSystem 查询所有具有 SpriteComponent 和 TransformComponent 的实体
	 * 2. 计算最终渲染位置：transform.position + sprite_component.offset_
	 * 3. 应用缩放：sprite_component.size_ * transform.scale_
	 * 4. 提交渲染命令到 Renderer
	 *
	 * @par 与动画的配合
	 * AnimationSystem 可以修改 SpriteComponent 中的 sprite_.src_rect_，
	 * 实现动画帧切换，而无需修改组件的其他属性。
	 */
	struct SpriteComponent {
		Sprite sprite_;                     ///< 基础精灵数据，包含纹理引用和裁剪信息
		glm::vec2 offset_{ 0.0f, 0.0f };    ///< 渲染位置偏移（相对于 TransformComponent 的位置），用于微调渲染位置
		glm::vec2 size_{ 0.0f, 0.0f };      ///< 渲染目标大小，(0,0)表示使用源矩形原始大小

		SpriteComponent() = default;        ///< 默认构造函数

		/**
		 * @brief 通过精灵数据构造组件（自动提取大小）
		 * @param sprite 基础精灵数据
		 *
		 * @note size_ 会自动设置为 src_rect_ 的尺寸
		 */
		SpriteComponent(Sprite sprite)
			: sprite_(std::move(sprite)) {
			size_ = { sprite_.src_rect_.size.x, sprite_.src_rect_.size.y };
		}

		/**
		 * @brief 完整构造函数
		 * @param sprite 基础精灵数据
		 * @param size 渲染目标大小，可自定义缩放
		 * @param offset 渲染位置偏移，默认为 (0, 0)
		 */
		SpriteComponent(Sprite sprite, glm::vec2 size, glm::vec2 offset = { 0.0f, 0.0f })
			: sprite_(std::move(sprite)), size_(size), offset_(offset) {}
	};
}