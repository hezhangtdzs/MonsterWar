#pragma once
#include <string>
#include <vector>
#include <memory>
#include <entt/entity/registry.hpp>
namespace engine::core {
	class Context;
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
	entt::registry registry_;                      ///< 实体组件系统的注册表

	bool is_initialized_ = false;                   ///< 标记场景是否已完成初始化
	std::unique_ptr<engine::ui::UIManager> ui_manager_; ///< UI管理器，负责管理场景中的所有UI元素
	public:
		/**
		 * @brief 构造函数。
		 * @param scene_name 场景名称。
		 * @param context 引擎上下文。
		 * @param scene_manager 场景管理器引用。
		 */
		Scene(const std::string_view scene_name, engine::core::Context& context);
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

		/** @brief 获取UI管理器。 @return UI管理器指针。 */
		engine::ui::UIManager* getUIManager() { return ui_manager_.get(); }

		//getters and setter
		const std::string& getSceneName() const { return scene_name_; }
		bool isInitialized() const { return is_initialized_; }
		engine::core::Context& getContext() const { return context_; }
		entt::registry& getRegistry() { return registry_; }

		void setSceneName(const std::string& scene_name) { scene_name_ = scene_name; }
		void setInitialized(bool initialized) {
			is_initialized_= initialized;
		}
		/// @brief 请求弹出当前场景。
		void requestPopScene();

		/// @brief 请求压入一个新场景。
		void requestPushScene(std::unique_ptr<engine::scene::Scene>&& scene);
		
		/// @brief 请求替换当前场景。
		void requestReplaceScene(std::unique_ptr<engine::scene::Scene>&& scene);

		/// @brief 退出游戏。
		void quit();

	};
}