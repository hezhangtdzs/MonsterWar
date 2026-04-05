#pragma once

#include <entt/entt.hpp>
#include <entt/core/hashed_string.hpp>
#include <glm/vec2.hpp>

#include "../defs/event.h"
#include "../defs/tags.h"
#include "../defs/constants.h"
#include "../component/unit_prep_component.h"
#include "../component/place_occupied_component.h"

namespace engine::core {
    class Context;
}

namespace game::factory {
    class EntityFactory;
}

namespace game::system {

class PlaceUnitSystem final {
private:
    entt::registry& registry_;
    entt::dispatcher& dispatcher_;
    engine::core::Context& context_;
    game::factory::EntityFactory& entity_factory_;

    entt::entity active_prep_entity_{ entt::null };
    entt::entity target_place_entity_{ entt::null };

public:
    PlaceUnitSystem(entt::registry& registry,
                    entt::dispatcher& dispatcher,
                    engine::core::Context& context,
                    game::factory::EntityFactory& entity_factory);
    ~PlaceUnitSystem();

    void update(float delta_time);

private:
    void onPrepUnitEvent(const game::defs::PrepUnitEvent& event);
    void onRemovePlayerUnitEvent(const game::defs::RemovePlayerUnitEvent& event);
    bool onPlaceUnit();
    bool onCancelPrepUnit();
    entt::entity findNearestPlace(const glm::vec2& mouse_world, game::defs::PlayerType type) const;
    void clearActivePrep();
    void removePlaceOccupancy(entt::entity unit_entity);
};

} // namespace game::system
