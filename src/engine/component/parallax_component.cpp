#include "parallax_component.h"
#include "transform_component.h"
#include <spdlog/spdlog.h>
#include "../object/game_object.h"
#include "../core/context.h"
#include "../render/renderer.h"

namespace engine::component {

/**
 * @brief 构造函数
 * 
 * @param texture_id 纹理资源的路径或ID
 * @param parallax_factor 视差因子 (默认 1.0)
 * @param repeat 是否重复平铺 (默认 true)
 * @details 创建视差背景组件，初始化精灵和视差参数
 */
ParallaxComponent::ParallaxComponent(std::string& texture_id, const glm::vec2& parallax_factor, const glm::bvec2& repeat) :
	sprite_(engine::render::Sprite(texture_id)),
	parallax_factor_(parallax_factor),
	repeat_(repeat)
{
	spdlog::trace("ParallaxComponent 构造完成，纹理ID：{}", texture_id);
}

/**
 * @brief 析构函数
 */
ParallaxComponent::~ParallaxComponent() = default;

/**
 * @brief 组件初始化
 * 
 * @details 获取所属对象的 TransformComponent 组件，用于后续的位置和缩放计算
 */
void ParallaxComponent::init()
{
	if (!owner_) {
		spdlog::error("ParallaxComponent 初始化失败：所属对象为空");
		return;
	}
	transform_ = owner_->getComponent<TransformComponent>();
	if(!transform_){
		spdlog::error("ParallaxComponent 初始化失败：所属对象缺少 TransformComponent 组件");
	}
	else{
		spdlog::trace("ParallaxComponent 初始化完成");
	}
}

/**
 * @brief 更新组件逻辑
 * 
 * @param deltaTime 帧间隔时间
 * @param context 引擎上下文
 * @details 视差背景通常不需要在 update 中做任何事情
 */
void ParallaxComponent::update(float /*deltaTime*/, engine::core::Context& /*context*/)
{
	// 视差背景通常不需要在 update 中做任何事情
}

/**
 * @brief 渲染视差背景
 * 
 * @param context 引擎上下文
 * @details 根据摄像机位置和视差因子计算纹理偏移，绘制视差背景
 */
void ParallaxComponent::render(engine::core::Context& context)
{
	if (is_hidden_ || !transform_) {
		return;
	}
	context.getRenderer().drawParallax(context.getCamera(),
									   sprite_,transform_->getPosition(), 
									   parallax_factor_,repeat_,
									   transform_->getScale());
}

/**
 * @brief 处理输入事件
 * 
 * @param context 引擎上下文
 * @details 视差组件通常不处理输入
 */
void ParallaxComponent::handleInput(engine::core::Context& /*context*/)
{
	// 视差组件通常不处理输入
}

/**
 * @brief 组件清理
 * 
 * @details 视差组件没有需要手动释放的动态资源
 */
void ParallaxComponent::clean()
{
	// 视差组件没有需要手动释放的动态资源
}

}  // namespace engine::component

