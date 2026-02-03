#pragma once
#include "component.h"
#include <glm/vec2.hpp>
#include <string>
#include "../render/sprite.h"

namespace engine::component {
	class TransformComponent;

	/**
	 * @class ParallaxComponent
	 * @brief 视差背景组件
	 * 
	 * 该组件用于实现 2D 游戏中的视差滚动效果（Parallax Scrolling）。
	 * 它渲染一个背景精灵，并根据摄像机的移动自动调整其纹理偏移，从而产生深度感。
	 * 
	 * @note 需要依赖 TransformComponent 来确定基准位置和缩放。
	 */
	class ParallaxComponent final : public Component {
		friend class engine::object::GameObject;
	private:
		TransformComponent* transform_{ nullptr }; ///< 指向所属对象的变换组件的指针，用于获取位置信息

		engine::render::Sprite sprite_;           ///< 负责渲染的背景精灵对象
		
		/**
		 * @brief 视差因子
		 * 
		 * 决定背景相对于摄像机移动的速度比例。
		 * - (1.0, 1.0): 背景随摄像机同步移动（看起来像静态前景）。
		 * - (0.5, 0.5): 背景移动速度是摄像机的一半（看起来在远处）。
		 * - (0.0, 0.0): 背景完全静止（看起来在无穷远处）。
		 */
		glm::vec2 parallax_factor_{ 1.0f, 1.0f }; 

		glm::bvec2 repeat_{ true, true };         ///< 是否在水平(x)和垂直(y)方向上无限平铺背景
		bool is_hidden_{ false };                 ///< 是否隐藏该背景（不进行渲染）

	public:
		/**
		 * @brief 构造函数
		 * 
		 * @param texture_id 纹理资源的路径或ID
		 * @param parallax_factor 视差因子 (默认 1.0)
		 * @param repeat 是否重复平铺 (默认 true)
		 */
		ParallaxComponent(std::string& texture_id, const glm::vec2& parallax_factor = { 1.0f, 1.0f },
			const glm::bvec2& repeat = { true, true });
		
		~ParallaxComponent();

		// --- Getters and Setters ---

		/** @brief 获取当前视差因子 */
		const glm::vec2& getParallaxFactor() const { return parallax_factor_; }
		
		/** @brief 设置视差因子 */
		void setParallaxFactor(const glm::vec2& factor) { parallax_factor_ = factor; }
		
		/** @brief 获取平铺设置 */
		const glm::bvec2& getRepeat() const { return repeat_; }
		
		/** @brief 设置是否在 XY 方向平铺 */
		void setRepeat(const glm::bvec2& repeat) { repeat_ = repeat; }
		
		/** @brief 检查组件是否处于隐藏状态 */
		bool isHidden() const { return is_hidden_; }
		
		/** @brief 设置组件的隐藏状态 */
		void setHidden(bool hidden) { is_hidden_ = hidden; }
		
		/** @brief 获取内部的 Sprite 对象（拷贝） */
		engine::render::Sprite getSprite() const { return sprite_; }
		
		/** @brief 设置内部的 Sprite 对象 */
		void setSprite(engine::render::Sprite sprite) { sprite_ = sprite; }

	protected:
		/**
		 * @brief 初始化生命周期
		 * 
		 * 获取并缓存所属 GameObject 的 TransformComponent。
		 */
		void init() override;

		/**
		 * @brief 更新生命周期
		 * 
		 * 视差计算通常在渲染阶段完成，因此 Update 通常为空。
		 * @param deltaTime 帧间隔时间
		 * @param context 引擎上下文
		 */
		void update(float deltaTime, engine::core::Context& context) override;

		/**
		 * @brief 渲染生命周期
		 * 
		 * 调用 Renderer 绘制视差背景。
		 * @param context 引擎上下文
		 */
		void render(engine::core::Context& context) override;

		/**
		 * @brief 输入处理生命周期
		 * @param context 引擎上下文
		 */
		void handleInput(engine::core::Context& context) override;

		/**
		 * @brief 清理生命周期
		 * 
		 * 释放资源或解除引用。
		 */
		void clean() override;
	};
} // namespace engine::component