#include "object_builder.h"
#include "game_object.h"
#include "../scene/level_loader.h"
#include "../scene/scene.h"
#include "../component/transform_component.h"
#include "../component/sprite_component.h"
#include "../component/animation_component.h"
#include "../component/health_component.h"
#include "../component/audio_component.h"
#include "../component/tilelayer_component.h"
#include "../render/sprite.h"
#include "../render/animation.h"
#include "../resource/resource_manager.h"
#include "../core/context.h"
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>

namespace engine::object {

    ObjectBuilder::ObjectBuilder(engine::scene::LevelLoader& level_loader, engine::core::Context& context)
        : level_loader_(level_loader),
          context_(context),
          game_object_(nullptr),
          object_json_(nullptr),
          tile_json_(nullptr) {
        spdlog::trace("ObjectBuilder 构造成功");
    }

    ObjectBuilder::~ObjectBuilder() = default;

    ObjectBuilder* ObjectBuilder::configure(const nlohmann::json* object_json) {
        reset();
        object_json_ = object_json;
        tile_json_ = nullptr;
        
        if (object_json_) {
            name_ = object_json_->value("name", "Unnamed");
            dst_size_ = glm::vec2(object_json_->value("width", 0.0f), object_json_->value("height", 0.0f));
            src_size_ = dst_size_;
        }
        
        return this;
    }

    ObjectBuilder* ObjectBuilder::configure(const nlohmann::json* object_json,
                                           const nlohmann::json* tile_json,
                                           engine::component::TileInfo tile_info) {
        reset();
        object_json_ = object_json;
        tile_json_ = tile_json;
        tile_info_ = std::move(tile_info);
        
        if (object_json_) {
            name_ = object_json_->value("name", "Unnamed");
            dst_size_ = glm::vec2(object_json_->value("width", 0.0f), object_json_->value("height", 0.0f));
        }
        
        auto src_rect_opt = tile_info_.sprite.getSourceRect();
        if (src_rect_opt) {
            src_size_ = glm::vec2(src_rect_opt->w, src_rect_opt->h);
        } else {
            src_size_ = dst_size_;
        }
        
        return this;
    }

    void ObjectBuilder::build() {
        if (!object_json_) {
            spdlog::error("ObjectBuilder::build() 被调用，但 object_json_ 为空");
            return;
        }

        // 按顺序构建各个组件
        buildBase();
        buildTransform();
        buildSprite();
        buildAnimation();
        buildAudio();
        buildHealth();
    }

    std::unique_ptr<GameObject> ObjectBuilder::getGameObject() {
        return std::move(game_object_);
    }

    void ObjectBuilder::reset() {
        game_object_.reset();
        object_json_ = nullptr;
        tile_json_ = nullptr;
        tile_info_ = engine::component::TileInfo();
        name_.clear();
        dst_size_ = glm::vec2(0.0f);
        src_size_ = glm::vec2(0.0f);
    }

    void ObjectBuilder::buildBase() {
        game_object_ = std::make_unique<GameObject>(name_);
    }

    void ObjectBuilder::buildTransform() {
        if (!game_object_ || !object_json_) return;

        auto position = glm::vec2(object_json_->value("x", 0.0f), object_json_->value("y", 0.0f));
        auto rotation = object_json_->value("rotation", 0.0f);
        
        // 如果是带图像的对象，需要进行坐标转换
        if (tile_json_ != nullptr) {
            position = glm::vec2(position.x, position.y - dst_size_.y);
        }
        
        // 计算缩放
        glm::vec2 scale(1.0f);
        if (src_size_.x > 0 && src_size_.y > 0) {
            scale = dst_size_ / src_size_;
        }
        
        game_object_->addComponent<engine::component::TransformComponent>(position, rotation, scale);
    }

    void ObjectBuilder::buildSprite() {
        if (!game_object_ || !tile_json_) return;
        
        if (tile_info_.sprite.getTextureId() == engine::resource::InvalidResourceId) {
            spdlog::warn("ObjectBuilder: 对象 '{}' 没有图像纹理", name_);
            return;
        }
        
        // 创建一个新的Sprite，而不是移动tile_info_.sprite
        auto src_rect_opt = tile_info_.sprite.getSourceRect();
        if (!src_rect_opt) {
            spdlog::warn("ObjectBuilder: 对象 '{}' 没有源矩形", name_);
            return;
        }
        
        engine::render::Sprite sprite(tile_info_.sprite.getTextureId(), *src_rect_opt);
        game_object_->addComponent<engine::component::SpriteComponent>(
            std::move(sprite), 
            context_.getResourceManager()
        );
    }


