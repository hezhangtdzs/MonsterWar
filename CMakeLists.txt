cmake_minimum_required(VERSION 3.10.0)
project(SunnyLand VERSION 0.1.0 LANGUAGES C CXX)

# C++ 标准
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# 编译选项
if(MSVC)
    add_compile_options(/W4)
    add_compile_options("$<$<C_COMPILER_ID:MSVC>:/utf-8>")
    add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# 输出目录
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_SOURCE_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_SOURCE_DIR})

set(TARGET ${PROJECT_NAME}-${CMAKE_SYSTEM_NAME})

# 查找依赖库
find_package(SDL3 REQUIRED)
find_package(SDL3_image REQUIRED)
find_package(SDL3_mixer REQUIRED)
find_package(SDL3_ttf REQUIRED)
find_package(glm REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(spdlog REQUIRED)
find_package(EnTT REQUIRED)

# 核心源文件
set(CORE_SOURCES
    src/main.cpp

    src/engine/core/config.cpp
    src/engine/core/context.cpp
    src/engine/core/game_app.cpp
    src/engine/core/time.cpp
    src/engine/core/game_state.cpp

    src/engine/resource/resource_manager.cpp
    src/engine/resource/texture_manager.cpp
    src/engine/resource/audio_manager.cpp
    src/engine/resource/font_manager.cpp

    src/engine/audio/audio_locator.cpp
    src/engine/audio/audio_player.cpp
    src/engine/audio/log_audio_player.cpp

    src/engine/render/animation.cpp
    src/engine/render/camera.cpp
    src/engine/render/renderer.cpp
    src/engine/render/text_renderer.cpp

    src/engine/system/movement_system.cpp
    src/engine/system/animation_system.cpp
    src/engine/system/render_system.cpp

    src/engine/input/input_manager.cpp

    src/engine/ui/ui_button.cpp
    src/engine/ui/ui_element.cpp
    src/engine/ui/ui_image.cpp
    src/engine/ui/ui_manager.cpp
    src/engine/ui/ui_panel.cpp
    src/engine/ui/ui_text.cpp
    src/engine/ui/ui_interactive.cpp
    src/engine/ui/state/normal_state.cpp
    src/engine/ui/state/hover_state.cpp
    src/engine/ui/state/pressed_state.cpp


    # src/engine/scene/level_loader.cpp
    src/engine/scene/scene.cpp
    src/engine/scene/scene_manager.cpp


    src/engine/interface/subject.cpp
    src/engine/interface/observer.cpp
    
    src/game/scene/game_scene.cpp
)

# 扩展库源文件
set(EXTERNAL_SOURCES
    external/imgui/imgui.cpp
    external/imgui/imgui_demo.cpp
    external/imgui/imgui_draw.cpp
    external/imgui/imgui_tables.cpp
    external/imgui/imgui_widgets.cpp
    external/imgui/backends/imgui_impl_sdl3.cpp
    external/imgui/backends/imgui_impl_sdlrenderer3.cpp
)

# 可执行目标
add_executable(
    ${TARGET}
    ${CORE_SOURCES}
    ${EXTERNAL_SOURCES}
)


# 包含目录
target_include_directories(${TARGET} PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/external/imgui
    ${CMAKE_SOURCE_DIR}/external/imgui/backends
)

# 链接库
target_link_libraries(${TARGET}
    ${SDL3_LIBRARIES}
    SDL3_image::SDL3_image
    SDL3_mixer::SDL3_mixer
    SDL3_ttf::SDL3_ttf
    glm::glm
    nlohmann_json::nlohmann_json
    spdlog::spdlog
    EnTT::EnTT
)
