#pragma once

namespace game::component {

/**
 * @struct StatsComponent
 * @brief 属性组件，存储实体的战斗属性
 *
 * @details
 * StatsComponent 是ECS架构中的战斗属性组件，负责存储和管理实体的生命值、
 * 攻击力、防御力等战斗相关数据。该组件由战斗系统查询和修改。
 */
struct StatsComponent {
    float hp_ = 0.0f;           ///< 当前生命值
    float max_hp_ = 0.0f;       ///< 最大生命值
    float atk_ = 0.0f;          ///< 攻击力
    float def_ = 0.0f;          ///< 防御力
    float range_ = 0.0f;        ///< 攻击范围
    float atk_interval_ = 0.0f; ///< 攻击间隔（秒）
    float atk_timer_ = 0.0f;    ///< 攻击计时器
    int level_ = 1;             ///< 等级
    int rarity_ = 1;            ///< 稀有度
};

} // namespace game::component
