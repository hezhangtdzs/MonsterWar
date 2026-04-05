#pragma once

#include "../../engine/utils/math.h"

namespace game::defs {

constexpr float BLOCK_RADIUS = 40.0f;   ///< @brief 阻挡半径
constexpr float UNIT_RADIUS = 16.0f;    ///< @brief 单位判定半径，用于射程补偿
constexpr float HOVER_RADIUS = 30.0f;   ///< @brief 鼠标悬浮/点选判定半径
constexpr float PLACE_RADIUS = 24.0f;   ///< @brief 放置点判定半径
constexpr engine::utils::FColor RANGE_COLOR = { 0.2f, 0.95f, 0.35f, 0.22f }; ///< @brief 远程范围预览颜色
constexpr int INITIAL_BASE_HP = 10;      ///< @brief 基地初始生命值
constexpr int INITIAL_GOLD = 100;        ///< @brief 初始金币
constexpr float WAVE_SPAWN_INTERVAL = 0.9f;   ///< @brief 单个敌人刷出间隔
constexpr float WAVE_BREAK_DURATION = 3.0f;    ///< @brief 波次间隔
constexpr int WAVE_BASE_COUNT = 4;      ///< @brief 基础波次敌人数

/// @brief 玩家类型枚举
enum class PlayerType {
    UNKNOWN,
    MELEE,      ///< @brief 近战型，只能放在近战区域
    RANGED,     ///< @brief 远程型，只能放在远程区域
    MIXED       ///< @brief 混合型，可以放在任意区域（暂不实现，未来可拓展）
};

}   // namespace game::defs