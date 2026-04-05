#include "ui_config.h"

#include <fstream>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>
#include "../../engine/utils/logging.h"

namespace game::data {
namespace {
engine::render::Image parseImage(const nlohmann::json& json_data)
{
    const auto texture_path = json_data.value("sprite_sheet", std::string{});
    const float x = json_data.value("x", 0.0f);
    const float y = json_data.value("y", 0.0f);
    const float width = json_data.value("width", 0.0f);
    const float height = json_data.value("height", 0.0f);
    if (texture_path.empty()) {
        return {};
    }
    return engine::render::Image(texture_path, std::optional<SDL_FRect>{ SDL_FRect{ x, y, width, height } });
}
} // namespace

bool UIConfig::load(std::string_view ui_config_path)
{
    std::ifstream input_file{ std::string(ui_config_path) };
    if (!input_file.is_open()) {
        ENGINE_LOG_ERROR("无法打开 UI 配置文件: {}", ui_config_path);
        return false;
    }

    nlohmann::json json_data;
    try {
        input_file >> json_data;
    }
    catch (const std::exception& e) {
        ENGINE_LOG_ERROR("解析 UI 配置失败: {}", e.what());
        return false;
    }

    icon_map_.clear();
    portrait_map_.clear();
    portrait_frame_map_.clear();

    if (json_data.contains("icon") && json_data["icon"].is_object()) {
        for (auto& [name, item] : json_data["icon"].items()) {
            icon_map_[entt::hashed_string(name.c_str())] = parseImage(item);
        }
    }

    if (json_data.contains("portrait") && json_data["portrait"].is_object()) {
        for (auto& [name, item] : json_data["portrait"].items()) {
            portrait_map_[entt::hashed_string(name.c_str())] = parseImage(item);
        }
    }

    if (json_data.contains("portrait_frame") && json_data["portrait_frame"].is_object()) {
        for (auto& [name, item] : json_data["portrait_frame"].items()) {
            const int level = item.value("level", 1);
            portrait_frame_map_[level] = parseImage(item);
            ENGINE_LOG_TRACE("UI portrait frame loaded: {} -> level {}", name, level);
        }
    }

    if (json_data.contains("layout") && json_data["layout"].is_object()) {
        const auto& layout_json = json_data["layout"];
        if (layout_json.contains("unit_panel") && layout_json["unit_panel"].is_object()) {
            const auto& panel_json = layout_json["unit_panel"];
            unit_panel_layout_.padding_ = panel_json.value("padding", unit_panel_layout_.padding_);
            if (panel_json.contains("frame_size") && panel_json["frame_size"].is_object()) {
                const auto& frame_size_json = panel_json["frame_size"];
                unit_panel_layout_.frame_size_ = {
                    frame_size_json.value("width", unit_panel_layout_.frame_size_.x),
                    frame_size_json.value("height", unit_panel_layout_.frame_size_.y)
                };
            }
            unit_panel_layout_.font_size_ = panel_json.value("font_size", unit_panel_layout_.font_size_);
            unit_panel_layout_.font_path_ = panel_json.value("font_path", unit_panel_layout_.font_path_);
            if (panel_json.contains("font_offset") && panel_json["font_offset"].is_object()) {
                const auto& font_offset_json = panel_json["font_offset"];
                unit_panel_layout_.font_offset_ = {
                    font_offset_json.value("x", unit_panel_layout_.font_offset_.x),
                    font_offset_json.value("y", unit_panel_layout_.font_offset_.y)
                };
            }
        }
    }

    ENGINE_LOG_INFO("UIConfig 加载完成: icons={}, portraits={}, frames={}", icon_map_.size(), portrait_map_.size(), portrait_frame_map_.size());
    return true;
}

const engine::render::Image* UIConfig::getIcon(entt::id_type class_id) const
{
    if (auto it = icon_map_.find(class_id); it != icon_map_.end()) {
        return &it->second;
    }
    return nullptr;
}

const engine::render::Image* UIConfig::getPortrait(entt::id_type portrait_id) const
{
    if (auto it = portrait_map_.find(portrait_id); it != portrait_map_.end()) {
        return &it->second;
    }
    return nullptr;
}

const engine::render::Image* UIConfig::getPortraitFrame(int rarity) const
{
    const int frame_level = rarity <= 1 ? 1 : 2;
    if (auto it = portrait_frame_map_.find(frame_level); it != portrait_frame_map_.end()) {
        return &it->second;
    }
    if (!portrait_frame_map_.empty()) {
        return &portrait_frame_map_.begin()->second;
    }
    return nullptr;
}

} // namespace game::data