    void ObjectBuilder::buildAnimation() {
        if (!game_object_ || !tile_json_) return;

        auto anim_string = getTileProperty<std::string>(*tile_json_, "animation");
        if (!anim_string) return;

        // 解析动画JSON字符串
        nlohmann::json anim_json;
        try {
            anim_json = nlohmann::json::parse(anim_string.value());
        }
        catch (const nlohmann::json::parse_error& e) {
            spdlog::error("解析动画 JSON 字符串失败: {}", e.what());
            return;
        }

        auto* ac = game_object_->addComponent<engine::component::AnimationComponent>();
        addAnimationFromTileJson(anim_json, ac, src_size_);
    }

    void ObjectBuilder::buildAudio() {
        if (!game_object_ || !tile_json_) return;

        auto sound_string = getTileProperty<std::string>(*tile_json_, "sound");
        if (!sound_string) return;

        nlohmann::json sound_json;
        try {
            sound_json = nlohmann::json::parse(sound_string.value());
        }
        catch (const nlohmann::json::parse_error& e) {
            spdlog::error("解析音效 JSON 字符串失败: {}", e.what());
            return;
        }

        if (!sound_json.is_object()) return;

        auto* audio = game_object_->addComponent<engine::component::AudioComponent>();
        for (const auto& kv : sound_json.items()) {
            if (kv.value().is_string()) {
                audio->registerSound(kv.key(), kv.value().get<std::string>());
            }
        }
    }

    void ObjectBuilder::buildHealth() {
        if (!game_object_ || !tile_json_) return;

        if (auto health = getTileProperty<int>(*tile_json_, "health"); health) {
            game_object_->addComponent<engine::component::HealthComponent>(health.value());
        }
    }

    template<typename T>
    std::optional<T> ObjectBuilder::getTileProperty(const nlohmann::json& tile_json, std::string_view property_name) {
        return level_loader_.getTileProperty<T>(tile_json, std::string(property_name));
    }

    engine::component::TileType ObjectBuilder::getTileType(const nlohmann::json& tile_json) {
        return level_loader_.getTileType(tile_json);
    }

    std::optional<engine::utils::Rect> ObjectBuilder::getCollisionRect(const nlohmann::json& tile_json) {
        return level_loader_.getCollisionRect(&tile_json);
    }

    void ObjectBuilder::addAnimationFromTileJson(const nlohmann::json& anim_json, 
                                                 engine::component::AnimationComponent* ac, 
                                                 const glm::vec2& sprite_size) {
        if (!anim_json.is_object() || !ac) {
            spdlog::error("无效的动画 JSON 或 AnimationComponent 指针");
            return;
        }

        for (const auto& anim : anim_json.items()) {
            const std::string& anim_name = anim.key();
            const auto& anim_info = anim.value();
            
            if (!anim_info.is_object()) {
                spdlog::warn("动画 '{}' 的信息无效或为空", anim_name);
                continue;
            }

            auto duration_ms = anim_info.value("duration", 100);
            auto duration = static_cast<float>(duration_ms) / 1000.0f;
            auto row = anim_info.value("row", 0);
            auto loop = anim_info.value("loop", true);

            if (!anim_info.contains("frames") || !anim_info["frames"].is_array()) {
                spdlog::warn("动画 '{}' 缺少 'frames' 数组", anim_name);
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
                    column * sprite_size.x,
                    row * sprite_size.y,
                    sprite_size.x,
                    sprite_size.y
                };
                animation->addFrame(src_rect, duration);
            }
            ac->addAnimation(std::move(animation));
        }
    }

    void ObjectBuilder::addSound(const nlohmann::json& sound_json, engine::component::AudioComponent* audio_component) {
        // 此方法在 buildAudio 中已实现内联逻辑
        // 保留此方法用于可能的扩展
    }

} // namespace engine::object
