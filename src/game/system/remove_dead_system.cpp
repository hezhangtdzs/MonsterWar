/**
 * @file remove_dead_system.cpp
 * @brief RemoveDeadSystem 类的实现，负责清理标记为死亡的实体。
 * 
 * @details
 * 本文件实现了 RemoveDeadSystem::update() 方法，处理带有 DeadTag 的实体的删除逻辑。
 * 该系统实现了实体的延迟删除机制，确保实体在适当的时机被清理。
 * 
 * @see game::system::RemoveDeadSystem 死亡实体清理系统类定义
 * @see game::defs::DeadTag 死亡标签组件
 */

#include "remove_dead_system.h"
#include "game/defs/tags.h"
#include "game/defs/event.h"
#include <spdlog/spdlog.h>

/**
 * @brief 更新系统，清理标记为死亡的实体
 * @param registry EnTT 实体注册表，包含所有实体和组件
 * 
 * @details
 * 该方法实现了死亡实体的清理逻辑：
 * 1. 查询所有带有 DeadTag 的实体
 * 2. 遍历这些实体
 * 3. 从注册表中删除每个实体
 * 4. 记录删除结果
 * 
 * @par 执行流程
 * 1. 创建实体视图：registry.view<game::defs::DeadTag>()
 * 2. 遍历视图中的所有实体
 * 3. 获取实体的ID用于日志记录
 * 4. 调用 registry.destroy(entity) 删除实体
 * 5. 记录删除操作
 * 
 * @par 注意事项
 * - 该方法应该在所有其他系统更新之后调用，确保死亡实体的所有逻辑都已处理完毕
 * - 删除实体时，EnTT 会自动清理实体的所有组件
 * - 该方法是线程安全的，因为它只在主线程中执行
 */
void game::system::RemoveDeadSystem::update(entt::registry& registry) {
    // 查询所有带有 DeadTag 的实体
    auto view = registry.view<defs::DeadTag>();
    
    // 遍历并删除标记为死亡的实体
    for (auto entity : view) {
        // 获取实体的ID用于日志记录
        auto entity_id = static_cast<entt::id_type>(entity);
        
        // 从注册表中删除实体
        registry.destroy(entity);
        
        // 记录删除操作
        spdlog::info("Entity {} destroyed", entity_id);
    }
}
