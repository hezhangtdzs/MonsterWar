#include "ai_component.h"
#include <spdlog/spdlog.h>
#include "../object/game_object.h"

namespace engine::component {

/**
 * @brief AIComponent 构造函数
 * 
 * @param behavior AI行为实例，使用智能指针管理
 * @details 创建AIComponent并设置初始行为
 */
AIComponent::AIComponent(std::unique_ptr<AIBehavior> behavior)
    : behavior_(std::move(behavior))
{
    if (!behavior_) {
        spdlog::error("AIComponent 创建失败：传入的 AIBehavior 指针为空");
    } else {
        spdlog::trace("AIComponent 创建完成");
    }
}

/**
 * @brief 设置AI行为
 * 
 * @param behavior 新的AI行为实例
 * @details 清理旧行为并设置新行为，如果组件已初始化则同时初始化新行为
 */
void AIComponent::setBehavior(std::unique_ptr<AIBehavior> behavior)
{
    if (behavior_) {
        behavior_->clean();
    }
    behavior_ = std::move(behavior);
    if (behavior_ && owner_) {
        behavior_->init(owner_);
    }
    spdlog::trace("AIComponent 行为已更新");
}

/**
 * @brief 获取当前AI行为
 * 
 * @return AIBehavior* 当前行为的指针
 * @details 返回当前设置的AI行为实例指针
 */
AIBehavior* AIComponent::getBehavior() const
{
    return behavior_.get();
}

/**
 * @brief 组件初始化
 * 
 * @details 初始化AI行为，传入所有者游戏对象
 */
void AIComponent::init()
{
    if (owner_ && behavior_) {
        behavior_->init(owner_);
        spdlog::trace("AIComponent 初始化完成");
    } else {
        spdlog::error("AIComponent 初始化失败：缺少所有者或行为");
    }
}

/**
 * @brief 更新组件逻辑
 * 
 * @param deltaTime 自上一帧以来的时间间隔（秒）
 * @param context 游戏核心上下文
 * @details 调用当前AI行为的更新方法
 */
void AIComponent::update(float deltaTime, engine::core::Context& context)
{
    if (owner_ && behavior_) {
        behavior_->update(owner_, deltaTime, context);
    }
}

/**
 * @brief 组件清理
 * 
 * @details 清理当前AI行为的资源
 */
void AIComponent::clean()
{
    if (behavior_) {
        behavior_->clean();
    }
    spdlog::trace("AIComponent 清理完成");
}

}  // namespace engine::component
