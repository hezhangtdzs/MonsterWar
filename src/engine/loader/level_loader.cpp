#include "level_loader.h"
#include "basic_entity_builder.h"
#include "../component/name_component.h"
#include "../component/parallax_component.h"
#include "../component/transform_component.h"
#include "../component/tilelayer_component.h"
#include "../component/sprite_component.h"
#include "../component/animation_component.h"
#include "../component/audio_component.h"
#include "../scene/scene.h"
#include "../core/context.h"
#include "../resource/resource_manager.h"
#include "../render/animation.h"
#include "../utils/math.h"
#include <nlohmann/json.hpp>
#include <fstream>
#include <spdlog/spdlog.h>
#include <glm/vec2.hpp>
#include <filesystem>
#include <SDL3/SDL_rect.h>
#include <entt/core/hashed_string.hpp>
#include <entt/entity/entity.hpp>

namespace engine::loader {

    LevelLoader::~LevelLoader() = default;

    bool LevelLoader::loadLevel(const std::string& level_path, engine::scene::Scene* scene) {
        scene_ = scene; // 存储当前场景指针，方便后续加载过程中直接添加对象
        if (!entity_builder_) {
            entity_builder_ = std::make_unique<BasicEntityBuilder>(*this, scene->getContext(), scene->getRegistry());
        }
        // 1. 加载 JSON 文件
        std::ifstream file(level_path);
        if (!file.is_open()) {
            spdlog::error("无法打开关卡文件: {}", level_path);
            return false;
        }

        // 2. 解析 JSON 数据
        nlohmann::json json_data;
        try {
            file >> json_data;
        }
        catch (const nlohmann::json::parse_error& e) {
            spdlog::error("解析 JSON 数据失败: {}", e.what());
            return false;
        }

        // 3. 获取基本地图信息 (名称、地图尺寸、瓦片尺寸)
        map_path_ = level_path;
        map_size_ = glm::ivec2(json_data.value("width", 0), json_data.value("height", 0));
        tile_size_ = glm::ivec2(json_data.value("tilewidth", 0), json_data.value("tileheight", 0));

        // 4. 加载 tileset 数据
        if (json_data.contains("tilesets") && json_data["tilesets"].is_array()) {
            for (const auto& tileset_json : json_data["tilesets"]) {
                if (!tileset_json.contains("source") || !tileset_json["source"].is_string() ||
                    !tileset_json.contains("firstgid") || !tileset_json["firstgid"].is_number_integer()) {
                    spdlog::error("tilesets 对象中缺少有效 'source' 或 'firstgid' 字段。");
                    continue;
                }
                auto tileset_path = resolvePath(tileset_json["source"], map_path_);  // 支持隐式转换，可以省略.get<T>()方法，
                auto first_gid = tileset_json["firstgid"];
                loadTileset(tileset_path, first_gid);
            }
        }

        // 5. 加载图层数据
        if (!json_data.contains("layers") || !json_data["layers"].is_array()) {       // 地图文件中必须有 layers 数组
            spdlog::error("地图文件 '{}' 中缺少或无效的 'layers' 数组。", level_path);
            return false;
        }
        for (const auto& layer_json : json_data["layers"]) {
            // 获取各图层对象中的类型（type）字段
            std::string layer_type = layer_json.value("type", "none");
            if (!layer_json.value("visible", true)) {
                spdlog::info("图层 '{}' 不可见，跳过加载。", layer_json.value("name", "Unnamed"));
                continue;
            }

            // 根据图层类型决定加载方法
            if (layer_type == "imagelayer") {
                loadImageLayer(layer_json);
            }
            else if (layer_type == "tilelayer") {
                loadTileLayer(layer_json);
            }
            else if (layer_type == "objectgroup") {
                loadObjectLayer(layer_json);
            }
            else {
                spdlog::warn("不支持的图层类型: {}", layer_type);
            }
        }

        spdlog::info("关卡加载完成: {}", level_path);
        return true;
    }

