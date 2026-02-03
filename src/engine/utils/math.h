#pragma once
/**
 * @file math.h
 * @brief 包含游戏引擎使用的数学工具和结构体定义。
 */

#include <glm/glm.hpp>

namespace engine::utils {

    /**
     * @struct Rect
     * @brief 表示一个二维矩形区域。
     * 
     * 该结构体用于定义游戏中的矩形区域，包含位置和大小信息，
     * 广泛用于碰撞检测、渲染裁剪和UI布局等场景。
     */
    struct Rect
    {
        glm::vec2 position; ///< 矩形的左上角位置坐标
        glm::vec2 size;     ///< 矩形的宽度和高度
    };
    /**
 * @struct FColor
 * @brief 浮点颜色结构，使用 0.0f-1.0f 范围的浮点数表示 RGBA 颜色。
 */
struct FColor {
    float r; ///< 红色通道 (0.0f-1.0f)
    float g; ///< 绿色通道 (0.0f-1.0f)
    float b; ///< 蓝色通道 (0.0f-1.0f)
    float a; ///< alpha通道 (0.0f-1.0f)
    
    /**
     * @brief 构造函数，默认创建白色不透明颜色。
     */
    FColor() : r(1.0f), g(1.0f), b(1.0f), a(1.0f) {}
    
    /**
     * @brief 构造函数，指定 RGBA 值。
     * @param r 红色通道值
     * @param g 绿色通道值
     * @param b 蓝色通道值
     * @param a alpha通道值
     */
    FColor(float r, float g, float b, float a = 1.0f) : r(r), g(g), b(b), a(a) {}
};

}