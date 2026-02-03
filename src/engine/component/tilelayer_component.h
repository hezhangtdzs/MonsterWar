#pragma once
#include "component.h"
#include <vector>
#include <glm/vec2.hpp>
#include "../render/sprite.h"

namespace engine::physics {
	class PhysicsEngine;
}

namespace engine::object {
	class GameObject;
}
namespace engine::render {
	class Sprite;
}
namespace engine::component {
	/**
	 * @enum TileType
	 * @brief 定义瓦片的物理或逻辑类型。
	 */
	enum class TileType {
		EMPTY,     ///< 空瓦片，不进行渲染和碰撞检测
		NORMAL,    ///< 普通瓦片，只渲染，无特殊逻辑
		SOLID,     ///< 实体瓦片，用于碰撞检测
		UNISOLID,  ///< 非实体瓦片，仅供装饰
		SLOPE_0_1, ///< 1x1 上升斜坡 (左低右高, 0->1)
		SLOPE_1_0, ///< 1x1 下降斜坡 (左高右低, 1->0)
		SLOPE_0_2, ///< 2x1 上升斜坡的前半段 (左0右0.5)
		SLOPE_2_1, ///< 2x1 上升斜坡的后半段 (左0.5右1)
		SLOPE_1_2, ///< 2x1 下降斜坡的前半段 (左1右0.5)
		SLOPE_2_0, ///< 2x1 下降斜坡的后半段 (左0.5右0)
		HAZARD,    ///< 危险区域，玩家接触后会受到伤害
		LADDER,    ///< 梯子，玩家可以攀爬
		LEVEL_EXIT,///< 关卡出口，玩家接触后进入下一关
	};


	/**
	 * @struct TileInfo
	 * @brief 单个瓦片的信息结构体。
	 */
	struct TileInfo {
		engine::render::Sprite sprite; ///< 瓦片的渲染精灵
		TileType type{ TileType::NORMAL }; ///< 瓦片类型

		/**
		 * @brief 构造函数
		 * @param spr 精灵对象
		 * @param t 瓦片类型
		 */
		TileInfo() = default;
		TileInfo(engine::render::Sprite spr, TileType t = TileType::NORMAL)
			: sprite(std::move(spr)), type(t) {}
	};

	/**
	 * @class TileLayerComponent
	 * @brief 瓦片图层组件，用于管理和渲染由大量瓦片组成的地图层。
	 * 
	 * 该组件基于网格系统存储瓦片（TileInfo），支持视锥体剔除（Culling）以优化渲染性能。
	 * 适用于 Tiled 地图中的 Tile Layer 层。
	 */
	class TileLayerComponent final : public Component {
		friend class engine::object::GameObject;
	private:

		engine::physics::PhysicsEngine* physics_engine_{ nullptr }; ///< 指向物理引擎的指针，用于注册碰撞图层
		glm::ivec2 tile_size_;          ///< 单个瓦片的像素尺寸 (width, height)
		glm::ivec2 map_size_;           ///< 地图的网格尺寸 (columns, rows)
		std::vector<TileInfo> tiles_;   ///< 拍平的一维瓦片数组，行优先存储
		glm::vec2 offset_{ 0.0f, 0.0f };///< 图层相对于世界原点的偏移量

		bool is_hidden_{ false };       ///< 是否隐藏该图层
	public:
		/**
		 * @brief 构造函数
		 * 
		 * @param tile_size 单个瓦片的尺寸（如 16x16）
		 * @param map_size 地图的网格尺寸（如 100x20）
		 * @param tiles 包含所有瓦片数据的向量，大小必须等于 map_size.x * map_size.y
		 */
		TileLayerComponent(const glm::ivec2& tile_size, const glm::ivec2& map_size,
			const std::vector<TileInfo>& tiles);
		~TileLayerComponent();

		// --- Getters and Setters ---
		
		/** @brief 获取单个瓦片的尺寸 */
		const glm::ivec2& getTileSize() const { return tile_size_; }
		
		/** @brief 获取地图的网格大小 (cols, rows) */
		const glm::ivec2& getMapSize() const { return map_size_; }
		
		/** @brief 获取所有瓦片数据的只读引用 */
		const std::vector<TileInfo>& getTiles() const { return tiles_; }
		
		/** @brief 获取图层偏移量 */
		const glm::vec2& getOffset() const { return offset_; }
		
		/** @brief 获取地图在世界空间中的总像素尺寸 */
		glm::vec2 getWorldSize() const {
			return glm::vec2(map_size_.x * tile_size_.x, map_size_.y * tile_size_.y);
		}
		
		/** @brief 设置图层偏移量 */
		void setOffset(const glm::vec2& offset) { offset_ = offset; }
		
		/** @brief 检查图层是否隐藏 */
		bool isHidden() const { return is_hidden_; }
		
		/** @brief 设置图层显隐状态 */
		void setHidden(bool hidden) { is_hidden_ = hidden; }

		/**
		 * @brief 根据网格坐标获取瓦片信息
		 * @param tile_coords 瓦片的网格坐标 (x, y)
		 * @return 瓦片信息的引用。如果坐标越界，返回默认的空瓦片。
		 */
		const TileInfo& getTileAt(const glm::ivec2& tile_coords) const;

		/**
		 * @brief 获取指定网格坐标的瓦片类型
		 * @param tile_coords 瓦片的网格坐标
		 * @return 瓦片类型
		 */
		TileType getTileTypeAt(const glm::ivec2& tile_coords) const;

		/**
		 * @brief 获取世界坐标处的瓦片类型
		 * 
		 * 将世界坐标转换为网格坐标后查询类型。常用于物理碰撞检测。
		 * @param world_pos 世界空间坐标
		 * @return 该位置的瓦片类型
		 */
		TileType getTileTypeAtWorldPos(const glm::vec2& world_pos) const;

		void setPhysicsEngine(engine::physics::PhysicsEngine* physics_engine) {
			physics_engine_ = physics_engine;
		}
	protected:
		/** @brief 初始化组件 */
		void init() override;
		
		/** 
		 * @brief 渲染图层
		 * 
		 * 仅渲染主要摄像机视野范围内的瓦片（Culling）。
		 * @param context 引擎上下文
		 */
		void render(engine::core::Context& context) override;
		
		/** @brief 更新图层逻辑 (通常为空) */
		void update(float deltaTime, engine::core::Context& context) override;

		void clean() override;

	};
}  // namespace engine::component