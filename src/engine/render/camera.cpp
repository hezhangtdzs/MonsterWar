/**
 * @file camera.cpp
 * @brief Camera 类的实现，提供相机管理和坐标转换功能。
 */

#include "camera.h"
#include "../utils/math.h"
#include <spdlog/spdlog.h>
#include <glm/common.hpp>

namespace engine::render {

/**
 * @brief Camera 构造函数
 * @param viewport_size 视口大小
 * @param position 相机位置
 * @param limit_bounds 限制相机的移动范围
 */
Camera::Camera(glm::vec2 viewport_size, 
               glm::vec2 position, 
               std::optional<engine::utils::Rect> limit_bounds)
    : viewport_size_(std::move(viewport_size)), 
      position_(std::move(position)), 
      limit_bounds_(std::move(limit_bounds)) {
    spdlog::trace("Camera 初始化成功，位置: {},{}", position_.x, position_.y);
}

/**
 * @brief 设置相机位置
 * @param position 新的相机位置
 */
void Camera::setPosition(glm::vec2 position) {
    position_ = std::move(position);
    clampPosition();
}

/**
 * @brief 移动相机
 * @param offset 移动偏移量
 */
void Camera::move(const glm::vec2 &offset)
{
    position_ += offset;
    clampPosition();
}

/**
 * @brief 设置限制相机的移动范围
 * @param limit_bounds 新的限制边界
 */
void Camera::setLimitBounds(std::optional<engine::utils::Rect> limit_bounds)
{
    limit_bounds_ = std::move(limit_bounds);
    clampPosition(); // 设置边界后，立即应用限制
}

/**
 * @brief 获取相机位置
 * @return 相机当前位置
 */
const glm::vec2& Camera::getPosition() const {
    return position_;
}

/**
 * @brief 限制相机位置在边界内
 * 
 * @details
 * 此方法确保相机视口始终完全包含在设置的限制边界内：
 * 1. 计算相机允许的最小和最大位置
 * 2. 确保最大位置不小于最小位置（处理视口大于世界的情况）
 * 3. 使用 glm::clamp 限制相机位置
 */
void Camera::clampPosition()
{
    // 边界检查需要确保相机视图（position 到 position + viewport_size）在 limit_bounds 内
    if (limit_bounds_.has_value() && limit_bounds_->size.x > 0 && limit_bounds_->size.y > 0) {
        // 计算允许的相机位置范围
        glm::vec2 min_cam_pos = limit_bounds_->position;
        glm::vec2 max_cam_pos = limit_bounds_->position + limit_bounds_->size - viewport_size_;

        // 确保 max_cam_pos 不小于 min_cam_pos (视口可能比世界还大)
        max_cam_pos.x = std::max(min_cam_pos.x, max_cam_pos.x);
        max_cam_pos.y = std::max(min_cam_pos.y, max_cam_pos.y);

        position_ = glm::clamp(position_, min_cam_pos, max_cam_pos);
    }
    // 如果 limit_bounds 无效则不进行限制
}

/**
 * @brief 世界坐标转屏幕坐标
 * @param world_pos 世界坐标
 * @return 转换后的屏幕坐标
 * 
 * @details
 * 计算方法：屏幕坐标 = 世界坐标 - 相机位置
 */
glm::vec2 Camera::worldToScreen(const glm::vec2& world_pos) const {
    // 将世界坐标减去相机左上角位置
    return world_pos - position_;
}

/**
 * @brief 世界坐标转屏幕坐标，考虑视差滚动
 * @param world_pos 世界坐标
 * @param scroll_factor 视差滚动因子
 * @return 转换后的屏幕坐标（考虑视差效果）
 * 
 * @details
 * 计算方法：屏幕坐标 = 世界坐标 - 相机位置 * 视差因子
 * 视差因子小于1时，图层移动速度比相机慢，产生远景效果
 * 视差因子大于1时，图层移动速度比相机快，产生近景效果
 */
glm::vec2 Camera::worldToScreenWithParallax(const glm::vec2 &world_pos, const glm::vec2 &scroll_factor) const
{
    // 相机位置应用滚动因子
    return world_pos - position_ * scroll_factor;
}

/**
 * @brief 屏幕坐标转世界坐标
 * @param screen_pos 屏幕坐标
 * @return 转换后的世界坐标
 * 
 * @details
 * 计算方法：世界坐标 = 屏幕坐标 + 相机位置
 */
glm::vec2 Camera::screenToWorld(const glm::vec2 &screen_pos) const
{
    // 将屏幕坐标加上相机左上角位置
    return screen_pos + position_;
}

/**
 * @brief 获取视口大小
 * @return 视口大小
 */
glm::vec2 Camera::getViewportSize() const {
    return viewport_size_;
}

/**
 * @brief 获取限制相机的移动范围
 * @return 相机移动限制边界
 */
std::optional<engine::utils::Rect> Camera::getLimitBounds() const {
    return limit_bounds_;
}

} // namespace engine::render 