#include "remove_dead_system.h"
#include "game/defs/tags.h"
#include "game/defs/event.h"
#include <spdlog/spdlog.h>
void game::system::RemoveDeadSystem::update(entt::registry& registry) {
    auto view = registry.view<defs::DeadTag>();
    for (auto entity : view) {
        auto entity_id = static_cast<entt::id_type>(entity);
        registry.destroy(entity);
        spdlog::info("Entity {} destroyed", entity_id);
    }
}
