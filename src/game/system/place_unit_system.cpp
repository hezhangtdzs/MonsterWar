#include "place_unit_system.h"

#include "../component/place_occupied_component.h"
#include "../component/unit_prep_component.h"
#include "../component/player_component.h"
#include "../component/stats_component.h"
#include "../defs/event.h"
#include "../defs/tags.h"
#include "../factory/entity_factory.h"
#include "../../engine/component/render_component.h"
#include "../../engine/component/transform_component.h"
#include "../../engine/component/sprite_component.h"
#include "../../engine/core/context.h"
#include "../../engine/input/input_manager.h"
#include "../../engine/audio/audio_locator.h"
#include "../../engine/utils/logging.h"
#include <algorithm>
#include <limits>

using namespace entt::literals;

namespace game::system {

PlaceUnitSystem::PlaceUnitSystem(entt::registry& registry,
                                 entt::dispatcher& dispatcher,
                                 engine::core::Context& context,
                                 game::factory::EntityFactory& entity_factory)
    : registry_(registry),
      dispatcher_(dispatcher),
      context_(context),
      entity_factory_(entity_factory) {
    dispatcher_.sink<game::defs::PrepUnitEvent>().connect<&PlaceUnitSystem::onPrepUnitEvent>(this);
    dispatcher_.sink<game::defs::RemovePlayerUnitEvent>().connect<&PlaceUnitSystem::onRemovePlayerUnitEvent>(this);
    context_.getInputManager().onAction("mouse_left"_hs).connect<&PlaceUnitSystem::onPlaceUnit>(this);
    context_.getInputManager().onAction("mouse_right"_hs).connect<&PlaceUnitSystem::onCancelPrepUnit>(this);
}

PlaceUnitSystem::~PlaceUnitSystem() {
    dispatcher_.disconnect(this);
    context_.getInputManager().onAction("mouse_left"_hs).disconnect<&PlaceUnitSystem::onPlaceUnit>(this);
    context_.getInputManager().onAction("mouse_right"_hs).disconnect<&PlaceUnitSystem::onCancelPrepUnit>(this);
}

void PlaceUnitSystem::update(float) {
    if (!registry_.valid(active_prep_entity_)) {
        active_prep_entity_ = entt::null;
        target_place_entity_ = entt::null;
        return;
    }

    auto* transform = registry_.try_get<engine::component::TransformComponent>(active_prep_entity_);
    auto* render = registry_.try_get<engine::component::RenderComponent>(active_prep_entity_);
    auto* prep = registry_.try_get<game::component::UnitPrepComponent>(active_prep_entity_);
    if (!transform || !render || !prep) {
        clearActivePrep();
        return;
    }

    const auto mouse_world = context_.getCamera().screenToWorld(context_.getInputManager().getLogicalMousePosition());
    transform->position_ = mouse_world;
    target_place_entity_ = findNearestPlace(mouse_world, prep->type_);
    render->color_ = target_place_entity_ != entt::null
        ? engine::utils::FColor{ 0.32f, 1.0f, 0.32f, 1.0f }
        : engine::utils::FColor{ 1.0f, 0.34f, 0.34f, 1.0f };
}

void PlaceUnitSystem::onPrepUnitEvent(const game::defs::PrepUnitEvent& event) {
    clearActivePrep();

    const auto mouse_world = context_.getCamera().screenToWorld(context_.getInputManager().getLogicalMousePosition());
    active_prep_entity_ = entity_factory_.createUnitPrep(event.name_id_, event.class_id_, mouse_world, event.cost_, event.level_, event.rarity_);
    if (!registry_.valid(active_prep_entity_)) {
        active_prep_entity_ = entt::null;
        target_place_entity_ = entt::null;
        return;
    }

    ENGINE_LOG_INFO("进入出击准备: name_id={}, class_id={}, cost={}", event.name_id_, event.class_id_, event.cost_);
}

void PlaceUnitSystem::onRemovePlayerUnitEvent(const game::defs::RemovePlayerUnitEvent& event) {
    if (!registry_.valid(event.entity_)) {
        return;
    }

    removePlaceOccupancy(event.entity_);

    if (event.entity_ == active_prep_entity_) {
        clearActivePrep();
    }

    if (!registry_.all_of<game::defs::DeadTag>(event.entity_)) {
        registry_.emplace_or_replace<game::defs::DeadTag>(event.entity_);
    }
}

bool PlaceUnitSystem::onPlaceUnit() {
    if (!registry_.valid(active_prep_entity_) || target_place_entity_ == entt::null) {
        return false;
    }

    auto* prep = registry_.try_get<game::component::UnitPrepComponent>(active_prep_entity_);
    auto* place_transform = registry_.try_get<engine::component::TransformComponent>(target_place_entity_);
    const auto* place_sprite = registry_.try_get<engine::component::SpriteComponent>(target_place_entity_);
    if (!prep || !place_transform) {
        return false;
    }

    if (registry_.all_of<game::component::PlaceOccupiedComponent>(target_place_entity_)) {
        return false;
    }

    const glm::vec2 place_center = place_transform->position_ + (place_sprite ? (place_sprite->size_ * 0.5f) : glm::vec2{ 32.0f, 32.0f });
    const auto player_entity = entity_factory_.createPlayerUnit(prep->class_id_, place_center, prep->level_, prep->rarity_);
    if (player_entity == entt::null) {
        return false;
    }

    auto& occupied = registry_.emplace_or_replace<game::component::PlaceOccupiedComponent>(target_place_entity_);
    occupied.occupied_by_ = player_entity;

    if (const auto* place_render = registry_.try_get<engine::component::RenderComponent>(target_place_entity_)) {
        if (auto* player_render = registry_.try_get<engine::component::RenderComponent>(player_entity)) {
            player_render->layer_index_ = place_render->layer_index_ + 1;
        }
    }

    dispatcher_.enqueue(game::defs::RemoveUIPortraitEvent{ prep->name_id_ });
    dispatcher_.enqueue(game::defs::RemovePlayerUnitEvent{ active_prep_entity_ });

    engine::audio::AudioLocator::get().playSound(entt::hashed_string("unit_placed").value());

    ENGINE_LOG_INFO("出击确认成功: name_id={}, place_entity={}, player_entity={}", prep->name_id_, entt::to_integral(target_place_entity_), entt::to_integral(player_entity));
    clearActivePrep();
    return true;
}

bool PlaceUnitSystem::onCancelPrepUnit() {
    if (!registry_.valid(active_prep_entity_)) {
        return false;
    }

    dispatcher_.enqueue(game::defs::RemovePlayerUnitEvent{ active_prep_entity_ });
    clearActivePrep();
    ENGINE_LOG_INFO("已取消出击准备");
    return true;
}

entt::entity PlaceUnitSystem::findNearestPlace(const glm::vec2& mouse_world, game::defs::PlayerType type) const {
    entt::entity nearest = entt::null;
    float best_distance_sq = std::numeric_limits<float>::max();

    auto consider_place = [&](auto view) {
        for (auto entity : view) {
            if (registry_.all_of<game::component::PlaceOccupiedComponent>(entity)) {
                continue;
            }
            const auto* transform = registry_.try_get<engine::component::TransformComponent>(entity);
            const auto* sprite = registry_.try_get<engine::component::SpriteComponent>(entity);
            if (!transform) {
                continue;
            }
            const glm::vec2 place_center = transform->position_ + (sprite ? (sprite->size_ * 0.5f) : glm::vec2{ 32.0f, 32.0f });
            const float distance_sq = engine::utils::distanceSquared(mouse_world, place_center);
            if (distance_sq > game::defs::PLACE_RADIUS * game::defs::PLACE_RADIUS) {
                continue;
            }
            if (distance_sq < best_distance_sq) {
                best_distance_sq = distance_sq;
                nearest = entity;
            }
        }
    };

    switch (type) {
    case game::defs::PlayerType::MELEE:
        consider_place(registry_.view<game::defs::MeleePlaceTag, engine::component::TransformComponent>());
        break;
    case game::defs::PlayerType::RANGED:
        consider_place(registry_.view<game::defs::RangePlaceTag, engine::component::TransformComponent>());
        break;
    case game::defs::PlayerType::MIXED:
    default:
        consider_place(registry_.view<game::defs::MeleePlaceTag, engine::component::TransformComponent>());
        consider_place(registry_.view<game::defs::RangePlaceTag, engine::component::TransformComponent>());
        break;
    }

    return nearest;
}

void PlaceUnitSystem::clearActivePrep() {
    if (registry_.valid(active_prep_entity_)) {
        if (!registry_.all_of<game::defs::DeadTag>(active_prep_entity_)) {
            registry_.emplace_or_replace<game::defs::DeadTag>(active_prep_entity_);
        }
    }
    active_prep_entity_ = entt::null;
    target_place_entity_ = entt::null;
}

void PlaceUnitSystem::removePlaceOccupancy(entt::entity unit_entity) {
    auto view = registry_.view<game::component::PlaceOccupiedComponent>();
    for (auto place_entity : view) {
        auto& occupied = view.get<game::component::PlaceOccupiedComponent>(place_entity);
        if (occupied.occupied_by_ == unit_entity) {
            registry_.remove<game::component::PlaceOccupiedComponent>(place_entity);
            break;
        }
    }
}

} // namespace game::system
