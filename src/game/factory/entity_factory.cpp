/**
 * @file entity_factory.cpp
 * @brief EntityFactory 类的实现，负责根据蓝图创建游戏实体。
 * 
 * @details
 * 本文件实现了 EntityFactory 类的所有方法，包括：
 * - 构造函数
 * - 创建敌人单位的主方法
 * - 各种组件装配辅助方法
 * 
 * EntityFactory 是游戏工厂系统的核心，实现了数据驱动的实体创建：
 * - 从 BlueprintManager 获取蓝图数据
 * - 按顺序装配各种组件
 * - 根据蓝图数据设置组件属性
 * - 处理特殊情况（如朝向、攻击类型）
 * 
 * @see game::factory::EntityFactory 实体工厂类定义
 * @see game::factory::BlueprintManager 提供蓝图数据的管理器
 */

#include "entity_factory.h"
#include "../component/stats_component.h"
#include "../component/class_name_component.h"
#include "../component/enemy_component.h"
#include "../component/player_component.h"
#include "../component/hero_skill_component.h"
#include "../component/projectile_visual_component.h"
#include "../component/unit_prep_component.h"
#include "../component/blocker_component.h"
#include "../defs/tags.h"
#include "../factory/blueprint_manager.h"
#include "../../engine/component/transform_component.h"
#include "../../engine/component/sprite_component.h"
#include "../../engine/component/animation_component.h"
#include "../../engine/component/audio_component.h"
#include "../../engine/component/render_component.h"
#include "../../engine/component/velocity_component.h"
#include "../../engine/utils/math.h"
#include "../../engine/utils/logging.h"
#include <cmath>

