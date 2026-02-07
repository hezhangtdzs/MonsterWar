#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <entt/core/hashed_string.hpp>
#include <entt/entity/entity.hpp>
#include <glm/vec2.hpp>
#include "../../engine/utils/math.h"

namespace game::data {

/**
 * @struct StatsBlueprint
 * @brief 属性蓝图，存储单位的基础战斗属性
 */
struct StatsBlueprint {
    float hp_ = 0.0f;
    float atk_ = 0.0f;
    float def_ = 0.0f;
    float range_ = 0.0f;
    float atk_interval_ = 0.0f;
};

/**
 * @struct SpriteBlueprint
 * @brief 精灵蓝图，存储单位的外观渲染信息
 */
struct SpriteBlueprint {
    entt::id_type id_ = entt::null;
    std::string path_;
    engine::utils::Rect src_rect_;
    glm::vec2 size_;
    glm::vec2 offset_;
    bool face_right_ = true;
};

/**
 * @struct AnimationBlueprint
 * @brief 动画蓝图，存储单个动画序列的配置
 */
struct AnimationBlueprint {
    float ms_per_frame_ = 100.0f;
    int row_ = 0;
    std::vector<int> frames_;
};

/**
 * @struct SoundBlueprint
 * @brief 音效蓝图，存储动作到音效的映射
 */
struct SoundBlueprint {
    std::unordered_map<entt::id_type, entt::id_type> sounds_;
};

/**
 * @struct EnemyBlueprint
 * @brief 敌人特性蓝图，存储敌人特有的属性
 */
struct EnemyBlueprint {
    bool ranged_ = false;
    float speed_ = 0.0f;
};

/**
 * @struct DisplayInfoBlueprint
 * @brief 显示信息蓝图，存储单位的名称和描述
 */
struct DisplayInfoBlueprint {
    std::string name_;
    std::string description_;
};

/**
 * @struct EnemyClassBlueprint
 * @brief 敌人类型完整蓝图，组合所有子蓝图
 */
struct EnemyClassBlueprint {
    entt::id_type class_id_ = entt::null;
    std::string class_name_;
    StatsBlueprint stats_;
    EnemyBlueprint enemy_;
    SoundBlueprint sounds_;
    SpriteBlueprint sprite_;
    DisplayInfoBlueprint display_info_;
    std::unordered_map<entt::id_type, AnimationBlueprint> animations_;
};

} // namespace game::data
