/**
 * @file tags.h
 * @brief 定义游戏中使用的标签组件。
 * 
 * @details
 * 该文件定义了游戏中使用的各种标签组件。在 EnTT ECS 架构中，
 * 空结构体可以作为"标签组件"使用，它们不占用内存空间，
 * 是给实体添加分类或状态标记的最佳方式。
 * 
 * @par 标签组件的优势
 * - 零内存开销：空结构体不占用任何存储空间
 * - 快速查询：使用 EnTT 的 view 功能可以快速筛选带标签的实体
 * - 清晰语义：通过类型名明确表达实体的状态或分类
 * - 灵活组合：可以同时给一个实体添加多个标签
 * 
 * @par 使用示例
 * @code
 * // 添加标签到实体
 * registry.emplace<game::defs::DeadTag>(entity);
 * 
 * // 查询带标签的实体
 * auto dead_entities = registry.view<game::defs::DeadTag>();
 * for (auto entity : dead_entities) {
 *     // 处理死亡实体
 * }
 * @endcode
 * 
 * @see entt::registry EnTT 的实体注册表
 * @see entt::view EnTT 的实体视图，用于查询符合条件的实体
 */

#pragma once

namespace game::defs {

/**
 * @struct DeadTag
 * @brief 死亡标签，用于标记实体死亡并延时删除。
 * 
 * @details
 * 当实体被标记为死亡时，添加此标签。
 * RemoveDeadSystem 会定期清理带有此标签的实体。
 * 
 * @par 用途
 * - 允许实体在死亡后播放死亡动画或特效
 * - 避免在游戏逻辑中直接删除实体导致的迭代器失效
 * - 统一管理实体的生命周期
 * 
 * @see game::system::RemoveDeadSystem 处理带 DeadTag 的实体
 */
struct DeadTag {};              ///< 死亡标签，用于标记实体死亡并延时删除

/**
 * @struct FaceLeftTag
 * @brief 朝左标签，用于标记精灵默认朝左（需要翻转显示）。
 * 
 * @details
 * 当精灵资源默认朝向右侧，但实体需要朝向左侧时，添加此标签。
 * 渲染系统会根据此标签自动翻转精灵。
 * 
 * @par 用途
 * - 统一管理精灵的朝向
 * - 避免在代码中硬编码精灵翻转逻辑
 * - 与移动方向系统配合使用
 */
struct FaceLeftTag {};          ///< 朝左标签，用于标记精灵默认朝左（需要翻转显示）

/**
 * @struct MeleeUnitTag
 * @brief 近战单位标签。
 * 
 * @details
 * 标记实体为近战单位。战斗系统可以根据此标签应用近战特定的逻辑。
 * 
 * @par 用途
 * - 区分近战和远程单位的攻击逻辑
 * - 应用不同的攻击范围和动画
 * - 实现近战单位的特殊能力或行为
 */
struct MeleeUnitTag {};         ///< 近战单位标签

/**
 * @struct RangedUnitTag
 * @brief 远程单位标签。
 * 
 * @details
 * 标记实体为远程单位。战斗系统可以根据此标签应用远程特定的逻辑。
 * 
 * @par 用途
 * - 区分远程和近战单位的攻击逻辑
 * - 应用不同的攻击范围和动画
 * - 实现远程单位的特殊能力或行为
 */
struct RangedUnitTag {};        ///< 远程单位标签
/**
 * @struct HealerTag
 * @brief 治疗单位标签。
 * 
 * @details
 * 标记实体为治疗单位。战斗系统可以根据此标签应用治疗特定的逻辑。
 * 
 * @par 用途
 * - 区分治疗单位和其他类型单位的行为
 * - 应用不同的技能和动画
 * - 实现治疗单位的特殊能力或行为
 */
struct HealerTag {};        ///< 治疗单位标签

/**
 * @struct AttackReadyTag
 * @brief 攻击准备就绪标签。
 * 
 * @details
 * 当单位的攻击冷却结束，可以发起下一次攻击时，添加此标签。
 * TimerSystem 负责计时并添加此标签，AttackStarterSystem 负责发起攻击并移除此标签。
 */
struct AttackReadyTag {};

/**
 * @struct InjuredTag
 * @brief 受伤标签。
 * 
 * @details
 * 当单位当前的生命值（HP）小于最大生命值（Max HP）时，添加此标签。
 * 治疗单位（Healer）会优先寻找带有此标签的友好单位作为目标。
 */
struct InjuredTag {};

/**
 * @struct ActionLockTag
 * @brief 动作锁定标签（硬直）。
 * 
 * @details
 * 标记实体当前正在执行一个不可中断的动作（如播放攻击动画）。
 * 带有此标签的实体通常不允许进行移动。
 * 动画播放系统会在动画结束时负责移除此标签。
 */
struct ActionLockTag {};

}   // namespace game::defs