/**
 * @file alignment.h
 * @brief 定义对象或组件的对齐方式枚举。
 * 
 * 该文件提供了对齐方式的枚举定义，用于UI元素、精灵等的位置对齐。
 */

#pragma once

namespace engine::utils {

    /**
     * @enum class Alignment
     * @brief 定义对象或组件相对于其位置参考点的对齐方式。
     *
     * 对齐方式用于确定对象相对于其锚点的偏移计算方式。
     * 例如，CENTER 对齐会将对象的中心点放在锚点位置，
     * 而 TOP_LEFT 对齐会将对象的左上角放在锚点位置。
     */
    enum class Alignment {
        NONE,           ///< 不指定对齐方式，偏移量通常为 (0,0) 或手动设置
        TOP_LEFT,       ///< 左上角对齐
        TOP_CENTER,     ///< 顶部中心对齐
        TOP_RIGHT,      ///< 右上角对齐
        CENTER_LEFT,    ///< 中心左侧对齐
        CENTER,         ///< 正中心对齐（几何中心）
        CENTER_RIGHT,   ///< 中心右侧对齐
        BOTTOM_LEFT,    ///< 左下角对齐
        BOTTOM_CENTER,  ///< 底部中心对齐
        BOTTOM_RIGHT    ///< 右下角对齐
    };

} // namespace engine::utils 