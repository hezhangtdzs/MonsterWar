#pragma once
#include <string>
#include <typeindex>
#include <unordered_map>
#include <memory>
#include <utility>
#include <spdlog/spdlog.h>
#include "../component/component.h" // 必须包含定义以支持模板方法中的 is_base_of 和函数调用

namespace engine::core
{
	class Context;
}
namespace engine::object
{
	/**
	 * @class GameObject
	 * @brief 游戏对象基类，采用组件化架构。
	 */
	class GameObject final {
	private:
		std::string name_; ///< 对象的名称
		std::string tag_;  ///< 对象的标签，用于分类和查询
		/** @brief 组件映射表，按类型索引存储组件 */
		std::unordered_map<std::type_index, std::unique_ptr<engine::component::Component>> components_;

		bool need_remove_ = false; ///< 标记对象是否在下一帧需要被从场景中移除
	public:
		/**
		 * @brief 构造函数。
		 */
		GameObject(const std::string& name = "", const std::string& tag = "");
		
		/**
		 * @brief 析构函数。
		 */
		~GameObject();

		// 禁止拷贝和移动
		GameObject(const GameObject&) = delete;
		GameObject& operator=(const GameObject&) = delete;
		GameObject(GameObject&&) = delete;
		GameObject& operator=(GameObject&&) = delete;

		/** @brief 获取对象名称 */
		const std::string& getName() const {
			return name_;
		}
		/** @brief 设置对象名称 */
		void setName(const std::string& name) {
			name_ = name;
		}
		/** @brief 获取对象标签 */
		const std::string& getTag() const {
			return tag_;
		}
		/** @brief 设置对象标签 */
		void setTag(const std::string& tag) {
			tag_ = tag;
		}
		
		/** 
		 * @brief 标记该对象为待移除。
		 */
		void setNeedRemove(bool need_remove) { need_remove_ = need_remove; }
		
		/** @brief 检查对象是否已被标记为移除 */
		bool getNeedRemove() const { return need_remove_; }

		/**
		 * @brief 为游戏对象动态添加组件。
		 */
		template<typename T, typename... Args>
		T* addComponent(Args&&... args) {
			static_assert(std::is_base_of<engine::component::Component, T>::value, "T必须继承自Component");

			auto type_index = std::type_index(typeid(T));
			if (hasComponent<T>()) {
				return getComponent<T>();
			}
		auto new_component = std::make_unique<T>(std::forward<Args>(args)...);
		T* raw_ptr = new_component.get();
		components_[type_index] = std::unique_ptr<engine::component::Component>(new_component.release());
			// 这里需要 Component 的完整定义
			raw_ptr->setOwner(this);
			raw_ptr->init();
			spdlog::debug("为游戏对象 {} 添加组件 {}", name_, typeid(T).name());
			return raw_ptr;
		}

		/**
		 * @brief 获取对象持有的指定类型组件。
		 */
		template<typename T>
		T* getComponent() const {
			static_assert(std::is_base_of<engine::component::Component, T>::value, "T必须继承自Component");
			auto type_index = std::type_index(typeid(T));
			if (auto it = components_.find(type_index); it != components_.end()) {
				return static_cast<T*>(it->second.get());
			}
			return nullptr;
		}

		/**
		 * @brief 检查对象是否包含特定类型的组件。
		 */
		template<typename T>
		bool hasComponent() const {
			static_assert(std::is_base_of<engine::component::Component, T>::value, "T必须继承自Component");
			return components_.contains(std::type_index(typeid(T)));
		}

		/**
		 * @brief 移除指定类型的组件。
		 */
		template<typename T>
		void removeComponent() {
			static_assert(std::is_base_of<engine::component::Component, T>::value, "T必须继承自Component");
			auto type_index = std::type_index(typeid(T));
			if (auto it = components_.find(type_index); it != components_.end()) {
				it->second->clean();
				components_.erase(it);
			}
		}

	public:
		/** @brief 更新循环：处理所有组件的输入逻辑 */
		void handleInput(engine::core::Context& context);
		/** 
		 * @brief 更新循环：处理所有组件的每帧逻辑。
		 * @param delta_time 自上一帧以来的经过时间（秒）。
		 */
		void update(float delta_time, engine::core::Context& context);
		/** @brief 更新循环：渲染所有相关的组件内容 */
		void render(engine::core::Context& context);
		/** @brief 清理函数，在对象销毁或准备重置时调用 */
		void clean();
	};
}