#pragma once

#include <entt/core/hashed_string.hpp>
#include <glm/vec2.hpp>
#include <string>
#include <unordered_map>
#include "../../engine/render/image.h"

namespace game::data {

struct UnitPanelLayout {
    int padding_{ 10 };
    glm::vec2 frame_size_{ 128.0f, 128.0f };
    int font_size_{ 40 };
    std::string font_path_{ "assets/fonts/VonwaonBitmap-16px.ttf" };
    glm::vec2 font_offset_{ 16.0f, 72.0f };
};

class UIConfig final {
public:
    [[nodiscard]] bool load(std::string_view ui_config_path);

    [[nodiscard]] const engine::render::Image* getIcon(entt::id_type class_id) const;
    [[nodiscard]] const engine::render::Image* getPortrait(entt::id_type portrait_id) const;
    [[nodiscard]] const engine::render::Image* getPortraitFrame(int rarity) const;

    [[nodiscard]] const UnitPanelLayout& getUnitPanelLayout() const { return unit_panel_layout_; }

private:
    std::unordered_map<entt::id_type, engine::render::Image> icon_map_;
    std::unordered_map<entt::id_type, engine::render::Image> portrait_map_;
    std::unordered_map<int, engine::render::Image> portrait_frame_map_;
    UnitPanelLayout unit_panel_layout_;
};

} // namespace game::data
