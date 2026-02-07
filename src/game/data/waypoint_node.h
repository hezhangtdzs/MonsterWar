#pragma once
#include <glm/vec2.hpp>
#include <vector>

namespace game::data {
	/**
	 * @struct WaypointNode
	 * @brief 路径点节点
	 * 
	 * 用于定义路径上的一个点，包含位置信息。
	 */
	struct WaypointNode {
		int id_; ///< 路径点的唯一标识符
        glm::vec2 position_; ///< 路径点的位置
        std::vector<int> next_node_ids_; ///< 连接到其他路径点的标识符
	};
}