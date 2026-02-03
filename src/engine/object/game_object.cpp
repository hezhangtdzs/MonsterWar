/**
 * @file game_object.cpp
 * @brief GameObject 类的实现，负责游戏对象的管理和组件系统的操作。
 */

#include "game_object.h"
#include "../component/component.h"
#include "../render/renderer.h"
#include "../input/input_manager.h" 
#include "../render/camera.h"
#include <spdlog/spdlog.h>

namespace engine::object {
    /**
     * @brief 构造函数，创建一个新的游戏对象。
     * @param name 对象的名称
     * @param tag 对象的标签，用于分类和查询
     */
    GameObject::GameObject(const std::string& name, const std::string& tag) : name_(name), tag_(tag)
    {
        spdlog::trace("GameObject created: {} {}", name_, tag_);
    }

    /**
     * @brief 析构函数，清理对象资源。
     */
    GameObject::~GameObject() {
        clean();
    }

    /**
     * @brief 更新循环：处理所有组件的每帧逻辑。
     * @param delta_time 自上一帧以来的经过时间（秒）
     * @param context 游戏核心上下文
     */
    void GameObject::update(float delta_time, engine::core::Context& context) {
        // 遍历所有组件并调用它们的 update 方法
        for (auto& pair : components_) {
            pair.second->update(delta_time,context);
        }
    }

    /**
     * @brief 更新循环：渲染所有相关的组件内容。
     * @param context 游戏核心上下文
     */
    void GameObject::render(engine::core::Context& context) {
        // 遍历所有组件并调用它们的 render 方法
        for (auto& pair : components_) {
            pair.second->render(context);
        }
    }

    /**
     * @brief 清理函数，在对象销毁或准备重置时调用。
     */
    void GameObject::clean() {
        spdlog::trace("Cleaning GameObject: {}", name_);
        // 遍历所有组件并调用它们的 clean 方法
        for (auto& pair : components_) {
            pair.second->clean();
        }
        components_.clear(); // 清空 map, unique_ptr 会自动释放内存
    }

    /**
     * @brief 更新循环：处理所有组件的输入逻辑。
     * @param context 游戏核心上下文
     */
    void GameObject::handleInput(engine::core::Context& context) {
        // 遍历所有组件并调用它们的 handleInput 方法
        for (auto& pair : components_) {
            pair.second->handleInput(context);
        }
    }

} // namespace engine::object