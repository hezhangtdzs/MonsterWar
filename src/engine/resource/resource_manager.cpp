#include "resource_manager.h"
#include "texture_manager.h"
#include "audio_manager.h"
#include "font_manager.h" 
#include <SDL3_mixer/SDL_mixer.h>
#include <SDL3_ttf/SDL_ttf.h> 
#include <glm/glm.hpp>
#include <spdlog/spdlog.h>
#include <nlohmann/json.hpp>
#include <fstream>
#include <cstdio>

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

std::string_view engine::resource::ResourceManager::resolvePath(
	const std::unordered_map<ResourceId, std::string>& mapping,
	ResourceId id,
	std::string_view fallback) const {
	if (!fallback.empty()) {
		return fallback;
	}
	auto it = mapping.find(id);
	if (it != mapping.end()) {
		return it->second;
	}
	return {};
}

bool engine::resource::ResourceManager::loadResources(std::string_view mapping_path, bool preload) {
	const std::string mapping_path_str(mapping_path);
	std::FILE* fp = std::fopen(mapping_path_str.c_str(), "rb");
	if (!fp) {
		spdlog::error("无法打开资源映射文件: {}", mapping_path);
		return false;
	}

	std::string content;
	std::fseek(fp, 0, SEEK_END);
	const long size = std::ftell(fp);
	std::fseek(fp, 0, SEEK_SET);
	if (size > 0) {
		content.resize(static_cast<size_t>(size));
		std::fread(content.data(), 1, static_cast<size_t>(size), fp);
	}
	std::fclose(fp);

	nlohmann::json j;
	try {
		j = nlohmann::json::parse(content);
	} catch (const std::exception& e) {
		spdlog::error("解析资源映射文件失败: {}", e.what());
		return false;
	}

	auto load_string_map = [&](const char* key, std::unordered_map<ResourceId, std::string>& mapping,
		auto loader) {
		if (!j.contains(key) || !j[key].is_object()) {
			return;
		}
		for (auto it = j[key].begin(); it != j[key].end(); ++it) {
			if (!it.value().is_string()) {
				continue;
			}
			const std::string name = it.key();
			const std::string path = it.value().get<std::string>();
			const ResourceId id = toResourceId(name);
			mapping[id] = path;
			if (preload) {
				try {
					loader(id, path);
				} catch (const std::exception& e) {
					spdlog::error("预加载资源失败: {} ({})", path, e.what());
				}
			}
		}
	};

	load_string_map("sound", sound_mapping_, [this](ResourceId id, std::string_view path) {
		loadSound(id, path);
	});
	load_string_map("sounds", sound_mapping_, [this](ResourceId id, std::string_view path) {
		loadSound(id, path);
	});
	load_string_map("music", music_mapping_, [this](ResourceId id, std::string_view path) {
		loadMusic(id, path);
	});
	load_string_map("texture", texture_mapping_, [this](ResourceId id, std::string_view path) {
		loadTexture(id, path);
	});
	load_string_map("textures", texture_mapping_, [this](ResourceId id, std::string_view path) {
		loadTexture(id, path);
	});
	load_string_map("font", font_mapping_, [](ResourceId, std::string_view) {});
	load_string_map("fonts", font_mapping_, [](ResourceId, std::string_view) {});

	spdlog::info("资源映射表加载完成: {}", mapping_path);
	return true;
}


// --- 统一资源访问接口 ---

/**
 * @brief 从原始文件载入纹理资源。
 * @param file_path 纹理文件的相对路径。
 * @return 加载成功返回 SDL_Texture 指针，失败返回 nullptr。
 */
SDL_Texture* engine::resource::ResourceManager::loadTexture(ResourceId id, std::string_view file_path) {
	return texture_manager_->loadTexture(id, file_path);
}

/**
 * @brief 尝试获取已加载纹理的指针，如果未加载则尝试加载该文件。
 * @param file_path 纹理文件的相对路径。
 * @return SDL_Texture 指针。
 */
SDL_Texture* engine::resource::ResourceManager::getTexture(ResourceId id, std::string_view file_path) {
	const auto resolved = resolvePath(texture_mapping_, id, file_path);
	return texture_manager_->getTexture(id, resolved);
}

/**
 * @brief 卸载指定的纹理资源并从缓存中移除。
 * @param file_path 要卸载的纹理文件路径。
 */
void engine::resource::ResourceManager::unloadTexture(ResourceId id) {
	texture_manager_->unloadTexture(id);
}

/**
 * @brief 获取指定纹理的逻辑尺寸。
 * @param file_path 纹理文件路径。
 * @return 包含宽度（x）和高度（y）的 glm::vec2。
 */
