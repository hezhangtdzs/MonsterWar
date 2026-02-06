/**
 * @file fwd.h
 * @brief 系统模块的前向声明文件。
 *
 * @details
 * 本文件包含 engine::system 命名空间下所有系统类的前向声明。
 * 使用此文件可以避免在头文件中包含完整的系统头文件，减少编译依赖。
 *
 * 前向声明适用于以下场景：
 * - 只需要使用类指针或引用（如 std::unique_ptr<RenderSystem>）
 * - 不需要访问类的成员或完整定义
 *
 * @par 使用示例
 * @code
 * // 在头文件中使用前向声明
 * #include "engine/system/fwd.h"
 *
 * class MyClass {
 *     std::unique_ptr<engine::system::RenderSystem> render_system_;  // 只需要前向声明
 * };
 *
 * // 在实现文件中包含完整定义
 * #include "engine/system/render_system.h"
 * @endcode
 *
 * @note 如果需要创建对象或访问成员，必须在实现文件中包含对应的完整头文件
 */

#pragma once

namespace engine::system {

class RenderSystem;      ///< 渲染系统前向声明
class AnimationSystem;   ///< 动画系统前向声明
class MovementSystem;    ///< 移动系统前向声明

}   // namespace engine::system