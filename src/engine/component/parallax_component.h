#pragma once
#include <glm/vec2.hpp>
#include <string>

namespace engine::component {
	
	/**
	 * @struct parallax_component
	 * @brief 视差滚动组件
	 * 
	 * 用于实现背景或前景的视差滚动效果。通过不同的滚动因子使层以不同于相机的速度移动，从而产生深度感。
	 */
	struct ParallaxComponent
	{
		glm::vec2 scroll_factor_{};         ///< 滚动速度因子 (0=相对相机静止/UI, 1=随相机完全同步移动, <1=移动比相机慢产生远景感)
		glm::bvec2 repeat_{true};           ///< 在 X 或 Y 轴上是否循环平铺背景
		bool is_visible_{true};             ///< 这一层视差图层当前是否可见

		/**
		 * @brief 构造函数
		 * @param scroll_factor 滚动因子，默认为 {1.0, 1.0}
		 * @param repeat 是否循环平铺，默认为 {true, true}
		 */
		ParallaxComponent(const glm::vec2& scroll_factor = { 1.0f, 1.0f }, const glm::bvec2& repeat = { true, true })
			: scroll_factor_(scroll_factor), repeat_(repeat) {}
	};
} // namespace engine::component