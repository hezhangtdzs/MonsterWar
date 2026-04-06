#include "game_app.h"
#include <spdlog/spdlog.h>
#include <SDL3/SDL.h>
#include "Time.h"
#include "game_state.h"
#include "../resource/resource_manager.h"
#include "../render/camera.h"
#include "../render/renderer.h"
#include "../render/text_renderer.h"
#include "config.h"
#include "../input/input_manager.h"
#include "context.h"
#include "../scene/scene_manager.h"
#include "../audio/audio_player.h"
#include "../audio/audio_locator.h"
#include "../audio/log_audio_player.h"
#include "../utils/events.h"
#include "../../game/defs/event.h"
#include "../scene/scene.h"
#include "../../game/data/game_stats.h"
#include "../../game/ui/hero_inspector_ui.h"
#include "../../game/component/class_name_component.h"
#include "../../game/component/player_component.h"
#include "../../game/component/stats_component.h"
#include "../../game/component/target_component.h"
#include "../../game/defs/tags.h"
#include <imgui.h>
#include <backends/imgui_impl_sdl3.h>
#include <backends/imgui_impl_sdlrenderer3.h>


/**
 * @brief 构造函数，默认初始化 GameApp。
 */
engine::core::GameApp::GameApp() = default;

/**
 * @brief 析构函数，清理游戏资源。
 */
engine::core::GameApp::~GameApp()
{
	if (!is_running_)
	{
		spdlog::warn("GameApp 被销毁时没有显式关闭。现在关闭。 ...");
		close();
	}
	
}

/**
 * @brief 启动游戏主循环。
 */
void engine::core::GameApp::run()
{
	if (!init()) {
		spdlog::error("游戏应用程序初始化失败，无法运行！");
		return;
	}
	time_->setTargetFPS(config_->target_fps_);
	time_->setTimeScale(1.0);
	while(is_running_) {
		time_->update();
       float delta_time = time_->getScaledDeltaTime();

		handleEvents();
		update(delta_time);
		render();
		//spdlog::info("delta_time: {}", delta_time);
	}
	close();
}

void engine::core::GameApp::setOnInitCallback(std::function<void(engine::core::Context&)> callback)
{
    on_init_ = std::move(callback);
}

/**
 * @brief 初始化所有游戏系统。
 * @return 初始化成功返回 true，否则返回 false。
 */
bool engine::core::GameApp::init()
{
	if (initConfig() &&
		initSDL() &&
		initDispatcher() &&
		initInputManager() &&
		initTime() && 
		initResourceManager()&&
		initAudioPlayer()&&
		initRenderer()&&
		initGameState()&&
		initTextRenderer()&&
		initCamera()&&
		initContext()&&
        initSceneManager()&&
		initImGui()) 
	{
		spdlog::info("游戏应用程序初始化成功。");
		
		
		// 调用初始化回调函数
		if (on_init_) {
			on_init_(*context_);
		}
     hero_inspector_ui_ = std::make_unique<game::ui::HeroInspectorUI>();
		dispatcher_->sink<utils::QuitEvent>().connect<&GameApp::onQuitEvent>(this);

		// // 创建并推送第一个游戏场景，传入会话数据
		// auto scene = std::make_unique<game::scene::TitleScene>(
		// 	*context_, 
		// 	*scene_manager_);
		// scene_manager_->requestPushScene(std::move(scene));
		return true;
	}

	spdlog::error("游戏应用程序初始化失败。");
	return false;
	
}

/**
 * @brief 处理 SDL 事件。
 */
void engine::core::GameApp::handleEvents()
{
	input_manager_->Update();

	// 修复：将输入事件分发给场景管理器
	if (scene_manager_) {
		scene_manager_->handleInput();
	}
}

/**
 * @brief 更新游戏状态。
 * @param delta_time 帧间时间间隔（秒）。
 */
void engine::core::GameApp::update(float& delta_time)
{
	if (scene_manager_) {
		scene_manager_->update(delta_time);
	}
	dispatcher_->update();
}

/**
 * @brief 渲染游戏画面。
 */
void engine::core::GameApp::render()
{
	// 1. 清除屏幕
	renderer_->clearScreen();

	if (imgui_initialized_) {
		ImGui_ImplSDL3_NewFrame();
		ImGui_ImplSDLRenderer3_NewFrame();
		ImGui::NewFrame();
	}

	// 2. 具体渲染代码
	if (scene_manager_) {
		scene_manager_->render();
	}

	renderImGui();

	// UI 在渲染阶段 enqueue 的事件需要在本帧尽快派发，避免按钮操作延迟到下一帧才生效。
	dispatcher_->update();

	if (imgui_initialized_) {
		ImGui::Render();
		ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), sdl_renderer_);
	}

	// 3. 更新屏幕显示
	renderer_->present();
}

