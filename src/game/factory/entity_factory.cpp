#include "entity_factory.h"
#include "../component/stats_component.h"
#include "../component/class_name_component.h"
#include "../component/enemy_component.h"
#include "../defs/tags.h"
#include "../../engine/component/transform_component.h"
#include "../../engine/component/sprite_component.h"
#include "../../engine/component/animation_component.h"
#include "../../engine/component/audio_component.h"
#include "../../engine/component/render_component.h"
#include "../../engine/component/velocity_component.h"
#include "../../engine/utils/math.h"
#include <spdlog/spdlog.h>

namespace game::factory {

EntityFactory::EntityFactory(entt::registry& registry, const BlueprintManager& blueprint_manager)
    : registry_(registry), blueprint_manager_(blueprint_manager) {
}

entt::entity EntityFactory::createEnemyUnit(entt::id_type class_id,
                                            const glm::vec2& position,
                                            int target_waypoint_id,
                                            int level,
                                            int rarity) {
    // 检查蓝图是否存在
    if (!blueprint_manager_.hasEnemyClassBlueprint(class_id)) {
        spdlog::error("找不到敌人类型ID: {}", class_id);
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

    spdlog::info("创建敌人单位: {} (等级: {}, 稀有度: {})", blueprint.display_info_.name_, level, rarity);
    return entity;
}

void EntityFactory::addTransformComponent(entt::entity entity, const glm::vec2& position) {
    registry_.emplace<engine::component::TransformComponent>(entity, position);
}

void EntityFactory::addVelocityComponent(entt::entity entity) {
    registry_.emplace<engine::component::VelocityComponent>(entity, glm::vec2(0.0f, 0.0f));
}

void EntityFactory::addSpriteComponent(entt::entity entity, const data::SpriteBlueprint& sprite) {
    spdlog::debug("创建SpriteComponent: path={}, src_rect=[{},{},{},{}], size=[{},{}], offset=[{},{}]",
                  sprite.path_, sprite.src_rect_.position.x, sprite.src_rect_.position.y,
                  sprite.src_rect_.size.x, sprite.src_rect_.size.y,
                  sprite.size_.x, sprite.size_.y, sprite.offset_.x, sprite.offset_.y);
    
    engine::component::Sprite sprite_data(sprite.path_, sprite.src_rect_, !sprite.face_right_);
    registry_.emplace<engine::component::SpriteComponent>(entity, std::move(sprite_data), sprite.size_, sprite.offset_);
}

void EntityFactory::addAnimationComponent(entt::entity entity,
                                          const std::unordered_map<entt::id_type, data::AnimationBlueprint>& animations,
                                          const data::SpriteBlueprint& sprite,
                                          entt::id_type default_anim_id) {
    std::unordered_map<entt::id_type, engine::component::Animation> anim_map;

    for (const auto& [anim_id, anim_blueprint] : animations) {
        std::vector<engine::component::AnimationFrame> frames;

        float frame_width = sprite.src_rect_.size.x;
        float frame_height = sprite.src_rect_.size.y;

        for (int frame_idx : anim_blueprint.frames_) {
            float x = frame_idx * frame_width;
            float y = anim_blueprint.row_ * frame_height;
            engine::utils::Rect frame_rect{x, y, frame_width, frame_height};
            frames.emplace_back(frame_rect, anim_blueprint.ms_per_frame_);
        }

        anim_map.emplace(anim_id, engine::component::Animation{std::move(frames), true});
    }

    // 如果没有指定默认动画，使用第一个可用的动画
    if (default_anim_id == entt::null && !anim_map.empty()) {
        default_anim_id = anim_map.begin()->first;
    }

    registry_.emplace<engine::component::AnimationComponent>(entity, std::move(anim_map), default_anim_id);
}

void EntityFactory::addAudioComponent(entt::entity entity, const data::SoundBlueprint& sounds) {
    engine::component::AudioComponent audio;
    audio.action_sounds_ = sounds.sounds_;
    registry_.emplace<engine::component::AudioComponent>(entity, std::move(audio));
}

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
    registry_.emplace<game::component::StatsComponent>(entity, std::move(stats_comp));
}

void EntityFactory::addEnemyComponent(entt::entity entity, const data::EnemyBlueprint& enemy, int target_waypoint_id) {
    registry_.emplace<game::component::EnemyComponent>(entity, target_waypoint_id, enemy.speed_);
}

void EntityFactory::addRenderComponent(entt::entity entity, int layer_index) {
    registry_.emplace<engine::component::RenderComponent>(entity, layer_index);
}

} // namespace game::factory
