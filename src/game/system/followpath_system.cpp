/**
 * @file followpath_system.cpp
 * @brief FollowPathSystem 类的实现，负责处理敌人的路径跟随逻辑。
 * 
 * @details
 * 本文件实现了 FollowPathSystem::update() 方法，处理敌人实体沿着预设路径移动的逻辑。
 * 该系统实现了以下功能：
 * - 敌人自动沿着路径点移动
 * - 到达路径点后自动切换到下一个路径点
 * - 支持路径分支（随机选择下一个路径点）
 * - 到达最终路径点时触发事件
 * - 自动清理到达终点的敌人
 * 
 * @see game::system::FollowPathSystem 路径跟随系统类定义
 * @see game::component::EnemyComponent 存储敌人路径导航数据的组件
 */

#include "followpath_system.h"
#include "game/component/enemy_component.h"
#include "engine/component/transform_component.h"
#include "engine/component/velocity_component.h"
#include "engine/utils/math.h"
#include "game/data/waypoint_node.h"
#include "game/defs/tags.h"
#include "game/defs/event.h"
#include <spdlog/spdlog.h>
#include <glm/glm.hpp>

namespace game::system {

    /**
     * @brief 更新所有敌人的路径跟随状态
     * @param registry EnTT 实体注册表，包含所有实体和组件
     * @param dispatcher EnTT 事件分发器，用于发布 EnemyArriveHomeEvent
     * @param waypoint_nodes 路径点ID到路径点数据的映射
     * 
     * @details
     * 该方法实现了敌人的路径跟随逻辑：
     * 1. 查询所有具有 EnemyComponent、TransformComponent 和 VelocityComponent 的实体
     * 2. 对每个敌人实体：
     *    a. 获取当前目标路径点
     *    b. 计算到目标路径点的方向向量
     *    c. 检查是否到达目标路径点
     *    d. 如果到达，处理路径点切换或终点到达
     *    e. 更新敌人的速度向量
     * 
     * @par 路径点切换逻辑
     * - 当敌人距离目标路径点小于5.0f像素时，认为到达该路径点
     * - 如果路径点有多个下一个路径点，随机选择一个
     * - 如果路径点没有下一个路径点，认为到达终点
     * 
     * @par 终点到达处理
     * - 发布 EnemyArriveHomeEvent 事件
     * - 为敌人实体添加 DeadTag 标记
     * - 敌人将在后续被 RemoveDeadSystem 清理
     */
    void FollowPathSystem::update(
        entt::registry& registry,
        entt::dispatcher& dispatcher,
        std::unordered_map<int, game::data::WaypointNode>& waypoint_nodes)
    {
        // 创建实体视图，获取所有需要路径跟随的敌人实体
        auto view = registry.view<
            game::component::EnemyComponent,
            engine::component::TransformComponent,
            engine::component::VelocityComponent>();

        // 遍历所有敌人实体
        for (auto entity : view) {
            // 获取必要的组件
            auto& enemy = registry.get<game::component::EnemyComponent>(entity);
            auto& transform = registry.get<engine::component::TransformComponent>(entity);
            auto& velocity = registry.get<engine::component::VelocityComponent>(entity);

            // 获取当前目标路径点
            auto target_node = waypoint_nodes.at(enemy.target_waypoint_id_);

            // 计算当前位置到目标位置的方向向量
            glm::vec2 direction = target_node.position_ - transform.position_;

            // 如果距离小于阈值，则切换到下一个节点
            if (glm::length(direction) < 5.0f) {
                // 获取下一个路径点ID列表的大小
                auto size = target_node.next_node_ids_.size();
                
                // 如果下一个节点ID列表为空，代表到达终点
                if (size == 0) {
                    spdlog::info("敌人到达终点");
                    // 发送敌人到达基地事件
                    dispatcher.enqueue<game::defs::EnemyArriveHomeEvent>();
                    // 添加死亡标记，后续会被 RemoveDeadSystem 清理
                    registry.emplace<game::defs::DeadTag>(entity);
                    continue;
                }

                // 随机选择下一个节点
                auto target_index = engine::utils::randomInt(0, size - 1);
                enemy.target_waypoint_id_ = target_node.next_node_ids_[target_index];

                // 更新目标节点与方向矢量
                target_node = waypoint_nodes.at(enemy.target_waypoint_id_);
                direction = target_node.position_ - transform.position_;
            }

            // 更新速度组件：velocity = 方向矢量（归一化） * 速度
            velocity.velocity_ = glm::normalize(direction) * enemy.speed_;
        }
    }

} // namespace game::system