/**
 * @brief 关闭游戏，清理资源。
 */
void engine::core::GameApp::close()
{
	dispatcher_->sink<engine::utils::QuitEvent>().disconnect<&GameApp::onQuitEvent>(this);
	spdlog::trace("关闭 GameApp ...");
 shutdownImGui();
	if (sdl_renderer_ != nullptr) {
		SDL_DestroyRenderer(sdl_renderer_);
		sdl_renderer_ = nullptr;
	}
	if (window_ != nullptr) {
		SDL_DestroyWindow(window_);
		window_ = nullptr;
	}
	engine::audio::AudioLocator::provide(nullptr);
	scene_manager_->close();

	resource_manager_.reset();
	
	SDL_Quit();
	is_running_ = false;
}

/**
 * @brief 初始化配置系统。
 * @return 初始化成功返回 true，否则返回 false。
 */
bool engine::core::GameApp::initConfig()
{
	try{
		config_ = std::make_unique<engine::core::Config>("assets/config.json");
	}
	catch (const std::exception& e)
	{
		spdlog::error("初始化配置失败: {}", e.what());
		return false;
	}
	spdlog::trace("配置初始化成功。");
	return true;
}

bool engine::core::GameApp::initImGui()
{
	try {
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGui::StyleColorsDark();
        ImGuiIO& io = ImGui::GetIO();
		ImFont* chinese_font = io.Fonts->AddFontFromFileTTF("C:/Windows/Fonts/msyh.ttc", 18.0f, nullptr, io.Fonts->GetGlyphRangesChineseFull());
		if (!chinese_font) {
			spdlog::warn("ImGui 中文字体加载失败，路径: C:/Windows/Fonts/msyh.ttc");
			io.Fonts->AddFontDefault();
		} else {
			io.FontDefault = chinese_font;
		}
		if (!ImGui_ImplSDL3_InitForSDLRenderer(window_, sdl_renderer_) || !ImGui_ImplSDLRenderer3_Init(sdl_renderer_)) {
			spdlog::error("初始化 ImGui 后端失败");
			return false;
		}
		imgui_initialized_ = true;
	}
	catch (const std::exception& e) {
		spdlog::error("初始化 ImGui 失败: {}", e.what());
		return false;
	}
	spdlog::trace("ImGui 初始化成功。");
	return true;
}

