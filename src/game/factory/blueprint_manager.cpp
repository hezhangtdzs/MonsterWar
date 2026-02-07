#include "blueprint_manager.h"

#include <string>
#include <fstream>
#include <iostream>
#include <unordered_map>
#include <vector>

#include <spdlog/spdlog.h>
#include <nlohmann/json.hpp>
#include <entt/core/hashed_string.hpp>

#include "../../engine/resource/resource_manager.h"

namespace game::factory {

BlueprintManager::BlueprintManager(engine::resource::ResourceManager& resource_manager)
    : resource_manager_(resource_manager) {
}

bool BlueprintManager::loadEnemyClassBlueprints(std::string_view enemy_json_path) {
    std::string path_str(enemy_json_path);
    std::ifstream input_file(path_str);
    if (!input_file.is_open()) {
        spdlog::error("无法打开敌人蓝图文件: {}", enemy_json_path);
        return false;
    }

    nlohmann::json json_data;
    try {
        input_file >> json_data;
    } catch (const nlohmann::json::exception& e) {
        spdlog::error("解析敌人蓝图 JSON 失败: {}", e.what());
        return false;
    }

    // 遍历 JSON 中的每个敌人类型
    for (auto& [class_name, data_json] : json_data.items()) {
        entt::id_type class_id = entt::hashed_string(class_name.c_str());

        // 解析各个子蓝图
        data::StatsBlueprint stats = parseStats(data_json);
        data::SpriteBlueprint sprite = parseSprite(data_json);
        auto animations = parseAnimationsMap(data_json);
        data::SoundBlueprint sounds = parseSound(data_json);
        data::EnemyBlueprint enemy = parseEnemy(data_json);
        data::DisplayInfoBlueprint display_info = parseDisplayInfo(data_json);

        // 组装完整蓝图
        data::EnemyClassBlueprint blueprint;
        blueprint.class_id_ = class_id;
        blueprint.class_name_ = class_name;
        blueprint.stats_ = stats;
        blueprint.enemy_ = enemy;
        blueprint.sounds_ = sounds;
        blueprint.sprite_ = sprite;
        blueprint.display_info_ = display_info;
        blueprint.animations_ = std::move(animations);

        // 存入映射表
        enemy_class_blueprints_.emplace(class_id, std::move(blueprint));
        spdlog::info("已加载敌人蓝图: {} (ID: {})", class_name, class_id);
    }

    spdlog::info("成功加载 {} 个敌人蓝图", enemy_class_blueprints_.size());
    return true;
}

const data::EnemyClassBlueprint& BlueprintManager::getEnemyClassBlueprint(entt::id_type id) const {
    return enemy_class_blueprints_.at(id);
}

bool BlueprintManager::hasEnemyClassBlueprint(entt::id_type id) const {
    return enemy_class_blueprints_.find(id) != enemy_class_blueprints_.end();
}

data::StatsBlueprint BlueprintManager::parseStats(const nlohmann::json& json) const {
    data::StatsBlueprint stats;
    stats.hp_ = json.value("hp", 100.0f);
    stats.atk_ = json.value("atk", 10.0f);
    stats.def_ = json.value("def", 0.0f);
    stats.range_ = json.value("range", 20.0f);
    stats.atk_interval_ = json.value("atk_interval", 1.0f);
    return stats;
}

data::SpriteBlueprint BlueprintManager::parseSprite(const nlohmann::json& json) const {
    data::SpriteBlueprint sprite;
    sprite.path_ = json.value("sprite_sheet", "");
    sprite.id_ = entt::hashed_string(sprite.path_.c_str());
    sprite.face_right_ = json.value("face_right", true);

    float width = json.value("width", 192.0f);
    float height = json.value("height", 192.0f);
    float offset_x = json.value("offset_x", -96.0f);
    float offset_y = json.value("offset_y", -148.0f);

    sprite.src_rect_ = engine::utils::Rect{0.0f, 0.0f, width, height};
    sprite.size_ = glm::vec2{width, height};
    sprite.offset_ = glm::vec2{offset_x, offset_y};

    return sprite;
}

std::unordered_map<entt::id_type, data::AnimationBlueprint>
BlueprintManager::parseAnimationsMap(const nlohmann::json& json) const {
    std::unordered_map<entt::id_type, data::AnimationBlueprint> animations;

    if (!json.contains("animation")) {
        return animations;
    }

    for (auto& [anim_name, anim_data] : json["animation"].items()) {
        entt::id_type anim_name_id = entt::hashed_string(anim_name.c_str());

        data::AnimationBlueprint anim;
        anim.ms_per_frame_ = anim_data.value("duration", 50.0f);
        anim.row_ = anim_data.value("row", 0);

        if (anim_data.contains("frames") && anim_data["frames"].is_array()) {
            anim.frames_ = anim_data["frames"].get<std::vector<int>>();
        }

        animations.emplace(anim_name_id, std::move(anim));
    }

    return animations;
}

data::SoundBlueprint BlueprintManager::parseSound(const nlohmann::json& json) {
    data::SoundBlueprint sounds;

    if (!json.contains("sounds")) {
        return sounds;
    }

    for (auto& [sound_key, sound_value] : json["sounds"].items()) {
        std::string sound_path = sound_value.get<std::string>();
        entt::id_type sound_id = entt::hashed_string(sound_path.c_str());

        // 预加载音效资源
        resource_manager_.loadSound(sound_id, sound_path);

        entt::id_type sound_key_id = entt::hashed_string(sound_key.c_str());
        sounds.sounds_.emplace(sound_key_id, sound_id);
    }

    return sounds;
}

data::EnemyBlueprint BlueprintManager::parseEnemy(const nlohmann::json& json) const {
    data::EnemyBlueprint enemy;
    enemy.ranged_ = json.value("ranged", false);
    enemy.speed_ = json.value("speed", 100.0f);
    return enemy;
}

data::DisplayInfoBlueprint BlueprintManager::parseDisplayInfo(const nlohmann::json& json) const {
    data::DisplayInfoBlueprint display_info;
    display_info.name_ = json.value("name", "Unknown");
    display_info.description_ = json.value("description", "");
    return display_info;
}

} // namespace game::factory
