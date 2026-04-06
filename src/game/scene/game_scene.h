#pragma once
#include "../data/waypoint_node.h"
#include "../data/game_stats.h"
#include "../defs/constants.h"
#include "../defs/event.h"
#include "../system/fwd.h"
#include "../../engine/scene/scene.h"
#include "../../engine/system/fwd.h"
#include <entt/core/hashed_string.hpp>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace game::factory {
    class EntityFactory;
    class BlueprintManager;
}

namespace game::data {
    class SessionData;
    class UIConfig;
}

namespace engine::ui {
    class UIPanel;
    class UIButton;
    class UIText;
}

namespace game::scene {

class GameScene final: public engine::scene::Scene {
private:
    struct WaveConfig {
        std::vector<entt::id_type> enemy_queue_;
        float spawn_interval_ = 0.5f;
        float next_wave_interval_ = 20.0f;
    };

    std::unique_ptr<engine::system::RenderSystem> render_system_;
    std::unique_ptr<engine::system::MovementSystem> movement_system_;
    std::unique_ptr<engine::system::AnimationSystem> animation_system_;
    std::unique_ptr<engine::system::YSortSystem> ysort_system_;
	std::unique_ptr<engine::system::AudioSystem> audio_system_;

    std::unique_ptr<game::system::FollowPathSystem> follow_path_system_;
    std::unique_ptr<game::system::RemoveDeadSystem> remove_dead_system_;
    std::unique_ptr<game::system::BlockSystem> block_system_;
    std::unique_ptr<game::system::SetTargetSystem> set_target_system_;
    std::unique_ptr<game::system::TimerSystem> timer_system_;
    std::unique_ptr<game::system::AttackStarterSystem> attack_starter_system_;
    std::unique_ptr<game::system::ProjectileVisualSystem> projectile_visual_system_;
    std::unique_ptr<game::system::AnimationStateSystem> animation_state_system_;
    std::unique_ptr<game::system::OrientationSystem> orientation_system_;
	std::unique_ptr<game::system::AnimationEventsSystem> animation_events_system_;
	std::unique_ptr<game::system::CombatResolveSystem> combat_resolve_system_;
    std::unique_ptr<game::system::SelectionSystem> selection_system_;
    std::unique_ptr<game::system::HeroSkillSystem> hero_skill_system_;
    std::unique_ptr<game::system::PlaceUnitSystem> place_unit_system_;
    std::unique_ptr<game::system::RenderRangeSystem> render_range_system_;
    std::unique_ptr<game::system::GameRuleSystem> game_rule_system_;

    std::unordered_map<int, game::data::WaypointNode> waypoint_nodes_;  // 路径节点ID到节点数据的映射
    std::vector<int> start_points_;                                     // 起点ID列表
    std::vector<WaveConfig> level_waves_;
    std::vector<entt::id_type> pending_wave_enemies_;
    std::size_t pending_wave_index_ = 0;
    float wave_spawn_timer_ = 0.0f;
    float wave_break_timer_ = 0.0f;
    float wave_break_duration_ = game::defs::WAVE_BREAK_DURATION;
    float wave_spawn_interval_ = game::defs::WAVE_SPAWN_INTERVAL;
    float wave_banner_timer_ = 0.0f;
    int current_wave_ = 0;
    int enemy_level_ = 1;
    int enemy_rarity_ = 1;
    int base_hp_ = game::defs::INITIAL_BASE_HP;
    int gold_ = game::defs::INITIAL_GOLD;
    bool wave_running_ = false;

    std::size_t selected_level_index_ = 0;
    std::string level_config_path_ = "assets/data/level_config.json";
    std::string current_level_name_ = "Level 1";
    std::string current_map_path_ = "assets/maps/level1.tmj";
    float level_prep_time_ = 5.0f;

    entt::entity hovered_unit_{ entt::null };
    entt::entity selected_unit_{ entt::null };

    std::unique_ptr<game::data::SessionData> session_data_;
    std::unique_ptr<game::data::UIConfig> ui_config_;
    game::data::GameStats game_stats_;

    std::string font_path_ = "assets/fonts/VonwaonBitmap-16px.ttf";

