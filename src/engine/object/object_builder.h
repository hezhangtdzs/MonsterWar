#pragma once
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <glm/vec2.hpp>
#include <nlohmann/json_fwd.hpp>
#include "../component/tilelayer_component.h"

namespace engine::core {
    class Context;
}
namespace engine::component {
    class AnimationComponent;
    class AudioComponent;
}
namespace engine::utils {
    struct Rect;
}
namespace engine::scene {
    struct TileData;
    class LevelLoader;
}
namespace engine::object {
    class GameObject;

    /**
     * @class ObjectBuilder
     * @brief 游戏对象生成器基类
     *
     * 生成器模式(Builder Pattern)的核心实现。
     * 将复杂对象的构建过程分解为一系列步骤，使得同样的构建过程可以创建不同的表示。
     *
     * 角色：AbstractBuilder（抽象生成器）
     * 职责：定义构建游戏对象的通用步骤和接口
     *
     * 使用示例：
     * @code
     * ObjectBuilder builder(level_loader, context);
     * auto game_object = builder
     *     .configure(&object_json, &tile_json, tile_info)
     *     .build()
     *     .getGameObject();
     * @endcode
     */
    class ObjectBuilder {
        friend class LevelLoader;

    private:
        engine::scene::LevelLoader& level_loader_;      ///< 用于访问 LevelLoader 的私有方法
        engine::core::Context& context_;
        std::unique_ptr<GameObject> game_object_;

        // --- 解析游戏对象所需要的关键信息 ---
        const nlohmann::json* object_json_ = nullptr;
        const nlohmann::json* tile_json_ = nullptr;
        engine::component::TileInfo tile_info_;

        // --- 保存会多次用到的变量，避免重复解析 ---
        std::string name_;
        glm::vec2 dst_size_;
        glm::vec2 src_size_;

    public:
        /**
         * @brief 构造函数
         * @param level_loader 关卡加载器引用，用于访问瓦片数据
         * @param context 引擎上下文引用
         */
        explicit ObjectBuilder(engine::scene::LevelLoader& level_loader, engine::core::Context& context);

        /**
         * @brief 虚析构函数
         * @details 确保子类可以正确析构，这是多态基类的必要设计
         */
        virtual ~ObjectBuilder();

        // 禁止拷贝和移动，保证唯一所有权
        ObjectBuilder(const ObjectBuilder&) = delete;
        ObjectBuilder& operator=(const ObjectBuilder&) = delete;
        ObjectBuilder(ObjectBuilder&&) = delete;
        ObjectBuilder& operator=(ObjectBuilder&&) = delete;

        // --- 三个关键方法：配置、构建、返回 ---

        /**
         * @brief 配置生成器（针对自定义形状对象）
         * @param object_json Tiled对象JSON数据
         * @return this指针，支持链式调用
         */
        ObjectBuilder* configure(const nlohmann::json* object_json);

        /**
         * @brief 配置生成器（针对基于图块的对象）
         * @param object_json Tiled对象JSON数据
         * @param tile_json 图块JSON数据
         * @param tile_info 图块信息
         * @return this指针，支持链式调用
         */
        ObjectBuilder* configure(const nlohmann::json* object_json,
                                const nlohmann::json* tile_json,
                                engine::component::TileInfo tile_info);

        /**
         * @brief 构建游戏对象
         * @details 子类应该重写此方法以添加特定组件
         * @note 调用此方法前必须先调用 configure 进行配置
         */
        virtual void build();

        /**
         * @brief 获取构建好的游戏对象
         * @return 游戏对象的唯一指针（所有权转移给调用者）
         * @note 调用此方法后，builder内部的游戏对象指针会被置空
         */
        std::unique_ptr<GameObject> getGameObject();

    protected:
        /**
         * @brief 重置生成器状态
         * @details 每次构建前自动调用，确保状态干净
         */
        void reset();

        // --- 代理函数，让子类能获取到 LevelLoader 的私有方法 ---

        template<typename T>
        std::optional<T> getTileProperty(const nlohmann::json& tile_json, std::string_view property_name);

        engine::component::TileType getTileType(const nlohmann::json& tile_json);

        std::optional<engine::utils::Rect> getCollisionRect(const nlohmann::json& tile_json);

        void addAnimationFromTileJson(const nlohmann::json& anim_json,
                                      engine::component::AnimationComponent* ac,
                                      const glm::vec2& sprite_size);

        void addSound(const nlohmann::json& sound_json,
                      engine::component::AudioComponent* audio_component);

        // --- 构建步骤（子类可以重写以自定义构建流程） ---

        /**
         * @brief 构建基础对象
         * @details 创建 GameObject 实例并设置名称
         */
        virtual void buildBase();

        /**
         * @brief 构建变换组件
         * @details 解析位置、旋转、缩放信息
         */
        virtual void buildTransform();

        /**
         * @brief 构建精灵组件
         * @details 根据图块信息创建精灵
         */
        virtual void buildSprite();

        /**
         * @brief 构建物理组件
         * @details 根据类型创建碰撞器和物理组件
         */
        virtual void buildPhysics();

        /**
         * @brief 构建动画组件
         * @details 解析动画JSON配置
         */
        virtual void buildAnimation();

        /**
         * @brief 构建音频组件
         * @details 解析音效JSON配置
         */
        virtual void buildAudio();

        /**
         * @brief 构建生命组件
         * @details 根据属性设置生命值
         */
        virtual void buildHealth();
    };

} // namespace engine::object
