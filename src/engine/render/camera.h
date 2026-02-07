/**
 * @file camera.h
 * @brief 定义 Camera 类，负责管理相机位置、视口大小和坐标转换。
 * 
 * @details
 * Camera 类是游戏引擎中的核心渲染组件，提供以下功能：
 * - 管理相机在世界空间中的位置
 * - 提供世界坐标与屏幕坐标之间的转换
 * - 支持视差滚动效果
 * - 限制相机移动范围
 * - 相机移动平滑过渡
 * 
 * @see engine::system::RenderSystem 使用相机进行渲染的系统
 * @see engine::component::ParallaxComponent 与相机配合实现视差效果的组件
 */

#pragma once
#include "../utils/math.h"
#include <optional>

namespace engine::render {

/**
 * @class Camera
 * @brief 相机类负责管理相机位置和视口大小，并提供坐标转换功能。
 * 
 * @details
 * Camera 类实现了游戏中的相机系统，主要功能包括：
 * - 管理相机在世界空间中的位置
 * - 提供世界坐标与屏幕坐标的相互转换
 * - 支持视差滚动效果的坐标计算
 * - 限制相机移动范围，防止相机移出场景边界
 * - 相机移动的平滑过渡
 */
class Camera final {
private:
    glm::vec2 viewport_size_;                                                ///< 视口大小（屏幕大小）
    glm::vec2 position_;                                                     ///< 相机左上角的世界坐标
    std::optional<engine::utils::Rect> limit_bounds_;                        ///< 限制相机的移动范围，空值表示不限制
    float smooth_speed_ = 5.0f;                                              ///< 相机移动的平滑速度

public:
    /**
     * @brief 构造相机对象
     * @param viewport_size 视口大小
     * @param position 相机位置，默认为 (0.0f, 0.0f)
     * @param limit_bounds 限制相机的移动范围，默认为 std::nullopt
     */
    Camera(glm::vec2 viewport_size, 
           glm::vec2 position = glm::vec2(0.0f, 0.0f), 
           std::optional<engine::utils::Rect> limit_bounds = std::nullopt);
    
    /**
     * @brief 移动相机
     * @param offset 移动偏移量
     */
    void move(const glm::vec2& offset);

    /**
     * @brief 世界坐标转屏幕坐标
     * @param world_pos 世界坐标
     * @return 转换后的屏幕坐标
     */
    glm::vec2 worldToScreen(const glm::vec2& world_pos) const;
    
    /**
     * @brief 世界坐标转屏幕坐标，考虑视差滚动
     * @param world_pos 世界坐标
     * @param scroll_factor 视差滚动因子
     * @return 转换后的屏幕坐标（考虑视差效果）
     */
    glm::vec2 worldToScreenWithParallax(const glm::vec2& world_pos, const glm::vec2& scroll_factor) const;
    
    /**
     * @brief 屏幕坐标转世界坐标
     * @param screen_pos 屏幕坐标
     * @return 转换后的世界坐标
     */
    glm::vec2 screenToWorld(const glm::vec2& screen_pos) const;

    /**
     * @brief 设置相机位置
     * @param position 新的相机位置
     */
    void setPosition(glm::vec2 position);
    
    /**
     * @brief 设置限制相机的移动范围
     * @param limit_bounds 新的限制边界
     */
    void setLimitBounds(std::optional<engine::utils::Rect> limit_bounds);

    /**
     * @brief 获取相机位置
     * @return 相机当前位置
     */
    const glm::vec2& getPosition() const;
    
    /**
     * @brief 获取限制相机的移动范围
     * @return 相机移动限制边界
     */
    std::optional<engine::utils::Rect> getLimitBounds() const;
    
    /**
     * @brief 获取视口大小
     * @return 视口大小
     */
    glm::vec2 getViewportSize() const;

    /**
     * @brief 禁用拷贝和移动语义
     */
    Camera(const Camera&) = delete;
    Camera& operator=(const Camera&) = delete;
    Camera(Camera&&) = delete;
    Camera& operator=(Camera&&) = delete;

private:
    /**
     * @brief 限制相机位置在边界内
     * 
     * @details
     * 如果设置了限制边界，此方法会确保相机位置不会超出边界，
     * 同时保证相机视口始终完全包含在边界内。
     */
    void clampPosition();
};

} // namespace engine::render