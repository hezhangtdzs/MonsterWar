#pragma once
#include <vector>
#include <glm/vec2.hpp>
#include "../component/tilelayer_component.h"
#include "../utils/math.h"
namespace engine {
	namespace object {
		class GameObject;
	}
	namespace component {
		class PhysicsComponent;
	}
}
namespace engine::physics {
	/**
	 * @class PhysicsEngine
	 * @brief 物理引擎类，负责管理和更新物理组件
	 * 
	 * 该类处理所有物理相关的计算和更新，包括力的应用、速度和位置的更新等。
	 */
	class PhysicsEngine {
	private:
		std::vector<component::PhysicsComponent*> physics_components_;

		std::vector<std::pair<engine::object::GameObject*, engine::object::GameObject*>> collision_pairs_;
		std::vector<std::pair<engine::object::GameObject*, engine::component::TileType>> tile_trigger_events_;

		std::vector<component::TileLayerComponent*> tilelayer_components_;
		glm::vec2 gravity_ = { 0.0f, 980.0f };
		float max_speed_ = 5000.0f;
		glm::vec2 world_bounds_min_{ 0.0f, 0.0f };
		glm::vec2 world_bounds_max_{ 0.0f, 0.0f };
	public:
		/**
		 * @brief 更新所有物理组件
		 * 
		 * @param delta_time 时间增量（单位：秒）
		 */
		PhysicsEngine() = default;
		// 禁止拷贝和移动
		PhysicsEngine(const PhysicsEngine&) = delete;
		PhysicsEngine& operator=(const PhysicsEngine&) = delete;
		PhysicsEngine(PhysicsEngine&&) = delete;
		PhysicsEngine& operator=(PhysicsEngine&&) = delete;

		void registerPhysicsComponent(component::PhysicsComponent* physics_component);
		void registerCollisionLayer(component::TileLayerComponent* tilelayer_component);
		void unregisterPhysicsComponent(component::PhysicsComponent* physics_component);
		void unregisterCollisionLayer(component::TileLayerComponent* tilelayer_component);
		const auto& getCollisionPairs() const { return collision_pairs_; };

		void update(float delta_time);


		void setGravity(const glm::vec2& gravity) { gravity_ = gravity; }
		const glm::vec2& getGravity() const { return gravity_; }
		void setMaxSpeed(float max_speed) { max_speed_ = max_speed; }
		float getMaxSpeed() const { return max_speed_; }
		void setWorldBounds(const engine::utils::Rect& bounds) {
			world_bounds_min_ = bounds.position;
			world_bounds_max_ = bounds.position + bounds.size;
		}
		const std::vector<std::pair<engine::object::GameObject*, engine::object::GameObject*>>& getCollisions() const {
			return collision_pairs_;
		};

		const std::vector<std::pair<engine::object::GameObject*, engine::component::TileType>>& getTileTriggerEvents() const {
			return tile_trigger_events_;
		};

		engine::component::TileType getTileTypeAt(const glm::vec2& world_pos) const;
		bool tryGetLadderColumnCenterX(const glm::vec2& world_pos, float& out_center_x) const;

	private:

		// 新增：碰撞检测循环
		void checkObjectCollisions();
		float getTileHeightAtWidth(float width, engine::component::TileType type, glm::vec2 tile_size);

		void resolveTileCollisions(engine::component::PhysicsComponent* pc, float delta_time);
		void resolveSolidObjectCollisions(engine::object::GameObject* move_obj, engine::object::GameObject* solid_obj);

		void resolveXAxisCollision(
			engine::component::PhysicsComponent* pc,
			glm::vec2& aabb_pos,
			float dx,
			const glm::vec2& collider_size,
			engine::component::TileLayerComponent* layer);

		void resolveYAxisCollision(
			engine::component::PhysicsComponent* pc,
			glm::vec2& aabb_pos,
			float dy,
			const glm::vec2& collider_size,
			engine::component::TileLayerComponent* layer);

		void checkTileTriggers();
	};

}  // namespace engine::physics