/**
 * @file audio_component.h
 * @brief 定义 AudioComponent 组件，用于ECS架构中的实体音效管理。
 *
 * @details
 * AudioComponent 是ECS架构中的音频相关组件，负责存储实体与各种动作关联的音效映射。
 * 该组件允许实体根据不同的动作（如跳跃、攻击、受伤）播放对应的音效。
 *
 * @par 使用示例
 * @code
 * // 为玩家实体添加音频组件
 * auto entity = registry.create();
 * AudioComponent audio_comp;
 * audio_comp.action_sounds_["jump"_hs] = "jump_sound"_hs;
 * audio_comp.action_sounds_["attack"_hs] = "attack_sound"_hs;
 * registry.emplace<AudioComponent>(entity, std::move(audio_comp));
 * @endcode
 *
 * @see engine::audio::AudioPlayer 音频播放器
 * @see engine::audio::AudioLocator 音频服务定位器
 */

#pragma once
#include <unordered_map>
#include <entt/entity/fwd.hpp>

namespace engine::core { class Context; }

namespace engine::component {

    /**
     * @struct AudioComponent
     * @brief 音频组件，用于管理与实体动作关联的音效映射。
     *
     * @details
     * 该组件存储动作ID到音效资源ID的映射，允许系统根据实体动作播放相应音效。
     * 动作ID和音效ID都使用 entt::hashed_string 生成，以提高查找性能。
     */
    struct AudioComponent {
        /**
         * @brief 动作 ID 到音效资源 ID 的映射集合
         * @details
         * key: 动作标识 (例如 "jump", "hit" 的哈希值)
         * value: 对应的音效资源 ID
         */
        std::unordered_map<entt::id_type, entt::id_type> action_sounds_; 
    };

}