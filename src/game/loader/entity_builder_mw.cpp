#include "entity_builder_mw.h"
#include "spdlog/spdlog.h"
#include "../defs/tags.h"
#include "../../engine/component/tilelayer_component.h"
namespace game::loader {
    EntityBuilderMW::EntityBuilderMW(engine::loader::LevelLoader &level_loader, engine::core::Context &context, entt::registry &registry, std::unordered_map<int, game::data::WaypointNode> &waypoint_nodes, std::vector<int> &start_points)
    : BasicEntityBuilder(level_loader, context, registry),
    waypoint_nodes_(waypoint_nodes),
    start_points_(start_points)
    {
        spdlog::info("EntityBuilderMW 构造函数被调用");
    }
    EntityBuilderMW::~EntityBuilderMW() = default;

    EntityBuilderMW *EntityBuilderMW::build(){

        if (object_json_ && !tile_info_) {
            buildPath();
        } else {

            BasicEntityBuilder::build();
            buildPlacementTag();
        }
        return this;
    }
    void EntityBuilderMW::buildPath()
    {
        if (!object_json_) {
            spdlog::warn("buildPath: object_json_ is null");
            return;
        }
        bool is_point = object_json_->value("point", false);
        if (!is_point) {
            return;
        }
        auto id = object_json_->value("id", 0);
        if (id == 0) {
            return;
        }
        
        game::data::WaypointNode node;
        node.id_ = id;
        node.position_ = glm::vec2(object_json_->value("x", 0.0f), object_json_->value("y", 0.0f));
        
        // 安全地获取 properties 数组
        auto it = object_json_->find("properties");
        if (it != object_json_->end() && it->is_array()) {
            for (const auto& prop : *it) {
                std::string prop_name = prop.value("name", "");
                std::string prop_type = prop.value("type", "");

                if (prop_type == "object" && prop_name.starts_with("next")) {
                    auto next_id = prop.value("value", 0);
                    if (next_id != 0) {
                        node.next_node_ids_.push_back(next_id);

                    }
                } else if (prop_name == "start" && prop.value("value", false) == true) {
                    start_points_.push_back(id);

                }
            }
        } else {
            spdlog::info("  -> 未找到 properties 数组");
        }
        
        waypoint_nodes_[id] = std::move(node);
      
    }

    void EntityBuilderMW::buildPlacementTag()
    {
        if (!tile_info_ || !tile_info_->properties_.has_value()) {
            return;
        }

        const auto& tile_json = tile_info_->properties_.value();
        if (!tile_json.contains("properties") || !tile_json["properties"].is_array()) {
            return;
        }

        const auto& props = tile_json["properties"];
        for (const auto& prop : props) {
            if (!prop.contains("name") || !prop.contains("value")) {
                continue;
            }

            if (prop.value("name", "") != "place") {
                continue;
            }

            const auto place_type = prop.value("value", "");
            if (place_type == "melee") {
                registry_.emplace_or_replace<game::defs::MeleePlaceTag>(entity_id_);
            } else if (place_type == "range") {
                registry_.emplace_or_replace<game::defs::RangePlaceTag>(entity_id_);
            }
            return;
        }
    }

} // namespace game::loader
