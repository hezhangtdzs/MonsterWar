/**
 * @file tilelayer_component.h
 * @brief 定义瓦片地图相关的组件和枚举，用于ECS架构中的瓦片图层管理。
 *
 * @details
 * 该文件定义了瓦片地图系统的核心组件：
 * - TileType: 瓦片类型枚举，定义瓦片的物理和逻辑行为
 * - TileInfo: 单个瓦片的详细数据，包含精灵、类型、动画和属性
 * - TileLayerComponent: 瓦片图层组件，管理整个瓦片层
 *
 * 这些组件配合 Tiled 地图编辑器使用，支持从 Tiled 导出的地图数据加载。
 *
 * @par 使用示例
 * @code
 * // 创建瓦片信息
 * TileInfo tile_info(sprite, TileType::SOLID);
 *
 * // 创建瓦片图层组件
 * std::vector<entt::entity> tiles = {...}; // 瓦片实体列表
 * TileLayerComponent layer({32, 32}, {20, 15}, std::move(tiles));
 * @endcode
 *
 * @see engine::loader::LevelLoader 加载瓦片地图的加载器
 */

#pragma once
#include "sprite_component.h"
#include "animation_component.h"
#include <vector>
#include <optional>
#include <glm/vec2.hpp>
#include <nlohmann/json.hpp>
#include <entt/entity/entity.hpp>


namespace engine::component {
	/**
	 * @enum TileType
	 * @brief 瓦片类型枚举，定义瓦片的物理或逻辑行为。
	 *
	 * @details
	 * 不同的瓦片类型决定了瓦片的渲染和碰撞行为：
	 * - EMPTY: 空瓦片，不进行任何处理
	 * - NORMAL: 普通瓦片，仅渲染
	 * - SOLID: 实体瓦片，参与物理碰撞
	 * - SLOPE_*: 斜坡瓦片，用于实现斜坡地形
	 * - HAZARD: 危险瓦片，接触造成伤害
	 * - LADDER: 梯子瓦片，可攀爬
	 */
	enum class TileType {
		EMPTY,     ///< 空瓦片：不进行渲染和碰撞检测
		NORMAL,    ///< 普通瓦片：仅渲染，无特殊逻辑
		SOLID,     ///< 实体瓦片：用于物理碰撞检测
		UNISOLID,  ///< 单向实体瓦片：仅从一个方向阻挡
		SLOPE_0_1, ///< 斜坡类型 0_1：左低右高的斜坡
		SLOPE_1_0, ///< 斜坡类型 1_0：左高右低的斜坡
		SLOPE_0_2, ///< 斜坡类型 0_2
		SLOPE_2_1, ///< 斜坡类型 2_1
		SLOPE_1_2, ///< 斜坡类型 1_2
		SLOPE_2_0, ///< 斜坡类型 2_0
		HAZARD,    ///< 危险瓦片：接触后会对实体造成伤害
		LADDER     ///< 梯子瓦片：可以攀爬
	};


	/**
	 * @struct TileInfo
	 * @brief 单个瓦片的详细数据结构。
	 *
	 * @details
	 * TileInfo 包含瓦片的所有数据：
	 * - sprite_: 渲染用的精灵数据
	 * - type_: 瓦片类型，决定物理和逻辑行为
	 * - animation_: 可选的动画数据，支持动画瓦片
	 * - properties_: 自定义属性，从 Tiled 导出
	 */
	struct TileInfo {
		engine::component::Sprite sprite_;                      ///< 瓦片的精灵渲染数据
		engine::component::TileType type_;                      ///< 瓦片的逻辑类型
		std::optional<engine::component::Animation> animation_; ///< 瓦片的动画数据（支持 Tiled 动画图块）
		std::optional<nlohmann::json> properties_;              ///< 瓦片的自定义属性（从 Tiled 导出）

		TileInfo() = default;

		/**
		 * @brief 构造函数
		 * @param sprite 瓦片的精灵数据
		 * @param type 瓦片类型
		 * @param animation 可选的动画数据
		 * @param properties 可选的自定义属性
		 */
		TileInfo(engine::component::Sprite sprite, 
				engine::component::TileType type, 
				std::optional<engine::component::Animation> animation = std::nullopt, 
				std::optional<nlohmann::json> properties = std::nullopt) : 
				sprite_(std::move(sprite)), 
				type_(type), 
				animation_(std::move(animation)), 
				properties_(std::move(properties)) {}
	};

	/**
	 * @struct TileLayerComponent
	 * @brief 瓦片图层组件，管理整个瓦片地图层。
	 *
	 * @details
	 * TileLayerComponent 管理一个完整的瓦片图层，包含：
	 * - 瓦片尺寸：每个瓦片的大小（像素）
	 * - 地图尺寸：网格的行列数
	 * - 瓦片实体列表：按 row * width + col 顺序存储
	 *
	 * 该组件通常由 LevelLoader 创建，用于表示 Tiled 地图中的一个图层。
	 */
	struct TileLayerComponent {
		glm::ivec2 tile_size_;              ///< 单个瓦片的尺寸（像素）
		glm::ivec2 map_size_;               ///< 图层的网格大小（列数, 行数）
		std::vector<entt::entity> tiles_;   ///< 瓦片实体列表，按 row * width + col 顺序排列

		/**
		 * @brief 构造函数
		 * @param tile_size 瓦片尺寸（宽, 高）
		 * @param map_size 图层网格大小（列数, 行数）
		 * @param tiles 包含所有瓦片实体的列表
		 */
		TileLayerComponent(glm::ivec2 tile_size, 
						glm::ivec2 map_size, 
						std::vector<entt::entity> tiles) : 
						tile_size_(std::move(tile_size)), 
						map_size_(std::move(map_size)),
						tiles_(std::move(tiles)) {}
	};
}  // namespace engine::component