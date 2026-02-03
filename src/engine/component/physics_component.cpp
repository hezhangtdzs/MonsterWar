#include "physics_component.h"
#include <spdlog/spdlog.h>
#include "../object/game_object.h"
#include "transform_component.h"
#include "../physics/physics_engine.h"

/**
 * @brief 构造函数，创建一个新的物理组件。
 * @param physics_engine 指向物理引擎的指针
 * @param use_gravity 是否受重力影响，默认为true
 * @param mass 物体质量，默认为1.0
 */
engine::component::PhysicsComponent::PhysicsComponent(engine::physics::PhysicsEngine* physics_engine, bool use_gravity, float mass)
	: physics_engine_(physics_engine), use_gravity_(use_gravity), mass_(mass > 0.0f ? mass : 1.0f)
{
	if (!physics_engine_) {
		spdlog::error("PhysicsEngine is null");
	}
	spdlog::trace("物理组件创建完成，质量: {}, 使用重力: {}", mass_, use_gravity_);
}

/**
 * @brief 析构函数。
 */
engine::component::PhysicsComponent::~PhysicsComponent() = default;

/**
 * @brief 组件初始化，获取变换组件并注册到物理引擎。
 * 
 * 初始化过程包括：
 * 1. 检查所属游戏对象是否存在
 * 2. 获取所属游戏对象的 TransformComponent
 * 3. 注册到物理引擎
 */
void engine::component::PhysicsComponent::init()
{
	if (!owner_) {
		spdlog::error("PhysicsComponent 初始化失败：所属对象为空");
		return;
	}
	transform_component_ = owner_->getComponent<TransformComponent>();
	if (!transform_component_) {
		spdlog::error("PhysicsComponent 初始化失败：所属对象缺少 TransformComponent 组件");
		return;
	}
	if (physics_engine_) {
		physics_engine_->registerPhysicsComponent(this);
		spdlog::trace("PhysicsComponent 初始化完成并注册到 PhysicsEngine");
	} else {
		spdlog::error("PhysicsComponent 初始化失败：PhysicsEngine 为空");
	}
}

/**
 * @brief 更新组件，处理物理模拟和位置更新。
 * @param deltaTime 时间增量（秒）
 * @param context 引擎上下文
 * 
 * 注意：物理更新由 PhysicsEngine 统一处理，此方法为空实现。
 */
void engine::component::PhysicsComponent::update(float /*deltaTime*/, engine::core::Context& /*context*/)
{
	// 物理更新由 PhysicsEngine 统一处理
}

/**
 * @brief 组件清理，从物理引擎中注销。
 * 
 * 清理过程包括：
 * 1. 从物理引擎中注销组件
 */
void engine::component::PhysicsComponent::clean()
{
	if (physics_engine_) {
		physics_engine_->unregisterPhysicsComponent(this);
		spdlog::trace("PhysicsComponent 已从 PhysicsEngine 注销");
	}
}
