/**
 * @file parallax_component.h
 * @brief 定义 ParallaxComponent 组件，用于ECS架构中的视差滚动效果。
 *
 * @details
 * ParallaxComponent 是ECS架构中的视觉特效组件，用于实现背景或前景的视差滚动效果。
 * 通过不同的滚动因子使图层以不同于相机的速度移动，从而产生深度感和立体感。
 *
 * @par 视差效果原理
 * - scroll_factor_ = 0: 图层相对于相机静止（如UI层）
 * - scroll_factor_ = 1: 图层随相机完全同步移动（如前景）
 * - scroll_factor_ < 1: 图层移动比相机慢，产生远景感（如远山背景）
 * - scroll_factor_ > 1: 图层移动比相机快，产生近景感（如前景装饰）
 *
 * @par 使用示例
 * @code
 * // 创建远景背景（移动慢，产生深度感）
 * auto bg_entity = registry.create();
 * registry.emplace<ParallaxComponent>(bg_entity, glm::vec2(0.3f, 0.3f));
 * registry.emplace<SpriteComponent>(bg_entity, bg_sprite);
 *
 * // 创建近景装饰（移动快）
 * auto fg_entity = registry.create();
 * registry.emplace<ParallaxComponent>(fg_entity, glm::vec2(1.5f, 1.5f));
 * registry.emplace<SpriteComponent>(fg_entity, fg_sprite);
 * @endcode
 *
 * @see engine::render::Camera 相机类，决定视差计算的基础
 */

#pragma once
#include <glm/vec2.hpp>
#include <string>

namespace engine::component {
	
	/**
	 * @struct ParallaxComponent
	 * @brief 视差滚动组件，实现背景或前景的视差效果。
	 *
	 * @details
	 * 通过设置不同的滚动因子，使图层以不同速度相对于相机移动，
	 * 从而产生深度感和立体感。常用于2D游戏中的背景层。
	 */
	struct ParallaxComponent
	{
		glm::vec2 scroll_factor_{};         ///< 滚动速度因子 (0=相对相机静止/UI, 1=随相机完全同步移动, <1=移动比相机慢产生远景感, >1=移动比相机快产生近景感)
		glm::bvec2 repeat_{true};           ///< 在 X 或 Y 轴上是否循环平铺背景
		bool is_visible_{true};             ///< 这一层视差图层当前是否可见

		/**
		 * @brief 构造函数
		 * @param scroll_factor 滚动因子，控制图层相对于相机的移动速度，默认为 {1.0, 1.0}
		 * @param repeat 是否循环平铺，默认为 {true, true}
		 */
		ParallaxComponent(const glm::vec2& scroll_factor = { 1.0f, 1.0f }, const glm::bvec2& repeat = { true, true })
			: scroll_factor_(scroll_factor), repeat_(repeat) {}
	};
} // namespace engine::component