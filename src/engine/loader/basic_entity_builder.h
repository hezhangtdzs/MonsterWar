#pragma once

#include <entt/entity/entity.hpp>
#include <entt/entity/registry.hpp>
#include <glm/vec2.hpp>
#include <nlohmann/json.hpp>
#include <string>

namespace engine::component {
    struct TileInfo;
}

namespace engine::core {
    class Context;
}

namespace engine::loader {
    class LevelLoader;

    class BasicEntityBuilder {
    public:
        BasicEntityBuilder(LevelLoader& level_loader, engine::core::Context& context, entt::registry& registry);
        ~BasicEntityBuilder();

        void reset();

        BasicEntityBuilder* configure(const nlohmann::json* object_json);
        BasicEntityBuilder* configure(const nlohmann::json* object_json, const engine::component::TileInfo* tile_info);
        BasicEntityBuilder* configure(int index, const engine::component::TileInfo* tile_info);

        BasicEntityBuilder* build();
        entt::entity getEntityID();

        void buildBase();
        void buildSprite();
        void buildTransform();
        void buildRender();
        void buildAnimation();
        void buildAudio();

    private:
        LevelLoader& level_loader_;
        engine::core::Context& context_;
        entt::registry& registry_;

        const nlohmann::json* object_json_{nullptr};
        const engine::component::TileInfo* tile_info_{nullptr};
        int index_{-1};
        entt::entity entity_id_{entt::null};

        glm::vec2 position_{0.0f, 0.0f};
        glm::vec2 dst_size_{0.0f, 0.0f};
        glm::vec2 src_size_{0.0f, 0.0f};
    };

} // namespace engine::loader
