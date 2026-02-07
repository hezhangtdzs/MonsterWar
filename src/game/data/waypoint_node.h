/**
 * @file waypoint_node.h
 * @brief 定义 WaypointNode 结构体，用于游戏中的路径点数据。
 * 
 * @details
 * WaypointNode 结构体定义了游戏中路径系统的基本单元 - 路径点。
 * 它包含了路径点的唯一标识符、位置信息以及连接到其他路径点的关系。
 * 
 * 路径点系统主要用于：
 * - 敌人AI路径导航
 * - NPC移动路线
 * - 寻路算法的图节点
 * 
 * @see game::component::EnemyComponent 使用路径点系统的敌人组件
 * @see game::system::FollowPathSystem 处理路径跟随逻辑的系统
 */

#pragma once
#include <glm/vec2.hpp>
#include <vector>

namespace game::data {
	/**
	 * @struct WaypointNode
	 * @brief 路径点节点，定义路径系统中的一个点。
	 * 
	 * @details
	 * WaypointNode 结构体是游戏路径系统的基本构建块，用于定义：
	 * - 路径点的唯一标识符
	 * - 路径点在游戏世界中的位置
	 * - 路径点与其他路径点的连接关系
	 * 
	 * 这些数据用于构建路径网络，使敌人或NPC能够沿着预设的路线移动。
	 */
	struct WaypointNode {
		int id_; ///< 路径点的唯一标识符，用于在路径网络中识别
        glm::vec2 position_; ///< 路径点的位置坐标（世界空间）
        std::vector<int> next_node_ids_; ///< 连接到其他路径点的标识符列表，定义了路径的分支
	};
}
