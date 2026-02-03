#include "scene.h"
#include<spdlog/spdlog.h>
#include <algorithm> 
#include "../core/context.h"
#include "../core/game_state.h"
#include "../physics/physics_engine.h"
#include "../object/game_object.h"
#include "../render/camera.h" // 添加Camera头文件
#include "../ui/ui_manager.h" // 添加UI管理器头文件

/**
 * @brief 构造函数。
 * @param scene_name 场景名称。
 * @param context 引擎上下文。
 * @param scene_manager 场景管理器引用。
 */
engine::scene::Scene::Scene(const std::string& scene_name, engine::core::Context& context, engine::scene::SceneManager& scene_manager)
	:scene_name_(scene_name), context_(context), scene_manager_(scene_manager)
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
	// 先更新物理，再更新相机与对象逻辑，避免同一帧重复积分导致抖动/延迟感
	if(context_.getGameState().isPlaying()){
		context_.getPhysicsEngine().update(delta_time);
		context_.getCamera().update(delta_time); // 更新相机
	}

	for (auto it = game_objects_.begin(); it != game_objects_.end();)
	{
			if (*it && !(*it)->getNeedRemove()){
				(*it)->update(delta_time, context_);
			++it;
			}
			else {
				if (*it) {
					(*it)->clean();
				}
				it = game_objects_.erase(it);
			}
		
	}
	processPendingGameObjects();
	
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
		for (const auto& obj : game_objects_) {
			if (obj) {
				obj->render(context_);
			}
		}
		
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
		
		// 如果UI没有处理事件，再处理游戏对象的输入事件
		if (!ui_handled) {
			for (auto it = game_objects_.begin(); it != game_objects_.end();)
			{
				if (*it && !(*it)->getNeedRemove()) {
					(*it)->handleInput(context_);
					++it;
				}
				else {
					if (*it) {
						(*it)->clean();
					}
					it = game_objects_.erase(it);
				}
			}
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
		for (auto& obj : game_objects_) {
			if (obj) {
				obj->clean();
			}
		}
		game_objects_.clear();
		
		// 清理UI管理器
		if (ui_manager_) {
			ui_manager_->clear();
		}
		
		is_initialized_ = false;
		spdlog::trace("Scene {} 清理完成", scene_name_);
	}
}

/**
 * @brief 立即添加一个游戏对象到场景容器中。
 * @param game_object 要添加的游戏对象（使用移动语义）。
 */
void engine::scene::Scene::addGameObject(std::unique_ptr<engine::object::GameObject>&& game_object)
{
	if (game_object) game_objects_.emplace_back(std::move(game_object));
	spdlog::trace("Scene {} 添加游戏对象，当前对象数量：{}", scene_name_, game_objects_.size());
}

/**
 * @brief 安全地添加游戏对象，存入待处理队列在下一帧加入。
 * @param game_object 要添加的游戏对象（使用移动语义）。
 */
void engine::scene::Scene::safeAddGameObject(std::unique_ptr<engine::object::GameObject>&& game_object)
{
	if (game_object) pending_game_objects_.emplace_back(std::move(game_object));
	spdlog::trace("Scene {} 安全添加游戏对象，待处理对象数量：{}", scene_name_, pending_game_objects_.size());
}

/**
 * @brief 从场景中立即移除指定的对象实例。
 * @param game_object 要移除的游戏对象指针。
 */
void engine::scene::Scene::removeGameObject(engine::object::GameObject* game_object)
{
	if (!game_object) {
		spdlog::warn("尝试从场景 '{}' 中移除一个空的游戏对象指针。", scene_name_);
		return;
	}
	auto it = std::remove_if(game_objects_.begin(), game_objects_.end(),
			[game_object](const std::unique_ptr<engine::object::GameObject>& obj) {
				return obj.get() == game_object;
			});
	if (it != game_objects_.end()) {
		(*it)->clean();
		game_objects_.erase(it, game_objects_.end());
		spdlog::trace("Scene {} 移除游戏对象，当前对象数量：{}", scene_name_, game_objects_.size());
	}
	else {
		spdlog::warn("在场景 '{}' 中未找到要移除的游戏对象。", scene_name_);
	}
}

/**
 * @brief 安全地标记移除游戏对象（通过 setNeedRemove）。
 * @param game_object 要标记移除的游戏对象指针。
 */
void engine::scene::Scene::safeRemoveGameObject(engine::object::GameObject* game_object)
{
	game_object->setNeedRemove(true);
}

/**
 * @brief 获取当前场景中指定名称的对象。
 * @param name 要查找的游戏对象名称。
 * @return 找到的对象指针，否则返回 nullptr。
 */
engine::object::GameObject* engine::scene::Scene::findGameObjectByName(const std::string& name) const
{
	for (const auto& obj : game_objects_) {
		if (obj->getName() == name) {
			return obj.get();
		}
	}
	return nullptr;
}

/**
 * @brief 处理积压的游戏对象添加请求，确保容器操作的安全性。
 */
void engine::scene::Scene::processPendingGameObjects()
{
	for (auto& obj : pending_game_objects_) {
		addGameObject(std::move(obj));
	}
	pending_game_objects_.clear();

}
