#pragma once
/**
 * @file config.h
 * @brief 定义 Config 类，用于管理游戏配置。
 */

#include <string>
#include <vector>
#include <unordered_map>
#include <nlohmann/json_fwd.hpp>    // nlohmann_json 提供的前向声明

namespace engine::core {

    /**
     * @class Config
     * @brief 配置管理类，负责加载、保存和管理游戏配置。
     * 
     * Config 类提供了对游戏配置的统一管理，包括窗口设置、图形设置、
     * 性能设置、音频设置和输入映射等。配置数据存储在 JSON 文件中，
     * 支持从文件加载和保存到文件。
     */
    class Config final {
    public:
        // --- 默认配置值 --- (为了方便拓展，全部设置为公有)
        // 窗口设置
        std::string window_title_ = "SunnyLand";    ///< 窗口标题
        int window_width_ = 1280;                   ///< 窗口宽度
        int window_height_ = 720;                   ///< 窗口高度
        bool window_resizable_ = true;              ///< 窗口是否可调整大小

        // 图形设置
        bool vsync_enabled_ = true;             ///< 是否启用垂直同步

        // 性能设置
        int target_fps_ = 144;                  ///< 目标 FPS 设置，0 表示不限制

        // 音频设置
        float master_volume_ = 0.5f;             ///< 主音量 (0.0 - 1.0)
        float music_volume_ = 0.5f;             ///< 音乐音量 (0.0 - 1.0)
        float sound_volume_ = 0.5f;             ///< 音效音量 (0.0 - 1.0)

        // 存储动作名称到 SDL Scancode 名称列表的映射
        std::unordered_map<std::string, std::vector<std::string>> input_mappings_ = {   ///< 输入映射表
            // 提供一些合理的默认值，以防配置文件加载失败或缺少此部分
            {"move_left", {"A", "Left"}},
            {"move_right", {"D", "Right"}},
            {"move_up", {"W", "Up"}},
            {"move_down", {"S", "Down"}},
            {"jump", {"J", "Space"}},
            {"attack", {"K", "MouseLeft"}},
            {"pause", {"P", "Escape"}},
            {"switch_player", {"Tab"}},
            // 可以继续添加更多默认动作
        };

        /**
         * @brief 构造函数，指定配置文件路径。
         * @param filepath 配置文件路径
         */
        explicit Config(const std::string& filepath);

        // 删除拷贝和移动语义
        Config(const Config&) = delete;
        Config& operator=(const Config&) = delete;
        Config(Config&&) = delete;
        Config& operator=(Config&&) = delete;

        /**
         * @brief 从指定的 JSON 文件加载配置。
         * @param filepath 配置文件路径
         * @return 成功返回 true，否则返回 false
         */
        bool loadFromFile(const std::string& filepath);
        
        /**
         * @brief 将当前配置保存到指定的 JSON 文件。
         * @param filepath 配置文件路径
         * @return 成功返回 true，否则返回 false
         */
        [[nodiscard]] bool saveToFile(const std::string& filepath);

    private:
        /**
         * @brief 从 JSON 对象反序列化配置。
         * @param j JSON 对象
         */
        void fromJson(const nlohmann::json& j);
        
        /**
         * @brief 将当前配置转换为 JSON 对象（按顺序）。
         * @return JSON 对象
         */
        nlohmann::ordered_json toJson() const;
    };

} // namespace engine::core