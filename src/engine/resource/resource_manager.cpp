#include "resource_manager.h"
#include "texture_manager.h"
#include "audio_manager.h"
#include "font_manager.h" 
#include <SDL3_mixer/SDL_mixer.h>
#include <SDL3_ttf/SDL_ttf.h> 
#include <glm/glm.hpp>
#include <spdlog/spdlog.h>

/**
 * @brief 构造函数，初始化各个子资源管理器。
 * @param renderer SDL_Renderer 指针，用于创建纹理。
 */
engine::resource::ResourceManager::ResourceManager(SDL_Renderer* renderer)
	: texture_manager_(std::make_unique<TextureManager>(renderer)),
	  font_manager_(std::make_unique<FontManager>()),
	  audio_manager_(std::make_unique<AudioManager>()) {
	spdlog::trace("ResourceManager 构造成功。");
}

/**
 * @brief 析构函数，确保所有持有的资源管理器被正确销毁。
 */
engine::resource::ResourceManager::~ResourceManager() = default;


// --- 统一资源访问接口 ---

/**
 * @brief 从原始文件载入纹理资源。
 * @param file_path 纹理文件的相对路径。
 * @return 加载成功返回 SDL_Texture 指针，失败返回 nullptr。
 */
SDL_Texture* engine::resource::ResourceManager::loadTexture(const std::string& file_path) {
	return texture_manager_->loadTexture(file_path);
}

/**
 * @brief 尝试获取已加载纹理的指针，如果未加载则尝试加载该文件。
 * @param file_path 纹理文件的相对路径。
 * @return SDL_Texture 指针。
 */
SDL_Texture* engine::resource::ResourceManager::getTexture(const std::string& file_path) {
	return texture_manager_->getTexture(file_path);
}

/**
 * @brief 卸载指定的纹理资源并从缓存中移除。
 * @param file_path 要卸载的纹理文件路径。
 */
void engine::resource::ResourceManager::unloadTexture(const std::string& file_path) {
	texture_manager_->unloadTexture(file_path);
}

/**
 * @brief 获取指定纹理的逻辑尺寸。
 * @param file_path 纹理文件路径。
 * @return 包含宽度（x）和高度（y）的 glm::vec2。
 */
glm::vec2 engine::resource::ResourceManager::getTextureSize(const std::string& file_path) {
	return texture_manager_->getTextureSize(file_path);
}

/**
 * @brief 清空所有已加载的纹理资源。
 */
void engine::resource::ResourceManager::clearTextures(){
	texture_manager_->clearTextures();
}


//--- 统一音乐音效访问接口 ---

/**
 * @brief 载入音效（Sound Effect）资源。
 * @param file_path 音频文件的相对路径。
 * @return MIX_Audio 指针。
 */
MIX_Audio* engine::resource::ResourceManager::loadSound(const std::string& file_path) {
	return audio_manager_->loadSound(file_path);
}

/**
 * @brief 播放音效。
 * @param file_path 音频文件相对路径。
 */
void engine::resource::ResourceManager::playSound(const std::string& file_path) {
	audio_manager_->playSound(file_path);
}

/**
 * @brief 停止当前播放的音效。
 */
void engine::resource::ResourceManager::stopSound() {
	audio_manager_->stopSound();
}

/**
 * @brief 尝试获取已加载音效的指针，如果未加载则尝试从文件加载。
 * @param file_path 音频文件的相对路径。
 * @return MIX_Audio 指针。
 */
MIX_Audio* engine::resource::ResourceManager::getSound(const std::string& file_path) {
	return audio_manager_->getSound(file_path);
}

/**
 * @brief 卸载指定的音效资源。
 * @param file_path 音频文件路径。
 */
void engine::resource::ResourceManager::unloadSound(const std::string& file_path) {
	audio_manager_->unloadSound(file_path);
}

/**
 * @brief 清空所有已加载的音效资源。
 */
