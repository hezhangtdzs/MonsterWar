#include "hero_inspector_ui.h"

#include "../component/class_name_component.h"
#include "../component/hero_skill_component.h"
#include "../component/player_component.h"
#include "../component/stats_component.h"
#include "../data/game_stats.h"
#include "../data/selection_state.h"
#include "../data/session_data.h"
#include "../defs/constants.h"
#include "../defs/event.h"
#include "../factory/blueprint_manager.h"
#include "../../engine/core/context.h"
#include "../../engine/component/transform_component.h"
#include "../../engine/scene/scene.h"
#include "../../engine/utils/math.h"
#include <imgui.h>
#include <memory>

namespace game::ui {

namespace {

bool isValidUnit(entt::registry& registry, entt::entity entity) {
    return entity != entt::null && registry.valid(entity) &&
        registry.all_of<game::component::ClassNameComponent, engine::component::TransformComponent, game::component::StatsComponent>(entity);
}

void renderUnitTooltip(entt::registry& registry,
                       entt::entity entity,
                       const game::factory::BlueprintManager& blueprint_manager) {
    if (!isValidUnit(registry, entity)) {
        return;
    }

    const auto& name = registry.get<game::component::ClassNameComponent>(entity);
    const auto& transform = registry.get<engine::component::TransformComponent>(entity);
    const auto& stats = registry.get<game::component::StatsComponent>(entity);
    const auto* skill = registry.try_get<game::component::HeroSkillComponent>(entity);

    ImGui::BeginTooltip();
    ImGui::Text("单位：%s", name.class_name_.c_str());
    ImGui::Separator();
    ImGui::Text("等级：%d", stats.level_);
    ImGui::Text("生命：%.0f / %.0f", stats.hp_, stats.max_hp_);
    ImGui::Text("攻击：%.1f", stats.atk_);
    ImGui::Text("防御：%.1f", stats.def_);
    ImGui::Text("射程：%.1f", stats.range_);
    ImGui::Text("攻击间隔：%.2f", stats.atk_interval_);
    ImGui::Text("坐标：%.1f, %.1f", transform.position_.x, transform.position_.y);

    if (skill && blueprint_manager.hasSkillBlueprint(skill->skill_id_)) {
        const auto& skill_bp = blueprint_manager.getSkillBlueprint(skill->skill_id_);
        ImGui::SeparatorText("技能");
        ImGui::Text("%s", skill_bp.name_.c_str());
        ImGui::TextWrapped("%s", skill_bp.description_.c_str());
    }

    ImGui::EndTooltip();
}

void renderPortraitTooltip(const game::data::UnitData& unit,
                           const game::factory::BlueprintManager& blueprint_manager) {
    if (!blueprint_manager.hasPlayerClassBlueprint(unit.class_id_)) {
        return;
    }

    const auto& blueprint = blueprint_manager.getPlayerClassBlueprint(unit.class_id_);
    const float hp = engine::utils::statModify(blueprint.stats_.hp_, unit.level_, unit.rarity_);
    const float atk = engine::utils::statModify(blueprint.stats_.atk_, unit.level_, unit.rarity_);
    const float def = engine::utils::statModify(blueprint.stats_.def_, unit.level_, unit.rarity_);
    const float range = engine::utils::statModify(blueprint.stats_.range_, unit.level_, unit.rarity_);
    const float interval = engine::utils::statModify(blueprint.stats_.atk_interval_, unit.level_, unit.rarity_);

    ImGui::BeginTooltip();
    ImGui::Text("肖像：%s", unit.name_.c_str());
    ImGui::Text("职业：%s", unit.class_name_.c_str());
    ImGui::Separator();
    ImGui::Text("等级：%d", unit.level_);
    ImGui::Text("稀有度：%d", unit.rarity_);
    ImGui::Text("生命：%.0f", hp);
    ImGui::Text("攻击：%.1f", atk);
    ImGui::Text("防御：%.1f", def);
    ImGui::Text("射程：%.1f", range);
    ImGui::Text("攻击间隔：%.2f", interval);
    ImGui::EndTooltip();
}

void renderSelectedUnitPanel(entt::registry& registry,
                             entt::entity selected_unit,
                             const game::factory::BlueprintManager& blueprint_manager,
                             engine::scene::Scene& scene) {
    if (selected_unit == entt::null || !isValidUnit(registry, selected_unit) || !registry.all_of<game::component::PlayerComponent>(selected_unit)) {
        return;
    }

    ImGui::SetNextWindowPos(ImVec2(18.0f, 18.0f), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(420.0f, 0.0f), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowBgAlpha(0.90f);
    const ImGuiWindowFlags flags = ImGuiWindowFlags_NoSavedSettings |
        ImGuiWindowFlags_NoFocusOnAppearing |
        ImGuiWindowFlags_NoNav |
        ImGuiWindowFlags_NoDecoration |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoMouseInputs;

    if (!ImGui::Begin("英雄信息", nullptr, flags)) {
        ImGui::End();
        return;
    }

    ImGui::SetWindowFontScale(1.15f);
    const auto& name = registry.get<game::component::ClassNameComponent>(selected_unit);
    const auto& stats = registry.get<game::component::StatsComponent>(selected_unit);
    const auto& player = registry.get<game::component::PlayerComponent>(selected_unit);
    const auto* skill = registry.try_get<game::component::HeroSkillComponent>(selected_unit);

    ImGui::Text("名称：%s", name.class_name_.c_str());
    ImGui::Text("等级：%d", stats.level_);
    ImGui::Text("生命：%.0f / %.0f", stats.hp_, stats.max_hp_);
    ImGui::Text("攻击：%.1f", stats.atk_);
    ImGui::Text("防御：%.1f", stats.def_);
    ImGui::Text("射程：%.1f", stats.range_);
    ImGui::Text("攻击间隔：%.2f", stats.atk_interval_);
    ImGui::Text("费用：%d", player.cost_);
    ImGui::Text("投射物：0x%llx", static_cast<unsigned long long>(player.projectile_id_));
    ImGui::Separator();
    ImGui::TextDisabled("快捷键：U 升级英雄  K 释放技能  R 撤退英雄");

    if (skill && blueprint_manager.hasSkillBlueprint(skill->skill_id_)) {
        const auto& skill_bp = blueprint_manager.getSkillBlueprint(skill->skill_id_);
        ImGui::SeparatorText("技能");
        ImGui::Text("%s", skill_bp.name_.c_str());
        ImGui::TextWrapped("%s", skill_bp.description_.c_str());
        if (skill->passive_) {
            ImGui::TextColored(ImVec4(0.45f, 0.85f, 1.0f, 1.0f), "状态：被动技能激活中");
        } else if (skill->active_) {
            ImGui::TextColored(ImVec4(0.35f, 1.0f, 0.45f, 1.0f), "状态：激活中，剩余 %.1f 秒", skill->duration_timer_);
        } else if (skill->cooldown_timer_ > 0.0f) {
            ImGui::TextColored(ImVec4(1.0f, 0.75f, 0.25f, 1.0f), "状态：冷却中，剩余 %.1f 秒", skill->cooldown_timer_);
        } else {
            ImGui::TextColored(ImVec4(0.65f, 1.0f, 0.65f, 1.0f), "状态：就绪，可释放");
        }

        if (skill_bp.passive_) {
            ImGui::Text("被动回费：%.2f", skill_bp.cost_regen_);
        } else {
            ImGui::Text("总冷却：%.1f 秒", skill_bp.cooldown_);
            ImGui::Text("剩余冷却：%.1f 秒", skill->cooldown_timer_);
            ImGui::Text("总持续：%.1f 秒", skill_bp.duration_);
            ImGui::Text("剩余持续：%.1f 秒", skill->duration_timer_);
            ImGui::Text("攻击 x%.2f  防御 x%.2f  射程 x%.2f  间隔 x%.2f",
                        skill_bp.atk_multiplier_,
                        skill_bp.def_multiplier_,
                        skill_bp.range_multiplier_,
                        skill_bp.atk_interval_multiplier_);
        }
    } else {
        ImGui::SeparatorText("技能");
        ImGui::TextUnformatted("未配置技能蓝图");
    }

    auto& dispatcher = scene.getContext().getDispatcher();
    if (ImGui::IsKeyPressed(ImGuiKey_U, false)) {
        dispatcher.trigger(game::defs::UpgradeUnitEvent{ selected_unit, player.cost_ });
    }
    if (ImGui::IsKeyPressed(ImGuiKey_Q, false)) {
        dispatcher.trigger(game::defs::RetreatEvent{ selected_unit, player.cost_ });
    }
    if (skill && blueprint_manager.hasSkillBlueprint(skill->skill_id_) && !skill->passive_ && ImGui::IsKeyPressed(ImGuiKey_K, false)) {
        dispatcher.trigger(game::defs::ReleaseHeroSkillEvent{ selected_unit });
    }

    ImGui::End();
}

} // namespace

void HeroInspectorUI::render(engine::scene::Scene& scene) {
    auto& registry = scene.getRegistry();
    if (!registry.ctx().contains<std::shared_ptr<game::factory::BlueprintManager>>() ||
        !registry.ctx().contains<std::shared_ptr<game::data::SessionData>>() ||
        !registry.ctx().contains<game::data::SelectionState>()) {
        return;
    }

    auto& blueprint_ptr = registry.ctx().get<std::shared_ptr<game::factory::BlueprintManager>>();
    if (!blueprint_ptr) {
        return;
    }

    auto& session_ptr = registry.ctx().get<std::shared_ptr<game::data::SessionData>>();
    if (!session_ptr) {
        return;
    }

    auto& selection = registry.ctx().get<game::data::SelectionState>();

    if (selection.hovered_unit_ != entt::null && isValidUnit(registry, selection.hovered_unit_)) {
        renderUnitTooltip(registry, selection.hovered_unit_, *blueprint_ptr);
    }

    if (selection.hovered_portrait_name_id_ != 0) {
        const auto& units = session_ptr->getUnitMap();
        const auto it = units.find(selection.hovered_portrait_name_id_);
        if (it != units.end()) {
            renderPortraitTooltip(it->second, *blueprint_ptr);
        }
    }

    if (selection.selected_unit_ != entt::null && isValidUnit(registry, selection.selected_unit_)) {
        renderSelectedUnitPanel(registry, selection.selected_unit_, *blueprint_ptr, scene);
    }
}

} // namespace game::ui