    void LevelLoader::loadImageLayer(const nlohmann::json& layer_json) {
        // 获取纹理相对路径 （会自动处理'\/'符号）
        const std::string& image_path = layer_json.value("image", "");
        if (image_path.empty()) {
            spdlog::error("图层 '{}' 缺少 'image' 属性。", layer_json.value("name", "Unnamed"));
            return;
        }
        auto texture_path = resolvePath(image_path, map_path_);
        auto* resource_manager = &scene_->getContext().getResourceManager();
        auto texture_size = resource_manager->getTextureSize(entt::hashed_string(texture_path.c_str()),texture_path);
        auto sprite = engine::component::Sprite(texture_path,engine::utils::Rect{0,0,texture_size.x,texture_size.y});
        // 获取图层偏移量（json中没有则代表未设置，给默认值即可）
        const glm::vec2 offset = glm::vec2(layer_json.value("offsetx", 0.0f), layer_json.value("offsety", 0.0f));

        // 获取视差因子及重复标志
        const glm::vec2 scroll_factor = glm::vec2(layer_json.value("parallaxx", 1.0f), layer_json.value("parallaxy", 1.0f));
        const glm::bvec2 repeat = glm::bvec2(layer_json.value("repeatx", false), layer_json.value("repeaty", false));

        // 获取图层名称
        const std::string& layer_name = layer_json.value("name", "Unnamed");
        entt::id_type name_id = entt::hashed_string(layer_name.c_str());
        //Todo：使用ecs组件系统来管理图层对象，避免过度依赖GameObject的构造函数和成员函数，增强灵活性和可扩展性
        auto& registry = scene_->getRegistry();
        auto entity = registry.create();
            registry.emplace<engine::component::NameComponent>(entity, name_id, layer_name);
            registry.emplace<engine::component::TransformComponent>(entity, offset);
            registry.emplace<engine::component::ParallaxComponent>(entity, scroll_factor, repeat);
            registry.emplace<engine::component::SpriteComponent>(entity, sprite);
        // // 创建游戏对象
        // auto game_object = std::make_unique<engine::object::GameObject>(layer_name);
        // // 依次添加Transform，Parallax组件
        // game_object->addComponent<engine::component::TransformComponent>(offset);
        // game_object->addComponent<engine::component::ParallaxComponent>(texture_id, scroll_factor, repeat);
        // // 添加到场景中
        // scene.addGameObject(std::move(game_object));
        spdlog::info("加载图层: '{}' 完成", layer_name);
    }

