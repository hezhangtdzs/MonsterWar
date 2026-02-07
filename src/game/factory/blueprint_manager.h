#pragma once
#include <string_view>
#include <unordered_map>
#include <nlohmann/json.hpp>
#include <entt/core/hashed_string.hpp>
#include "../data/entity_blueprint.h"

namespace engine::resource { class ResourceManager; }

namespace game::factory {

/**
 * @class BlueprintManager
 * @brief 蓝图管理器，负责加载和缓存单位蓝图数据
 *
 * @details
 * BlueprintManager 从 JSON 文件加载敌人/单位的蓝图配置，解析为 C++ 结构体，
 * 并使用哈希ID作为 key 存储在内存中供后续查询。同时负责预加载音效资源。
 */
class BlueprintManager {
public:
    /**
     * @brief 构造函数
     * @param resource_manager 资源管理器引用，用于预加载音效
     */
    explicit BlueprintManager(engine::resource::ResourceManager& resource_manager);

    /**
     * @brief 从 JSON 文件加载敌人蓝图
     * @param enemy_json_path 敌人数据 JSON 文件路径
     * @return 加载成功返回 true
     */
    [[nodiscard]] bool loadEnemyClassBlueprints(std::string_view enemy_json_path);

    /**
     * @brief 获取指定类型的敌人蓝图
     * @param id 敌人类型ID（entt::hashed_string 值）
     * @return 敌人蓝图的常量引用
     * @throws std::out_of_range 如果蓝图不存在
     */
    const data::EnemyClassBlueprint& getEnemyClassBlueprint(entt::id_type id) const;

    /**
     * @brief 检查是否存在指定类型的蓝图
     * @param id 敌人类型ID
     * @return 存在返回 true
     */
    bool hasEnemyClassBlueprint(entt::id_type id) const;

private:
    engine::resource::ResourceManager& resource_manager_;
    std::unordered_map<entt::id_type, data::EnemyClassBlueprint> enemy_class_blueprints_;

    // 解析辅助函数
    [[nodiscard]] data::StatsBlueprint parseStats(const nlohmann::json& json) const;
    [[nodiscard]] data::SpriteBlueprint parseSprite(const nlohmann::json& json) const;
    [[nodiscard]] std::unordered_map<entt::id_type, data::AnimationBlueprint> parseAnimationsMap(const nlohmann::json& json) const;
    [[nodiscard]] data::SoundBlueprint parseSound(const nlohmann::json& json);
    [[nodiscard]] data::EnemyBlueprint parseEnemy(const nlohmann::json& json) const;
    [[nodiscard]] data::DisplayInfoBlueprint parseDisplayInfo(const nlohmann::json& json) const;
};

} // namespace game::factory
