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
	 * @brief 瓦片类型
	 * 
	 * 定义瓦片的物理或逻辑行为。
	 */
	enum class TileType {
		EMPTY,     ///< 空瓦片：不进行渲染和碰撞检测
		NORMAL,    ///< 普通瓦片：仅渲染，无特殊逻辑
		SOLID,     ///< 实体瓦片：用于物理碰撞检测
		UNISOLID,  ///< 单向实体瓦片
		SLOPE_0_1, ///< 斜坡类型 0_1
		SLOPE_1_0, ///< 斜坡类型 1_0
		SLOPE_0_2, ///< 斜坡类型 0_2
		SLOPE_2_1, ///< 斜坡类型 2_1
		SLOPE_1_2, ///< 斜坡类型 1_2
		SLOPE_2_0, ///< 斜坡类型 2_0
		HAZARD,    ///< 危险瓦片：接触后会对实体造成伤害
		LADDER     ///< 梯子瓦片
	};


	/**
	 * @struct TileInfo
	 * @brief 单个瓦片的详细数据
	 */
	struct TileInfo {
		engine::component::Sprite sprite_;                      ///< 瓦片的精灵渲染数据
		engine::component::TileType type_;                      ///< 瓦片的逻辑类型
		std::optional<engine::component::Animation> animation_; ///< 瓦片的动画数据（支持 Tiled 动画图块）
		std::optional<nlohmann::json> properties_;              ///< 瓦片的自定义属性（从 Tiled 导出）

		TileInfo() = default;

		/**
		 * @brief 构造函数
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
	 * @brief 瓦片图层组件
	 * 
	 * 管理和维护由大量瓦片组成的网格地图层。支持 Tiled 中的 Tile Layer。
	 */
	struct TileLayerComponent {
		glm::ivec2 tile_size_;              ///< 单个瓦片的尺寸（像素）
		glm::ivec2 map_size_;               ///< 图层的网格大小（行数和列数）
		std::vector<entt::entity> tiles_;   ///< 瓦片实体列表，按 row * width + col 顺序排列

		/**
		 * @brief 构造函数
		 * @param tile_size 瓦片尺寸
		 * @param map_size 图层网格大小
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