    void LevelLoader::loadTileLayer(const nlohmann::json& layer_json)
    {
        if (!layer_json.contains("data") || !layer_json["data"].is_array()) {
            spdlog::error("图层 '{}' 缺少 'data' 属性。", layer_json.value("name", "Unnamed"));
            return;
        }

        if (!entity_builder_) {
            spdlog::error("实体生成器未初始化，无法加载瓦片图层。");
            return;
        }

        auto layer_name = layer_json.value("name", "Unnamed");
        auto name_id = entt::hashed_string(layer_name.c_str());
        auto& registry = scene_->getRegistry();
        auto entity = registry.create();
        registry.emplace<engine::component::NameComponent>(entity, name_id, layer_name);

        const glm::ivec2 layer_map_size(layer_json.value("width", 0), layer_json.value("height", 0));
        if (layer_map_size.x <= 0 || layer_map_size.y <= 0) {
            spdlog::error("图层 '{}' 缺少或无效的 width/height。", layer_json.value("name", "Unnamed"));
            return;
        }
        // 准备 TileInfo Vector (瓦片数量 = 地图宽度 * 地图高度)
        std::vector<entt::entity> tiles;

        tiles.reserve(static_cast<size_t>(layer_map_size.x) * static_cast<size_t>(layer_map_size.y));

        // 获取图层数据 (瓦片 ID 列表)
        const auto& data = layer_json["data"];
        size_t index = 0; // 用于跟踪当前瓦片在图层中的位置（从0开始）
        // 根据gid获取必要信息，并依次填充 TileInfo Vector
        for (const auto& gid : data) {
            if (gid == 0) {
                index++;
                tiles.push_back(entt::null);
                continue;
            }

            auto tile_info = getTileInfoByGid(gid);

            if (tile_info.type_ == engine::component::TileType::EMPTY &&
                tile_info.sprite_.texture_id_ == entt::null &&
                tile_info.sprite_.texture_path_.empty()) {
                index++;
                tiles.push_back(entt::null);
                continue;
            }

            auto tile_entity = entity_builder_->configure(static_cast<int>(index), &tile_info)->build()->getEntityID();
            tiles.push_back(tile_entity);
            index++;
        }

        // 添加 TileLayerComponent
        registry.emplace<engine::component::TileLayerComponent>(entity, tile_size_, layer_map_size, std::move(tiles));
        
        // 可选：添加 TransformComponent 处理图层偏移
        glm::vec2 layer_offset(layer_json.value("offsetx", 0.0f), layer_json.value("offsety", 0.0f));
        registry.emplace<engine::component::TransformComponent>(entity, layer_offset);
        
        spdlog::info("图层 '{}' 加载完成 ({}x{})", layer_name, layer_map_size.x, layer_map_size.y);
    }

    void LevelLoader::loadObjectLayer(const nlohmann::json& layer_json)
    {
        if (!layer_json.contains("objects") || !layer_json["objects"].is_array()) {
            spdlog::error("对象图层 '{}' 缺少 'objects' 属性。", layer_json.value("name", "Unnamed"));
            return;
        }

        if (!entity_builder_) {
            spdlog::error("实体生成器未初始化，无法加载对象图层。");
            return;
        }

        const auto& objects = layer_json["objects"];
        for (const auto& object : objects) {
            if (!object.value("visible", true)) {
                continue;
            }

            const int gid = object.value("gid", 0);
            if (gid == 0) {
                entity_builder_->configure(&object)->build();
                continue;
            }

            auto tile_data = getTileDataByGid(gid);
            auto& tile_info = tile_data.info;
            if (tile_info.type_ == engine::component::TileType::EMPTY &&
                tile_info.sprite_.texture_id_ == entt::null &&
                tile_info.sprite_.texture_path_.empty()) {
                spdlog::warn("对象图层 '{}' 中 gid 为 {} 的瓦片无效，已跳过。", layer_json.value("name", "Unnamed"), gid);
                continue;
            }

            entity_builder_->configure(&object, &tile_info)->build();
        }
    } 
    
    const nlohmann::json* LevelLoader::findTileset(int gid)
    {
        // 清除GID的最高三位（翻转信息），得到原始GID值
        const int FLIP_MASK = 0x1FFFFFFF;
        int original_gid = gid & FLIP_MASK;
        
        if (original_gid <= 0) return nullptr;

        // 检查缓存
        if (cache_.data && original_gid >= cache_.first_gid && (cache_.next_first_gid == -1 || original_gid < cache_.next_first_gid)) {
            return cache_.data;
        }

        // upper_bound：查找tileset_data_中键大于 original_gid 的第一个元素
        auto it = tileset_data_.upper_bound(original_gid);
        if (it == tileset_data_.begin()) {
            return nullptr;
        }

        auto current_it = std::prev(it);
        cache_.first_gid = current_it->first;
        cache_.next_first_gid = (it == tileset_data_.end()) ? -1 : it->first;
        cache_.data = &current_it->second;

        return cache_.data;
    }


