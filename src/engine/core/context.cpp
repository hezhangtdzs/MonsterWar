#include "context.h"
#include "../input/input_manager.h"
#include "../render/renderer.h"
#include "../render/text_renderer.h"
#include "../render/camera.h"
#include "../resource/resource_manager.h"
#include "../physics/physics_engine.h"
#include "game_state.h"

#include<spdlog/spdlog.h>
engine::core::Context::Context(engine::render::Renderer& renderer, 
							   engine::render::TextRenderer& text_renderer,
							   engine::render::Camera& camera, 
							   engine::resource::ResourceManager& resource_manager, 
							   engine::input::InputManager& input_manager,
							   engine::physics::PhysicsEngine& physics_engine,
							   engine::core::GameState& game_state)
							 : renderer_(renderer),
							   text_renderer_(text_renderer),
							   camera_(camera),
							   resource_manager_(resource_manager),
							   input_manager_(input_manager),
							   physics_engine_(physics_engine),
							   game_state_(game_state)
{
	spdlog::info("Context created.");
}
