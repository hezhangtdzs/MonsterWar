/**
 * @file context.cpp
 * @brief Context 类的实现，提供对引擎核心系统的统一访问。
 */

#include "context.h"
#include "../input/input_manager.h"
#include "../render/renderer.h"
#include "../render/text_renderer.h"
#include "../render/camera.h"
#include "../resource/resource_manager.h"

#include "game_state.h"

#include<spdlog/spdlog.h>
/**
 * @brief 构造函数，初始化上下文并保存各系统引用。
 * @param renderer 渲染器引用
 * @param text_renderer 文本渲染器引用
 * @param dispatcher 事件分发器引用
 * @param camera 摄像机引用
 * @param resource_manager 资源管理器引用
 * @param input_manager 输入管理器引用
 * @param game_state 游戏状态引用
 */
engine::core::Context::Context(engine::render::Renderer& renderer,
							   engine::render::TextRenderer& text_renderer,
							   entt::dispatcher& dispatcher,
							   engine::render::Camera& camera,
							   engine::resource::ResourceManager& resource_manager,
							   engine::input::InputManager& input_manager,
							   engine::core::GameState& game_state)
							 : renderer_(renderer),
							   text_renderer_(text_renderer),
							   dispatcher_(dispatcher),
							   camera_(camera),
							   resource_manager_(resource_manager),
							   input_manager_(input_manager),
							   game_state_(game_state)
{
	spdlog::info("Context created.");
}