    TileData LevelLoader::getTileDataByGid(int gid)
    {
        auto make_empty_data = []() {
            return TileData{ engine::component::TileInfo(engine::component::Sprite(), engine::component::TileType::EMPTY), nullptr };
        };

        // 清除GID的最高三位（翻转信息），得到原始GID值
        const int FLIP_MASK = 0x1FFFFFFF;
        int original_gid = gid & FLIP_MASK;

        const nlohmann::json* tileset_ptr = findTileset(original_gid);
        if (!tileset_ptr) {
            if (original_gid != 0) spdlog::warn("gid为 {} 的瓦片未找到图块集。", original_gid);
            return make_empty_data();
        }

        const auto& tileset = *tileset_ptr;
        auto local_id = original_gid - cache_.first_gid;
        const std::string file_path = tileset.value("file_path", "");

        if (tileset.contains("image")) {
            // Case 1: 单一图片 (Tileset Image)
            std::string image_path = tileset.value("image", "");
            if (image_path.empty()) return make_empty_data();

            auto texture_path = resolvePath(image_path, file_path);
            
            int ts_tile_width = tileset.value("tilewidth", tile_size_.x);
            int ts_tile_height = tileset.value("tileheight", tile_size_.y);

            int columns = tileset.value("columns", 0);
            if (columns <= 0) {
                 int image_width = tileset.value("imagewidth", 0);
                 columns = (ts_tile_width > 0 && image_width > 0) ? image_width / ts_tile_width : 1;
            }

            auto coordinate_x = local_id % columns;
            auto coordinate_y = local_id / columns;
            
            engine::utils::Rect texture_rect = {
                glm::vec2(static_cast<float>(coordinate_x * ts_tile_width), static_cast<float>(coordinate_y * ts_tile_height)),
                glm::vec2(static_cast<float>(ts_tile_width), static_cast<float>(ts_tile_height))
            };
            
            engine::component::Sprite sprite{ texture_path, texture_rect };
            
            // 查找瓦片的特定 JSON 配置用于获取类型和碰撞框
            const nlohmann::json* tile_json = nullptr;
            if (tileset.contains("tiles")) {
                const auto& tiles_json = tileset["tiles"];
                if (tiles_json.is_array()) {
                    for (const auto& tj : tiles_json) {
                        if (tj.value("id", -1) == local_id) {
                            tile_json = &tj;
                            break;
                        }
                    }
                }
            }

            auto tile_type = tile_json ? getTileType(*tile_json) : engine::component::TileType::NORMAL;
            
            // 处理动画
            std::optional<engine::component::Animation> animation = std::nullopt;
            if (tile_json && tile_json->contains("animation")) {
                std::vector<engine::component::AnimationFrame> frames;
                const auto& anim_json = (*tile_json)["animation"];

                if (anim_json.is_array()) {
                    for (const auto& frame_data : anim_json) {
                        int frame_tile_id = frame_data.value("tileid", -1);
                        float duration = frame_data.value("duration", 100.0f);

                        if (frame_tile_id >= 0) {
                            auto f_cx = frame_tile_id % columns;
                            auto f_cy = frame_tile_id / columns;

                            engine::utils::Rect frame_rect = {
                                glm::vec2(static_cast<float>(f_cx * ts_tile_width), static_cast<float>(f_cy * ts_tile_height)),
                                glm::vec2(static_cast<float>(ts_tile_width), static_cast<float>(ts_tile_height))
                            };

                            frames.emplace_back(frame_rect, duration);
                        }
                    }
                }

                if (!frames.empty()) {
                    animation = engine::component::Animation(std::move(frames), true);
                }
            }

            return TileData{ engine::component::TileInfo(sprite, tile_type, animation, tile_json ? std::optional<nlohmann::json>(*tile_json) : std::nullopt), tile_json };
        }
        else {
            // Case 2: 多图片集合
            if (tileset.contains("tiles")) {
                const auto& tiles_json = tileset["tiles"];
                if (tiles_json.is_array()) {
                    for (const auto& tile_json : tiles_json) {
                        if (tile_json.value("id", -1) == local_id) {
                            if (!tile_json.contains("image")) return make_empty_data();

                            std::string image_path = tile_json.value("image", "");
                            auto texture_path = resolvePath(image_path, file_path);
                            
                            engine::utils::Rect texture_rect = {
                                glm::vec2(static_cast<float>(tile_json.value("x", 0)), static_cast<float>(tile_json.value("y", 0))),
                                glm::vec2(static_cast<float>(tile_json.value("width", tile_json.value("imagewidth", 0))),    
                                          static_cast<float>(tile_json.value("height", tile_json.value("imageheight", 0))))
                            };
                            return TileData{ engine::component::TileInfo(
                                engine::component::Sprite{ texture_path, texture_rect },
                                getTileType(tile_json),
                                std::nullopt,
                                std::optional<nlohmann::json>(tile_json)),
                                &tile_json };
                        }
                    }
                }
            }
        }
        
        return make_empty_data();
    }

