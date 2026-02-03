/**
 * @file main.cpp
 * @brief 游戏的主入口文件，负责初始化和启动游戏应用。
 */

#include "engine/core/game_app.h"
#include<spdlog/spdlog.h>
#include "engine/scene/scene_manager.h"
#include "game/scene/game_scene.h"
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
void onInitSceneManager(engine::scene::SceneManager& scene_manager) {
    scene_manager.requestPushScene(std::make_unique<game::scene::GameScene>(scene_manager.getContext(), scene_manager));
}

int main(int /* argc */, char* /* argv */[]) {
    engine::core::GameApp app;
    spdlog::set_level(spdlog::level::info);
    app.setOnInitCallback(onInitSceneManager);
    app.run();
    return 0;
}