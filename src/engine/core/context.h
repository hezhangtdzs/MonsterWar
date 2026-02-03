#pragma once
/**
 * @file context.h
 * @brief 定义 Context 类，用于集中管理游戏引擎的核心系统引用。
 */

#include "../render/camera.h"
#include "../render/text_renderer.h"

namespace engine::input{
	class InputManager;
}
namespace engine::resource
{
	class ResourceManager;
}
namespace engine::render
{
	class Renderer;
	class TextRenderer;
	// Camera is included above.
}

namespace engine::core
{
	class GameState;
}
namespace engine::physics
{
	class PhysicsEngine;
}

namespace engine::core
{
	/**
	 * @class Context
	 * @brief 游戏引擎上下文类，用于集中管理和访问引擎的核心系统。
	 * 
	 * Context 类作为引擎各系统之间的桥梁，提供了对所有核心系统的统一访问点，
	 * 避免了系统之间的直接依赖，提高了代码的模块化和可维护性。
	 */
	class Context final
	{
	private:
		/// 资源管理器引用
		engine::resource::ResourceManager& resource_manager_;
		/// 渲染器引用
		engine::render::Renderer& renderer_;
		/// 文本渲染器引用
		engine::render::TextRenderer& text_renderer_;
		/// 摄像机引用
		engine::render::Camera& camera_;
		/// 输入管理器引用
		engine::input::InputManager& input_manager_;
		/// 物理引擎引用
		engine::physics::PhysicsEngine& physics_engine_;
		/// 游戏状态引用
		engine::core::GameState& game_state_;
	public:
		/**
		 * @brief 构造函数，初始化上下文并保存各系统引用。
		 * @param renderer 渲染器引用
		 * @param text_renderer 文本渲染器引用
		 * @param camera 摄像机引用
		 * @param resource_manager 资源管理器引用
		 * @param input_manager 输入管理器引用
		 * @param physics_engine 物理引擎引用
		 * @param game_state 游戏状态引用
		 */
		Context(engine::render::Renderer& renderer,
				engine::render::TextRenderer& text_renderer,
				engine::render::Camera& camera,
				engine::resource::ResourceManager& resource_manager,
				engine::input::InputManager& input_manager,
				engine::physics::PhysicsEngine& physics_engine,
				engine::core::GameState& game_state);
			

		/// 禁止拷贝构造和移动
		Context(const Context&) = delete;
		Context& operator=(const Context&) = delete;
		Context(Context&&) = delete;
		Context& operator=(Context&&) = delete;

		/**
		 * @brief 获取渲染器引用。
		 * @return engine::render::Renderer& 渲染器引用
		 */
		engine::render::Renderer& getRenderer()
		{
			return renderer_;
		}
		
		/**
		 * @brief 获取文本渲染器引用。
		 * @return engine::render::TextRenderer& 文本渲染器引用
		 */
		engine::render::TextRenderer& getTextRenderer()
		{
			return text_renderer_;
		}
		
		/**
		 * @brief 获取摄像机引用。
		 * @return engine::render::Camera& 摄像机引用
		 */
		engine::render::Camera& getCamera()
		{
			return camera_;
		}
		
		/**
		 * @brief 获取资源管理器引用。
		 * @return engine::resource::ResourceManager& 资源管理器引用
		 */
		engine::resource::ResourceManager& getResourceManager()
		{
			return resource_manager_;
		}
		
		/**
		 * @brief 获取输入管理器引用。
		 * @return engine::input::InputManager& 输入管理器引用
		 */
		engine::input::InputManager& getInputManager()
		{
			return input_manager_;
		}
		
		/**
		 * @brief 获取物理引擎引用。
		 * @return engine::physics::PhysicsEngine& 物理引擎引用
		 */
		engine::physics::PhysicsEngine& getPhysicsEngine()
		{
			return physics_engine_;
		}
		
		/**
		 * @brief 获取游戏状态引用。
		 * @return GameState& 游戏状态引用
		 */
		engine::core::GameState& getGameState()
		{
			return game_state_;
		}
	};

}