#include "physics_engine.h"
#include <spdlog/spdlog.h>
#include "../component/physics_component.h"
#include "../component/transform_component.h"
#include "../component/collider_component.h"
#include "../component/tilelayer_component.h"
#include "collision.h"
#include "../object/game_object.h"
#include <glm/glm.hpp>
#include <algorithm>
#include <set>
#include <cmath>

namespace engine::physics {

/**
 * @brief 注册物理组件到物理引擎
 * 
 * @param physics_component 物理组件指针
 * @details 将物理组件添加到物理引擎的管理列表中，使其参与物理更新和碰撞检测
 */
void PhysicsEngine::registerPhysicsComponent(component::PhysicsComponent* physics_component)
{
	physics_components_.push_back(physics_component);
	spdlog::info("物理组件注册 {}", static_cast<void*>(physics_component));
}

/**
 * @brief 注册碰撞图层到物理引擎
 * 
 * @param tilelayer_component 瓦片图层组件指针
 * @details 将瓦片图层组件添加到物理引擎的管理列表中，使其参与碰撞检测
 */
void PhysicsEngine::registerCollisionLayer(component::TileLayerComponent* tilelayer_component)
{
	tilelayer_component->setPhysicsEngine(this);
	tilelayer_components_.push_back(tilelayer_component);
	spdlog::info("瓦片图层组件注册 {}", static_cast<void*>(tilelayer_component));
}

/**
 * @brief 从物理引擎中注销物理组件
 * 
 * @param physics_component 物理组件指针
 * @details 将物理组件从物理引擎的管理列表中移除，使其不再参与物理更新和碰撞检测
 */
void PhysicsEngine::unregisterPhysicsComponent(component::PhysicsComponent* physics_component)
{
	physics_components_.erase(std::remove(physics_components_.begin(), physics_components_.end(), physics_component), physics_components_.end());
	spdlog::info("物理组件注册注销 {}", static_cast<void*>(physics_component));
}

/**
 * @brief 从物理引擎中注销碰撞图层
 * 
 * @param tilelayer_component 瓦片图层组件指针
 * @details 将瓦片图层组件从物理引擎的管理列表中移除，使其不再参与碰撞检测
 */
void PhysicsEngine::unregisterCollisionLayer(component::TileLayerComponent* tilelayer_component)
{
	tilelayer_components_.erase(std::remove(tilelayer_components_.begin(), tilelayer_components_.end(), tilelayer_component), tilelayer_components_.end());
	spdlog::info("瓦片图层组件注册注销 {}", static_cast<void*>(tilelayer_component));
}

/**
 * @brief 获取指定世界位置的瓦片类型
 * 
 * @param world_pos 世界坐标位置
 * @return 瓦片类型枚举值
 * @details 获取指定世界位置的瓦片类型，优先返回梯子类型
 */
engine::component::TileType PhysicsEngine::getTileTypeAt(const glm::vec2& world_pos) const
{
	engine::component::TileType result = engine::component::TileType::NORMAL;
	for (auto* layer : tilelayer_components_) {
		if (layer) {
			auto type = layer->getTileTypeAtWorldPos(world_pos);
			if (type == engine::component::TileType::LADDER) return type; // 优先探测梯子
			if (type != engine::component::TileType::EMPTY && type != engine::component::TileType::NORMAL) {
				result = type; // 记录其他非空类型（如 SOLID），但继续寻找 LADDER
			}
		}
	}
	return result;
}

/**
 * @brief 尝试获取梯子列的中心X坐标
 * 
 * @param world_pos 世界坐标位置
 * @param out_center_x 输出参数，梯子列的中心X坐标
 * @return 是否成功获取梯子列中心X坐标
 * @details 尝试获取指定世界位置所在梯子列的中心X坐标
 */
bool PhysicsEngine::tryGetLadderColumnCenterX(const glm::vec2& world_pos, float& out_center_x) const
{
	using engine::component::TileType;
	for (auto* layer : tilelayer_components_) {
		if (!layer || layer->isHidden()) continue;

		const glm::vec2 tile_size = glm::vec2(layer->getTileSize());
		if (tile_size.x <= 0.0f || tile_size.y <= 0.0f) continue;

		// 计算图层世界坐标偏移
		glm::vec2 layer_world_offset = layer->getOffset();
		if (auto* layer_owner = layer->getOwner()) {
			if (auto* tc = layer_owner->getComponent<engine::component::TransformComponent>()) {
				layer_world_offset += tc->getPosition();
			}
		}

		glm::ivec2 tile_coords = glm::floor((world_pos - layer_world_offset) / tile_size);
		TileType type = layer->getTileTypeAt(tile_coords);
		if (type != TileType::LADDER) continue;

		// 返回梯子所在列的中心 X
		out_center_x = layer_world_offset.x + (static_cast<float>(tile_coords.x) + 0.5f) * tile_size.x;
		return true;
	}
	return false;
}

/**
 * @brief 更新所有物理组件
 * 
 * @param delta_time 时间增量（单位：秒）
 * @details 更新所有物理组件的状态，包括应用力、更新速度和位置、处理碰撞等
 */
void PhysicsEngine::update(float delta_time)
{
	collision_pairs_.clear();
    tile_trigger_events_.clear();
	// 防止卡顿/断点导致 dt 过大，从而一帧内位移过大直接飞出镜头
	const float dt = std::clamp(delta_time, 0.0f, 1.0f / 30.0f);
    for (auto* pc : physics_components_) {
        if (!pc || !pc->isEnabled()) { // 检查组件是否有效和启用
            continue;
        }

        // 重置碰撞标志
        pc->resetCollisionFlags();
		pc->tickSnapSuppression(dt);

		const float mass = pc->getMass();
		if (!(mass > 0.0f) || !std::isfinite(mass)) {
			continue;
		}
		
        // 应用重力 (如果组件受重力影响)：F = g * m
        if (pc->isUseGravity()) {
            pc->addForce(gravity_ * pc->getMass());
        }
        /* 还可以添加其它力影响，比如风力、摩擦力等，目前不考虑 */

        // 更新速度： v += a * dt，其中 a = F / m
        pc->velocity_ += (pc->getForce() / mass) * dt;
        pc->clearForce(); // 清除当前帧的力

		// 限制最大速度：v = min(v, max_speed)
		pc->velocity_ = glm::clamp(pc->velocity_, -max_speed_, max_speed_);

		resolveTileCollisions(pc, dt);

	}
	checkObjectCollisions();
	checkTileTriggers();
}

/**
 * @brief 检查对象之间的碰撞
 * 
 * @details 检查所有物理组件之间的碰撞，并处理碰撞响应
 */
void PhysicsEngine::checkObjectCollisions()

{
	// 基于 ColliderComponent 的碰撞检测
	for (size_t i = 0; i < physics_components_.size(); ++i) {
		auto* pcA = physics_components_[i];
		if (!pcA || !pcA->isEnabled()) continue;
		auto* ownerA = pcA->getOwner();
		if (!ownerA) continue;
		auto* colA = ownerA->getComponent<engine::component::ColliderComponent>();
		if (!colA || !colA->getIsActive()) continue;
		for (size_t j = i + 1; j < physics_components_.size(); ++j) {
			auto* pcB = physics_components_[j];
			if (!pcB || !pcB->isEnabled()) continue;
			auto* ownerB = pcB->getOwner();
			if (!ownerB) continue;
			auto* colB = ownerB->getComponent<engine::component::ColliderComponent>();
			if (!colB || !colB->getIsActive()) continue;

			if (engine::physics::collision::checkCollision(*colA, *colB)) {
                if (ownerA->getTag() != "solid" && ownerB->getTag() == "solid") {
                    resolveSolidObjectCollisions(ownerA, ownerB);
                }
                else if (ownerA->getTag() == "solid" && ownerB ->getTag() != "solid") {
                    resolveSolidObjectCollisions(ownerB, ownerA);
                }
                else {
                    // 记录碰撞对
                    collision_pairs_.emplace_back(ownerA, ownerB);
                }
			}
		}
	}
}

/**
 * @brief 获取指定宽度和瓦片类型的瓦片高度
 * 
 * @param width 瓦片宽度
 * @param type 瓦片类型
 * @param tile_size 瓦片尺寸
 * @return 瓦片高度
 * @details 根据瓦片类型和宽度计算瓦片的高度，用于斜坡碰撞处理
 */
float PhysicsEngine::getTileHeightAtWidth(float width, engine::component::TileType type, glm::vec2 tile_size)
{
	if (tile_size.x <= 0.0f || tile_size.y <= 0.0f) {
		return 0.0f;
	}
	auto relx = glm::clamp(width / tile_size.x, 0.0f, 1.0f);
    switch (type) {
    case engine::component::TileType::SLOPE_0_1:
        return relx * tile_size.y;
    case engine::component::TileType::SLOPE_1_0:
        return (1.0f - relx) * tile_size.y;
    case engine::component::TileType::SLOPE_0_2: // 2x1 slope, first segment (0% -> 50%)
        return relx * 0.5f * tile_size.y;
    case engine::component::TileType::SLOPE_2_1: // 2x1 slope, second segment (50% -> 100%)
        return (0.5f + relx * 0.5f) * tile_size.y;
    case engine::component::TileType::SLOPE_1_2: // 2x1 descending slope, first segment (100% -> 50%)
        return (1.0f - relx * 0.5f) * tile_size.y;
    case engine::component::TileType::SLOPE_2_0: // 2x1 descending slope, second segment (50% -> 0%)
        return (0.5f - relx * 0.5f) * tile_size.y;
    default:
        return 0.0f;
    }
}

/**
 * @brief 处理瓦片碰撞
 * 
 * @param pc 物理组件
 * @param delta_time 时间增量（单位：秒）
 * @details 处理物理组件与瓦片之间的碰撞
 */
void PhysicsEngine::resolveTileCollisions(engine::component::PhysicsComponent* pc, float delta_time)
{
    using engine::component::ColliderComponent;
    using engine::component::TileType;
    using engine::component::TransformComponent;

    // 1. 基础检查：确保物理组件有效且启用
    if (!pc || !pc->isEnabled()) {
        return;
    }

    auto* obj = pc->getOwner();
    if (!obj) {
        return;
    }
    // 2. 获取必要组件：计算位置和碰撞盒
    auto* tc = obj->getComponent<TransformComponent>();
    auto* cc = obj->getComponent<ColliderComponent>();
    // 如果没有碰撞盒、未激活或是触发器（不参与物理阻挡），则跳过
    if (!tc || !cc || cc->getIsTrigger()) {
        return;
    }

    // 3. 计算当前的 AABB (轴对齐包围盒)
    const glm::vec2 collider_offset = cc->getOffset();
    const glm::vec2 collider_size = cc->getWorldAABB().size;

    // 当前未移动前的 AABB 左上角世界坐标
    glm::vec2 aabb_pos = tc->getPosition() + collider_offset;

    // ds: 当前帧的位移增量 (velocity * dt)
    const glm::vec2 ds = pc->velocity_ * delta_time;
    // eps: 碰撞检测容差，防止浮点精度问题导致卡在墙内或穿墙
    const float eps = 0.001f;
	// 如果碰撞体未激活，则直接应用位移并返回
    if (!cc->getIsActive()) {
        tc->translate(ds);
        pc->velocity_ = glm::clamp(pc->velocity_, -max_speed_, max_speed_);
        return;
    }

    // 4. 遍历所有注册的瓦片图层进行检测
    for (auto* layer : tilelayer_components_) {
        if (!layer || layer->isHidden()) {
            continue;
        }

        // 5. X 轴碰撞处理
        if (ds.x != 0.0f) {
            resolveXAxisCollision(pc, aabb_pos, ds.x, collider_size, layer);
        }

        // 6. Y 轴碰撞处理
        if (ds.y != 0.0f) {
            resolveYAxisCollision(pc, aabb_pos, ds.y, collider_size, layer);
        }
    }

    // 9. World Bounds Check
    if (world_bounds_max_.x > world_bounds_min_.x) {
        if (aabb_pos.x < world_bounds_min_.x) {
            aabb_pos.x = world_bounds_min_.x;
            pc->velocity_.x = 0.0f;
            pc->setCollidedLeft(true);
        }
        else if (aabb_pos.x + collider_size.x > world_bounds_max_.x) {
            aabb_pos.x = world_bounds_max_.x - collider_size.x;
            pc->velocity_.x = 0.0f;
            pc->setCollidedRight(true);
        }
    }

    if (world_bounds_max_.y > world_bounds_min_.y) {
        // Only clamp top boundary (min Y), allow falling indefinitely
        if (aabb_pos.y < world_bounds_min_.y) {
            aabb_pos.y = world_bounds_min_.y;
            if (pc->velocity_.y < 0.0f) {
                pc->velocity_.y = 0.0f;
                pc->setCollidedAbove(true);
            }
        }
    }

    // 7. 更新 Transform 组件的位置
    // 变换位置 = 计算出的 AABB 位置 - 碰撞器偏移量
    tc->setPosition(aabb_pos - collider_offset);

    // 8. 应用速度限制
    pc->velocity_ = glm::clamp(pc->velocity_, -max_speed_, max_speed_);
}

/**
 * @brief 处理固体对象之间的碰撞
 * 
 * @param move_obj 移动对象
 * @param solid_obj 固体对象
 * @details 处理移动对象与固体对象之间的碰撞
 */
void PhysicsEngine::resolveSolidObjectCollisions(engine::object::GameObject* move_obj, engine::object::GameObject* solid_obj)
{
    auto* move_tc = move_obj->getComponent<engine::component::TransformComponent>();
    auto* move_pc = move_obj->getComponent<engine::component::PhysicsComponent>();
    auto* move_cc = move_obj->getComponent<engine::component::ColliderComponent>();

    auto* solid_cc = solid_obj->getComponent<engine::component::ColliderComponent>();

    auto move_aabb = move_cc->getWorldAABB();
    auto solid_aabb = solid_cc->getWorldAABB();

    // 1. 计算中心距离和重叠量
    const glm::vec2 move_half = move_aabb.size * 0.5f;
    const glm::vec2 solid_half = solid_aabb.size * 0.5f;
    const glm::vec2 move_center = move_aabb.position + move_half;
    const glm::vec2 solid_center = solid_aabb.position + solid_half;

    const glm::vec2 diff = move_center - solid_center;
    const glm::vec2 overlap = (move_half + solid_half) - glm::abs(diff);

    // 容差检查：如果并没有真正碰撞则退出
    if (overlap.x <= 0.0f || overlap.y <= 0.0f) return;

    // 2. 在重叠最小的轴上进行推离 (MTV 策略)
    if (overlap.x < overlap.y) {
        // X 轴碰撞：根据相对坐标确定推离方向 (1.0 代表右，-1.0 代表左)
        float push_dir = (diff.x > 0.0f) ? 1.0f : -1.0f;
        move_tc->translate(glm::vec2(overlap.x * push_dir, 0.0f));

        // 速度修正：如果物体的速度方向正对着 SOLID 物体，则将其归零
        if (glm::sign(move_pc->velocity_.x) != glm::sign(push_dir)) {
            move_pc->velocity_.x = 0.0f;
        }

        // 更新碰撞标志
        if (push_dir > 0) {
            move_pc->setCollidedLeft(true);
        } else {
            move_pc->setCollidedRight(true);
        }
    } 
    else {
        // Y 轴碰撞：根据相对坐标确定推离方向 (1.0 代表下，-1.0 代表上)
        float push_dir = (diff.y > 0.0f) ? 1.0f : -1.0f;
        move_tc->translate(glm::vec2(0.0f, overlap.y * push_dir));

        // 速度修正
        if (glm::sign(move_pc->velocity_.y) != glm::sign(push_dir)) {
            move_pc->velocity_.y = 0.0f;
        }

        // 更新碰撞标志
        if (push_dir > 0) {
            move_pc->setCollidedAbove(true);
        } else {
            move_pc->setCollidedBelow(true);
        }
    }
}

/**
 * @brief 处理X轴碰撞
 * 
 * @param pc 物理组件
 * @param aabb_pos AABB位置
 * @param dx X轴位移
 * @param collider_size 碰撞器尺寸
 * @param layer 瓦片图层
 * @details 处理物理组件在X轴方向与瓦片之间的碰撞
 */
void PhysicsEngine::resolveXAxisCollision(
    engine::component::PhysicsComponent* pc,
    glm::vec2& aabb_pos,
    float dx,
    const glm::vec2& collider_size,
    engine::component::TileLayerComponent* layer)
{
    using engine::component::TileType;

    const glm::vec2 tile_size_vec = layer->getTileSize();
    if (tile_size_vec.x <= 0.0f || tile_size_vec.y <= 0.0f) return;
    
    // 计算图层的世界坐标偏移
    glm::vec2 layer_offset = layer->getOffset();
    if (auto* layer_owner = layer->getOwner()) {
        if (auto* layer_tc = layer_owner->getComponent<engine::component::TransformComponent>()) {
            layer_offset += layer_tc->getPosition();
        }
    }

    const int layer_width = layer->getMapSize().x;
    const int layer_height = layer->getMapSize().y;
    const float eps = 0.001f;

    auto getTypeAt = [&](int tx, int ty) -> TileType {
        if (tx < 0 || ty < 0 || tx >= layer_width || ty >= layer_height) {
            return TileType::EMPTY;
        }
        return layer->getTileTypeAt({ tx, ty });
    };

    auto isSolid = [&](int tx, int ty) {
        return getTypeAt(tx, ty) == TileType::SOLID;
    };

    auto isSlope = [&](TileType t) {
        switch (t) {
        case TileType::SLOPE_0_1:
        case TileType::SLOPE_1_0:
        case TileType::SLOPE_0_2:
        case TileType::SLOPE_2_1:
        case TileType::SLOPE_1_2:
        case TileType::SLOPE_2_0:
            return true;
        default:
            return false;
        }
    };
    
    if (dx == 0.0f) return;

    glm::vec2 new_pos = aabb_pos;
    new_pos.x += dx;

    // Y-range indices
    const float top = new_pos.y + eps;
    const float bottom = new_pos.y + collider_size.y - eps;
    const int tile_y_top = static_cast<int>(std::floor((top - layer_offset.y) / tile_size_vec.y));
    const int tile_y_bottom = static_cast<int>(std::floor((bottom - layer_offset.y) / tile_size_vec.y));

    if (dx > 0.0f) {
        // ---> Moving Right
        const float right = new_pos.x + collider_size.x - eps;
        const int tile_x = static_cast<int>(std::floor((right - layer_offset.x) / tile_size_vec.x));

        if (tile_x >= 0 && tile_x < layer_width) {
            bool hit = false;
            bool hit_top = false;
            bool hit_bottom = false;
            
            if (tile_y_top >= 0 && tile_y_top < layer_height && isSolid(tile_x, tile_y_top)) { hit = true; hit_top = true; }
            if (tile_y_bottom >= 0 && tile_y_bottom < layer_height && isSolid(tile_x, tile_y_bottom)) { hit = true; hit_bottom = true; }

            // Slope transition (Up-Right)
            if (hit && hit_bottom && !hit_top) {
                 int curr_tx = static_cast<int>(std::floor((aabb_pos.x + collider_size.x - eps - layer_offset.x) / tile_size_vec.x));
                 TileType ct = getTypeAt(curr_tx, tile_y_bottom);
                 if (isSlope(ct)) {
                      float h = getTileHeightAtWidth(tile_size_vec.x, ct, tile_size_vec);
                      float ground_y = layer_offset.y + static_cast<float>(tile_y_bottom + 1) * tile_size_vec.y - h;
                      float solid_top = layer_offset.y + static_cast<float>(tile_y_bottom) * tile_size_vec.y;
                      if (ground_y <= solid_top + eps) {
                          new_pos.y = solid_top - collider_size.y;
                          hit = false;
                      }
                 }
            }

            if (hit) {
                new_pos.x = layer_offset.x + static_cast<float>(tile_x) * tile_size_vec.x - collider_size.x;
                pc->velocity_.x = 0.0f;
                pc->setCollidedRight(true);
            } else if (tile_y_bottom >= 0 && tile_y_bottom < layer_height) {
                // Check if walked INTO slope (embedded)
                TileType t = getTypeAt(tile_x, tile_y_bottom);
                if (isSlope(t)) {
                    float tile_origin_x = layer_offset.x + static_cast<float>(tile_x) * tile_size_vec.x;
                    float rel_x = (new_pos.x + collider_size.x - eps) - tile_origin_x;
                    float h = getTileHeightAtWidth(rel_x, t, tile_size_vec);
                    float ground_y = layer_offset.y + static_cast<float>(tile_y_bottom + 1) * tile_size_vec.y - h;
                    if (new_pos.y + collider_size.y >= ground_y - eps) {
                         new_pos.y = ground_y - collider_size.y;
                         pc->setCollidedBelow(true);
                         pc->velocity_.y = 0.0f;
                    }
                }
            }
        }
    } else {
        // <--- Moving Left
        const float left = new_pos.x + eps;
        const int tile_x = static_cast<int>(std::floor((left - layer_offset.x) / tile_size_vec.x));
        
        if (tile_x >= 0 && tile_x < layer_width) {
             bool hit = false;
             bool hit_top = false;
             bool hit_bottom = false;
             
             if (tile_y_top >= 0 && tile_y_top < layer_height && isSolid(tile_x, tile_y_top)) { hit = true; hit_top = true; }
             if (tile_y_bottom >= 0 && tile_y_bottom < layer_height && isSolid(tile_x, tile_y_bottom)) { hit = true; hit_bottom = true; }

             // Slope transition (Up-Left)
             if (hit && hit_bottom && !hit_top) {
                 int curr_tx = static_cast<int>(std::floor((aabb_pos.x + eps - layer_offset.x) / tile_size_vec.x));
                 TileType ct = getTypeAt(curr_tx, tile_y_bottom);
                 if (isSlope(ct)) {
                      float h = getTileHeightAtWidth(0.0f, ct, tile_size_vec);
                      float ground_y = layer_offset.y + static_cast<float>(tile_y_bottom + 1) * tile_size_vec.y - h;
                      float solid_top = layer_offset.y + static_cast<float>(tile_y_bottom) * tile_size_vec.y;
                      if (ground_y <= solid_top + eps) {
                           new_pos.y = solid_top - collider_size.y;
                           hit = false;
                      }
                 }
             }

              if (hit) {
                  new_pos.x = layer_offset.x + static_cast<float>(tile_x + 1) * tile_size_vec.x;
                  pc->velocity_.x = 0.0f;
                  pc->setCollidedLeft(true);
              } else if (tile_y_bottom >= 0 && tile_y_bottom < layer_height) {
                  // Slope embedding check
                  TileType t = getTypeAt(tile_x, tile_y_bottom);
                  if (isSlope(t)) {
                      float tile_origin_x = layer_offset.x + static_cast<float>(tile_x) * tile_size_vec.x;
                      float rel_x = (new_pos.x + eps) - tile_origin_x;
                      float h = getTileHeightAtWidth(rel_x, t, tile_size_vec);
                      float ground_y = layer_offset.y + static_cast<float>(tile_y_bottom + 1) * tile_size_vec.y - h;
                      if (new_pos.y + collider_size.y >= ground_y - eps) {
                          new_pos.y = ground_y - collider_size.y;
                          pc->setCollidedBelow(true);
                          pc->velocity_.y = 0.0f;
                      }
                  }
              }
        }
    }
    aabb_pos = new_pos;
}

/**
 * @brief 处理Y轴碰撞
 * 
 * @param pc 物理组件
 * @param aabb_pos AABB位置
 * @param dy Y轴位移
 * @param collider_size 碰撞器尺寸
 * @param layer 瓦片图层
 * @details 处理物理组件在Y轴方向与瓦片之间的碰撞
 */
void PhysicsEngine::resolveYAxisCollision(
    engine::component::PhysicsComponent* pc,
    glm::vec2& aabb_pos,
    float dy,
    const glm::vec2& collider_size,
    engine::component::TileLayerComponent* layer)
{
    using engine::component::TileType;
    const glm::vec2 tile_size_vec = layer->getTileSize();
    if (tile_size_vec.x <= 0.0f || tile_size_vec.y <= 0.0f) return;

    glm::vec2 layer_offset = layer->getOffset();
    if (auto* layer_owner = layer->getOwner()) {
        if (auto* layer_tc = layer_owner->getComponent<engine::component::TransformComponent>()) {
            layer_offset += layer_tc->getPosition();
        }
    }
    const int layer_width = layer->getMapSize().x;
    const int layer_height = layer->getMapSize().y;
    const float eps = 0.001f;

    auto getTypeAt = [&](int tx, int ty) -> TileType {
        if (tx < 0 || ty < 0 || tx >= layer_width || ty >= layer_height) return TileType::EMPTY;
        return layer->getTileTypeAt({ tx, ty });
    };

    auto isSolid = [&](int tx, int ty) {
        return getTypeAt(tx, ty) == TileType::SOLID;
    };
     auto isUnisolid = [&](int tx, int ty) {
        TileType type = getTypeAt(tx, ty);
        if (type == TileType::UNISOLID) return true;
        // 如果是梯子顶端，且玩家不在攀爬状态，则视为单向平台（支持在梯子顶端站立和走过）
        if (type == TileType::LADDER && !pc->isClimbing()) {
            if (getTypeAt(tx, ty - 1) != TileType::LADDER) return true;
        }
        return false;
    };

    auto isSlope = [&](TileType t) {
        switch (t) {
        case TileType::SLOPE_0_1:
        case TileType::SLOPE_1_0:
        case TileType::SLOPE_0_2:
        case TileType::SLOPE_2_1:
        case TileType::SLOPE_1_2:
        case TileType::SLOPE_2_0:
            return true;
        default:
            return false;
        }
    };

    if (dy == 0.0f) return;

    glm::vec2 new_pos = aabb_pos;
    new_pos.y += dy;

    const float left = new_pos.x + eps;
    const float right = new_pos.x + collider_size.x - eps;
    const int tile_x_left = static_cast<int>(std::floor((left - layer_offset.x) / tile_size_vec.x));
    const int tile_x_right = static_cast<int>(std::floor((right - layer_offset.x) / tile_size_vec.x));

    if (dy > 0.0f) {
        // vvv Moving Down (Falling)
        const float bottom = new_pos.y + collider_size.y - eps;
        const int tile_y = static_cast<int>(std::floor((bottom - layer_offset.y) / tile_size_vec.y));
        
        if (tile_y >= 0 && tile_y < layer_height) {
            bool hit = false;
            if (tile_x_left >= 0 && tile_x_left < layer_width && (isSolid(tile_x_left, tile_y) || isUnisolid(tile_x_left, tile_y))) hit = true;
            if (tile_x_right >= 0 && tile_x_right < layer_width && (isSolid(tile_x_right, tile_y) || isUnisolid(tile_x_right, tile_y))) hit = true;

            if (hit) {
                new_pos.y = layer_offset.y + static_cast<float>(tile_y) * tile_size_vec.y - collider_size.y;
                pc->velocity_.y = 0.0f;
                pc->setCollidedBelow(true);
			} else {
				// 吸附逻辑 (Stickiness)：处理斜坡和单向平台（包含梯子顶端）
				// 起跳和向上运动期间禁用吸附，避免被斜坡重新“拽回地面”产生滑行/贴地。
				// 某些情况下（例如同时按左右触发额外的水平碰撞修正），会产生很小的向下 dy，导致误触发吸附。
				if (pc->isSnapSuppressed() || pc->velocity_.y < 0.0f) {
					aabb_pos.y = new_pos.y;
					return;
				}
                const float snap_distance = 8.0f; 
                float min_ground_y = 1e9f;
                bool found_target = false;

                auto checkSnapTarget = [&](int tx, int ty) {
                    if (tx < 0 || tx >= layer_width || ty < 0 || ty >= layer_height) return;
                    TileType t = getTypeAt(tx, ty);
                    if (isSlope(t)) {
                        float tile_x_origin = layer_offset.x + static_cast<float>(tx) * tile_size_vec.x;
                        float h_left = getTileHeightAtWidth(glm::clamp((new_pos.x + eps) - tile_x_origin, 0.0f, tile_size_vec.x), t, tile_size_vec);
                        float h_right = getTileHeightAtWidth(glm::clamp((new_pos.x + collider_size.x - eps) - tile_x_origin, 0.0f, tile_size_vec.x), t, tile_size_vec);
                        float h = std::max(h_left, h_right);
                        
                        float g_y = layer_offset.y + static_cast<float>(ty + 1) * tile_size_vec.y - h;
                        if (g_y < min_ground_y) {
                            min_ground_y = g_y;
                            found_target = true;
                        }
                    } else if (isUnisolid(tx, ty)) {
                        float g_y = layer_offset.y + static_cast<float>(ty) * tile_size_vec.y;
                        // 确保玩家是从平台上方向下落 (脚底在平台上方或略微陷入)
                        if (g_y < min_ground_y && (aabb_pos.y + collider_size.y) <= g_y + eps) {
                            min_ground_y = g_y;
                            found_target = true;
                        }
                    }
                };

                // 检查当前行和下一行
                checkSnapTarget(tile_x_left, tile_y);
                checkSnapTarget(tile_x_right, tile_y);
                checkSnapTarget(tile_x_left, tile_y + 1);
                checkSnapTarget(tile_x_right, tile_y + 1);

                if (found_target) {
                    float current_bottom = new_pos.y + collider_size.y;
                    if (current_bottom >= min_ground_y - eps || (pc->velocity_.y >= 0 && current_bottom > min_ground_y - snap_distance)) {
                        new_pos.y = min_ground_y - collider_size.y;
                        pc->velocity_.y = 0.0f;
                        pc->setCollidedBelow(true);
                    }
                }
            }
        }
    } else {
        // ^^^ Moving Up (Jumping)
        const float top = new_pos.y + eps;
        const int tile_y = static_cast<int>(std::floor((top - layer_offset.y) / tile_size_vec.y));
        if (tile_y >= 0 && tile_y < layer_height) {
             bool hit = false;
             if (tile_x_left >= 0 && tile_x_left < layer_width && isSolid(tile_x_left, tile_y)) hit = true;
             if (tile_x_right >= 0 && tile_x_right < layer_width && isSolid(tile_x_right, tile_y)) hit = true;
             if (hit) {
                 new_pos.y = layer_offset.y + static_cast<float>(tile_y + 1) * tile_size_vec.y;
                 pc->velocity_.y = 0.0f;
                 pc->setCollidedAbove(true);
             }
        }
    }
    aabb_pos.y = new_pos.y;
}

/**
 * @brief 检查瓦片触发器
 * 
 * @details 检查物理组件与瓦片触发器之间的碰撞
 */
void PhysicsEngine::checkTileTriggers()
{
    for (auto* pc : physics_components_) {
        if (!pc || !pc->isEnabled()) continue;
        
        auto* obj = pc->getOwner();
        if (!obj) continue;

        auto* cc = obj->getComponent<engine::component::ColliderComponent>();
        if (!cc || !cc->getIsActive()) continue;

        const auto world_aabb = cc->getWorldAABB();
        // 使用 set 防止同一帧内由于接触多个同类瓦片而重复触发相同事件
        std::set<engine::component::TileType> triggers_set;

        for (auto* layer : tilelayer_components_) {
            if (!layer || layer->isHidden()) continue;

            const glm::vec2 tile_size = layer->getTileSize();
            if (tile_size.x <= 0.0f || tile_size.y <= 0.0f) continue;

            // 计算图层世界坐标偏移
            glm::vec2 layer_offset = layer->getOffset();
            if (auto* layer_owner = layer->getOwner()) {
                if (auto* layer_tc = layer_owner->getComponent<engine::component::TransformComponent>()) {
                    layer_offset += layer_tc->getPosition();
                }
            }

            // 计算物体覆盖的瓦片索引范围
            int start_x = static_cast<int>(std::floor((world_aabb.position.x - layer_offset.x) / tile_size.x));
            int end_x   = static_cast<int>(std::floor((world_aabb.position.x + world_aabb.size.x - layer_offset.x) / tile_size.x));
            int start_y = static_cast<int>(std::floor((world_aabb.position.y - layer_offset.y) / tile_size.y));
            int end_y   = static_cast<int>(std::floor((world_aabb.position.y + world_aabb.size.y - layer_offset.y) / tile_size.y));

            const auto map_size = layer->getMapSize();

            for (int x = start_x; x <= end_x; ++x) {
                if (x < 0 || x >= map_size.x) continue;
                for (int y = start_y; y <= end_y; ++y) {
                    if (y < 0 || y >= map_size.y) continue;

                    auto tile_type = layer->getTileTypeAt({ x, y });
                    // 目前检测 HAZARD 类型，可根据需要扩展其它触发器类型
                    if (tile_type == engine::component::TileType::HAZARD) {
                        triggers_set.insert(tile_type);
                    }
                }
            }
        }

        // 将本帧触发的所有唯一类型的事件记录下来
        for (const auto& type : triggers_set) {
            tile_trigger_events_.emplace_back(obj, type);
        }
    }
}

} // namespace engine::physics