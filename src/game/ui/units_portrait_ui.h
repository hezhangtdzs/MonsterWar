#pragma once

#include <functional>
#include <vector>
#include <entt/core/hashed_string.hpp>

namespace engine::core {
    class Context;
}
namespace engine::ui {
    class UIPanel;
}
namespace game::data {
    class SessionData;
    class UIConfig;
    struct GameStats;
    struct UnitData;
}
namespace game::factory {
    class EntityFactory;
}

namespace game::ui {

using PortraitClickCallback = std::function<void(const game::data::UnitData&, int)>;

void rebuildUnitsPortraitUI(engine::ui::UIPanel& anchor_panel,
                            engine::core::Context& context,
                            const game::data::SessionData& session_data,
                            const game::data::UIConfig& ui_config,
                            const game::factory::EntityFactory& entity_factory,
                            const game::data::GameStats& game_stats,
                            const std::vector<entt::id_type>& hidden_unit_portrait_ids,
                            float scroll_offset_x,
                            float* content_width,
                            PortraitClickCallback on_portrait_selected);

} // namespace game::ui
