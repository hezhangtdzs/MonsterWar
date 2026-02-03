#include"texture_manager.h"
#include <SDL3_image/SDL_image.h>
#include <SDL3/SDL_render.h>
#include <spdlog/spdlog.h>  

/**
 * @brief 构造函数，初始化纹理管理器。
 * @param renderer 指向有效的 SDL_Renderer 上下文的指针。不能为空。
 * @throws std::runtime_error 如果 renderer 为 nullptr。
 */
engine::resource::TextureManager::TextureManager(SDL_Renderer* renderer):renderer_(renderer){
	if (!renderer) {
		throw std::runtime_error("TextureManager initialization failed: renderer is nullptr");
	}
}

/**
 * @brief 从指定文件路径载入纹理资源并存入缓存。
 * @param file_path 磁盘上纹理文件的完整或相对路径。
 * @return SDL_Texture* 指向新加载纹理的原始指针。如果加载失败，返回 nullptr。
 */
SDL_Texture* engine::resource::TextureManager::loadTexture(const std::string& file_path) {
    // 检查是否已加载
    auto it = textures_.find(file_path);
    if (it != textures_.end()) {
        return it->second.get();
    }


    // 如果没加载则尝试加载纹理
    SDL_Texture* raw_texture = IMG_LoadTexture(renderer_, file_path.c_str());

    if (!raw_texture) {
        spdlog::error("加载纹理失败: '{}': {}", file_path, SDL_GetError());
        return nullptr;
    }
    if (raw_texture) {
        if (!SDL_SetTextureScaleMode(raw_texture, SDL_SCALEMODE_NEAREST)) {
            spdlog::warn("无法设置纹理缩放模式为最邻近插值");
        }
    }

    // 使用带有自定义删除器的 unique_ptr 存储加载的纹理
    textures_.emplace(file_path, std::unique_ptr<SDL_Texture, SDLTextureDeleter>(raw_texture));
    spdlog::debug("成功加载并缓存纹理: {}", file_path);

    return raw_texture;
}

/**
 * @brief 尝试获取已加载纹理的指针。
 * @param file_path 纹理文件的路径。
 * @return SDL_Texture* 如果命中缓存则直接返回，否则尝试实时从磁盘加载。
 */
SDL_Texture* engine::resource::TextureManager::getTexture(const std::string& file_path) {
    auto it = textures_.find(file_path);
    if (it != textures_.end()) {
        return it->second.get();
    }
	spdlog::debug("纹理未缓存，尝试加载: {}", file_path);
    return loadTexture(file_path);
}

/**
 * @brief 从缓存中卸载指定的纹理资源并释放内存。
 * @param file_path 要卸载的纹理文件的路径。
 */
void engine::resource::TextureManager::unloadTexture(const std::string& file_path) {
    auto it = textures_.find(file_path);
    if (it != textures_.end()) {
        textures_.erase(it);
        spdlog::debug("已卸载纹理: {}", file_path);
    } else {
        spdlog::warn("尝试卸载未加载的纹理: {}", file_path);
    }
}

/**
 * @brief 获取指定纹理的逻辑尺寸。
 * @param file_path 纹理文件的路径。
 * @return glm::vec2 包含该纹理宽度 (x) 和高度 (y) 的向量。若纹理无效则返回 {0, 0}。
 */
glm::vec2 engine::resource::TextureManager::getTextureSize(const std::string& file_path) {
    SDL_Texture* texture = getTexture(file_path);
    if (!texture) {
        spdlog::error("无法获取纹理: {}", file_path);
        return glm::vec2(0);
    }

    // 获取纹理尺寸
    glm::vec2 size;
    if (!SDL_GetTextureSize(texture, &size.x, &size.y)) {
        spdlog::error("无法查询纹理尺寸: {}", file_path);
        return glm::vec2(0);
    }
    return size;
}

/**
 * @brief 清空当前所有的纹理缓存，释放所有占用的 SDL 纹理资源。
 */
void engine::resource::TextureManager::clearTextures() {
	if (textures_.empty()) {
        spdlog::debug("纹理资源已为空，无需清空");
        return;
    }
    textures_.clear();
    spdlog::debug("已清空所有纹理资源");
}