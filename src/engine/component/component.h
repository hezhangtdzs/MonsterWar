#pragma once

namespace engine::core
{
	class Context;
}
// 前向声明，避免循环包含
namespace engine::object {
	class GameObject;
}

namespace engine::component {
	/**
	 * @brief 组件基类，定义了游戏对象组件的基础接口和属性。
	 *
	 * 所有具体功能的组件（如渲染、物理、脚本等）都应继承自此类。
	 * 组件的生命周期由其所属的 GameObject 管理。
	 */
	class Component {
	protected:
		friend class engine::object::GameObject;

		/**
		 * @brief 指向该组件所属的游戏对象实例。
		 */
		engine::object::GameObject* owner_ = nullptr;

	public:
		Component() = default;
		virtual ~Component() = default;

		// 禁止拷贝和移动语义，以确保组件与其所属 GameObject 的唯一绑定关系。
		Component(const Component&) = delete;
		Component& operator=(const Component&) = delete;
		Component(Component&&) = delete;
		Component& operator=(Component&&) = delete;

		/**
		 * @brief 设置组件的所有者。
		 * @param owner 指向所属 GameObject 的指针。
		 */
		void setOwner(engine::object::GameObject* owner) { owner_ = owner; }

		/**
		 * @brief 获取组件所属的游戏对象。
		 * @return engine::object::GameObject* 指向所有者的指针。
		 */
		engine::object::GameObject* getOwner() const { return owner_; }

	protected:
		/**
		 * @brief 组件初始化。在组件被添加到对象后调用，用于资源的预加载或初值设定。
		 */
		virtual void init() {}

		/**
		 * @brief 处理输入事件。每帧由所属对象调用。
		 */
		virtual void handleInput(engine::core::Context& /*context*/) {}

		/**
		 * @brief 更新组件逻辑。
		 * @param deltaTime 自上一帧以来的时间间隔（秒）。
		 */
		virtual void update(float /*deltaTime*/, engine::core::Context& /*context*/) {}

		/**
		 * @brief 渲染组件内容。每帧在更新之后由渲染管线调用。
		 */
		virtual void render(engine::core::Context& /*context*/) {}

		/**
		 * @brief 组件清理。在组件销毁或移除时被调用，用于释放特定资源。
		 */
		virtual void clean() {}
	};
}