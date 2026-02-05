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
		float delta_time = time_->getDeltaTime();

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
		initInputManager() &&
		initTime() && 
		initResourceManager()&&
		initAudioPlayer()&&
		initRenderer()&&
		initTextRenderer()&&
		initCamera()&&
		initGameState()&&
		initDispatcher()&&
		initContext()&&
		initSceneManager()) 
	{
		spdlog::info("游戏应用程序初始化成功。");
		
		
		// 调用初始化回调函数
		if (on_init_) {
			on_init_(*context_);
		}
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

	// 2. 具体渲染代码
	if (scene_manager_) {
		scene_manager_->render();
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

	window_ = SDL_CreateWindow(config_->window_title_.c_str(),config_->window_width_, config_->window_height_, SDL_WINDOW_RESIZABLE);
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
	
	SDL_SetRenderLogicalPresentation(sdl_renderer_, config_->window_width_/2, config_->window_height_/2, SDL_LOGICAL_PRESENTATION_LETTERBOX);
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
		camera_ = std::make_unique<engine::render::Camera>( glm::vec2(config_->window_width_/2, config_->window_height_/2));

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
