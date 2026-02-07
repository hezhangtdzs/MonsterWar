#pragma once
#include <string>
#include <entt/core/hashed_string.hpp>
#include <entt/entity/entity.hpp>

namespace game::component {

/**
 * @struct ClassNameComponent
 * @brief 类名组件，存储实体的类型标识
 *
 * @details
 * ClassNameComponent 用于标识实体的类型，包含类型的哈希ID和名称字符串。
 * 这对于工厂创建的实体非常重要，可以追踪实体是由哪个蓝图创建的。
 */
struct ClassNameComponent {
    entt::id_type class_id_ = entt::null;   ///< 类型ID（哈希值）
    std::string class_name_;                ///< 类型名称

    /**
     * @brief 构造函数
     * @param class_id 类型ID（使用 entt::hashed_string 生成）
     * @param class_name 类型名称
     */
    ClassNameComponent(entt::id_type class_id, std::string class_name)
        : class_id_(class_id), class_name_(std::move(class_name)) {}
};

} // namespace game::component
