#pragma once

#include "../../engine/scene/scene.h"

namespace game::scene {

    /**
     * @class GameScene
     * @brief 游戏的基础场景类，继承自引擎的 Scene 类。
     * 
     * 该类作为所有具体游戏场景（如标题场景、关卡场景等）的基类，提供了游戏特定的初始化和管理功能。
     */
    class GameScene : public engine::scene::Scene {
    public:
        /**
         * @brief 构造函数。
         * @param scene_name 场景名称。
         * @param context 引擎上下文引用。
         * @param scene_manager 场景管理器引用。
         */
        GameScene(engine::core::Context& context, engine::scene::SceneManager& scene_manager);
        virtual ~GameScene();

        // 禁止拷贝和移动构造
        GameScene(const GameScene&) = delete;
        GameScene& operator=(const GameScene&) = delete;
        GameScene(GameScene&&) = delete;
        GameScene& operator=(GameScene&&) = delete;

        /** @brief 初始化游戏场景内容。在进入场景时由 SceneManager 调用。 */
        void init() override;

        // 其他游戏场景特定的方法可以在此处添加
    };

} // namespace game::scene