void engine::core::GameApp::renderImGui()
{
	if (!imgui_initialized_) {
		return;
	}

	auto* current_scene = scene_manager_ ? scene_manager_->getCurrentScene() : nullptr;
 if (current_scene) {
		auto& registry = current_scene->getRegistry();
		if (registry.ctx().contains<game::data::GameStats&>()) {
			auto& game_stats = registry.ctx().get<game::data::GameStats&>();
			ImGui::SetNextWindowPos(ImVec2(12.0f, 12.0f), ImGuiCond_Always, ImVec2(0.0f, 0.0f));
			ImGui::SetNextWindowBgAlpha(0.25f);
			const ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration |
				ImGuiWindowFlags_AlwaysAutoResize |
				ImGuiWindowFlags_NoSavedSettings |
				ImGuiWindowFlags_NoMove |
				ImGuiWindowFlags_NoFocusOnAppearing |
				ImGuiWindowFlags_NoNav;
			ImGui::Begin("战况", nullptr, flags);
			ImGui::Text("场景：%s", current_scene->getSceneName().c_str());
			ImGui::Text("金币：%.1f", game_stats.cost_);
			ImGui::Text("基地生命：%d", game_stats.home_hp_);
			ImGui::Text("敌人击杀：%d", game_stats.enemy_killed_count_);
			ImGui::End();
		}
	}

	ImGui::SetNextWindowPos(ImVec2(12.0f, 130.0f), ImGuiCond_Once);
	ImGui::SetNextWindowBgAlpha(0.85f);
	ImGui::Begin("设置", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
    if (current_scene) {
		auto& game_state = current_scene->getContext().getGameState();
		if (ImGui::Button(game_state.isPaused() ? "继续" : "暂停")) {
			game_state.setState(game_state.isPaused() ? engine::core::GameStateType::Playing
													   : engine::core::GameStateType::Paused);
		}
	}
	if (time_) {
		float scale_value = time_->getTimeScale();
		if (ImGui::SliderFloat("倍速", &scale_value, 0.25f, 2.0f, "%.2fx")) {
			time_->setTimeScale(scale_value);
		}
		if (ImGui::Button("0.5x")) {
			time_->setTimeScale(0.5);
		}
		ImGui::SameLine();
		if (ImGui::Button("1.0x")) {
			time_->setTimeScale(1.0);
		}
		ImGui::SameLine();
		if (ImGui::Button("2.0x")) {
			time_->setTimeScale(2.0);
		}
	}
	if (audio_player_) {
		float master = audio_player_->getMasterVolume();
		float sound = audio_player_->getSoundVolume();
		float music = audio_player_->getMusicVolume();
		if (ImGui::SliderFloat("主音量", &master, 0.0f, 1.0f, "%.2f")) {
			audio_player_->setMasterVolume(master);
		}
		if (ImGui::SliderFloat("音效音量", &sound, 0.0f, 1.0f, "%.2f")) {
			audio_player_->setSoundVolume(sound);
		}
		if (ImGui::SliderFloat("音乐音量", &music, 0.0f, 1.0f, "%.2f")) {
			audio_player_->setMusicVolume(music);
		}
	}
	ImGui::End();

	if (current_scene) {
		auto& dispatcher = current_scene->getContext().getDispatcher();
		ImGui::SetNextWindowPos(ImVec2(12.0f, 350.0f), ImGuiCond_Once);
		ImGui::SetNextWindowBgAlpha(0.85f);
		ImGui::Begin("调试工具", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
		if (ImGui::Button("重开关卡")) {
			dispatcher.trigger(game::defs::RestartEvent{});
		}
		if (ImGui::Button("回标题")) {
			dispatcher.trigger(game::defs::BackToTitleEvent{});
		}
		if (ImGui::Button("保存")) {
			dispatcher.trigger(game::defs::SaveEvent{});
		}
		if (ImGui::Button("通关")) {
			dispatcher.trigger(game::defs::LevelClearEvent{});
		}
		if (current_scene->getRegistry().ctx().contains<game::data::GameStats&>()) {
			auto& game_stats = current_scene->getRegistry().ctx().get<game::data::GameStats&>();
			if (ImGui::Button("COST +10")) {
				game_stats.cost_ += 10.0f;
			}
			ImGui::SameLine();
			if (ImGui::Button("COST +100")) {
				game_stats.cost_ += 100.0f;
			}
		}
		ImGui::End();
	}

	if (current_scene && hero_inspector_ui_) {
		hero_inspector_ui_->render(*current_scene);
	}
}

void engine::core::GameApp::shutdownImGui()
{
	if (!imgui_initialized_) {
		return;
	}
	ImGui_ImplSDLRenderer3_Shutdown();
	ImGui_ImplSDL3_Shutdown();
	ImGui::DestroyContext();
	imgui_initialized_ = false;
}

/**
 * @brief 初始化 SDL 系统。
 * @return 初始化成功返回 true，否则返回 false。
 */
bool engine::core::GameApp::initSDL()
{
	spdlog::trace("初始化游戏应用程序...");
	if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO)) {
		spdlog::error("SDL 初始化失败! SDL错误: {}", SDL_GetError());
		return false;
	}

	window_ = SDL_CreateWindow(config_->window_title_.c_str(), static_cast<int>(config_->window_width_ * config_->window_scale_), static_cast<int>(config_->window_height_ * config_->window_scale_), SDL_WINDOW_RESIZABLE);
	if (window_ == nullptr) {
		spdlog::error("无法创建窗口! SDL错误: {}", SDL_GetError());
		return false;
	}

	sdl_renderer_ = SDL_CreateRenderer(window_, nullptr);
	if (sdl_renderer_ == nullptr) {
		spdlog::error("无法创建渲染器! SDL错误: {}", SDL_GetError());
		return false;
	}
	int vsync_mode = config_->vsync_enabled_ ? SDL_RENDERER_VSYNC_ADAPTIVE : SDL_RENDERER_VSYNC_DISABLED;
	SDL_SetRenderVSync(sdl_renderer_, vsync_mode);
	
	SDL_SetRenderLogicalPresentation(sdl_renderer_, static_cast<int>(config_->window_width_ * config_->logical_scale_), static_cast<int>(config_->window_height_ * config_->logical_scale_), SDL_LOGICAL_PRESENTATION_LETTERBOX);
	is_running_ = true;
	return true;
}

/**
 * @brief 初始化时间系统。
 * @return 初始化成功返回 true，否则返回 false。
 */
bool engine::core::GameApp::initTime()
{
	try {
		time_ = std::make_unique<Time>();
	}
	catch (const std::exception& e) {
		spdlog::error("初始化时间管理失败: {}", e.what());
		return false;
	}
	spdlog::trace("时间管理初始化成功。");
	return true;
}

/**
 * @brief 初始化资源管理器。
 * @return 初始化成功返回 true，否则返回 false。
 */
