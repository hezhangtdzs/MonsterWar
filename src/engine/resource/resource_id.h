/**
 * @file resource_id.h
 * @brief 定义资源ID类型和相关工具函数。
 * 
 * 该文件提供了资源管理系统的核心类型定义，使用 entt 库的哈希字符串
 * 作为资源标识符，支持编译期和运行期的资源ID生成。
 */

#pragma once

#include <entt/entt.hpp>
#include <string_view>

namespace engine::resource {
	/**
	 * @brief 资源ID类型，使用 entt::id_type 作为底层类型。
	 * 
	 * 资源ID是通过哈希字符串生成的唯一标识符，用于快速查找和引用资源。
	 */
	using ResourceId = entt::id_type;

	/**
	 * @brief 无效资源ID常量。
	 * 
	 * 用于表示无效或未初始化的资源ID。
	 */
	constexpr ResourceId InvalidResourceId = ResourceId{};

	/**
	 * @brief 将字符串转换为资源ID。
	 * 
	 * 使用 entt::hashed_string 在编译期或运行期计算字符串的哈希值。
	 * 
	 * @param key 要转换的字符串视图
	 * @return 对应的资源ID
	 */
	constexpr ResourceId toResourceId(std::string_view key) noexcept {
		return entt::hashed_string{ key.data(), key.size() }.value();
	}

	/**
	 * @brief 获取类型的唯一ID。
	 * 
	 * 使用 entt::type_id 生成类型的唯一哈希标识符，可用于类型映射。
	 * 
	 * @tparam T 要获取ID的类型
	 * @return 类型的唯一ID
	 */
	template<typename T>
	constexpr ResourceId typeId() noexcept {
		return entt::type_id<T>().hash();
	}
}
