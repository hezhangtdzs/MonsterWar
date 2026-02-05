/**
 * @file game_scene.h
 * @brief 定义 GameScene 类，游戏的主场景。
 * 
 * 该场景用于测试引擎的各种功能，包括资源ID、音频播放、UI元素和场景管理。
 */

#pragma once
#include "../../engine/scene/scene.h"

namespace game::scene {

/**
 * @class GameScene
 * @brief 游戏主场景类。
 * 
 * 继承自 engine::scene::Scene，实现了游戏的主要逻辑。
 * 包含资源ID测试、音频测试、UI测试和场景切换测试功能。
 */
class GameScene final: public engine::scene::Scene {
public:
    /**
     * @brief 构造函数。
     * @param context 引擎上下文引用
     */
    GameScene(engine::core::Context& context);

    /** @brief 析构函数。 */
    ~GameScene();

    /**
     * @brief 初始化场景。
     * 
     * 创建测试UI元素，注册输入事件，播放测试音频。
     */
    void init() override;

    /**
     * @brief 清理场景。
     * 
     * 断开输入事件连接，释放资源。
     */
    void clean() override;

private:
    int scene_num_{0};  ///< 场景编号，用于区分不同的场景实例

    // --- 输入回调事件处理函数 ---

    /**
     * @brief 替换场景回调。
     * @return 处理成功返回 true
     */
    bool onReplace();

    /**
     * @brief 压入新场景回调。
     * @return 处理成功返回 true
     */
    bool onPush();

    /**
     * @brief 弹出当前场景回调。
     * @return 处理成功返回 true
     */
    bool onPop();

    /**
     * @brief 退出游戏回调。
     * @return 处理成功返回 true
     */
    bool onQuit();
};

} // namespace game::scene