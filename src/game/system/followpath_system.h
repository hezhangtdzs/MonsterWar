/**
 * @file followpath_system.h
 * @brief 定义 FollowPathSystem 类，负责处理敌人的路径跟随逻辑。
 * 
 * @details
 * FollowPathSystem 是游戏特定的系统，负责处理敌人实体沿着预设路径移动的逻辑。
 * 该系统与 EnemyComponent 配合使用，实现敌人的自动导航功能。
 * 
 * @par ECS架构角色
 * - 查询：获取同时具有 EnemyComponent、TransformComponent 和 VelocityComponent 的实体
 * - 处理：计算敌人到目标路径点的方向和距离
 * - 修改：更新 VelocityComponent 的速度向量
 * - 发布：当敌人到达最终路径点时发布 EnemyArriveHomeEvent
 * 
 * @par 路径跟随原理
 * 1. 读取敌人的目标路径点ID
 * 2. 从路径点地图中获取目标路径点的位置
 * 3. 计算敌人当前位置到目标路径点的方向向量
 * 4. 归一化方向向量并乘以敌人速度，得到速度向量
 * 5. 更新敌人的速度组件
 * 6. 检查敌人是否到达目标路径点
 * 7. 如果到达，更新目标路径点为下一个路径点
 * 8. 如果到达最终路径点，发布 EnemyArriveHomeEvent
 * 
 * @see game::component::EnemyComponent 存储敌人路径导航数据的组件
 * @see game::data::WaypointNode 定义路径点数据的结构
 * @see game::defs::EnemyArriveHomeEvent 敌人到达基地的事件
 */

#pragma once
#include <entt/entt.hpp>
#include <unordered_map>
#include "game/data/waypoint_node.h"

namespace game::system {
    /**
     * @class FollowPathSystem
     * @brief 路径跟随系统，负责处理敌人沿着预设路径移动的逻辑。
     * 
     * @details
     * FollowPathSystem 实现了敌人的自动导航功能，使敌人能够：
     * - 沿着预设的路径点移动
     * - 自动更新目标路径点
     * - 到达最终路径点时触发事件
     * 
     * 该系统使用了简单的线性路径跟随算法，适合塔防游戏等需要敌人沿固定路径移动的场景。
     */
    class FollowPathSystem {
    public:
        /**
         * @brief 更新所有敌人的路径跟随状态
         * @param registry EnTT 实体注册表，包含所有实体和组件
         * @param dispatcher EnTT 事件分发器，用于发布 EnemyArriveHomeEvent
         * @param waypoint_nodes 路径点ID到路径点数据的映射
         * 
         * @details
         * 该方法实现了以下功能：
         * 1. 查询所有具有 EnemyComponent、TransformComponent 和 VelocityComponent 的实体
         * 2. 对每个敌人实体：
         *    a. 获取目标路径点数据
         *    b. 计算移动方向和速度
         *    c. 更新速度组件
         *    d. 检查是否到达路径点
         *    e. 更新目标路径点或发布到达事件
         * 
         * @par 注意事项
         * - 该系统假设路径点网络是有效的，即每个路径点都有正确的下一个路径点
         * - 当敌人到达没有下一个路径点的路径点时，认为其到达了最终目标
         */
        void update(entt::registry& registry, entt::dispatcher& dispatcher, 
        std::unordered_map<int, game::data::WaypointNode>& waypoint_nodes);
    };
}