glm::vec2 engine::resource::ResourceManager::getTextureSize(ResourceId id, std::string_view file_path) {
	const auto resolved = resolvePath(texture_mapping_, id, file_path);
	return texture_manager_->getTextureSize(id, resolved);
}

SDL_Texture* engine::resource::ResourceManager::loadTexture(const std::string& file_path) {
	return loadTexture(toResourceId(file_path), file_path);
}

SDL_Texture* engine::resource::ResourceManager::loadTexture(entt::hashed_string str_hs) {
	return loadTexture(str_hs.value(), std::string_view{ str_hs.data(), str_hs.size() });
}

SDL_Texture* engine::resource::ResourceManager::getTexture(const std::string& file_path) {
	return getTexture(toResourceId(file_path), file_path);
}

SDL_Texture* engine::resource::ResourceManager::getTexture(entt::hashed_string str_hs) {
	const auto id = str_hs.value();
	const auto resolved = resolvePath(texture_mapping_, id, {});
	if (!resolved.empty()) {
		return getTexture(id, resolved);
	}
	return getTexture(id, std::string_view{ str_hs.data(), str_hs.size() });
}

void engine::resource::ResourceManager::unloadTexture(const std::string& file_path) {
	unloadTexture(toResourceId(file_path));
}

void engine::resource::ResourceManager::unloadTexture(entt::hashed_string str_hs) {
	unloadTexture(str_hs.value());
}

glm::vec2 engine::resource::ResourceManager::getTextureSize(const std::string& file_path) {
	return getTextureSize(toResourceId(file_path), file_path);
}

