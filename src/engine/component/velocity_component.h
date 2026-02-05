#pragma once
#include <glm/vec2.hpp>
namespace engine::component {
    /**
     * @struct VelocityComponent
     * @brief 速度组件
     *
     * 记录实体的当前移动速度。通常由物理系统或移动系统处理并应用于 TransformComponent。
     */
    struct VelocityComponent {
        glm::vec2 velocity_{ 0.0f, 0.0f }; ///< 当前移动速度向量 (像素/秒)

        /**
         * @brief 构造函数
         * @param velocity 初始速度，默认为零
         */
        explicit VelocityComponent(const glm::vec2& velocity = { 0.0f, 0.0f })
            : velocity_(velocity) {}
    };
}