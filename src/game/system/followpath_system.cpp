#include "followpath_system.h"
#include "game/component/enemy_component.h"
#include "engine/component/transform_component.h"
#include "engine/component/velocity_component.h"
#include "engine/utils/math.h"
#include "game/data/waypoint_node.h"
#include "game/defs/tags.h"
#include "game/defs/event.h"
#include <spdlog/spdlog.h>
#include <glm/glm.hpp>

namespace game::system {

    void FollowPathSystem::update(
        entt::registry& registry,
        entt::dispatcher& dispatcher,
        std::unordered_map<int, game::data::WaypointNode>& waypoint_nodes)
    {
        auto view = registry.view<
            game::component::EnemyComponent,
            engine::component::TransformComponent,
            engine::component::VelocityComponent>();

        for (auto entity : view) {
            auto& enemy = registry.get<game::component::EnemyComponent>(entity);
            auto& transform = registry.get<engine::component::TransformComponent>(entity);
            auto& velocity = registry.get<engine::component::VelocityComponent>(entity);

            // 获取当前目标节点
            auto target_node = waypoint_nodes.at(enemy.target_waypoint_id_);

            // 计算当前位置到目标位置的向量
            glm::vec2 direction = target_node.position_ - transform.position_;

            // 如果距离小于阈值，则切换到下一个节点
            if (glm::length(direction) < 5.0f) {
                // 如果下一个节点ID列表为空，代表到达终点
                auto size = target_node.next_node_ids_.size();
                if (size == 0) {
                    spdlog::info("到达终点");
                    // 发送信号并添加删除标记
                    dispatcher.enqueue<game::defs::EnemyArriveHomeEvent>();
                    registry.emplace<game::defs::DeadTag>(entity);
                    continue;
                }

                // 随机选择下一个节点
                auto target_index = engine::utils::randomInt(0, size - 1);
                enemy.target_waypoint_id_ = target_node.next_node_ids_[target_index];

                // 更新目标节点与方向矢量
                target_node = waypoint_nodes.at(enemy.target_waypoint_id_);
                direction = target_node.position_ - transform.position_;
            }

            // 更新速度组件：velocity = 方向矢量 * speed
            velocity.velocity_ = glm::normalize(direction) * enemy.speed_;
        }
    }

} // namespace game::system
