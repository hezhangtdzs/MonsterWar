#pragma once
#include <entt/entity/registry.hpp>
#include <entt/core/hashed_string.hpp>
#include <glm/vec2.hpp>
#include <string_view>
#include "blueprint_manager.h"

namespace game::factory {

/**
 * @class EntityFactory
 * @brief 实体工厂，负责根据蓝图创建游戏实体
 *
 * @details
 * EntityFactory 使用 BlueprintManager 提供的蓝图数据，按步骤将组件装配到实体上。
 * 这是数据驱动架构的核心，将"创建逻辑"与"数据配置"分离。
 */
class EntityFactory {
public:
    /**
     * @brief 构造函数
     * @param registry EnTT 注册表引用
     * @param blueprint_manager 蓝图管理器引用
     */
    EntityFactory(entt::registry& registry, const BlueprintManager& blueprint_manager);

    /**
     * @brief 创建敌人单位
     * @param class_id 敌人类型ID（使用 entt::hashed_string 生成，如 "wolf"_hs）
     * @param position 初始位置
     * @param target_waypoint_id 目标路径点ID
     * @param level 等级（默认为1）
     * @param rarity 稀有度（默认为1）
     * @return 创建的实体句柄
     */
    entt::entity createEnemyUnit(entt::id_type class_id,
                                 const glm::vec2& position,
                                 int target_waypoint_id,
                                 int level = 1,
                                 int rarity = 1);

private:
    entt::registry& registry_;
    const BlueprintManager& blueprint_manager_;

    // 组件装配辅助函数
    void addTransformComponent(entt::entity entity, const glm::vec2& position);
    void addSpriteComponent(entt::entity entity, const data::SpriteBlueprint& sprite);
    void addAnimationComponent(entt::entity entity,
                               const std::unordered_map<entt::id_type, data::AnimationBlueprint>& animations,
                               const data::SpriteBlueprint& sprite,
                               entt::id_type default_anim_id);
    void addAudioComponent(entt::entity entity, const data::SoundBlueprint& sounds);
    void addStatsComponent(entt::entity entity, const data::StatsBlueprint& stats, int level, int rarity);
    void addEnemyComponent(entt::entity entity, const data::EnemyBlueprint& enemy, int target_waypoint_id);
    void addRenderComponent(entt::entity entity, int layer_index);
    void addVelocityComponent(entt::entity entity);
};

} // namespace game::factory
