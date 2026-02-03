#include "config.h"
#include "spdlog/spdlog.h"
#include <fstream>
#include <nlohmann/json.hpp>

/**
 * @brief 构造函数，指定配置文件路径并尝试加载配置。
 * @param filepath 配置文件路径
 */
engine::core::Config::Config(const std::string& filepath)
{
    loadFromFile(filepath);
}

/**
 * @brief 从指定的 JSON 文件加载配置。
 * @param filepath 配置文件路径
 * @return 成功返回 true，否则返回 false
 */
bool engine::core::Config::loadFromFile(const std::string& filepath)
{
    std::ifstream file(filepath);
    if (!file.is_open()) {
        spdlog::warn("配置文件 '{}' 未找到。使用默认设置并尝试创建默认配置文件。", filepath);
        if (saveToFile(filepath)) {
            // 虽然原始文件不存在，但由于成功保存了默认配置，我们认为状态是正确的
            return true;
        }
        return false;
    }
    try {
        nlohmann::json j;
        file >> j;
        fromJson(j);
        spdlog::info("成功加载配置文件 '{}'.", filepath);
        return true;
    }
    catch (const std::exception& e) {
        spdlog::error("加载配置文件 '{}' 时出错: {}", filepath, e.what());
        return false;
    }
}

/**
 * @brief 将当前配置保存到指定的 JSON 文件。
 * @param filepath 配置文件路径
 * @return 成功返回 true，否则返回 false
 */
bool engine::core::Config::saveToFile(const std::string& filepath)
{
    std::ofstream file(filepath);
    if (!file.is_open()) {
        spdlog::error("无法打开配置文件 '{}' 进行写入。", filepath);
        return false;
    }
    try {
        nlohmann::ordered_json j = toJson();
        file << j.dump(4);
        spdlog::debug("已保存配置文件 '{}'.", filepath);
        return true;
    }
    catch (const std::exception& e) {
        spdlog::error("保存配置文件 '{}' 时出错: {}", filepath, e.what());
        return false;
    }
}

/**
 * @brief 从 JSON 对象反序列化配置。
 * @param j JSON 对象
 */
void engine::core::Config::fromJson(const nlohmann::json& j)
{
    if (j.contains("window") && j["window"].is_object()) {
        const auto& window_config = j["window"];
        window_title_ = window_config.value("title", window_title_);
        window_width_ = window_config.value("width", window_width_);
        window_height_ = window_config.value("height", window_height_);
        window_resizable_ = window_config.value("resizable", window_resizable_);
    }

    if (j.contains("graphics") && j["graphics"].is_object()) {
        const auto& graphics_config = j["graphics"];
        vsync_enabled_ = graphics_config.value("vsync", vsync_enabled_);
    }

    if (j.contains("performance") && j["performance"].is_object()) {
        const auto& perf_config = j["performance"];
        target_fps_ = perf_config.value("target_fps", target_fps_);
        if (target_fps_ < 0) {
            spdlog::warn("配置警告：目标 FPS ({}) 不能为负数。已重置为 0（无限制）。", target_fps_);
            target_fps_ = 0;
        }
    }

    if (j.contains("audio") && j["audio"].is_object()) {
        const auto& audio_config = j["audio"];
        master_volume_ = audio_config.value("master_volume", master_volume_);
        if (master_volume_ < 0.0f || master_volume_ > 1.0f) {
            spdlog::warn("配置警告：主音量 ({}) 必须在 0.0 - 1.0 范围内。已重置为 0.5。", master_volume_);
            master_volume_ = 0.5f;
        }
        music_volume_ = audio_config.value("music_volume", music_volume_);
        if (music_volume_ < 0.0f || music_volume_ > 1.0f) {
            spdlog::warn("配置警告：音乐音量 ({}) 必须在 0.0 - 1.0 范围内。已重置为 0.5。", music_volume_);
            music_volume_ = 0.5f;
        }
        sound_volume_ = audio_config.value("sound_volume", sound_volume_);
        if (sound_volume_ < 0.0f || sound_volume_ > 1.0f) {
            spdlog::warn("配置警告：音效音量 ({}) 必须在 0.0 - 1.0 范围内。已重置为 0.5。", sound_volume_);
            sound_volume_ = 0.5f;
        }
    }

    // 合并输入映射，而不是整个替换
    if (j.contains("input_mappings") && j["input_mappings"].is_object()) {
        const auto& mappings_json = j["input_mappings"];
        for (auto it = mappings_json.begin(); it != mappings_json.end(); ++it) {
            try {
                if (it.value().is_array()) {
                    input_mappings_[it.key()] = it.value().get<std::vector<std::string>>();
                }
            } catch (const std::exception& e) {
                spdlog::warn("解析输入映射 '{}' 时出错: {}", it.key(), e.what());
            }
        }
        spdlog::trace("已从配置更新输入映射。");
    }
}

/**
 * @brief 将当前配置转换为 JSON 对象（按顺序）。
 * @return JSON 对象
 */
nlohmann::ordered_json engine::core::Config::toJson() const
{
    return nlohmann::ordered_json{
        {"window", {
            {"title", window_title_},
            {"width", window_width_},
            {"height", window_height_},
            {"resizable", window_resizable_}
        }},
        {"graphics", {
            {"vsync", vsync_enabled_}
        }},
        {"performance", {
            {"target_fps", target_fps_}
        }},
        {"audio", {
            {"music_volume", music_volume_},
            {"sound_volume", sound_volume_}
        }},
        {"input_mappings", input_mappings_}
    };
}
