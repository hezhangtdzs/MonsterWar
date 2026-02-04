/**
 * @file main.cpp
 * @brief 游戏的主入口文件，负责初始化和启动游戏应用。
 */

#include "engine/core/game_app.h"
#include<spdlog/spdlog.h>
#include "engine/scene/scene_manager.h"
#include "game/scene/game_scene.h"
#include "engine/core/context.h"
#include <entt/signal/dispatcher.hpp>
/**
 * @brief 游戏的主入口函数。
 * @param argc 命令行参数数量
 * @param argv 命令行参数数组
 * @return 程序退出状态码，0表示正常退出
 * 
 * @details 该函数是游戏的入口点，负责以下操作：
 * 1. 创建 GameApp 实例
 * 2. 设置 spdlog 日志级别为 trace，用于详细的调试信息
 * 3. 启动游戏应用的运行循环
 * 4. 返回退出状态码
 */
void setupInitialScene(engine::core::Context& context) {
    // GameApp在调用run方法之前，先创建并设置初始场景
    auto game_scene = std::make_unique<game::scene::GameScene>(context);
    context.getDispatcher().trigger<engine::utils::PushSceneEvent>(engine::utils::PushSceneEvent{std::move(game_scene)});
}


int main(int /* argc */, char* /* argv */[]) {
    engine::core::GameApp app;
    spdlog::set_level(spdlog::level::info);
    app.setOnInitCallback(setupInitialScene);
    app.run();
    return 0;
}