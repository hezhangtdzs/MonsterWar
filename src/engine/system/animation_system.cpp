/**
 * @file animation_system.cpp
 * @brief AnimationSystem 类的实现，ECS动画系统的具体逻辑。
 *
 * @details
 * 本文件实现了 AnimationSystem::update() 方法，处理所有具有
 * AnimationComponent 和 SpriteComponent 的实体的动画更新。
 *
 * @par 实现细节
 * - 使用 registry.view<>() 高效查询需要动画更新的实体
 * - 根据时间增量和播放速度推进动画进度
 * - 在适当的时候切换到下一帧
 * - 更新 SpriteComponent 的源矩形以显示正确的帧
 *
 * @par 帧切换逻辑
 * 1. 累加时间：current_time_ms += dt * 1000 * speed
 * 2. 如果 current_time_ms >= frame_duration，切换到下一帧
 * 3. 处理循环：如果到达最后一帧且 loop_ 为 true，回到第一帧
 * 4. 更新 SpriteComponent 的 src_rect_ 为当前帧的矩形
 */

#include "animation_system.h"
#include "../component/animation_component.h"
#include "../component/sprite_component.h"
#include <entt/entity/registry.hpp>

namespace engine::system {

void AnimationSystem::update(entt::registry& registry, float dt) {
    // 创建组件视图：获取同时具有 AnimationComponent 和 SpriteComponent 的实体
    auto view = registry.view<engine::component::AnimationComponent, engine::component::SpriteComponent>();

    for (auto entity : view) {
        // 获取动画组件（存储动画状态和帧数据）
        auto& anim_component = view.get<engine::component::AnimationComponent>(entity);

        // 获取精灵组件（将被修改以显示正确的帧）
        auto& sprite_component = view.get<engine::component::SpriteComponent>(entity);

        // 查找当前动画
        auto it = anim_component.animations_.find(anim_component.current_animation_id_);
        if (it == anim_component.animations_.end()) {
            continue;  // 动画不存在，跳过
        }

        // 获取当前动画数据
        auto& current_animation = it->second;

        // 检查动画是否有帧
        if (current_animation.frames_.empty()) {
            continue;  // 没有帧，跳过
        }

        // 更新当前播放时间（推进计时器）
        // 时间增量 = 帧时间（秒）× 1000（转毫秒）× 播放速度倍率
        anim_component.current_time_ms_ += dt * 1000.0f * anim_component.speed_;

        // 获取当前帧
        const auto& current_frame = current_animation.frames_[anim_component.current_frame_index_];

        // 检查是否需要切换到下一帧
        if (anim_component.current_time_ms_ >= current_frame.duration_ms_) {
            // 减去当前帧持续时间，保留剩余时间
            anim_component.current_time_ms_ -= current_frame.duration_ms_;
            anim_component.current_frame_index_++;

            // 处理动画播放完成（到达最后一帧）
            if (anim_component.current_frame_index_ >= current_animation.frames_.size()) {
                if (current_animation.loop_) {
                    // 循环播放：回到第一帧
                    anim_component.current_frame_index_ = 0;
                } else {
                    // 不循环：停在最后一帧
                    anim_component.current_frame_index_ = current_animation.frames_.size() - 1;
                }
            }
        }

        // 更新 SpriteComponent 的源矩形为当前帧的矩形
        // 这是动画系统与渲染系统的连接点
        const auto& next_frame = current_animation.frames_[anim_component.current_frame_index_];
        sprite_component.sprite_.src_rect_ = next_frame.src_rect_;
    }
}

} // namespace engine::system