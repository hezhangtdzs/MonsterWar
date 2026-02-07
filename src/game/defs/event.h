/**
 * @file event.h
 * @brief 定义游戏中使用的事件类型。
 * 
 * @details
 * 该文件定义了游戏中使用的各种事件类型，这些事件用于游戏系统之间的通信。
 * 使用 EnTT 的事件分发系统，系统可以订阅和发布这些事件，实现松耦合的通信。
 * 
 * @par 事件系统工作原理
 * 1. 定义事件结构体（通常为空结构体，仅作为类型标识）
 * 2. 系统通过 dispatcher.sink<EventType>().connect<&System::onEvent>(this) 订阅事件
 * 3. 其他系统或逻辑通过 dispatcher.enqueue(EventType()) 发布事件
 * 4. EnTT 自动将事件分发给所有订阅者
 * 
 * @see engine::core::Context 提供事件分发器的上下文
 * @see entt::dispatcher EnTT 的事件分发器类
 */

#pragma once

namespace game::defs {

/**
 * @struct EnemyArriveHomeEvent
 * @brief 敌人到达基地的事件。
 * 
 * @details
 * 当敌人成功到达玩家基地时触发此事件。
 * 通常由 FollowPathSystem 在敌人到达最终路径点时发布。
 * 
 * @par 用途
 * - 通知游戏状态系统减少玩家生命值
 * - 触发游戏结束检查
 * - 播放相应的音效或视觉效果
 * - 移除到达基地的敌人实体
 */
struct EnemyArriveHomeEvent {};         ///< 敌人到达基地的事件

}   // namespace game::defs