namespace game::factory {

/**
 * @brief EntityFactory 构造函数
 * @param registry EnTT 注册表引用，用于创建和管理实体
 * @param blueprint_manager 蓝图管理器引用，用于获取实体蓝图数据
 */
EntityFactory::EntityFactory(entt::registry& registry, const BlueprintManager& blueprint_manager)
    : registry_(registry), blueprint_manager_(blueprint_manager) {
    ENGINE_LOG_INFO("EntityFactory initialized");
}

/**
 * @brief 创建敌人单位
 * @param class_id 敌人类型ID（使用 entt::hashed_string 生成，如 "wolf"_hs）
 * @param position 初始位置
 * @param target_waypoint_id 目标路径点ID
 * @param level 等级（默认为1）
 * @param rarity 稀有度（默认为1）
 * @return 创建的实体句柄
 * 
 * @details
 * 该方法实现了敌人实体的完整创建流程：
 * 1. 检查蓝图是否存在
 * 2. 创建新实体
 * 3. 按顺序装配各种组件：
 *    - TransformComponent（位置）
 *    - VelocityComponent（速度）
 *    - SpriteComponent（外观）
 *    - AnimationComponent（动画）
 *    - AudioComponent（音效）
 *    - StatsComponent（属性）
 *    - EnemyComponent（敌人特定属性）
 *    - ClassNameComponent（类型标识）
 *    - RenderComponent（渲染排序）
 * 4. 根据蓝图数据添加标签（朝向、攻击类型）
 * 5. 记录创建结果
 * 
 * @par 组件装配顺序
 * 组件装配顺序很重要，因为某些组件可能依赖于其他组件的数据。
 * 本方法按照从基础到特定的顺序装配组件：
 * 1. 变换和运动组件
 * 2. 视觉和动画组件
 * 3. 音频和统计组件
 * 4. 敌人特定组件
 * 5. 类型和渲染组件
 */
entt::entity EntityFactory::createEnemyUnit(entt::id_type class_id,
                                            const glm::vec2& position,
                                            int target_waypoint_id,
                                            int level,
                                            int rarity) {
    // 检查蓝图是否存在
    if (!blueprint_manager_.hasEnemyClassBlueprint(class_id)) {
        ENGINE_LOG_ERROR("找不到敌人类型ID: {}", class_id);
        return entt::null;
    }

    const auto& blueprint = blueprint_manager_.getEnemyClassBlueprint(class_id);

    // 创建实体
    entt::entity entity = registry_.create();

    // 按顺序装配组件
    addTransformComponent(entity, position);
    addVelocityComponent(entity);
    addSpriteComponent(entity, blueprint.sprite_);
    addAnimationComponent(entity, blueprint.animations_, blueprint.sprite_, entt::hashed_string("walk"));
    addAudioComponent(entity, blueprint.sounds_);
    addStatsComponent(entity, blueprint.stats_, level, rarity);
    addEnemyComponent(entity, blueprint.enemy_, target_waypoint_id);

    // 添加类名组件
    registry_.emplace<game::component::ClassNameComponent>(entity, class_id, blueprint.display_info_.name_);

    // 添加渲染组件（默认主战斗图层编号为10）
    addRenderComponent(entity, 10);

    // 根据朝向添加标签
    if (!blueprint.sprite_.face_right_) {
        registry_.emplace<game::defs::FaceLeftTag>(entity);
    }

    // 根据攻击类型添加标签
    if (blueprint.enemy_.ranged_) {
        registry_.emplace<game::defs::RangedUnitTag>(entity);
    } else {
        registry_.emplace<game::defs::MeleeUnitTag>(entity);
    }

    ENGINE_LOG_INFO("创建敌人单位: {} (等级: {}, 稀有度: {})", blueprint.display_info_.name_, level, rarity);
    return entity;
}

/**
 * @brief 创建玩家单位
 * @param class_id 玩家类型ID（使用 entt::hashed_string 生成，如 "archer"_hs）
 * @param position 初始位置
 * @param level 等级（默认为1）
 * @param rarity 稀有度（默认为1）
 * @return 创建的实体句柄
 * 
 * @details
 * 该方法实现了玩家实体的完整创建流程：
 * 1. 检查蓝图是否存在
 * 2. 创建新实体
 * 3. 按顺序装配各种组件
 * 4. 记录创建结果
 */
entt::entity EntityFactory::createPlayerUnit(entt::id_type class_id,
                                             const glm::vec2& position,
                                             int level,
                                             int rarity) {
    // 检查蓝图是否存在
    if (!blueprint_manager_.hasPlayerClassBlueprint(class_id)) {
        ENGINE_LOG_ERROR("找不到玩家类型ID: {}", class_id);
        return entt::null;
    }

    const auto& blueprint = blueprint_manager_.getPlayerClassBlueprint(class_id);

    // 创建实体
    entt::entity entity = registry_.create();

    // 按顺序装配组件
    addTransformComponent(entity, position);
    addVelocityComponent(entity);
    addSpriteComponent(entity, blueprint.sprite_);
    // 玩家默认动画通常是 idle
    addAnimationComponent(entity, blueprint.animations_, blueprint.sprite_, entt::hashed_string("idle"));
    addAudioComponent(entity, blueprint.sounds_);
    addStatsComponent(entity, blueprint.stats_, level, rarity);
    addPlayerComponent(entity, blueprint.player_, rarity);
    if (blueprint.player_.skill_id_ != entt::null && blueprint_manager_.hasSkillBlueprint(blueprint.player_.skill_id_)) {
        const auto& skill = blueprint_manager_.getSkillBlueprint(blueprint.player_.skill_id_);
        registry_.emplace<game::component::HeroSkillComponent>(entity,
            skill.skill_id_,
            skill.cooldown_,
            0.0f,
            skill.duration_,
            0.0f,
            skill.passive_,
            false,
            skill.atk_multiplier_,
            skill.def_multiplier_,
            skill.range_multiplier_,
            skill.atk_interval_multiplier_);
        if (skill.passive_) {
            registry_.emplace<game::defs::PassiveSkillTag>(entity);
            registry_.emplace<game::defs::SkillReadyTag>(entity);
        }
    }

    // 添加类名组件
    registry_.emplace<game::component::ClassNameComponent>(entity, class_id, blueprint.display_info_.name_);

    // 添加渲染组件
    addRenderComponent(entity, 10);

    // 根据朝向添加标签
    if (!blueprint.sprite_.face_right_) {
        registry_.emplace<game::defs::FaceLeftTag>(entity);
    }

    ENGINE_LOG_INFO("创建玩家单位过程完成: {} (等级: {}, 稀有度: {})", blueprint.display_info_.name_, level, rarity);
    return entity;
}

entt::entity EntityFactory::createUnitPrep(entt::id_type name_id,
                                          entt::id_type class_id,
                                          const glm::vec2& position,
                                          int cost,
                                          int level,
                                          int rarity) {
    if (!blueprint_manager_.hasPlayerClassBlueprint(class_id)) {
        ENGINE_LOG_ERROR("找不到玩家类型ID: {}", class_id);
        return entt::null;
    }

    const auto& blueprint = blueprint_manager_.getPlayerClassBlueprint(class_id);
    entt::entity entity = registry_.create();

    addTransformComponent(entity, position);
    addSpriteComponent(entity, blueprint.sprite_);
    addAnimationComponent(entity, blueprint.animations_, blueprint.sprite_, entt::hashed_string("idle"));

    registry_.emplace<game::component::UnitPrepComponent>(entity,
        name_id,
        class_id,
        blueprint.player_.type_,
        cost,
        blueprint.stats_.range_,
        level,
        rarity);

    addRenderComponent(entity, 100);

    if (!blueprint.sprite_.face_right_) {
        registry_.emplace<game::defs::FaceLeftTag>(entity);
    }

    if (blueprint.player_.type_ == game::defs::PlayerType::RANGED) {
        registry_.emplace<game::defs::ShowRangeTag>(entity);
    }

    ENGINE_LOG_INFO("创建准备单位: {} (cost={}, level={}, rarity={})", blueprint.display_info_.name_, cost, level, rarity);
    return entity;
}

int EntityFactory::getPlayerUnitCost(entt::id_type class_id, int rarity) const {
    if (!blueprint_manager_.hasPlayerClassBlueprint(class_id)) {
        spdlog::error("找不到玩家类型ID: {}", class_id);
        return 0;
    }

    const auto& blueprint = blueprint_manager_.getPlayerClassBlueprint(class_id);
    return static_cast<int>(std::round(blueprint.player_.cost_ * (0.9f + 0.1f * rarity)));
}

/**
 * @brief 添加变换组件
 * @param entity 目标实体
 * @param position 初始位置
 */
void EntityFactory::addTransformComponent(entt::entity entity, const glm::vec2& position) {
    registry_.emplace_or_replace<engine::component::TransformComponent>(entity, position);
}

/**
 * @brief 添加速度组件
 * @param entity 目标实体
 * 
 * @details
 * 初始速度设置为 (0.0f, 0.0f)，敌人的实际速度将由 FollowPathSystem 根据路径计算。
 */
void EntityFactory::addVelocityComponent(entt::entity entity) {
    registry_.emplace_or_replace<engine::component::VelocityComponent>(entity, glm::vec2(0.0f, 0.0f));
}

/**
 * @brief 添加精灵组件
 * @param entity 目标实体
 * @param sprite 精灵蓝图数据
 */
void EntityFactory::addSpriteComponent(entt::entity entity, const data::SpriteBlueprint& sprite) {
    spdlog::debug("创建SpriteComponent: path={}, src_rect=[{},{},{},{}], size=[{},{}], offset=[{},{}]",
                  sprite.path_, sprite.src_rect_.position.x, sprite.src_rect_.position.y,
                  sprite.src_rect_.size.x, sprite.src_rect_.size.y,
                  sprite.size_.x, sprite.size_.y, sprite.offset_.x, sprite.offset_.y);
    
    engine::component::Sprite sprite_data(sprite.path_, sprite.src_rect_, !sprite.face_right_);
    registry_.emplace_or_replace<engine::component::SpriteComponent>(entity, std::move(sprite_data), sprite.size_, sprite.offset_);
}

/**
 * @brief 添加动画组件
 * @param entity 目标实体
 * @param animations 动画蓝图映射
 * @param sprite 精灵蓝图数据（用于计算帧大小）
 * @param default_anim_id 默认动画ID
 */
void EntityFactory::addAnimationComponent(entt::entity entity,
                                          const std::unordered_map<entt::id_type, data::AnimationBlueprint>& animations,
                                          const data::SpriteBlueprint& sprite,
                                          entt::id_type default_anim_id) {
    std::unordered_map<entt::id_type, engine::component::Animation> anim_map;

    for (const auto& [anim_id, anim_blueprint] : animations) {
        //float ms_per_frame_ = 100.0f;
        //int row_ = 0;
        //std::unordered_map<int, entt::id_type> events_;
        //std::vector<int> frames_;
        std::vector<engine::component::AnimationFrame> frames;
        std::unordered_map<int, entt::id_type> events;

        float frame_width = sprite.src_rect_.size.x;
        float frame_height = sprite.src_rect_.size.y;
        events = anim_blueprint.events_;
        for (int frame_idx : anim_blueprint.frames_) {
            float x = frame_idx * frame_width;
            float y = anim_blueprint.row_ * frame_height;
            engine::utils::Rect frame_rect{x, y, frame_width, frame_height};
            frames.emplace_back(frame_rect, anim_blueprint.ms_per_frame_);
        }

        anim_map.emplace(anim_id, engine::component::Animation{std::move(frames),std::move(events), true});
    }

    // 如果没有指定默认动画，使用第一个可用的动画
    if (default_anim_id == entt::null && !anim_map.empty()) {
        default_anim_id = anim_map.begin()->first;
    }

    registry_.emplace_or_replace<engine::component::AnimationComponent>(entity, std::move(anim_map), default_anim_id);
}

/**
 * @brief 添加音频组件
 * @param entity 目标实体
 * @param sounds 音效蓝图数据
 */
void EntityFactory::addAudioComponent(entt::entity entity, const data::SoundBlueprint& sounds) {
    engine::component::AudioComponent audio;
    audio.action_sounds_ = sounds.sounds_;
    registry_.emplace_or_replace<engine::component::AudioComponent>(entity, std::move(audio));
}

/**
 * @brief 添加统计组件
 * @param entity 目标实体
 * @param stats 统计蓝图数据
 * @param level 等级
 * @param rarity 稀有度
 * 
 * @details
 * 该方法使用 statModify 函数根据等级和稀有度调整实体属性，
 * 实现敌人强度的动态调整。
 */
void EntityFactory::addStatsComponent(entt::entity entity, const data::StatsBlueprint& stats, int level, int rarity) {
    game::component::StatsComponent stats_comp;
    stats_comp.max_hp_ = engine::utils::statModify(stats.hp_, level, rarity);
    stats_comp.hp_ = stats_comp.max_hp_;
    stats_comp.atk_ = engine::utils::statModify(stats.atk_, level, rarity);
    stats_comp.def_ = engine::utils::statModify(stats.def_, level, rarity);
    stats_comp.range_ = stats.range_;
    stats_comp.atk_interval_ = stats.atk_interval_;
    stats_comp.atk_timer_ = 0.0f;
    stats_comp.level_ = level;
    stats_comp.rarity_ = rarity;
    registry_.emplace_or_replace<game::component::StatsComponent>(entity, std::move(stats_comp));
}

/**
 * @brief 添加敌人组件
 * @param entity 目标实体
 * @param enemy 敌人蓝图数据
 * @param target_waypoint_id 目标路径点ID
 */
void EntityFactory::addEnemyComponent(entt::entity entity, const data::EnemyBlueprint& enemy, int target_waypoint_id) {
    registry_.emplace_or_replace<game::component::EnemyComponent>(entity, target_waypoint_id, enemy.speed_, enemy.projectile_id_);
}

void EntityFactory::addPlayerComponent(entt::entity entity, const data::PlayerBlueprint &player, int rarity)
{
    auto cost = static_cast<int>(std::round(player.cost_ * (0.9f + 0.1f * rarity)));
    registry_.emplace_or_replace<game::component::PlayerComponent>(entity, cost, player.projectile_id_);
    
    if(player.type_ == game::defs::PlayerType::MELEE) {
        registry_.emplace_or_replace<game::defs::MeleeUnitTag>(entity);
        registry_.emplace_or_replace<game::component::BlockerComponent>(entity, player.block_);
    } else if(player.type_ == game::defs::PlayerType::RANGED) {
        registry_.emplace_or_replace<game::defs::RangedUnitTag>(entity);
    } 
    
    if(player.is_healer_) {
        registry_.emplace_or_replace<game::defs::HealerTag>(entity);
    }
}

entt::entity EntityFactory::createProjectileVisual(entt::id_type projectile_id,
                                                   const glm::vec2& source_position,
                                                   const glm::vec2& target_position,
                                                   const glm::vec2& target_velocity)
{
    if (!blueprint_manager_.hasProjectileBlueprint(projectile_id)) {
        ENGINE_LOG_WARN("找不到投射物蓝图: {}", projectile_id);
        return entt::null;
    }

    const auto& blueprint = blueprint_manager_.getProjectileBlueprint(projectile_id);
    auto entity = [&]() {
        auto view = registry_.view<game::defs::VisualEffectTag, game::defs::InactiveVisualTag>();
        if (auto it = view.begin(); it != view.end()) {
            auto reused = *it;
            registry_.remove<game::defs::InactiveVisualTag>(reused);
            return reused;
        }
        return registry_.create();
    }();

    if (!registry_.all_of<game::defs::VisualEffectTag>(entity)) {
        registry_.emplace_or_replace<game::defs::VisualEffectTag>(entity);
    }

    const glm::vec2 delta = target_position - source_position;
    const float delta_len = std::sqrt(delta.x * delta.x + delta.y * delta.y);
    const glm::vec2 direction = delta_len > 0.001f ? delta / delta_len : glm::vec2{ 1.0f, 0.0f };
    const glm::vec2 start_position = source_position + direction * 24.0f;

    addTransformComponent(entity, start_position);
    auto sprite = blueprint.sprite_;
    sprite.face_right_ = true;
    addSpriteComponent(entity, sprite);
    addRenderComponent(entity, 12);

    const float flight_time = blueprint.total_flight_time_ > 0.1f ? blueprint.total_flight_time_ : 0.1f;
    const glm::vec2 lead_target_position = target_position + target_velocity * flight_time;
    const glm::vec2 lead_delta = lead_target_position - start_position;
    const float initial_tangent_y = lead_delta.y - blueprint.arc_height_ * 4.0f;
    registry_.emplace_or_replace<game::component::ProjectileVisualComponent>(
        entity,
        start_position,
        lead_target_position,
        blueprint.arc_height_,
        flight_time,
        0.0f,
        blueprint.rotation_offset_deg_);

    auto& transform = registry_.get<engine::component::TransformComponent>(entity);
    constexpr float rad_to_deg = 180.0f / 3.14159265358979323846f;
    transform.rotation_ = std::atan2(initial_tangent_y, lead_delta.x) * rad_to_deg + blueprint.rotation_offset_deg_;

    std::vector<engine::component::AnimationFrame> frames;
    frames.emplace_back(blueprint.sprite_.src_rect_, flight_time * 1000.0f);
    std::unordered_map<entt::id_type, engine::component::Animation> animations;
    animations.emplace(entt::hashed_string("fly"), engine::component::Animation{ std::move(frames), {}, false });
    registry_.emplace_or_replace<engine::component::AnimationComponent>(entity, std::move(animations), entt::hashed_string("fly"));
    registry_.emplace_or_replace<game::defs::VisualEffectTag>(entity);
    return entity;
}

entt::entity EntityFactory::createEffectVisual(entt::id_type effect_id,
                                               const glm::vec2& position)
{
    if (!blueprint_manager_.hasEffectBlueprint(effect_id)) {
        ENGINE_LOG_WARN("找不到特效蓝图: {}", effect_id);
        return entt::null;
    }

    const auto& blueprint = blueprint_manager_.getEffectBlueprint(effect_id);
    auto entity = registry_.create();

    if (!registry_.all_of<game::defs::VisualEffectTag>(entity)) {
        registry_.emplace<game::defs::VisualEffectTag>(entity);
    }

    addTransformComponent(entity, position);
    addSpriteComponent(entity, blueprint.sprite_);
    addRenderComponent(entity, 12);

    std::vector<engine::component::AnimationFrame> frames;
    for (int frame_index : blueprint.animation_.frames_) {
        const float x = blueprint.sprite_.src_rect_.position.x + blueprint.sprite_.src_rect_.size.x * static_cast<float>(frame_index);
        const float y = blueprint.sprite_.src_rect_.position.y + blueprint.animation_.row_ * blueprint.sprite_.src_rect_.size.y;
        engine::utils::Rect frame_rect{ x, y, blueprint.sprite_.src_rect_.size.x, blueprint.sprite_.src_rect_.size.y };
        frames.emplace_back(frame_rect, blueprint.animation_.ms_per_frame_);
    }

    std::unordered_map<entt::id_type, engine::component::Animation> animations;
    animations.emplace(blueprint.effect_id_, engine::component::Animation{ std::move(frames), {}, false });
    registry_.emplace_or_replace<engine::component::AnimationComponent>(entity, std::move(animations), blueprint.effect_id_);
    registry_.emplace_or_replace<game::defs::VisualEffectTag>(entity);
    return entity;
}

/**
 * @brief 添加渲染组件
 * @param entity 目标实体
 * @param layer_index 渲染图层索引
 * 
 * @details
 * 渲染图层索引决定了实体的渲染顺序，较小的索引先渲染（在底层），
 * 较大的索引后渲染（在上层）。
 */
void EntityFactory::addRenderComponent(entt::entity entity, int layer_index) {
    registry_.emplace_or_replace<engine::component::RenderComponent>(entity, layer_index);
}

} // namespace game::factory
