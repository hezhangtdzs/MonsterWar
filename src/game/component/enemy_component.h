/**
 * @file enemy_component.h
 * @brief 定义 EnemyComponent 组件，用于ECS架构中的敌人实体。
 * 
 * @details
 * EnemyComponent 是游戏特定的组件，用于存储敌人实体的路径导航和移动数据。
 * 该组件与 FollowPathSystem 配合使用，实现敌人沿着预设路径移动的功能。
 * 
 * @par ECS架构角色
 * - 存储：敌人的目标路径点ID和移动速度
 * - 输入：FollowPathSystem 读取组件数据
 * - 更新：FollowPathSystem 根据路径更新目标路径点
 * 
 * @see game::system::FollowPathSystem 处理敌人路径跟随逻辑的系统
 * @see game::data::WaypointNode 定义路径点数据的结构
 */

#pragma once

namespace game::component {
    /**
     * @struct EnemyComponent
     * @brief 敌人组件，存储敌人的路径导航和移动数据。
     * 
     * @details
     * EnemyComponent 存储了敌人实体在路径导航时所需的数据：
     * - target_waypoint_id_：当前目标路径点的ID
     * - speed_：敌人的移动速度（像素/秒）
     * 
     * 该组件与 FollowPathSystem 配合使用，实现敌人沿着预设路径移动的功能。
     */
    struct EnemyComponent {
        int target_waypoint_id_; ///< 当前目标路径点的ID，用于路径导航
        float speed_; ///< 敌人的移动速度（像素/秒）
    };
}