    engine::component::TileInfo LevelLoader::getTileInfoByGid(int gid)
    {
        auto data = getTileDataByGid(gid);
        
        // 处理翻转信息 (Tiled 使用 GID 的高位存储翻转)
        const unsigned FLIPPED_HORIZONTALLY_FLAG = 0x80000000;

        if (gid & FLIPPED_HORIZONTALLY_FLAG) {
            data.info.sprite_.is_flipped_ = true;
        }
        // 目前仅支持水平翻转，垂直翻转等可后续扩展

        return data.info;
    }

    void LevelLoader::loadTileset(const std::string& tileset_path, int first_gid)
    {
        std::ifstream tileset_file(tileset_path);
        if (!tileset_file.is_open()) {
            spdlog::error("无法打开 Tileset 文件: {}", tileset_path);
            return;
        }

        nlohmann::json ts_json;
        try {
            tileset_file >> ts_json;
        }
        catch (const nlohmann::json::parse_error& e) {
            spdlog::error("解析 Tileset JSON 文件 '{}' 失败: {} (at byte {})", tileset_path, e.what(), e.byte);
            return;
        }
        ts_json["file_path"] = tileset_path;    // 将文件路径存储到json中，后续解析图片路径时需要
        tileset_data_[first_gid] = std::move(ts_json);
        spdlog::info("Tileset 文件 '{}' 加载完成，firstgid: {}", tileset_path, first_gid);
    }

    std::string LevelLoader::resolvePath(const std::string& relative_path, const std::string& file_path)
    {
        try {
            // 获取地图文件的父目录（相对于可执行文件） "assets/maps/level1.tmj" -> "assets/maps"
            auto map_dir = std::filesystem::path(file_path).parent_path();
            // 合并路径（相对于可执行文件）并返回。
            auto final_path = std::filesystem::canonical(map_dir / relative_path);
            return final_path.string();
        }
        catch (const std::exception& e) {
            spdlog::error("解析路径失败: {}", e.what());
            return relative_path;
        }
    }

    engine::component::TileType LevelLoader::getTileType(const nlohmann::json& tile_json)
    {
        if (tile_json.contains("properties")) {
            for (const auto& property : tile_json["properties"]) {
                if (property.value("name", "") == "solid") {
                    return property.value("value", false) ? engine::component::TileType::SOLID : engine::component::TileType::NORMAL;
                }
                else if (property.value("name", "") == "unisolid") {
                    return property.value("value", false) ? engine::component::TileType::UNISOLID : engine::component::TileType::NORMAL;
                }
                else if (property.value("name", "") == "slope") {
                    std::string type_str = property.value("value", "");

                    if (type_str == "0_1" || type_str == "slope_0_1") return engine::component::TileType::SLOPE_0_1;
                    else if (type_str == "1_0" || type_str == "slope_1_0") return engine::component::TileType::SLOPE_1_0;
                    else if (type_str == "0_2" || type_str == "slope_0_2") return engine::component::TileType::SLOPE_0_2;
                    else if (type_str == "2_1" || type_str == "slope_2_1") return engine::component::TileType::SLOPE_2_1;
                    else if (type_str == "1_2" || type_str == "slope_1_2") return engine::component::TileType::SLOPE_1_2;
                    else if (type_str == "2_0" || type_str == "slope_2_0") return engine::component::TileType::SLOPE_2_0;
                }
                else if (property.value("name", "") == "hazard") {
                    return property.value("value", false) ? engine::component::TileType::HAZARD : engine::component::TileType::NORMAL;
                }
                else if (property.value("name", "") == "ladder") {
                    return property.value("value", false) ? engine::component::TileType::LADDER : engine::component::TileType::NORMAL;
                }
            }
        }

        return engine::component::TileType::NORMAL;
    }