bool engine::core::GameApp::initResourceManager() {
	try {
		resource_manager_ = std::make_unique<engine::resource::ResourceManager>(sdl_renderer_);
		resource_manager_->loadResources("assets/data/resource_mapping.json");
	}
	catch (const std::exception& e) {
		spdlog::error("初始化资源管理器失败: {}", e.what());
		return false;
	}
	spdlog::trace("资源管理器初始化成功。");
	return true;
}

/**
 * @brief 初始化渲染器。
 * @return 初始化成功返回 true，否则返回 false。
 */
bool engine::core::GameApp::initRenderer()
{
	try {
		renderer_ = std::make_unique<engine::render::Renderer>(sdl_renderer_, resource_manager_.get());
	}
	catch (const std::exception& e) {
		spdlog::error("初始化渲染器失败: {}", e.what());
		return false;
	}
	spdlog::trace("渲染器初始化成功。");
	return true;
}

/**
 * @brief 初始化文本渲染器。
 * @return 初始化成功返回 true，否则返回 false。
 */
bool engine::core::GameApp::initTextRenderer()
{
	try {
		text_renderer_ = std::make_unique<engine::render::TextRenderer>(sdl_renderer_, resource_manager_.get());
	}
	catch (const std::exception& e) {
		spdlog::error("初始化文本渲染器失败: {}", e.what());
		return false;
	}
	spdlog::trace("文本渲染器初始化成功。");
	return true;
}

/**
 * @brief 初始化摄像机。
 * @return 初始化成功返回 true，否则返回 false。
 */
bool engine::core::GameApp::initCamera()
{
	try {
		camera_ = std::make_unique<engine::render::Camera>(game_state_->getWindowLogicalSize());

	}
	catch (const std::exception& e) {
		spdlog::error("初始化相机失败: {}", e.what());
		return false;
	}
	spdlog::trace("相机初始化成功。");
	return true;
}

/**
 * @brief 初始化输入管理器。
 * @return 初始化成功返回 true，否则返回 false。
 */
bool engine::core::GameApp::initInputManager()
{
	try {
		input_manager_ = std::make_unique<engine::input::InputManager>(sdl_renderer_, dispatcher_.get(), config_.get());
	}
	catch (const std::exception& e) {
		spdlog::error("初始化输入管理器失败: {}", e.what());
		return false;
	}
	return true;
}

/**
 * @brief 初始化游戏上下文。
 * @return 初始化成功返回 true，否则返回 false。
 */
bool engine::core::GameApp::initContext()
{
	try {
		context_= std::make_unique<engine::core::Context>(
			*renderer_,
			*text_renderer_,
			*dispatcher_,
			*camera_,
			*resource_manager_,
			*input_manager_,
			*game_state_);
	}
	catch (const std::exception& e) {
		spdlog::error("初始化上下文失败: {}", e.what());
		return false;
	}
	return true;
}

/**
 * @brief 初始化音频播放器。
 * @return 初始化成功返回 true，否则返回 false。
 */
bool engine::core::GameApp::initAudioPlayer()
{
	try
	{
		audio_player_ = std::make_unique<engine::audio::AudioPlayer>(*resource_manager_, *config_);
	#ifdef ENABLE_AUDIO_LOG
		// ...就用 LogAudioPlayer 把刚才创建的 audio_player_ “包”起来
		audio_player_ = std::make_unique<engine::audio::LogAudioPlayer>(std::move(audio_player_));
	#endif
		engine::audio::AudioLocator::provide(audio_player_.get());
	}
	catch (const std::exception& e)
	{
		spdlog::error("初始化音频播放器失败: {}", e.what());
		return false;
	}
	return true;
}

bool engine::core::GameApp::initGameState()
{
    try {
		game_state_ = std::make_unique<engine::core::GameState>(renderer_->getSDLRenderer(), window_, engine::core::GameStateType::Title);
	}
	catch (const std::exception& e) {
		spdlog::error("初始化游戏状态失败: {}", e.what());
		return false;
	}
	return true;
}

bool engine::core::GameApp::initDispatcher()
{
    try {
		dispatcher_ = std::make_unique<entt::dispatcher>();
	}
	catch (const std::exception& e) {
		spdlog::error("初始化事件分发器失败: {}", e.what());
		return false;
	}
	return true;
}

void engine::core::GameApp::onQuitEvent()
{
	is_running_ = false;
	spdlog::trace("GameApp 收到 QuitEvent 事件，准备退出游戏主循环。");
}

/**
 * @brief 初始化场景管理器。
 * @return 初始化成功返回 true，否则返回 false。
 */
bool engine::core::GameApp::initSceneManager()
{
	try {
		scene_manager_ = std::make_unique<engine::scene::SceneManager>(*context_);
	}
	catch (const std::exception& e) {
		spdlog::error("初始化场景管理器失败: {}", e.what());
		return false;
	}
	return true;
}
