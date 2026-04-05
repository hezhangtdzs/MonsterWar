#include "selection_system.h"

#include "../component/class_name_component.h"
#include "../component/player_component.h"
#include "../component/stats_component.h"
#include "../defs/constants.h"
#include "../defs/tags.h"
#include "../data/selection_state.h"
#include "../../engine/component/transform_component.h"
#include "../../engine/core/context.h"
#include "../../engine/input/input_manager.h"
#include "../../engine/utils/math.h"
#include <limits>

namespace game::system {

namespace {
} // namespace

void SelectionSystem::update(entt::registry& registry, engine::core::Context& context) {
    if (!registry.ctx().contains<game::data::SelectionState>()) {
        return;
    }

    updateHoveredUnit(registry, context);
    updateSelectedUnit(registry, context);
}

void SelectionSystem::updateHoveredUnit(entt::registry& registry, engine::core::Context& context) {
    auto& selection = registry.ctx().get<game::data::SelectionState>();
    selection.hovered_unit_ = entt::null;

    const auto mouse_world = context.getCamera().screenToWorld(context.getInputManager().getLogicalMousePosition());
    auto view = registry.view<game::component::ClassNameComponent, engine::component::TransformComponent, game::component::StatsComponent>();

    float best_distance_sq = std::numeric_limits<float>::max();
    for (auto entity : view) {
        const auto& transform = view.get<engine::component::TransformComponent>(entity);
        const auto distance_sq = engine::utils::distanceSquared(mouse_world, transform.position_);
        if (distance_sq <= game::defs::HOVER_RADIUS * game::defs::HOVER_RADIUS && distance_sq < best_distance_sq) {
            best_distance_sq = distance_sq;
            selection.hovered_unit_ = entity;
        }
    }
}

void SelectionSystem::updateSelectedUnit(entt::registry& registry, engine::core::Context& context) {
    auto& selection = registry.ctx().get<game::data::SelectionState>();
    const auto previous_selected = selection.selected_unit_;

    if (previous_selected != entt::null && !registry.valid(previous_selected)) {
        selection.selected_unit_ = entt::null;
    }

    const auto left_pressed = context.getInputManager().isActionPressed(entt::hashed_string("mouse_left").value());
    const auto right_pressed = context.getInputManager().isActionPressed(entt::hashed_string("mouse_right").value());
    const auto hovered_unit = selection.hovered_unit_;

    if (right_pressed) {
        selection.selected_unit_ = entt::null;
    } else if (left_pressed && hovered_unit != entt::null && registry.valid(hovered_unit) && registry.all_of<game::component::PlayerComponent>(hovered_unit)) {
        selection.selected_unit_ = hovered_unit;
    }

    if (previous_selected != selection.selected_unit_) {
        if (previous_selected != entt::null && registry.valid(previous_selected) && registry.all_of<game::defs::ShowRangeTag>(previous_selected)) {
            registry.remove<game::defs::ShowRangeTag>(previous_selected);
        }
        if (selection.selected_unit_ != entt::null && registry.valid(selection.selected_unit_) && registry.all_of<game::component::PlayerComponent>(selection.selected_unit_)) {
            const auto& stats = registry.get<game::component::StatsComponent>(selection.selected_unit_);
            if (stats.range_ > 0.0f && !registry.all_of<game::defs::ShowRangeTag>(selection.selected_unit_)) {
                registry.emplace<game::defs::ShowRangeTag>(selection.selected_unit_);
            }
        }
    }
}

} // namespace game::system
