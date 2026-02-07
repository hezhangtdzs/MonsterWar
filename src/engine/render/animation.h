/**
 * @file animation.h
 * @brief 定义动画相关的结构体和类，用于处理精灵动画。
 * 
 * @details
 * 该文件定义了动画系统的核心数据结构：
 * - AnimationFrame：单个动画帧的数据结构
 * - Animation：动画管理类，处理动画帧序列和播放逻辑
 * 
 * 这些类与组件模块中的动画组件配合使用，实现游戏中的精灵动画效果。
 */

#pragma once
#include <SDL3/SDL_rect.h>
#include <vector>
#include <string>

namespace engine::render {
	/**
	 * @struct AnimationFrame
	 * @brief 动画帧结构体，包含单个动画帧的纹理区域和持续时间。
	 * 
	 * @details
	 * AnimationFrame 定义了动画序列中的一帧，包含：
	 * - src_rect：帧在纹理图集中的位置和大小
	 * - duration：该帧显示的时间长度（秒）
	 */
	struct AnimationFrame {
		SDL_FRect src_rect;  ///< 纹理源矩形，定义帧在纹理中的位置和大小
		float duration;  ///< 帧持续时间（秒）
	};

	/**
	 * @class Animation
	 * @brief 动画类，管理一系列动画帧和动画播放逻辑。
	 * 
	 * @details
	 * Animation 类提供了完整的动画管理功能：
	 * - 管理动画帧序列
	 * - 计算总动画时长
	 * - 支持循环播放
	 * - 根据时间获取当前帧
	 * 
	 * @see engine::component::AnimationComponent ECS架构中的动画组件
	 */
	class Animation final {
	private:
		std::string name_;  ///< 动画名称，用于标识不同的动画
		std::vector<AnimationFrame> frames_;  ///< 动画帧列表，按播放顺序存储
		float total_duration_;  ///< 动画总持续时间（秒）
		bool loop_;  ///< 是否循环播放

	public:
		/**
		 * @brief 构造函数，创建一个新的动画实例。
		 * @param name 动画名称，默认为 "default"
		 * @param loop 是否循环播放，默认为 true
		 */
		Animation(const std::string& name = "default", bool loop = true);

		/**
		 * @brief 默认析构函数
		 */
		~Animation() = default;

		/**
		 * @brief 向动画添加一个帧。
		 * @param source_rect 帧的纹理源矩形
		 * @param duration 帧的持续时间（秒）
		 * 
		 * @note 调用此方法会自动更新总动画时长
		 */
		void addFrame(const SDL_FRect& source_rect, float duration);

		/**
		 * @brief 根据当前时间获取对应的动画帧。
		 * @param time 当前时间（秒）
		 * @return 对应的动画帧引用
		 * 
		 * @details
		 * 该方法会根据时间和循环设置计算当前应该显示的帧：
		 * - 如果动画为空，返回默认帧
		 * - 如果时间超过总时长且循环，计算时间的余数
		 * - 如果时间超过总时长且不循环，返回最后一帧
		 */
		const AnimationFrame& getFrame(float time) const;

		/**
		 * @brief 获取动画名称。
		 * @return 动画名称的常量引用
		 */
		const std::string& getName() const { return name_; }

		/**
		 * @brief 获取动画总持续时间。
		 * @return 总持续时间（秒）
		 */
		float getTotalDuration() const { return total_duration_; }

		/**
		 * @brief 检查动画是否循环播放。
		 * @return 循环播放返回 true，否则返回 false
		 */
		bool isLooping() const { return loop_; }

		/**
		 * @brief 检查动画是否为空（没有帧）。
		 * @return 为空返回 true，否则返回 false
		 */
		bool isEmpty() const { return frames_.empty(); }
	};

}