    engine::ui::UIText* hud_text_ = nullptr;
    engine::ui::UIText* gold_text_ = nullptr;
    engine::ui::UIText* wave_banner_text_ = nullptr;
    engine::ui::UIPanel* pause_overlay_ = nullptr;
    engine::ui::UIText* pause_title_text_ = nullptr;
    engine::ui::UIButton* pause_button_ = nullptr;
    engine::ui::UIButton* resume_button_ = nullptr;
    engine::ui::UIButton* quit_button_ = nullptr;
    engine::ui::UIPanel* health_bar_layer_ = nullptr;
    engine::ui::UIPanel* unit_panel_ = nullptr;
    int last_unit_panel_cost_ = -1;
    float unit_panel_scroll_x_ = 0.0f;
    float unit_panel_content_width_ = 0.0f;
    float unit_panel_max_scroll_x_ = 0.0f;
    entt::id_type selected_unit_id_{ 0 };
    std::string selected_unit_name_;
    std::vector<entt::id_type> hidden_unit_portrait_ids_;

    struct HealthBarWidget {
        engine::ui::UIPanel* container_{ nullptr };
        engine::ui::UIPanel* fill_{ nullptr };
    };
    std::unordered_map<entt::id_type, HealthBarWidget> health_bar_widgets_;

    std::unique_ptr<game::factory::EntityFactory> entity_factory_;      // 实体工厂，负责创建和管理实体

    // 管理数据的实例很可能同时被多个场景使用，因此使用共享指针
    std::shared_ptr<game::factory::BlueprintManager> blueprint_manager_;// 蓝图管理器，负责管理蓝图数据
    
public:
    GameScene(engine::core::Context& context, std::size_t level_index = 0);
    ~GameScene();

    void init() override;
    void update(float delta_time) override;
    void render() override;
    void clean() override;

private:
    [[nodiscard]] bool loadLevel();
    [[nodiscard]] bool loadLevelConfig();
    [[nodiscard]] bool initSessionData();
    [[nodiscard]] bool initUIConfig();
    [[nodiscard]] bool initEventConnections();
    [[nodiscard]] bool initInputConnections();
    [[nodiscard]] bool initEntityFactory();
    [[nodiscard]] bool initRegistryContext();
    [[nodiscard]] bool initSelectionSystem();
    [[nodiscard]] bool initPlacementSystem();
    [[nodiscard]] bool initGameRuleSystem();
    [[nodiscard]] bool initHeroSkillSystem();
    [[nodiscard]] bool initUI();
    void startNextWave();
    void updateWaveFlow(float delta_time);
    void updateUi(float delta_time);
    void updateHealthBars();
    void refreshHudText();
    void setPauseOverlayVisible(bool visible);
    void createUnitsPortraitUI();
    [[nodiscard]] bool togglePause();
    void spawnNextEnemy();
    [[nodiscard]] int getUnitCost(entt::id_type class_id, int rarity = 1) const;
    [[nodiscard]] bool trySpendGold(int amount);

    // 事件回调函数
    void onEnemyArriveHome(const game::defs::EnemyArriveHomeEvent& event);
    void onSpawnProjectileVisual(const game::defs::SpawnProjectileVisualEvent& event);
    void onSpawnEffectVisual(const game::defs::SpawnEffectVisualEvent& event);
    void onRemoveUIPortrait(const game::defs::RemoveUIPortraitEvent& event);
    void onUIPortraitHoverEnter(const game::defs::UIPortraitHoverEnterEvent& event);
    void onUIPortraitHoverLeave(const game::defs::UIPortraitHoverLeaveEvent& event);
    void onRestartRequested(const game::defs::RestartEvent& event);
    void onBackToTitleRequested(const game::defs::BackToTitleEvent& event);
    void onSaveRequested(const game::defs::SaveEvent& event);
    void onLevelClearRequested(const game::defs::LevelClearEvent& event);

    // 测试函数
    void createTestEnemy();
    bool onCreateTestPlayerMelee();
    bool onCreateTestPlayerRanged();
    bool onCreateTestPlayerHealer();
    bool tryCreateTestPlayerUnit(entt::id_type class_id, const char* log_name, bool injured);
    bool onUpgradeClosestPlayer();
    bool onSellClosestPlayer();
    bool onReleaseSelectedHeroSkill();
    bool onClearAllPlayers();

};

} // namespace game::scene