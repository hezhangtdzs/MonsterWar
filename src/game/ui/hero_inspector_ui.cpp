#include "hero_inspector_ui.h"

#include "../component/class_name_component.h"
#include "../component/hero_skill_component.h"
#include "../component/player_component.h"
#include "../component/stats_component.h"
#include "../data/game_stats.h"
#include "../data/selection_state.h"
#include "../defs/constants.h"
#include "../defs/event.h"
#include "../factory/blueprint_manager.h"
#include "../../engine/core/context.h"
#include "../../engine/component/transform_component.h"
#include "../../engine/scene/scene.h"
#include "../../engine/utils/math.h"
#include <imgui.h>

namespace game::ui {

namespace {

struct PopupState {
    entt::entity selected_unit_{ entt::null };
    ImVec2 window_pos_{ 0.0f, 0.0f };
    ImVec2 anchor_pos_{ 0.0f, 0.0f };
    double last_activity_time_{ 0.0 };
    bool hidden_{ false };
};

PopupState& popupState() {
    static PopupState state;
    return state;
}

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

void renderSelectedUnitPanel(entt::registry& registry,
                             entt::entity selected_unit,
                             const game::factory::BlueprintManager& blueprint_manager,
                             engine::scene::Scene& scene,
                             const ImVec2& anchor_screen_pos) {
    if (selected_unit == entt::null || !isValidUnit(registry, selected_unit) || !registry.all_of<game::component::PlayerComponent>(selected_unit)) {
        popupState().selected_unit_ = entt::null;
        popupState().hidden_ = false;
        return;
    }

    auto& popup_state = popupState();
    const double now = ImGui::GetTime();

    if (popup_state.selected_unit_ != selected_unit) {
        popup_state.selected_unit_ = selected_unit;
        popup_state.hidden_ = false;
        popup_state.last_activity_time_ = now;
        popup_state.anchor_pos_ = anchor_screen_pos;

        const ImGuiIO& io = ImGui::GetIO();
        const bool place_right = anchor_screen_pos.x < io.DisplaySize.x * 0.5f;
        popup_state.window_pos_ = place_right
            ? ImVec2(anchor_screen_pos.x + 28.0f, anchor_screen_pos.y - 18.0f)
            : ImVec2(anchor_screen_pos.x - 28.0f, anchor_screen_pos.y - 18.0f);
    }

    if (popup_state.hidden_) {
        return;
    }

    const ImGuiIO& io = ImGui::GetIO();
    if (popup_state.window_pos_.x < 0.0f || popup_state.window_pos_.y < 0.0f ||
        popup_state.window_pos_.x > io.DisplaySize.x || popup_state.window_pos_.y > io.DisplaySize.y) {
        return;
    }

    const ImVec2 window_pivot = popup_state.window_pos_.x < io.DisplaySize.x * 0.5f ? ImVec2(0.0f, 0.0f) : ImVec2(1.0f, 0.0f);

    ImGui::SetNextWindowPos(popup_state.window_pos_, ImGuiCond_Always, window_pivot);
    ImGui::SetNextWindowSize(ImVec2(420.0f, 0.0f), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowBgAlpha(0.88f);
    const ImGuiWindowFlags flags = ImGuiWindowFlags_NoSavedSettings |
        ImGuiWindowFlags_NoFocusOnAppearing |
        ImGuiWindowFlags_NoNav;

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
        if (ImGui::Button("升级英雄")) {
            scene.getContext().getDispatcher().enqueue(game::defs::UpgradeHeroEvent{ selected_unit });
        }
        ImGui::SameLine();
        if (ImGui::Button("出售英雄")) {
            scene.getContext().getDispatcher().enqueue(game::defs::RemovePlayerUnitEvent{ selected_unit });
        }
    }

    const ImVec2 window_min = ImGui::GetWindowPos();
    const ImVec2 window_max = ImVec2(window_min.x + ImGui::GetWindowSize().x, window_min.y + ImGui::GetWindowSize().y);
    const float clamped_x = popup_state.window_pos_.x < window_min.x ? window_min.x : (popup_state.window_pos_.x > window_max.x ? window_max.x : popup_state.window_pos_.x);
    const float clamped_y = popup_state.window_pos_.y < window_min.y ? window_min.y : (popup_state.window_pos_.y > window_max.y ? window_max.y : popup_state.window_pos_.y);
    const ImVec2 line_start(clamped_x, clamped_y);
    auto* draw_list = ImGui::GetWindowDrawList();
    draw_list->AddLine(line_start, popup_state.anchor_pos_, IM_COL32(255, 80, 80, 220), 2.5f);
    draw_list->AddCircleFilled(popup_state.anchor_pos_, 4.0f, IM_COL32(255, 80, 80, 220), 12);

    const bool is_hovered = ImGui::IsWindowHovered(ImGuiHoveredFlags_AllowWhenBlockedByActiveItem | ImGuiHoveredFlags_AllowWhenBlockedByPopup);
    const bool is_interacting = is_hovered || ImGui::IsAnyItemActive() || ImGui::IsAnyMouseDown();
    if (is_interacting) {
        popup_state.last_activity_time_ = now;
    } else if (now - popup_state.last_activity_time_ >= 1.0) {
        popup_state.hidden_ = true;
    }

    ImGui::End();
}

} // namespace

void HeroInspectorUI::render(engine::scene::Scene& scene) {
    auto& registry = scene.getRegistry();
    if (!registry.ctx().contains<std::shared_ptr<game::factory::BlueprintManager>>() ||
        !registry.ctx().contains<game::data::SelectionState>()) {
        return;
    }

    auto& blueprint_ptr = registry.ctx().get<std::shared_ptr<game::factory::BlueprintManager>>();
    if (!blueprint_ptr) {
        return;
    }

    auto& selection = registry.ctx().get<game::data::SelectionState>();

    if (selection.hovered_unit_ != entt::null && isValidUnit(registry, selection.hovered_unit_)) {
        renderUnitTooltip(registry, selection.hovered_unit_, *blueprint_ptr);
    }

    auto& popup_state = popupState();
    if (popup_state.hidden_ && selection.selected_unit_ != entt::null && selection.hovered_unit_ == selection.selected_unit_) {
        popup_state.hidden_ = false;
        popup_state.last_activity_time_ = ImGui::GetTime();
    }

    if (selection.selected_unit_ != entt::null && isValidUnit(registry, selection.selected_unit_)) {
        const ImGuiIO& io = ImGui::GetIO();
        renderSelectedUnitPanel(registry, selection.selected_unit_, *blueprint_ptr, scene, io.MousePos);
    }

    if (selection.selected_unit_ != entt::null && isValidUnit(registry, selection.selected_unit_)) {
        const auto& transform = registry.get<engine::component::TransformComponent>(selection.selected_unit_);
        const glm::vec2 screen_pos = scene.getContext().getCamera().worldToScreen(transform.position_);
        const float radius = game::defs::HOVER_RADIUS;
        auto* draw_list = ImGui::GetForegroundDrawList();
        const ImVec2 min(screen_pos.x - radius, screen_pos.y - radius);
        const ImVec2 max(screen_pos.x + radius, screen_pos.y + radius);
        draw_list->AddRectFilled(min, max, IM_COL32(255, 230, 0, 24), 4.0f);
        draw_list->AddRect(min, max, IM_COL32(255, 230, 0, 220), 4.0f, 0, 2.5f);
    }
}

} // namespace game::ui
