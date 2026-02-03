#pragma once
#include <string>
#include <vector>
#include <memory>
namespace engine::core {
	class Context;
}
namespace engine::object{
	class GameObject;
}
namespace engine::resource {
	class ResourceManager;
}
namespace engine::ui {
	class UIManager;
}
namespace engine::scene {
	class SceneManager;

	/**
	 * @class Scene
	 * @brief 场景基类，负责管理属于该场景的所有游戏对象及其生命周期。
	 * 
	 * 开发者应通过继承此类来创建具体的游戏关卡（如 MenuScene, Level 等）。
	 */
	class Scene {
protected:
	std::string scene_name_;                        ///< 场景的唯一标识名称
	engine::core::Context& context_;               ///< 指向引擎全局上下文的引用
	engine::scene::SceneManager& scene_manager_;    ///< 对管理该场景的场景管理器的引用

	bool is_initialized_ = false;                   ///< 标记场景是否已完成初始化
	std::vector<std::unique_ptr<engine::object::GameObject>> game_objects_;         ///< 当前活动的游戏对象容器
	std::vector <std::unique_ptr<engine::object::GameObject>> pending_game_objects_; ///< 待加入的游戏对象缓存列表
	std::unique_ptr<engine::ui::UIManager> ui_manager_; ///< UI管理器，负责管理场景中的所有UI元素
	public:
		/**
		 * @brief 构造函数。
		 * @param scene_name 场景名称。
		 * @param context 引擎上下文。
		 * @param scene_manager 场景管理器引用。
		 */
		Scene(const std::string& scene_name, engine::core::Context& context, engine::scene::SceneManager& scene_manager);
		virtual ~Scene();

		// 禁止拷贝和移动构造
		Scene(const Scene&) = delete;
		Scene& operator=(const Scene&) = delete;
		Scene(Scene&&) = delete;
		Scene& operator=(Scene&&) = delete;

		/** @brief 初始化场景内容。在进入场景时由 SceneManager 调用。 */
		virtual void init();
		/** 
		 * @brief 更新场景逻辑。每帧由主循环调用。
		 * @param delta_time 自上一帧的时间间隔（秒）。
		 */
		virtual void update(float delta_time);
		/** @brief 渲染场景内容。负责触发所有游戏对象的渲染。 */
		virtual void render();
		/** @brief 处理场景特定的输入。 @return 是否处理了输入事件。 */
		virtual bool handleInput();
		/** @brief 清理场景资源。在退出或销毁场景前调用。 */
		virtual void clean();

		/** @brief 立即添加一个游戏对象到场景容器中。 */
		virtual void addGameObject(std::unique_ptr<engine::object::GameObject>&& game_object);
		/** @brief 安全地添加游戏对象，存入待处理队列在下一帧加入。 */
		virtual void safeAddGameObject(std::unique_ptr<engine::object::GameObject>&& game_object);
		/** @brief 从场景中立即移除指定的对象实例。 */
		virtual void removeGameObject(engine::object::GameObject* game_object);
		/** @brief 安全地标记移除游戏对象（通过 setNeedRemove）。 */
		virtual void safeRemoveGameObject(engine::object::GameObject* game_object);

		// TODO: getGameObjects 存在逻辑缺陷，暂时保留
		const std::vector<std::unique_ptr<engine::object::GameObject>>& getGameObjects() const { return game_objects_; }

		/** @brief 获取当前场景中指定名称的对象。 @return 找到的对象指针，否则返回 nullptr。 */
		engine::object::GameObject* findGameObjectByName(const std::string& name) const;

		/** @brief 获取UI管理器。 @return UI管理器指针。 */
		engine::ui::UIManager* getUIManager() { return ui_manager_.get(); }

		//getters and setter
		const std::string& getSceneName() const { return scene_name_; }
		bool isInitialized() const { return is_initialized_; }
		engine::core::Context& getContext() const { return context_; }
		engine::scene::SceneManager& getSceneManager() const {
			return scene_manager_;
		}

		void setSceneName(const std::string& scene_name) { scene_name_ = scene_name; }
		void setInitialized(bool initialized) {
			is_initialized_= initialized;
		}
	private:
		/** @brief 处理积压的游戏对象添加请求，确保容器操作的安全性。 */
		void processPendingGameObjects();


	};
}