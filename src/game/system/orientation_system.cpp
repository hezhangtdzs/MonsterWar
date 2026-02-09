#include "orientation_system.h"
#include "engine/component/transform_component.h"
#include "engine/component/sprite_component.h"
#include "engine/component/velocity_component.h"
#include "game/component/target_component.h"
#include "game/component/blocked_by_component.h"
#include "game/defs/tags.h"

namespace game::system {

using namespace engine::component;
using namespace game::component;
using namespace game::defs;

void OrientationSystem::update(entt::registry& registry) {
    auto view = registry.view<SpriteComponent, TransformComponent>();

    for (auto entity : view) {
        auto& sprite = view.get<SpriteComponent>(entity).sprite_;
        auto& pos = view.get<TransformComponent>(entity).position_;
        
        bool face_left = false;
        bool has_priority_target = false;

        // 1. 如果有锁定目标，面朝目标
        if (auto* target = registry.try_get<TargetComponent>(entity); target && registry.valid(target->entity_)) {
            if (auto* target_transform = registry.try_get<TransformComponent>(target->entity_)) {
                face_left = (target_transform->position_.x < pos.x);
                has_priority_target = true;
            }
        }

        // 2. 如果被阻挡，面朝阻挡者 (仅当没有锁定目标时)
        if (!has_priority_target) {
            if (auto* blocked = registry.try_get<BlockedByComponent>(entity); blocked && registry.valid(blocked->entity_)) {
                if (auto* blocker_transform = registry.try_get<TransformComponent>(blocked->entity_)) {
                    face_left = (blocker_transform->position_.x < pos.x);
                    has_priority_target = true;
                }
            }
        }

        // 3. 否则，如果是移动中的实体，面朝移动方向
        if (!has_priority_target) {
            if (auto* vel = registry.try_get<VelocityComponent>(entity)) {
                if (std::abs(vel->velocity_.x) > 0.1f) {
                    face_left = (vel->velocity_.x < 0.0f);
                } else {
                    // 如果速度几乎为0，保持当前朝向，不作处理
                    continue; 
                }
            } else {
                // 既没有目标也没有速度，不处理
                continue;
            }
        }

        // 应用 FaceLeftTag 修正 (如果素材默认朝左，则翻转逻辑反过来)
        if (registry.all_of<FaceLeftTag>(entity)) {
            sprite.is_flipped_ = !face_left;
        } else {
            sprite.is_flipped_ = face_left;
        }
    }
}

} // namespace game::system
