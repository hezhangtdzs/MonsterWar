#pragma once

#include <entt/core/hashed_string.hpp>
#include <string>
#include <unordered_map>

namespace game::data {

struct UnitData {
    entt::id_type name_id_{ 0 };
    entt::id_type class_id_{ 0 };
    std::string name_;
    std::string class_name_;
    int level_{ 1 };
    int rarity_{ 1 };
};

class SessionData final {
public:
    [[nodiscard]] bool load(std::string_view session_json_path);

    [[nodiscard]] const std::unordered_map<entt::id_type, UnitData>& getUnitMap() const { return unit_map_; }
    [[nodiscard]] int getLevel() const { return level_; }
    [[nodiscard]] int getPoint() const { return point_; }
    [[nodiscard]] bool isLevelClear() const { return level_clear_; }

private:
    std::unordered_map<entt::id_type, UnitData> unit_map_;
    int level_{ 1 };
    int point_{ 0 };
    bool level_clear_{ false };
};

} // namespace game::data
