#include "tilelayer_component.h"
#include "../object/game_object.h"
#include "transform_component.h"
#include "../core/context.h"
#include "../render/renderer.h"
#include "../physics/physics_engine.h"
#include "../render/camera.h"
#include <spdlog/spdlog.h>
#include <glm/ext/vector_int2.hpp> // 修复VCIC001警告

namespace engine::component {

using engine::component::TileType; // 添加此行以修复未定义标识符"TileType"错误

/**
 * @brief 构造函数
 * 
 * @param tile_size 单个瓦片的尺寸（如 16x16）
 * @param map_size 地图的网格尺寸（如 100x20）
 * @param tiles 包含所有瓦片数据的向量，大小必须等于 map_size.x * map_size.y
 * @details 创建瓦片图层组件，初始化瓦片数据和尺寸信息
 */
TileLayerComponent::TileLayerComponent(const glm::ivec2& tile_size, const glm::ivec2& map_size, const std::vector<TileInfo>& tiles)
:tile_size_(tile_size), map_size_(map_size), tiles_(tiles)
{

	if (tiles_.size() != static_cast<size_t>(map_size_.x * map_size_.y)) {
		spdlog::error("TileLayerComponent: 地图尺寸与提供的瓦片向量大小不匹配。瓦片数据将被清除。");
		tiles_.clear();
		map_size_ = { 0, 0 };
	}
	spdlog::trace("TileLayerComponent 构造完成");
}

/**
 * @brief 析构函数
 */
TileLayerComponent::~TileLayerComponent() = default;

/**
 * @brief 根据网格坐标获取瓦片信息
 * 
 * @param tile_coords 瓦片的网格坐标 (x, y)
 * @return 瓦片信息的引用。如果坐标越界，返回默认的空瓦片。
 * @details 检查坐标合法性并返回对应的瓦片信息
 */
const TileInfo& TileLayerComponent::getTileAt(const glm::ivec2& tile_coords) const
{
	// 检查坐标合法性
	if (tile_coords.x < 0 || tile_coords.y < 0 ||
		tile_coords.x >= map_size_.x || tile_coords.y >= map_size_.y) {
		static const TileInfo empty_tile(engine::render::Sprite(), TileType::EMPTY);
		return empty_tile;
	}
	size_t index = static_cast<size_t>(tile_coords.y) * static_cast<size_t>(map_size_.x) + static_cast<size_t>(tile_coords.x);
	if (index >= tiles_.size()) {
		static const TileInfo empty_tile(engine::render::Sprite(), TileType::EMPTY);
		return empty_tile;
	}
	return tiles_[index];
}

/**
 * @brief 获取指定网格坐标的瓦片类型
 * 
 * @param tile_coords 瓦片的网格坐标
 * @return 瓦片类型
 * @details 调用 getTileAt 获取瓦片信息并返回其类型
 */
TileType TileLayerComponent::getTileTypeAt(const glm::ivec2& tile_coords) const
{
	// 返回指定瓦片坐标的瓦片类型
	return getTileAt(tile_coords).type;
}

/**
 * @brief 获取世界坐标处的瓦片类型
 * 
 * @param world_pos 世界空间坐标
 * @return 该位置的瓦片类型
 * @details 将世界坐标转换为网格坐标后查询类型。常用于物理碰撞检测。
 */
TileType TileLayerComponent::getTileTypeAtWorldPos(const glm::vec2& world_pos) const
{
	// 计算世界坐标对应的瓦片坐标
	glm::vec2 layer_world_offset = offset_;
	if (owner_) {
		if (auto* tc = owner_->getComponent<engine::component::TransformComponent>()) {
			layer_world_offset += tc->getPosition();
		}
	}
	glm::ivec2 tile_coords = glm::floor((world_pos - layer_world_offset) / glm::vec2(tile_size_));
	return getTileTypeAt(tile_coords);
}

/**
 * @brief 初始化组件
 * 
 * @details 初始化瓦片图层组件，检查所有者设置
 */
void TileLayerComponent::init()
{
	if (!owner_) {
		spdlog::warn("TileLayerComponent 的 owner_ 未设置。");
	}
	spdlog::trace("TileLayerComponent 初始化完成");
}

/**
 * @brief 渲染图层
 * 
 * @param context 引擎上下文
 * @details 仅渲染主要摄像机视野范围内的瓦片（Culling），应用底部对齐和像素对齐
 */
void TileLayerComponent::render(engine::core::Context& context)
{
	if (is_hidden_) {
		return;
	}
	auto& renderer = context.getRenderer();
	auto& camera = context.getCamera();

	// --- 1. 视锥体剔除 (Culling) ---
	// 目的：仅渲染摄像机视野内的瓦片，极大提高大地图的渲染性能
	
	glm::vec2 cam_pos = camera.getPosition();
	glm::vec2 cam_size = camera.getViewportSize();

	glm::vec2 layer_world_offset = offset_;
	if (owner_) {
		if (auto* tc = owner_->getComponent<engine::component::TransformComponent>()) {
			layer_world_offset += tc->getPosition();
		}
	}

	// 计算视野范围对应的网格坐标 (包含一些冗余量以防边缘闪烁)
    // 增加渲染范围冗余量，已修正：处理超大图块（如树木、建筑）和负高度偏移
    // 某些大图块的锚点在网格内，但图像延伸出网格很远。如果只渲染视口内的网格，会导致这些大图块也被剔除。
    const int extra_padding = 20; 
	glm::ivec2 start_tile = glm::ivec2(glm::floor((cam_pos - layer_world_offset) / glm::vec2(tile_size_))) - glm::ivec2(extra_padding);
	glm::ivec2 end_tile = glm::ivec2(glm::ceil((cam_pos + cam_size - layer_world_offset) / glm::vec2(tile_size_))) + glm::ivec2(extra_padding);

	// 限制坐标在地图有效范围内 (Intersection)
	start_tile = glm::max(start_tile, glm::ivec2(0));
	end_tile = glm::min(end_tile, map_size_);

	// --- 2. 渲染循环 ---
	for (int y = start_tile.y; y < end_tile.y; ++y) {
		for (int x = start_tile.x; x < end_tile.x; ++x) {
			const TileInfo& tile = getTileAt({ x, y });
			
			if (tile.type != TileType::EMPTY) {
				glm::vec2 tile_world_pos = layer_world_offset + glm::vec2(x * tile_size_.x, y * tile_size_.y);
				
				// --- 3. 底部对齐逻辑 (Bottom Alignment) ---
				// Tiled 规则：如果图片高度 > 瓦片高度（如树木），图片底部应与网格底部对齐
				
				float sprite_h = static_cast<float>(tile_size_.y);
				// 安全访问 optional
				if (tile.sprite.getSourceRect().has_value()) {
					sprite_h = tile.sprite.getSourceRect()->h;
				}

				// 如果高度不一致，向上偏移差异值
				if (std::abs(sprite_h - tile_size_.y) > 0.1f) {
					tile_world_pos.y -= (sprite_h - static_cast<float>(tile_size_.y));
				}

				// 像素对齐：确保瓦片渲染在整数像素位置上，避免亚像素偏移导致的缝隙
				// 注意：这里直接对世界坐标进行取整，因为相机的 worldToScreen 会再次处理像素对齐
				tile_world_pos.x = std::round(tile_world_pos.x);
				tile_world_pos.y = std::round(tile_world_pos.y);

				// 对所有瓦片做充分重叠，确保边缘无缝拼接
				// 特别是在相机移动时，避免亚像素偏移导致的缝隙
				glm::vec2 scale(1.0f, 1.0f);
				float overlap_epsilon = 1.0f; // 进一步增加重叠量以确保完全覆盖
				scale.x = (static_cast<float>(tile_size_.x) + overlap_epsilon) / static_cast<float>(tile_size_.x);
				scale.y = (static_cast<float>(tile_size_.y) + overlap_epsilon) / static_cast<float>(tile_size_.y);

				renderer.drawSprite(camera, tile.sprite, tile_world_pos, scale);
			}
		}
	}
}

/**
 * @brief 更新图层逻辑
 * 
 * @param deltaTime 时间增量（秒）
 * @param context 引擎上下文
 * @details 瓦片图层通常不需要更新逻辑，此方法为空
 */
void TileLayerComponent::update(float deltaTime, engine::core::Context& context)
{
}

/**
 * @brief 清理组件资源
 * 
 * @details 清理瓦片图层组件，从物理引擎中注销碰撞图层
 */
void TileLayerComponent::clean()
{
	if (physics_engine_) {
		physics_engine_->unregisterCollisionLayer(this);
	}
}

}  // namespace engine::component
