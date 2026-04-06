#include "units_portrait_ui.h"

#include "../data/session_data.h"
#include "../data/ui_config.h"
#include "../data/game_stats.h"
#include "../defs/event.h"
#include "../factory/entity_factory.h"
#include "../../engine/core/context.h"
#include "../../engine/core/game_state.h"
#include "../../engine/render/image.h"
#include "../../engine/ui/ui_button.h"
#include "../../engine/ui/ui_image.h"
#include "../../engine/ui/ui_panel.h"
#include "../../engine/ui/ui_text.h"
#include "../../engine/ui/state/normal_state.h"
#include "../../engine/ui/state/hover_state.h"
#include "../../engine/ui/state/pressed_state.h"
#include <algorithm>
#include <glm/vec2.hpp>
#include <memory>
#include <string>
#include <utility>

namespace game::ui {

void rebuildUnitsPortraitUI(engine::ui::UIPanel& anchor_panel,
                            engine::core::Context& context,
                            const game::data::SessionData& session_data,
                            const game::data::UIConfig& ui_config,
                            const game::factory::EntityFactory& entity_factory,
                            const game::data::GameStats& game_stats,
                            const std::vector<entt::id_type>& hidden_unit_portrait_ids,
                            float scroll_offset_x,
                            float* content_width,
                            PortraitClickCallback on_portrait_selected) {
    anchor_panel.clearChildren();

    const auto& layout = ui_config.getUnitPanelLayout();
    const auto window_size = context.getGameState().getWindowLogicalSize();
    const float frame_w = layout.frame_size_.x;
    const float frame_h = layout.frame_size_.y;
    const float padding = static_cast<float>(layout.padding_);

    std::vector<const game::data::UnitData*> units;
    units.reserve(session_data.getUnitMap().size());
    for (const auto& [_, unit] : session_data.getUnitMap()) {
        units.push_back(&unit);
    }

    std::sort(units.begin(), units.end(), [&entity_factory](const auto* lhs, const auto* rhs) {
        const int lhs_cost = entity_factory.getPlayerUnitCost(lhs->class_id_, lhs->rarity_);
        const int rhs_cost = entity_factory.getPlayerUnitCost(rhs->class_id_, rhs->rarity_);
        if (lhs_cost != rhs_cost) {
            return lhs_cost < rhs_cost;
        }
        return lhs->name_ < rhs->name_;
    });

    const int visible_count = static_cast<int>(std::count_if(units.begin(), units.end(), [&hidden_unit_portrait_ids](const auto* unit) {
        return std::find(hidden_unit_portrait_ids.begin(), hidden_unit_portrait_ids.end(), unit->name_id_) == hidden_unit_portrait_ids.end();
    }));
    const float content_width_value = padding + static_cast<float>(visible_count) * (frame_w + padding);
    if (content_width) {
        *content_width = content_width_value;
    }

    const float panel_width = window_size.x - padding * 2.0f;
    const float panel_height = frame_h + padding * 2.0f;

    anchor_panel.setPosition(glm::vec2{ padding, window_size.y - panel_height });
    anchor_panel.setSize(glm::vec2{ panel_width, panel_height });

    size_t visible_index = 0;
    for (const auto* unit : units) {
        if (std::find(hidden_unit_portrait_ids.begin(), hidden_unit_portrait_ids.end(), unit->name_id_) != hidden_unit_portrait_ids.end()) {
            continue;
        }

        const auto* portrait = ui_config.getPortrait(unit->name_id_);
        const auto* icon = ui_config.getIcon(unit->class_id_);
        const int cost = entity_factory.getPlayerUnitCost(unit->class_id_, unit->rarity_);
        const bool affordable = cost <= game_stats.cost_;
        const auto* frame = ui_config.getPortraitFrame(affordable ? 2 : 1);

        auto frame_panel = std::make_unique<engine::ui::UIButton>(context, std::string{}, layout.font_path_, layout.font_size_);
        frame_panel->setPosition({ padding + static_cast<float>(visible_index) * (frame_w + padding) - scroll_offset_x, padding });
        frame_panel->setSize(layout.frame_size_);
        frame_panel->setId(unit->name_id_);
        frame_panel->setInteractive(true);
        frame_panel->setClickCallback([on_portrait_selected, unit, cost, affordable]() {
            if (affordable && on_portrait_selected) {
                on_portrait_selected(*unit, cost);
            }
        });
        frame_panel->setHoverEnterCallback([&context, name_id = unit->name_id_]() {
            context.getDispatcher().trigger(game::defs::UIPortraitHoverEnterEvent{ name_id });
        });
        frame_panel->setHoverLeaveCallback([&context]() {
            context.getDispatcher().trigger(game::defs::UIPortraitHoverLeaveEvent{});
        });

        if (portrait) {
            frame_panel->addChild(std::make_unique<engine::ui::UIImage>(context, *portrait, glm::vec2{ 8.0f, 8.0f }, glm::vec2{ frame_w - 16.0f, frame_h - 16.0f }));
        }
        if (frame) {
            frame_panel->addImage(engine::resource::typeId<engine::ui::state::NormalState>(), std::make_unique<engine::render::Image>(*frame));
            frame_panel->addImage(engine::resource::typeId<engine::ui::state::HoverState>(), std::make_unique<engine::render::Image>(*frame));
            frame_panel->addImage(engine::resource::typeId<engine::ui::state::PressedState>(), std::make_unique<engine::render::Image>(*frame));
        }
        if (icon) {
            frame_panel->addChild(std::make_unique<engine::ui::UIImage>(context, *icon, glm::vec2{ 8.0f, 8.0f }, glm::vec2{ 32.0f, 32.0f }));
        }

        auto cost_text = std::make_unique<engine::ui::UIText>(context, std::to_string(cost), layout.font_path_, layout.font_size_);
        cost_text->setPosition(layout.font_offset_);
        cost_text->setColor({ 1.0f, 1.0f, 0.85f, 1.0f });
        frame_panel->addChild(std::move(cost_text));

        auto cover_panel = std::make_unique<engine::ui::UIPanel>(context);
        cover_panel->setPosition({ 0.0f, 0.0f });
        cover_panel->setSize(layout.frame_size_);
        cover_panel->setBackgroundColor(affordable ? engine::utils::FColor{ 0.0f, 0.0f, 0.0f, 0.0f }
                                                   : engine::utils::FColor{ 0.45f, 0.45f, 0.45f, 0.45f });
        cover_panel->setVisible(!affordable);
        frame_panel->addChild(std::move(cover_panel));

        anchor_panel.addChild(std::move(frame_panel), cost);
        ++visible_index;
    }

    anchor_panel.sortChildrenByOrderIndex();
}

} // namespace game::ui
