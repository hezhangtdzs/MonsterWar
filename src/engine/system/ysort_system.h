#pragma once
#include <entt/entity/registry.hpp>

namespace engine::system {
    class YSortSystem {
    public:
        YSortSystem() = default;
        ~YSortSystem() = default;

        void update(entt::registry& registry);
    };
}