glm::vec2 engine::resource::ResourceManager::getTextureSize(entt::hashed_string str_hs) {
	const auto id = str_hs.value();
	const auto resolved = resolvePath(texture_mapping_, id, {});
	if (!resolved.empty()) {
		return getTextureSize(id, resolved);
	}
	return getTextureSize(id, std::string_view{ str_hs.data(), str_hs.size() });
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
MIX_Audio* engine::resource::ResourceManager::loadSound(ResourceId id, std::string_view file_path) {
	return audio_manager_->loadSound(id, file_path);
}

/**
 * @brief 播放音效。
 * @param file_path 音频文件相对路径。
 */
void engine::resource::ResourceManager::playSound(ResourceId id, std::string_view file_path) {
	const auto resolved = resolvePath(sound_mapping_, id, file_path);
	audio_manager_->playSound(id, resolved);
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
MIX_Audio* engine::resource::ResourceManager::getSound(ResourceId id, std::string_view file_path) {
	const auto resolved = resolvePath(sound_mapping_, id, file_path);
	return audio_manager_->getSound(id, resolved);
}

/**
 * @brief 卸载指定的音效资源。
 * @param file_path 音频文件路径。
 */
void engine::resource::ResourceManager::unloadSound(ResourceId id) {
	audio_manager_->unloadSound(id);
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
MIX_Audio* engine::resource::ResourceManager::loadMusic(ResourceId id, std::string_view file_path) {
	return audio_manager_->loadMusic(id, file_path);
}

/**
 * @brief 播放背景音乐。
 * @param file_path 音乐文件路径。
 */
void engine::resource::ResourceManager::playMusic(ResourceId id, std::string_view file_path) {
	const auto resolved = resolvePath(music_mapping_, id, file_path);
	audio_manager_->playMusic(id, resolved);
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
MIX_Audio* engine::resource::ResourceManager::getMusic(ResourceId id, std::string_view file_path) {
	const auto resolved = resolvePath(music_mapping_, id, file_path);
	return audio_manager_->getMusic(id, resolved);
}

/**
 * @brief 卸载指定的音乐资源。
 * @param file_path 音乐文件路径。
 */
void engine::resource::ResourceManager::unloadMusic(ResourceId id) {
	audio_manager_->unloadMusic(id);
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
TTF_Font* engine::resource::ResourceManager::loadFont(ResourceId id, std::string_view file_path, int point_size) {
	return font_manager_->loadFont(id, file_path, point_size);
}

/**
 * @brief 尝试获取已加载的字体指针，如果未加载则尝试加载。
 * @param file_path 字体文件的相对路径。
 * @param point_size 字体的大小（号）。
 * @return TTF_Font 指针。
 */
TTF_Font* engine::resource::ResourceManager::getFont(ResourceId id, std::string_view file_path, int point_size) {
	const auto resolved = resolvePath(font_mapping_, id, file_path);
	return font_manager_->getFont(id, resolved, point_size);
}

/**
 * @brief 卸载指定路径和大小的字体资源。
 * @param file_path 字体文件路径。
 * @param point_size 字体的大小。
 */
void engine::resource::ResourceManager::unloadFont(ResourceId id, int point_size) {
	font_manager_->unloadFont(id, point_size);
}

MIX_Audio* engine::resource::ResourceManager::loadSound(const std::string& file_path) {
	return loadSound(toResourceId(file_path), file_path);
}

MIX_Audio* engine::resource::ResourceManager::loadSound(entt::hashed_string str_hs) {
	return loadSound(str_hs.value(), std::string_view{ str_hs.data(), str_hs.size() });
}

void engine::resource::ResourceManager::playSound(const std::string& file_path) {
	playSound(toResourceId(file_path), file_path);
}

void engine::resource::ResourceManager::playSound(entt::hashed_string str_hs) {
	const auto id = str_hs.value();
	const auto resolved = resolvePath(sound_mapping_, id, {});
	if (!resolved.empty()) {
		playSound(id, resolved);
		return;
	}
	playSound(id, std::string_view{ str_hs.data(), str_hs.size() });
}

MIX_Audio* engine::resource::ResourceManager::getSound(const std::string& file_path) {
	return getSound(toResourceId(file_path), file_path);
}

MIX_Audio* engine::resource::ResourceManager::getSound(entt::hashed_string str_hs) {
	const auto id = str_hs.value();
	const auto resolved = resolvePath(sound_mapping_, id, {});
	if (!resolved.empty()) {
		return getSound(id, resolved);
	}
	return getSound(id, std::string_view{ str_hs.data(), str_hs.size() });
}

void engine::resource::ResourceManager::unloadSound(const std::string& file_path) {
	unloadSound(toResourceId(file_path));
}

void engine::resource::ResourceManager::unloadSound(entt::hashed_string str_hs) {
	unloadSound(str_hs.value());
}

MIX_Audio* engine::resource::ResourceManager::loadMusic(const std::string& file_path) {
	return loadMusic(toResourceId(file_path), file_path);
}

MIX_Audio* engine::resource::ResourceManager::loadMusic(entt::hashed_string str_hs) {
	return loadMusic(str_hs.value(), std::string_view{ str_hs.data(), str_hs.size() });
}

void engine::resource::ResourceManager::playMusic(const std::string& file_path) {
	playMusic(toResourceId(file_path), file_path);
}

void engine::resource::ResourceManager::playMusic(entt::hashed_string str_hs) {
	const auto id = str_hs.value();
	const auto resolved = resolvePath(music_mapping_, id, {});
	if (!resolved.empty()) {
		playMusic(id, resolved);
		return;
	}
	playMusic(id, std::string_view{ str_hs.data(), str_hs.size() });
}

MIX_Audio* engine::resource::ResourceManager::getMusic(const std::string& file_path) {
	return getMusic(toResourceId(file_path), file_path);
}

MIX_Audio* engine::resource::ResourceManager::getMusic(entt::hashed_string str_hs) {
	const auto id = str_hs.value();
	const auto resolved = resolvePath(music_mapping_, id, {});
	if (!resolved.empty()) {
		return getMusic(id, resolved);
	}
	return getMusic(id, std::string_view{ str_hs.data(), str_hs.size() });
}

void engine::resource::ResourceManager::unloadMusic(const std::string& file_path) {
	unloadMusic(toResourceId(file_path));
}

void engine::resource::ResourceManager::unloadMusic(entt::hashed_string str_hs) {
	unloadMusic(str_hs.value());
}

TTF_Font* engine::resource::ResourceManager::loadFont(const std::string& file_path, int point_size) {
	return loadFont(toResourceId(file_path), file_path, point_size);
}

TTF_Font* engine::resource::ResourceManager::loadFont(entt::hashed_string str_hs, int point_size) {
	return loadFont(str_hs.value(), std::string_view{ str_hs.data(), str_hs.size() }, point_size);
}

TTF_Font* engine::resource::ResourceManager::getFont(const std::string& file_path, int point_size) {
	return getFont(toResourceId(file_path), file_path, point_size);
}

TTF_Font* engine::resource::ResourceManager::getFont(entt::hashed_string str_hs, int point_size) {
	const auto id = str_hs.value();
	const auto resolved = resolvePath(font_mapping_, id, {});
	if (!resolved.empty()) {
		return getFont(id, resolved, point_size);
	}
	return getFont(id, std::string_view{ str_hs.data(), str_hs.size() }, point_size);
}

void engine::resource::ResourceManager::unloadFont(const std::string& file_path, int point_size) {
	unloadFont(toResourceId(file_path), point_size);
}

void engine::resource::ResourceManager::unloadFont(entt::hashed_string str_hs, int point_size) {
	unloadFont(str_hs.value(), point_size);
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