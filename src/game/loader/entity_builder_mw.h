#pragma once
#include <unordered_map>
#include <vector>
#include "../../engine/loader/basic_entity_builder.h"
#include "../data/waypoint_node.h"

namespace game::loader {
    class EntityBuilderMW final: public engine::loader::BasicEntityBuilder {
    private:
        std::unordered_map<int, game::data::WaypointNode>& waypoint_nodes_;
        std::vector<int>& start_points_;
    public:
        EntityBuilderMW(engine::loader::LevelLoader& level_loader,
            engine::core::Context& context,
            entt::registry& registry,
            std::unordered_map<int, game::data::WaypointNode>& waypoint_nodes,
            std::vector<int>& start_points
        );
        ~EntityBuilderMW() override;
        EntityBuilderMW* build() override;
        void buildPath();
    };
} // namespace game::loader