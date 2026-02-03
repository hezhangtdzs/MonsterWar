#include"scene_manager.h"
#include "scene.h"
#include <spdlog/spdlog.h>

namespace engine::scene {

/**
 * @brief 构造场景管理器。
 * @param context 游戏核心上下文。
 */
SceneManager::SceneManager(engine::core::Context& context)
	: context_(context),
	  session_data_initialized_(false)
{
	spdlog::info("SceneManager created");
}

/**
 * @brief 析构时会自动调用 close() 清理场景栈。
 */
SceneManager::~SceneManager() {
	spdlog::info("SceneManager destroyed");
	close();
};

/**
 * @brief 异步请求：将一个新场景压入栈顶（当前场景会被暂停但不销毁）。
 * @param scene 待压入的场景实例。
 */
void SceneManager::requestPushScene(std::unique_ptr<Scene>&& scene)
{
	pending_action_ = PendingAction::Push;
	pending_scene_ = std::move(scene);
}

/**
 * @brief 异步请求：弹出当前栈顶场景，返回到上一个场景。
 */
void SceneManager::requestPopScene()
{
	pending_action_ = PendingAction::Pop;
}

/**
 * @brief 异步请求：清空当前所有场景，并切换到全新的场景。
 * @param scene 待替换的场景实例。
 */
void SceneManager::requestReplaceScene(std::unique_ptr<Scene>&& scene)
{
	pending_action_ = PendingAction::Replace;
	pending_scene_ = std::move(scene);
}

/**
 * @brief 获取当前正处于栈顶的活动场景。
 * @return Scene* 指向当前场景的指针，如果栈为空则返回 nullptr。
 */
Scene* SceneManager::getCurrentScene() const
{
	if (!scene_stack_.empty()) {
		return scene_stack_.back().get();
	}
	return nullptr;
}

/**
 * @brief 帧更新逻辑：更新当前活跃场景，并随后处理挂起的场景操作。
 * @param delta_time 帧间隔时间。
 */
void SceneManager::update(float delta_time) {
	// 只更新栈顶（当前）场景
	Scene* current_scene = getCurrentScene();
	if (current_scene) {
		current_scene->update(delta_time);
	}
	// 执行可能的切换场景操作
	processPendingActions();
}

/**
 * @brief 帧渲染逻辑：从底向上叠加渲染场景栈中的所有场景。
 */
void SceneManager::render() {
	// 渲染时需要叠加渲染所有场景，而不只是栈顶
	for (const auto& scene : scene_stack_) {
		if (scene) {
			scene->render();
		}
	}
}

/**
 * @brief 处理输入逻辑：通常只传递给栈顶场景。
 */
void SceneManager::handleInput() {
	// 只考虑栈顶场景
	Scene* current_scene = getCurrentScene();
	if (current_scene) {
		current_scene->handleInput();
	}
}

/**
 * @brief 立即清理场景栈并调用所有场景的 clean() 方法。
 */
void SceneManager::close()
{
	spdlog::trace("正在关闭场景管理器并清理场景栈...");
	// 清理栈中所有剩余的场景（从顶到底）
	while (!scene_stack_.empty()) {
		if (scene_stack_.back()) {
			spdlog::debug("正在清理场景 '{}' 。", scene_stack_.back()->getSceneName());
			scene_stack_.back()->clean();
		}
		scene_stack_.pop_back();
	}
}

/**
 * @brief 处理由 requestXXX 函数提交的异步操作。
 */
void SceneManager::processPendingActions()
{
	if (pending_action_ == PendingAction::None) {
		return;
	}
	switch (pending_action_) {
	case PendingAction::Push:
		pushScene(std::move(pending_scene_));
		break;
	case PendingAction::Pop:
		popScene();
		break;
	case PendingAction::Replace:
		replaceScene(std::move(pending_scene_));
		break;
	default:
		break;
	}
	pending_action_ = PendingAction::None;
}

/**
 * @brief 内部执行：初始化新场景并压入栈。
 * @param scene 待压入的场景实例。
 */
void SceneManager::pushScene(std::unique_ptr<Scene>&& scene)
{
	if(!scene) return;
	spdlog::debug("正在将场景 '{}' 压入栈。", scene->getSceneName());
	// 初始化新场景
	if (!scene->isInitialized()) { // 确保只初始化一次
		scene->init();
	}

	// 将新场景移入栈顶
	scene_stack_.push_back(std::move(scene));
}

/**
 * @brief 内部执行：清理并移除栈顶场景。
 */
void SceneManager::popScene()
{
	if (scene_stack_.empty()) {
		spdlog::warn("尝试弹出场景失败：场景栈为空。");
		return;
	}
	if (scene_stack_.back()) {
		scene_stack_.back()->clean();       // 显式调用清理
	}
	scene_stack_.pop_back();
}

/**
 * @brief 内部执行：清理全栈并设置新场景。
 * @param scene 待替换的场景实例。
 */
void SceneManager::replaceScene(std::unique_ptr<Scene>&& scene)
{
	if (!scene) return;

	std::string old_scene_name = scene_stack_.empty() ? "None" : scene_stack_.back()->getSceneName();
	spdlog::debug("正在用场景 '{}' 替换场景 '{}' 。", scene->getSceneName(), old_scene_name);

	// 清理并移除场景栈中所有场景
	while (!scene_stack_.empty()) {
		if (scene_stack_.back()) {
			scene_stack_.back()->clean();
		}
		scene_stack_.pop_back();
	}

	// 初始化新场景
	if (!scene->isInitialized()) {
		scene->init();
	}

	// 将新场景压入栈顶
	scene_stack_.push_back(std::move(scene));
}

} // namespace engine::scene