void engine::resource::ResourceManager::clearSounds() {
	audio_manager_->clearSounds();
}

/**
 * @brief 载入音乐（Music）资源，通常用于背景音乐。
 * @param file_path 音乐文件的相对路径。
 * @return MIX_Audio 指针。
 */
MIX_Audio* engine::resource::ResourceManager::loadMusic(const std::string& file_path) {
	return audio_manager_->loadMusic(file_path);
}

/**
 * @brief 播放背景音乐。
 * @param file_path 音乐文件路径。
 */
void engine::resource::ResourceManager::playMusic(const std::string& file_path) {
	audio_manager_->playMusic(file_path);
}

/**
 * @brief 停止背景音乐播放。
 */
void engine::resource::ResourceManager::stopMusic() {
	audio_manager_->stopMusic();
}

/**
 * @brief 设置主音量增益。
 * @param gain 增益值（0.0f-1.0f）。
 */
void engine::resource::ResourceManager::setMasterGain(float gain) {
	audio_manager_->setMasterGain(gain);
}

/**
 * @brief 设置音效音量增益。
 * @param gain 增益值（0.0f-1.0f）。
 */
void engine::resource::ResourceManager::setSoundGain(float gain) {
	audio_manager_->setSoundGain(gain);
}

/**
 * @brief 设置音乐音量增益。
 * @param gain 增益值（0.0f-1.0f）。
 */
void engine::resource::ResourceManager::setMusicGain(float gain) {
	audio_manager_->setMusicGain(gain);
}

/**
 * @brief 获取已加载音乐的指针，如果未加载则尝试加载。
 * @param file_path 音乐文件的相对路径。
 * @return MIX_Audio 指针。
 */
MIX_Audio* engine::resource::ResourceManager::getMusic(const std::string& file_path) {
	return audio_manager_->getMusic(file_path);
}

/**
 * @brief 卸载指定的音乐资源。
 * @param file_path 音乐文件路径。
 */
void engine::resource::ResourceManager::unloadMusic(const std::string& file_path) {
	audio_manager_->unloadMusic(file_path);
}

/**
 * @brief 清空所有已加载的音乐资源。
 */
void engine::resource::ResourceManager::clearMusic() {
	audio_manager_->clearMusic();
}

/**
 * @brief 清理所有音频资源（包括音效和音乐）。
 */
void engine::resource::ResourceManager::cleanAudio()
{
	audio_manager_->clearAudio();
}

//--- 统一字体访问接口 ---

/**
 * @brief 载入特定点大小的字体资源。
 * @param file_path 字体文件的相对路径。
 * @param point_size 字体的大小（号）。
 * @return TTF_Font 指针。
 */
TTF_Font* engine::resource::ResourceManager::loadFont(const std::string& file_path, int point_size) {
	return font_manager_->loadFont(file_path, point_size);
}

/**
 * @brief 尝试获取已加载的字体指针，如果未加载则尝试加载。
 * @param file_path 字体文件的相对路径。
 * @param point_size 字体的大小（号）。
 * @return TTF_Font 指针。
 */
TTF_Font* engine::resource::ResourceManager::getFont(const std::string& file_path, int point_size) {
	return font_manager_->getFont(file_path, point_size);
}

/**
 * @brief 卸载指定路径和大小的字体资源。
 * @param file_path 字体文件路径。
 * @param point_size 字体的大小。
 */
void engine::resource::ResourceManager::unloadFont(const std::string& file_path, int point_size) {
	font_manager_->unloadFont(file_path, point_size);
}

/**
 * @brief 清空所有已加载的字体资源。
 */
void engine::resource::ResourceManager::clearFonts() {
	font_manager_->clearFonts();
}

/**
 * @brief 清空所有类型的资源缓存。
 */
void engine::resource::ResourceManager::clear() {
	clearTextures();
	clearFonts();
	cleanAudio();
}