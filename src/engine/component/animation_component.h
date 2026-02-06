/**
 * @file animation_component.h
 * @brief 定义动画相关的结构体和 AnimationComponent 组件，用于ECS架构中的实体动画。
 *
 * @details
 * AnimationComponent 是ECS架构中的动画相关组件，负责存储和管理实体的动画状态。
 * 该组件与 SpriteComponent 配合使用，由 AnimationSystem 每帧更新动画帧。
 *
 * 组件设计特点：
 * - 支持多动画状态机（如 idle、walk、attack）
 * - 动画数据与播放状态分离
 * - 通过修改 SpriteComponent 实现动画渲染
 *
 * @par 使用示例
 * @code
 * // 创建动画帧
 * std::vector<AnimationFrame> idle_frames = {
 *     AnimationFrame(Rect(0, 0, 32, 32), 100),
 *     AnimationFrame(Rect(32, 0, 32, 32), 100)
 * };
 *
 * // 创建动画
 * Animation idle_anim(std::move(idle_frames), true);  // 循环播放
 *
 * // 创建动画映射
 * std::unordered_map<entt::id_type, Animation> anims;
 * anims["idle"_hs] = std::move(idle_anim);
 *
 * // 添加动画组件到实体
 * auto entity = registry.create();
 * registry.emplace<AnimationComponent>(entity, std::move(anims), "idle"_hs);
 * registry.emplace<SpriteComponent>(entity, sprite);
 * @endcode
 *
 * @see engine::component::SpriteComponent 被动画系统修改的渲染组件
 * @see engine::system::AnimationSystem 处理动画更新的系统
 */

#pragma once
#include "../../engine/utils/math.h"
#include <entt/core/hashed_string.hpp>
#include <entt/entity/entity.hpp>
#include <unordered_map>
#include <vector>

namespace engine::component {

    /**
     * @struct AnimationFrame
     * @brief 动画帧数据结构，定义单个动画帧的信息。
     *
     * @details
     * AnimationFrame 定义了动画序列中的一帧，包含：
     * - 源矩形：在纹理图集中的位置和大小
     * - 持续时间：该帧显示的时间（毫秒）
     *
     * 多个 AnimationFrame 组成一个完整的动画序列。
     */
    struct AnimationFrame {
        engine::utils::Rect src_rect_{};        ///< 帧在纹理图集中的源矩形，定义裁剪区域
        float duration_ms_{100.0f};             ///< 这一帧持续的时间（毫秒），决定动画播放速度

        /**
         * @brief 构造函数
         * @param src_rect 帧在纹理中的源矩形区域
         * @param duration_ms 帧持续时间（毫秒），默认为100ms
         */
        AnimationFrame(engine::utils::Rect src_rect, float duration_ms = 100.0f)
            : src_rect_(std::move(src_rect)), duration_ms_(duration_ms) {}
    };

    /**
     * @struct Animation
     * @brief 单个动画序列的数据结构，包含一系列动画帧。
     *
     * @details
     * Animation 存储了一个完整的动画序列（如行走、攻击、待机动画）。
     * 它会自动计算总时长，并支持循环播放设置。
     *
     * @par 动画播放流程
     * 1. AnimationSystem 根据 current_time_ms_ 确定当前帧
     * 2. 更新 SpriteComponent 的 src_rect_ 为当前帧的源矩形
     * 3. 达到最后一帧时，根据 loop_ 设置决定是否回到第一帧
     */
    struct Animation {
        std::vector<AnimationFrame> frames_;    ///< 动画包含的所有帧序列，按播放顺序排列
        float total_duration_ms_{};             ///< 整个动画序列的总时长（毫秒），自动计算
        bool loop_{true};                       ///< 播放完毕后是否循环，false表示播放一次后停止

        /**
         * @brief 构造函数，创建动画序列
         * @param frames 动画帧序列，按播放顺序
         * @param loop 是否循环播放，默认为true
         *
         * @note 构造函数会自动计算 total_duration_ms_
         */
        Animation(std::vector<AnimationFrame> frames, bool loop = true)
            : frames_(std::move(frames)), loop_(loop) {
            // 计算动画总时长 (总时长 = 所有帧时长之和)
            total_duration_ms_ = 0.0f;
            for (const auto& frame : frames_) {
                total_duration_ms_ += frame.duration_ms_;
            }
        }
    };

    /**
     * @struct AnimationComponent
     * @brief ECS动画组件，管理实体的动画播放状态。
     *
     * @details
     * AnimationComponent 是ECS架构中的动画组件，存储实体的所有动画和当前播放状态。
     * 它本身不直接渲染，而是通过修改 SpriteComponent 来实现动画效果。
     *
     * @par 组件数据说明
     * - animations_：存储所有可用动画（如 idle、walk、attack）
     * - current_animation_id_：当前播放的动画ID（使用 entt::hashed_string）
     * - current_frame_index_：当前显示的帧索引
     * - current_time_ms_：当前动画已播放的时间，用于确定当前帧
     * - speed_：播放速度倍率，1.0为正常速度，2.0为双倍速
     *
     * @par 状态切换
     * 可以通过修改 current_animation_id_ 来切换动画状态：
     * @code
     * auto& anim = registry.get<AnimationComponent>(entity);
     * anim.current_animation_id_ = "walk"_hs;  // 切换到行走动画
     * anim.current_frame_index_ = 0;           // 重置到第一帧
     * anim.current_time_ms_ = 0;               // 重置时间
     * @endcode
     */
    struct AnimationComponent {
        std::unordered_map<entt::id_type, Animation> animations_;   ///< 动画ID到动画数据的映射集合，支持多动画状态
        entt::id_type current_animation_id_{entt::null};            ///< 当前正在播放的动画ID（entt::hashed_string值）
        size_t current_frame_index_{};                              ///< 当前播放到的帧索引，从0开始
        float current_time_ms_{};                                   ///< 当前动画已播放的时间（毫秒），用于帧切换计算
        float speed_{1.0f};                                         ///< 动画播放速度倍率，1.0为正常速度

        /**
         * @brief 构造函数，创建动画组件
         * @param animations 初始动画集合，动画ID映射到Animation对象
         * @param current_animation_id 初始播放的动画ID
         * @param current_frame_index 初始帧索引，默认为0
         * @param current_time_ms 初始播放时间，默认为0
         * @param speed 动画播放速度倍率，默认为1.0
         */
        AnimationComponent(std::unordered_map<entt::id_type, Animation> animations,
                        entt::id_type current_animation_id,
                        size_t current_frame_index = 0,
                        float current_time_ms = 0.0f,
                        float speed = 1.0f) :
                        animations_(std::move(animations)),
                        current_animation_id_(current_animation_id),
                        current_frame_index_(current_frame_index),
                        current_time_ms_(current_time_ms),
                        speed_(speed) {}
    };

}