    engine::component::TileType LevelLoader::getTileTypeById(const nlohmann::json& tileset, int local_id) const
    {
        if (tileset.contains("tiles")) {
            for (const auto& tile : tileset["tiles"]) {
                if (tile.value("id", -1) == local_id) {
                    return const_cast<LevelLoader*>(this)->getTileType(tile);
                }
            }
        }
        return engine::component::TileType::NORMAL;
    }

    std::optional<engine::utils::Rect> LevelLoader::getCollisionRect(const nlohmann::json* tile_json) const
    {
        if (!tile_json || !tile_json->contains("objectgroup")) return std::nullopt;
        auto& objectgroup = (*tile_json)["objectgroup"];
        if (!objectgroup.contains("objects")) return std::nullopt;
        auto& objects = objectgroup["objects"];
        for (const auto& object : objects) {
            auto rect = engine::utils::Rect(glm::vec2(object.value("x", 0.0f), object.value("y", 0.0f)),
                glm::vec2(object.value("width", 0.0f), object.value("height", 0.0f)));
            if (rect.size.x > 0 && rect.size.y > 0) {
                return rect;
            }
        }
        return std::nullopt;
    }

    const nlohmann::json* LevelLoader::getTileJsonByGid(int gid)
    {
        return getTileDataByGid(gid).json_ptr;
    }

    void LevelLoader::addAnimationFromTileJson(engine::component::AnimationComponent* anim_comp, const nlohmann::json& anim_json, glm::vec2& size)
    {
        if (!anim_json.is_object() || !anim_comp) {
            spdlog::error("无效的动画 JSON 或 AnimationComponent 指针。");
            return;
        }

        std::string first_anim;

        for (const auto& anim : anim_json.items()) {
            const std::string& anim_name = anim.key();
            if (first_anim.empty()) first_anim = anim_name;

            const auto& anim_info = anim.value();
            if (!anim_info.is_object()) {
                spdlog::warn("动画 '{}' 的信息无效或为空。", anim_name);
                continue;
            }

            auto duration_ms = anim_info.value("duration", 100);
            auto duration = static_cast<float>(duration_ms) / 1000.0f;
            auto row = anim_info.value("row", 0);
            auto loop = anim_info.value("loop", true); // 支持在 JSON 中定义 loop

            if (!anim_info.contains("frames") || !anim_info["frames"].is_array()) {
                spdlog::warn("动画 '{}' 缺少 'frames' 数组。", anim_name);
                continue;
            }

            auto animation = std::make_unique<engine::render::Animation>(anim_name, loop);

            for (const auto& frame : anim_info["frames"]) {
                if (!frame.is_number_integer()) {
                    spdlog::warn("动画 {} 中 frames 数组格式错误！", anim_name);
                    continue;
                }
                auto column = frame.get<int>();
                SDL_FRect src_rect = {
                    column * size.x,
                    row * size.y,
                    size.x,
                    size.y
                };
                animation->addFrame(src_rect, duration);
            }
            //anim_comp->addAnimation(std::move(animation));
        }
    }

} // namespace engine::loader