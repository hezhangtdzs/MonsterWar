#pragma once
#include <vector>
#include <memory>

namespace game::data {
	class SessionData;
}

namespace engine::core {
	class Context;
}

namespace engine::scene {
	class Scene;

	/**
	 * @class SceneManager
	 * @brief 场景管理器，负责游戏场景的切换、叠加（栈管理）和生命周期调度。
	 *
	 * 采用“延迟处理”机制（Pending Actions），确保在帧更新期间请求场景切换时，
	 * 不会立即破坏当前正在遍历的场景数据，而是在帧末尾统一处理。
	 */
	class SceneManager {
	private:
		engine::core::Context& context_;                   ///< 全局上下文引用
		std::vector<std::unique_ptr<Scene>> scene_stack_; ///< 场景栈，支持场景叠加（如在游戏场景上弹出UI菜单）
		std::shared_ptr<game::data::SessionData> session_data_; ///< 共享游戏数据，跨场景持久化
		bool session_data_initialized_; ///< 会话数据是否已初始化

		/** @brief 待处理的场景操作类型 */
		enum class PendingAction {
			None,    ///< 无操作
			Push,    ///< 压入新场景
			Pop,     ///< 弹出当前场景
			Replace  ///< 替换整个场景栈
		};

		PendingAction pending_action_ = PendingAction::None; ///< 当前帧末尾要执行的操作
		std::unique_ptr<Scene> pending_scene_;               ///< 待处理的目标场景实例

	public:
		/**
		 * @brief 构造场景管理器。
		 * @param context 游戏核心上下文。
		 */
		explicit SceneManager(engine::core::Context& context);
		
		/** @brief 析构时会自动调用 close() 清理场景栈。 */
		~SceneManager();

		// 禁止拷贝和移动语义，确保管理器唯一性
		SceneManager(const SceneManager&) = delete;
		SceneManager& operator=(const SceneManager&) = delete;
		SceneManager(SceneManager&&) = delete;
		SceneManager& operator=(SceneManager&&) = delete;

		/** @brief 异步请求：将一个新场景压入栈顶（当前场景会被暂停但不销毁）。 */
		void requestPushScene(std::unique_ptr<Scene>&& scene);

		/** @brief 异步请求：弹出当前栈顶场景，返回到上一个场景。 */
		void requestPopScene();

		/** @brief 异步请求：清空当前所有场景，并切换到全新的场景。 */
		void requestReplaceScene(std::unique_ptr<Scene>&& scene);

		/**
		 * @brief 获取当前正处于栈顶的活动场景。
		 * @return Scene* 指向当前场景的指针，如果栈为空则返回 nullptr。
		 */
		Scene* getCurrentScene() const;

		/** @brief 获取上下文引用。 */
		engine::core::Context& getContext() const { return context_; }

		/**
		 * @brief 帧更新逻辑：更新当前活跃场景，并随后处理挂起的场景操作。
		 * @param delta_time 帧间隔时间。
		 */
		void update(float delta_time);

		/**
		 * @brief 帧渲染逻辑：从底向上叠加渲染场景栈中的所有场景。
		 */
		void render();

		/** @brief 处理输入逻辑：通常只传递给栈顶场景。 */
		void handleInput();

		/** @brief 立即清理场景栈并调用所有场景的 clean() 方法。 */
		void close();

		/** @brief 获取会话数据实例 */
		std::shared_ptr<game::data::SessionData> getSessionData() {
			return session_data_;
		}

		/** @brief 设置会话数据实例 */
		void setSessionData(std::shared_ptr<game::data::SessionData> session_data) {
			session_data_ = std::move(session_data);
			session_data_initialized_ = true;
		}

		/** @brief 检查会话数据是否已初始化 */
		bool isSessionDataInitialized() const { return session_data_initialized_; }

	private:
		/** @brief 处理由 requestXXX 函数提交的异步操作。 */
		void processPendingActions();

		/** @brief 内部执行：初始化新场景并压入栈。 */
		void pushScene(std::unique_ptr<Scene>&& scene);

		/** @brief 内部执行：清理并移除栈顶场景。 */
		void popScene();

		/** @brief 内部执行：清理全栈并设置新场景。 */
		void replaceScene(std::unique_ptr<Scene>&& scene);
	};
}