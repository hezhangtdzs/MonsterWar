/**
 * @file basic_entity_builder.h
 * @brief 定义 BasicEntityBuilder 类，用于构建游戏实体。
 *
 * @details
 * BasicEntityBuilder 实现了建造者模式，用于从 JSON 配置或瓦片信息构建游戏实体。
 * 它提供了一系列配置方法和构建步骤，允许逐步添加组件到实体中。
 *
 * @par 构建流程
 * 1. 调用 configure() 方法配置构建参数
 * 2. 调用 build() 方法执行构建
 * 3. 使用 getEntityID() 获取构建的实体ID
 *
 * @par 使用示例
 * @code
 * BasicEntityBuilder builder(level_loader, context, registry);
 * auto entity = builder.configure(&object_json, &tile_info)
 *                      ->build()
 *                      ->getEntityID();
 * @endcode
 *
 * @see engine::loader::LevelLoader 使用此构建器加载关卡
 */

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

    /**
     * @class BasicEntityBuilder
     * @brief 基础实体构建器，实现建造者模式构建游戏实体。
     *
     * @details
     * 该类提供链式调用接口，支持从 JSON 配置或瓦片信息构建实体。
     * 构建过程包括：基础组件、精灵、变换、渲染、动画和音频组件。
     */
    class BasicEntityBuilder {
    public:
        BasicEntityBuilder(LevelLoader& level_loader, engine::core::Context& context, entt::registry& registry);
        virtual ~BasicEntityBuilder();

        void reset();

        BasicEntityBuilder* configure(const nlohmann::json* object_json);
        BasicEntityBuilder* configure(const nlohmann::json* object_json, const engine::component::TileInfo* tile_info);
        BasicEntityBuilder* configure(int index, const engine::component::TileInfo* tile_info);

        virtual BasicEntityBuilder* build();
        entt::entity getEntityID();

        void buildBase();
        void buildSprite();
        void buildTransform();
        void buildRender();
        void buildAnimation();
        void buildAudio();

    protected:
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
        float depth_{0.0f};
    };

} // namespace engine::loader
