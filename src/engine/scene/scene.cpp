#include "scene.h"
#include<spdlog/spdlog.h>
#include <algorithm> 
#include "../core/context.h"
#include "../core/game_state.h"
#include "../render/camera.h" // 添加Camera头文件
#include "../ui/ui_manager.h" // 添加UI管理器头文件
#include "../utils/events.h"
/**
 * @brief 构造函数。
 * @param scene_name 场景名称。
 * @param context 引擎上下文。
 * @param scene_manager 场景管理器引用。
 */
engine::scene::Scene::Scene(const std::string_view scene_name, engine::core::Context& context)
	:scene_name_(scene_name), context_(context)
{
	// 初始化UI管理器
	ui_manager_ = std::make_unique<engine::ui::UIManager>(context_);
	spdlog::info("Scene {} 创建，UI管理器初始化完成", scene_name_);
}

/**
 * @brief 析构函数。
 */
engine::scene::Scene::~Scene() = default;

/**
 * @brief 初始化场景内容。在进入场景时由 SceneManager 调用。
 */
void engine::scene::Scene::init()
{
	is_initialized_ = true;
	spdlog::trace("Scene {} 初始化", scene_name_);
}

/**
 * @brief 更新场景逻辑。每帧由主循环调用。
 * @param delta_time 自上一帧的时间间隔（秒）。
 */
void engine::scene::Scene::update(float delta_time)
{
	if(!is_initialized_) return;
	
	// 更新UI
	if (ui_manager_) {
		ui_manager_->update(delta_time);
	}
}

/**
 * @brief 渲染场景内容。负责触发所有游戏对象的渲染。
 */
void engine::scene::Scene::render()
{
	if(is_initialized_){
		
		// 渲染UI（在游戏对象之上）
		if (ui_manager_) {
			ui_manager_->render();
		}
	}
}

/**
 * @brief 处理场景特定的输入。
 * @return 是否处理了输入事件。
 */
bool engine::scene::Scene::handleInput()
{
	if (is_initialized_) {
		// 优先处理UI输入事件
		bool ui_handled = false;
		if (ui_manager_) {
			ui_handled = ui_manager_->handleInput();
		}
		
		// 返回是否处理了输入事件
		return ui_handled;
	}
	// 如果场景未初始化，返回false
	return false;
}

/**
 * @brief 清理场景资源。在退出或销毁场景前调用。
 */
void engine::scene::Scene::clean()
{
	if(is_initialized_){

		// 清理UI管理器
		if (ui_manager_) {
			ui_manager_->clear();
		}
		
		is_initialized_ = false;
		spdlog::trace("Scene {} 清理完成", scene_name_);
	}
}

void engine::scene::Scene::requestPopScene()
{
    context_.getDispatcher().trigger<engine::utils::PopSceneEvent>();
}

void engine::scene::Scene::requestPushScene(std::unique_ptr<engine::scene::Scene>&& scene)
{
    context_.getDispatcher().trigger<engine::utils::PushSceneEvent>(engine::utils::PushSceneEvent{std::move(scene)});
}

void engine::scene::Scene::requestReplaceScene(std::unique_ptr<engine::scene::Scene>&& scene)
{
    context_.getDispatcher().trigger<engine::utils::ReplaceSceneEvent>(engine::utils::ReplaceSceneEvent{std::move(scene)});
}

void engine::scene::Scene::quit()
{
    context_.getDispatcher().trigger<engine::utils::QuitEvent>();
}
