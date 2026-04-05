#include "session_data.h"

#include <fstream>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>
#include "../../engine/utils/logging.h"

namespace game::data {

bool SessionData::load(std::string_view session_json_path)
{
    std::ifstream input_file{ std::string(session_json_path) };
    if (!input_file.is_open()) {
        ENGINE_LOG_ERROR("无法打开存档文件: {}", session_json_path);
        return false;
    }

    nlohmann::json json_data;
    try {
        input_file >> json_data;
    }
    catch (const std::exception& e) {
        ENGINE_LOG_ERROR("解析存档文件失败: {}", e.what());
        return false;
    }

    unit_map_.clear();
    if (json_data.contains("unit") && json_data["unit"].is_object()) {
        for (auto& [name, unit_json] : json_data["unit"].items()) {
            const auto name_id = entt::hashed_string(name.c_str());
            const auto class_name = unit_json.value("class", std::string{});
            const auto class_id = entt::hashed_string(class_name.c_str());
            const int level = unit_json.value("level", 1);
            const int rarity = unit_json.value("rarity", 1);
            unit_map_[name_id] = UnitData{ name_id, class_id, name, class_name, level, rarity };
        }
    }

    level_ = json_data.value("level", level_);
    point_ = json_data.value("point", point_);
    level_clear_ = json_data.value("level_clear", level_clear_);

    ENGINE_LOG_INFO("SessionData 加载完成: units={}, level={}, point={}, clear={}", unit_map_.size(), level_, point_, level_clear_);
    return true;
}

} // namespace game::data
