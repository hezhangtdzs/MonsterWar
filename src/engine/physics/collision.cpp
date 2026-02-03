#include "collision.h"
#include "../component/collider_component.h"
#include "../component/transform_component.h"
#include "collider.h"

namespace engine::physics::collision {

/**
 * @brief 检查两个碰撞器组件是否重叠
 * 
 * @param a 第一个碰撞器组件
 * @param b 第二个碰撞器组件
 * @return true 如果碰撞器组件重叠，否则为 false
 * @details 首先检查AABB包围盒是否重叠，然后根据碰撞器类型进行更细致的碰撞检测
 */
bool checkCollision(const engine::component::ColliderComponent& a, const engine::component::ColliderComponent& b) {
    // 获取两个碰撞体及对应Transform信息
    const auto* a_collider = a.getCollider();
    const auto* b_collider = b.getCollider();
    const auto* a_transform = a.getTransform();
    const auto* b_transform = b.getTransform();

    if (a_collider == nullptr || b_collider == nullptr || a_transform == nullptr || b_transform == nullptr) {
        return false;
    }

    // 先计算最小包围盒是否碰撞，如果没有碰撞，那一定是返回false (不考虑AABB的旋转)
    const auto a_aabb = a.getWorldAABB();
    const auto b_aabb = b.getWorldAABB();
    const auto a_pos = a_aabb.position;
    const auto b_pos = b_aabb.position;
    const auto a_size = a_aabb.size;
    const auto b_size = b_aabb.size;
    if (!checkAABBOverlap(a_pos, a_size, b_pos, b_size)) {
        return false;
    }

    // --- 如果最小包围盒有碰撞，再进行更细致的判断 ---
    // AABB vs AABB, 直接返回真
    if (a_collider->getType() == engine::physics::ColliderType::AABB && b_collider->getType() == engine::physics::ColliderType::AABB) {
        return true;
    }

    // Circle vs Circle
    if (a_collider->getType() == engine::physics::ColliderType::CIRCLE && b_collider->getType() == engine::physics::ColliderType::CIRCLE)
    {
        const auto a_center = a_pos + 0.5f * a_size;
        const auto b_center = b_pos + 0.5f * b_size;

        float a_radius = 0.5f * a_size.x;
        float b_radius = 0.5f * b_size.x;
        if (const auto* ca = dynamic_cast<const engine::physics::CircleCollider*>(a_collider)) {
            a_radius = ca->getRadius();
        }
        if (const auto* cb = dynamic_cast<const engine::physics::CircleCollider*>(b_collider)) {
            b_radius = cb->getRadius();
        }
        return checkCircleOverlap(a_center, a_radius, b_center, b_radius);
    }

    // AABB vs Circle
    if (a_collider->getType() == engine::physics::ColliderType::AABB && b_collider->getType() == engine::physics::ColliderType::CIRCLE)
    {
        const auto b_center = b_pos + 0.5f * b_size;
        float b_radius = 0.5f * b_size.x;
        if (const auto* cb = dynamic_cast<const engine::physics::CircleCollider*>(b_collider)) {
            b_radius = cb->getRadius();
        }

        const auto nearest_point = glm::clamp(b_center, a_pos, a_pos + a_size);
        return checkPointInCircle(nearest_point, b_center, b_radius);
    }

    // Circle vs AABB
    if (a_collider->getType() == engine::physics::ColliderType::CIRCLE && b_collider->getType() == engine::physics::ColliderType::AABB)
    {
        const auto a_center = a_pos + 0.5f * a_size;
        float a_radius = 0.5f * a_size.x;
        if (const auto* ca = dynamic_cast<const engine::physics::CircleCollider*>(a_collider)) {
            a_radius = ca->getRadius();
        }

        const auto nearest_point = glm::clamp(a_center, b_pos, b_pos + b_size);
        return checkPointInCircle(nearest_point, a_center, a_radius);
    }

    return false;
}

/**
 * @brief 检查两个圆形是否重叠
 * 
 * @param a_center 第一个圆的中心
 * @param a_radius 第一个圆的半径
 * @param b_center 第二个圆的中心
 * @param b_radius 第二个圆的半径
 * @return true 如果两个圆重叠，否则为 false
 * @details 通过计算两个圆心之间的距离是否小于等于两个半径之和来判断圆形是否重叠
 */
bool checkCircleOverlap(const glm::vec2& a_center, const float a_radius, const glm::vec2& b_center, const float b_radius)
{
    const float r = a_radius + b_radius;
    return glm::dot(a_center - b_center, a_center - b_center) <= r * r;
}

/**
 * @brief 检查两个轴对齐包围盒 (AABB) 是否重叠
 * 
 * @param a_pos 第一个AABB的左上角坐标
 * @param a_size 第一个AABB的尺寸
 * @param b_pos 第二个AABB的左上角坐标
 * @param b_size 第二个AABB的尺寸
 * @return true 如果AABB重叠，否则为 false
 * @details 通过检查两个AABB在x和y轴上是否有重叠来判断是否碰撞
 */
bool checkAABBOverlap(const glm::vec2& a_pos, const glm::vec2& a_size, const glm::vec2& b_pos, const glm::vec2& b_size) {
    // 检查两个AABB是否重叠
    // 这里将"边缘接触"也视为碰撞：仅当出现严格分离时返回 false
    if (a_pos.x + a_size.x < b_pos.x || a_pos.x > b_pos.x + b_size.x ||
        a_pos.y + a_size.y < b_pos.y || a_pos.y > b_pos.y + b_size.y) {
        return false;
    }
    return true;
}

/**
 * @brief 检查两个矩形是否重叠
 * 
 * @param a 第一个矩形
 * @param b 第二个矩形
 * @return true 如果矩形重叠，否则为 false
 * @details 调用 checkAABBOverlap 函数检查两个矩形是否重叠
 */
bool checkRectOverlap(const engine::utils::Rect& a, const engine::utils::Rect& b) {
    return checkAABBOverlap(a.position, a.size, b.position, b.size);
}

/**
 * @brief 检查一个点是否在圆内
 * 
 * @param point 要检查的点
 * @param center 圆的中心
 * @param radius 圆的半径
 * @return true 如果点在圆内，否则为 false
 * @details 通过计算点到圆心的距离是否小于等于半径来判断点是否在圆内
 */
bool checkPointInCircle(const glm::vec2& point, const glm::vec2& center, const float radius)
{
    return glm::dot(point - center, point - center) <= radius * radius;
}

} // namespace engine::physics::collision