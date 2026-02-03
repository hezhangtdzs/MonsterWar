#pragma once
#include <string>
#include <glm/vec2.hpp>
#include <nlohmann/json.hpp>
#include <map>
#include <optional>

#include "../utils/math.h"
#include "../component/tilelayer_component.h"
namespace engine::component {
    struct TileInfo;
    enum class TileType;
	class AnimationComponent;

}
namespace engine::object {
    class ObjectBuilder;
}

namespace engine::scene {
    class Scene;

    /**
     * @brief 存储瓦片及其源数据 JSON 指针。
     * 封装了运行时生成的 TileInfo 和指向原始 JSON 定义的指针，便于访问自定义属性。
     */
    struct TileData {
        engine::component::TileInfo info; ///< 瓦片的基本渲染信息
        const nlohmann::json* json_ptr = nullptr; ///< 指向 tileset 中该瓦片 JSON 对象的指针
    };

    /**
     * @brief 负责从 Tiled JSON 文件 (.tmj) 加载关卡数据到 Scene 中。
     * 支持加载图片图层、瓦片图层和对象图层，并处理外部瓦片集 (.tsj) 的链接。
     */
    class LevelLoader final {
        // 友元类声明，允许 ObjectBuilder 访问私有方法
        friend class engine::object::ObjectBuilder;
        std::string map_path_;      ///< 地图文件所在的目录路径（用于解析相对路径）
        glm::ivec2 map_size_;       ///< 地图的总尺寸（以瓦片为单位，x为宽，y为高）
        glm::ivec2 tile_size_;      ///< 单个瓦片的标准尺寸（像素）
        std::map<int, nlohmann::json> tileset_data_;    ///< 已加载的瓦片集数据缓存，Key 为 firstgid

        /**
         * @brief 瓦片集查找缓存，用于加速连续访问相同瓦片集的 GID 查找。
         */
        struct CachedTileset {
            int first_gid = -1;       ///< 当前缓存瓦片集的起始全局 ID
            int next_first_gid = -1;  ///< 下一个瓦片集的起始全局 ID（用于边界判定）
            const nlohmann::json* data = nullptr; ///< 指向瓦片集 JSON 数据的指针
        };
        CachedTileset cache_;

    public:
        LevelLoader() = default;

        /**
         * @brief 加载关卡数据到指定的 Scene 对象中。
         * @param map_path Tiled JSON 地图文件的完整路径。
         * @param scene 指向要填充数据的场景实例。
         * @return bool 如果加载并解析成功则返回 true。
         */
        [[nodiscard]]bool loadLevel(const std::string& map_path, Scene& scene);

    private:
        /** @brief 解析并向场景添加图像图层（Image Layer）。 */
        void loadImageLayer(const nlohmann::json& layer_json, Scene& scene);
        /** @brief 解析并向场景添加瓦片图层（Tile Layer），包括处理每个瓦片的渲染数据。 */
        void loadTileLayer(const nlohmann::json& layer_json, Scene& scene);
        /** @brief 解析并向场景添加对象图层（Object Layer），如实体生成点、触发器等。 */
        void loadObjectLayer(const nlohmann::json& layer_json, Scene& scene);

        /**
         * @brief 获取瓦片的综合数据。
         * @param gid 全局瓦片 ID。
         * @return TileData 包含 TileInfo 和 JSON 指针的结构体。
         */
        TileData getTileDataByGid(int gid);

        /**
         * @brief 获取瓦片的渲染信息（纹理坐标、源路径等）。
         * @param gid 全局瓦片 ID。
         * @return engine::component::TileInfo 解析后的瓦片信息。
         */
        engine::component::TileInfo getTileInfoByGid(int gid);

        /**
         * @brief 内部查找包含特定 GID 的 Tileset JSON 对象。
         * @param gid 全局瓦片 ID。
         * @return const nlohmann::json* 返回对应的 tileset JSON 指针，未找到则返回 nullptr。
         */
        const nlohmann::json* findTileset(int gid);

        /**
         * @brief 加载并解析外部 Tiled tileset 文件 (.tsj)。
         * @param tileset_path 瓦片集文件的相对路径。
         * @param first_gid 该瓦片集在当前地图中的起始全局 ID。
         */
        void loadTileset(const std::string& tileset_path, int first_gid);

        /**
         * @brief 将资源的相对路径转换为基于地图文件的绝对/完整路径。
         * @param relative_path 资源在 JSON 中记录的路径。
         * @param file_path 当前处理的文件路径。
         * @return std::string 拼接后的完整有效路径。
         */
        std::string resolvePath(const std::string& relative_path, const std::string& file_path);

        /**
         * @brief 从瓦片的 JSON 对象中解析 TileType 属性。
         * @param tile_json 瓦片的 JSON 定义。
         * @return engine::component::TileType 映射后的瓦片类型。
         */
        engine::component::TileType getTileType(const nlohmann::json& tile_json);

        /**
         * @brief 通过本地 ID 在特定瓦片集中查找瓦片类型。
         * @param tileset 瓦片集 JSON。
         * @param local_id 瓦片在瓦片集内的索引。
         * @return engine::component::TileType 瓦片类型。
         */
        engine::component::TileType getTileTypeById(const nlohmann::json& tileset, int local_id) const;

        /**
         * @brief 通用的自定义属性获取模板。
         * @tparam T 属性的期望类型（如 int, float, std::string）。
         * @param tile_json 包含 "properties" 数组的 JSON 对象。
         * @param prop_name 属性名称。
         * @return std::optional<T> 如果属性存在且类型匹配则返回其值，否则返回空。
         */
        template<typename T>
        std::optional<T> getTileProperty(const nlohmann::json& tile_json, const std::string& prop_name) const {
            if (tile_json.contains("properties")) {
                for (const auto& prop : tile_json["properties"]) {
                    if (prop["name"] == prop_name) {
                        return prop["value"].get<T>();
                    }
                }
            }
            return std::nullopt;
        };

        /**
         * @brief 从瓦片的 objectgroup 中提取碰撞矩形。
         * @param tile_json 瓦片的 JSON 指针。
         * @return std::optional<engine::utils::Rect> 如果定义了碰撞体积则返回矩形区域。
         */
        std::optional<engine::utils::Rect> getCollisionRect(const nlohmann::json* tile_json) const;

        /**
         * @brief 根据全局 ID 获取瓦片集内部具体的瓦片定义 JSON。
         * @param gid 全局瓦片 ID。
         * @return const nlohmann::json* 返回 tiles 数组中对应 local id 的项，不存在则返回 nullptr。
         */
        const nlohmann::json* getTileJsonByGid(int gid);


		void addAnimationFromTileJson(engine::component::AnimationComponent* sprite_comp, const nlohmann::json& anim_json, glm::vec2& size );
    };

} // namespace engine::scene