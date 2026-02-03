#pragma once
/**
 * @file collider.h
 * @brief 定义碰撞体相关的类和枚举，用于物理碰撞检测。
 */

#include <glm/vec2.hpp>

namespace engine::physics {

	/**
	 * @enum ColliderType
	 * @brief 碰撞体类型枚举，定义了当前支持的碰撞体类型。
	 */
	enum class ColliderType {
		NONE,   ///< 无碰撞体类型
		AABB,   ///< 轴对齐包围盒碰撞体
		CIRCLE, ///< 圆形碰撞体
		// 未来可扩展更多碰撞体类型
	};

	/**
	 * @class Collider
	 * @brief 碰撞体基类，定义了所有碰撞体类型的通用接口。
	 * 
	 * 所有具体碰撞体类型（如 AABBCollider、CircleCollider）都继承自此类。
	 */
	class Collider {
	protected:
		/// 碰撞体的轴对齐包围盒尺寸
		glm::vec2 aabb_size_{ 0.0f, 0.0f };
	
	public:
		/// 默认虚析构函数
		virtual ~Collider() = default;

		/**
		 * @brief 获取碰撞体类型。
		 * @return 碰撞体类型枚举值。
		 */
		virtual ColliderType getType() const = 0;

		/**
		 * @brief 获取碰撞体的轴对齐包围盒尺寸。
		 * @return 包围盒尺寸向量。
		 */
		const glm::vec2& getAABBSize() const { return aabb_size_; }

		/**
		 * @brief 设置碰撞体的轴对齐包围盒尺寸。
		 * @param size 新的包围盒尺寸。
		 */
		void setAABBSize(const glm::vec2& size) { aabb_size_ = std::move(size); }
	};

	/**
	 * @class AABBCollider
	 * @brief 轴对齐包围盒碰撞体类，实现了基于矩形的碰撞检测。
	 * 
	 * 该类表示一个轴对齐的矩形碰撞体，适合用于大多数游戏对象的碰撞检测。
	 */
	class AABBCollider : public Collider {
	private:
		/// 碰撞体的尺寸
		glm::vec2 size_{ 0.0f, 0.0f };
	
	public:
		/**
		 * @brief 构造函数，创建一个新的 AABB 碰撞体。
		 * @param size 碰撞体的尺寸。
		 */
		AABBCollider(const glm::vec2& size) : size_(std::move(size)) {
			setAABBSize(size_);
		}

		/// 默认虚析构函数
		~AABBCollider() override = default;

		/**
		 * @brief 获取碰撞体类型。
		 * @return ColliderType::AABB。
		 */
		ColliderType getType() const override { return ColliderType::AABB; }

		/**
		 * @brief 获取碰撞体的尺寸。
		 * @return 碰撞体尺寸向量。
		 */
		const glm::vec2& getSize() const { return size_; }

		/**
		 * @brief 设置碰撞体的尺寸。
		 * @param size 新的碰撞体尺寸。
		 */
		void setSize(const glm::vec2& size) { size_ = std::move(size); }
	};

	/**
	 * @class CircleCollider
	 * @brief 圆形碰撞体类，实现了基于圆形的碰撞检测。
	 * 
	 * 该类表示一个圆形碰撞体，适合用于圆形或近似圆形的游戏对象。
	 */
	class CircleCollider : public Collider {
	private:
		/// 碰撞体的半径
		float radius_{ 0.0f };
	
	public:
		/**
		 * @brief 构造函数，创建一个新的圆形碰撞体。
		 * @param radius 碰撞体的半径。
		 */
		CircleCollider(float radius) : radius_(std::move(radius)) {
			setAABBSize({ radius_ * 2.0f, radius_ * 2.0f });
		}

		/// 默认虚析构函数
		~CircleCollider() override = default;

		/**
		 * @brief 获取碰撞体类型。
		 * @return ColliderType::CIRCLE。
		 */
		ColliderType getType() const override { return ColliderType::CIRCLE; }

		/**
		 * @brief 获取碰撞体的半径。
		 * @return 碰撞体半径。
		 */
		float getRadius() const { return radius_; }

		/**
		 * @brief 设置碰撞体的半径。
		 * @param radius 新的碰撞体半径。
		 */
		void setRadius(float radius) { radius_ = std::move(radius); }
	};
}  // namespace engine::physics