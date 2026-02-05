#pragma once
#include <unordered_map>
#include <entt/entity/fwd.hpp>

namespace engine::core { class Context; }

namespace engine::component {

    /**
     * @struct AudioComponent
     * @brief 音频组件
     * 
     * 用于管理与实体动作（如走路、受伤、攻击）关联的音效映射。
     */
    struct AudioComponent {
        /**
         * @brief 动作 ID 到音效资源 ID 的映射集合
         * key: 动作标识 (例如 "jump", "hit" 的哈希值)
         * value: 对应的音效资源 ID
         */
        std::unordered_map<entt::id_type, entt::id_type> action_sounds_; 
    };

}