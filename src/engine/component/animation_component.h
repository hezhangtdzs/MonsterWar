#pragma once
#include "../../engine/utils/math.h"
#include <entt/core/hashed_string.hpp>
#include <entt/entity/entity.hpp>
#include <unordered_map>
#include <vector>

namespace engine::component {

    /**
     * @struct AnimationFrame
     * @brief 动画帧数据结构
     * 
     * 包含帧在纹理中的源矩形以及该帧的持续时间。
     */
    struct AnimationFrame {
        engine::utils::Rect src_rect_{};        ///< 帧在纹理图集中的源矩形
        float duration_ms_{100.0f};             ///< 这一帧持续的时间（毫秒）

        AnimationFrame(engine::utils::Rect src_rect, float duration_ms = 100.0f)
            : src_rect_(std::move(src_rect)), duration_ms_(duration_ms) {}
    };

    /**
     * @struct Animation
     * @brief 单个动画序列的数据结构
     * 
     * 包含一系列的动画帧，并管理播放逻辑如总时长和循环属性。
     */
    struct Animation {
        std::vector<AnimationFrame> frames_;    ///< 动画包含的所有帧序列
        float total_duration_ms_{};             ///< 整个动画序列的总时长（毫秒）
        bool loop_{true};                       ///< 播放完毕后是否循环

        /**
         * @brief 构造函数
         * @param frames 动画帧序列
         * @param loop 是否循环播放，默认为真
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
     * @brief 动画组件，用于管理实体的动画播放状态
     * 
     * 存储实体的所有可选动画，并记录当前播放的动画、帧索引及进度。
     */
    struct AnimationComponent {
        std::unordered_map<entt::id_type, Animation> animations_;   ///< 动画 ID 到动画数据的映射集合
        entt::id_type current_animation_id_{entt::null};            ///< 当前正在播放的动画 ID
        size_t current_frame_index_{};                              ///< 当前播放到的帧索引
        float current_time_ms_{};                                   ///< 当前动画已播放的时间（毫秒）
        float speed_{1.0f};                                         ///< 动画播放速度倍率

        /**
         * @brief 构造函数
         * @param animations 初始动画集合
         * @param current_animation_id 初始播放的动画 ID
         * @param current_frame_index 初始帧索引
         * @param current_time_ms 初始播放时间
         * @param speed 动画播放速度
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