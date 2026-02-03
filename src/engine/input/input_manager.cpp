#include "input_manager.h"
#include "spdlog/spdlog.h"
#include "../core/config.h"

namespace engine::input {

/**
 * @brief 构造函数
 * 
 * @param sdl_renderer SDL 渲染器指针
 * @param config 配置对象，用于加载输入映射
 * @details 初始化输入管理器，设置渲染器，加载输入映射，获取初始鼠标位置
 */
InputManager::InputManager(SDL_Renderer* sdl_renderer, const engine::core::Config* config)
	:sdl_renderer_(sdl_renderer)
{
	if (!sdl_renderer_) {
		spdlog::error("输入管理器: SDL_Renderer 为空指针");
		throw std::runtime_error("输入管理器: SDL_Renderer 为空指针");
	}
	initializeMapFromConfig(config);

	float x, y;
	SDL_GetMouseState(&x, &y);
	mouse_position_ = { x,y };
	spdlog::trace("初始鼠标位置: ({}, {})", mouse_position_.x, mouse_position_.y);
	
}

/**
 * @brief 更新输入状态
 * 
 * @details 更新动作状态，处理所有 SDL 事件，维护输入状态
 */
void InputManager::Update()
{
	for (auto& [state_name, state] : action_states_) {
		if (state == ActionState::PRESSED_THIS_FRAME) {
			state = ActionState::HELD_DOWN;
		}
		else if (state == ActionState::RELEASED_THIS_FRAME) {
			state = ActionState::INACTIVE;
		}
	}
	SDL_Event event;
	while (SDL_PollEvent(&event)) {
		processEvent(event);
	}
}

/**
 * @brief 检查动作是否处于按下状态（包括刚按下和持续按下）
 * 
 * @param action_name 动作名称
 * @return 是否按下
 * @details 检查指定动作是否处于按下状态，包括刚按下和持续按下的情况
 */
bool InputManager::isActionDown(const std::string& action_name) const
{
	if (auto it = action_states_.find(action_name); it == action_states_.end()) {
		spdlog::warn("输入映射警告: 未找到动作 '{}'.", action_name);
		return false;
	}else return it->second == ActionState::HELD_DOWN || it->second == ActionState::PRESSED_THIS_FRAME;
}

/**
 * @brief 检查动作是否在本帧被按下
 * 
 * @param action_name 动作名称
 * @return 是否刚刚按下
 * @details 检查指定动作是否在本帧刚刚被按下
 */
bool InputManager::isActionPressed(const std::string& action_name) const
{
	if (auto it = action_states_.find(action_name); it == action_states_.end()) {
		spdlog::warn("输入映射警告: 未找到动作 '{}'.", action_name);
		return false;
	}else return it->second == ActionState::PRESSED_THIS_FRAME;

}

/**
 * @brief 检查动作是否在本帧被释放
 * 
 * @param action_name 动作名称
 * @return 是否刚刚释放
 * @details 检查指定动作是否在本帧刚刚被释放
 */
bool InputManager::isActionReleased(const std::string& action_name) const
{
	if (auto it = action_states_.find(action_name); it == action_states_.end()) {
		spdlog::warn("输入映射警告: 未找到动作 '{}'.", action_name);
		return false;
	}
	else return it->second == ActionState::RELEASED_THIS_FRAME;
}

/**
 * @brief 获取当前鼠标在窗口坐标系下的位置
 * 
 * @return 鼠标位置 (x, y)
 * @details 获取鼠标在窗口坐标系下的当前位置
 */
glm::vec2 InputManager::getMousePosition() const
{
	return mouse_position_;
}

/**
 * @brief 是否应该退出程序
 * 
 * @return 是否退出
 * @details 检查是否收到了退出信号
 */
bool InputManager::shouldQuit() const
{
	return should_quit_;
}

/**
 * @brief 设置退出信号
 * 
 * @param value 退出值
 * @details 设置程序是否应该退出
 */
void InputManager::setShouldQuit(bool value)
{
	should_quit_ = value;
}

/**
 * @brief 获取鼠标在逻辑渲染坐标系下的位置
 * 
 * @return 逻辑鼠标位置
 * @details 获取鼠标在逻辑渲染坐标系下的位置，考虑 SDL 的逻辑大小缩放
 */
glm::vec2 InputManager::getLogicalMousePosition() const
{
	glm::vec2 logical_pos;
	SDL_RenderCoordinatesFromWindow(sdl_renderer_, mouse_position_.x, mouse_position_.y, &logical_pos.x, &logical_pos.y);
	return logical_pos;
}

/**
 * @brief 处理单个 SDL 事件
 * 
 * @param event SDL 事件结构体
 * @details 处理单个 SDL 事件，更新相应的输入状态
 */
void InputManager::processEvent(const SDL_Event& event)
{
	switch (event.type) {
	case SDL_EVENT_KEY_DOWN:
	case SDL_EVENT_KEY_UP: {
		SDL_Scancode scancode = event.key.scancode;
		bool is_down = event.key.down;
		bool is_repeat = event.key.repeat;
		if (auto it = input_to_action_.find(scancode); it == input_to_action_.end()) {
			spdlog::warn("输入映射警告: 未找到按键 {} 的映射.", SDL_GetScancodeName(scancode));
			return;
		}
		else {
			const std::vector<std::string>& actions = it->second;
			for (const std::string& action_name : actions) {
				updateActionStates(action_name, is_down, is_repeat);
			}
		}
		break;
	}
	case SDL_EVENT_MOUSE_BUTTON_DOWN:
	case SDL_EVENT_MOUSE_BUTTON_UP: {
		Uint32 mouse_button = event.button.button;
		bool is_down = (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN);
		if (auto it = input_to_action_.find(mouse_button); it == input_to_action_.end()) {
			spdlog::warn("输入映射警告: 未找到鼠标按钮 {} 的映射.", mouse_button);
			return;
		}
		else {
			const std::vector<std::string>& actions = it->second;
			for (const std::string& action_name : actions) {
				updateActionStates(action_name, is_down, false);
			}
		}
		break;
	}
	case SDL_EVENT_MOUSE_MOTION: {
		mouse_position_ = glm::vec2(event.motion.x, event.motion.y);
		break;
	}
	case SDL_EVENT_QUIT: {
		should_quit_=true;
		break;
	}
	default:
		break;
	}

}

/**
 * @brief 从配置文件初始化输入映射
 * 
 * @param config 配置对象指针
 * @details 从配置文件加载输入映射，设置默认鼠标按钮映射
 */
void InputManager::initializeMapFromConfig(const engine::core::Config* config)
{
	spdlog::trace("初始化输入映射...");
	actions_to_keyname_ = config->input_mappings_;
	input_to_action_.clear();
	action_states_.clear();

	if (actions_to_keyname_.find("MouseLeftClick") == actions_to_keyname_.end()) {
		spdlog::debug("配置中没有定义 'MouseLeftClick' 动作, 添加默认映射到 'MouseLeft'.");
		actions_to_keyname_["MouseLeftClick"] = { "MouseLeft" };
	}
	if (actions_to_keyname_.find("MouseRightClick") == actions_to_keyname_.end()) {
		spdlog::debug("配置中没有定义 'MouseRightClick' 动作, 添加默认映射到 'MouseRight'.");
		actions_to_keyname_["MouseRightClick"] = { "MouseRight" };
	}

	for (const auto& [action_name, key_names] : actions_to_keyname_) {
		action_states_[action_name] = ActionState::INACTIVE;
		spdlog::trace("映射动作: {}", action_name);

		for (const std::string& key_name : key_names) {
			SDL_Scancode scancode = stringToScancode(key_name);
			Uint32 mouse_button = stringToMouseButton(key_name);
			if (scancode != SDL_SCANCODE_UNKNOWN) {      // 如果scancode有效,则将action添加到scancode_to_actions_map_中
				input_to_action_[scancode].push_back(action_name);
				spdlog::trace("  映射按键: {} (Scancode: {}) 到动作: {}", key_name, static_cast<int>(scancode), action_name);
			}
			else if (mouse_button != 0) {             // 如果鼠标按钮有效,则将action添加到mouse_button_to_actions_map_中
				input_to_action_[mouse_button].push_back(action_name);
				spdlog::trace("  映射鼠标按钮: {} (Button ID: {}) 到动作: {}", key_name, static_cast<int>(mouse_button), action_name);
				// else if: 未来可添加其它输入类型 ...
			}
			else {
				spdlog::warn("输入映射警告: 未知键或按钮名称 '{}' 用于动作 '{}'.", key_name, action_name);
			}
		}
	}
	
}

/**
 * @brief 更新特定动作的状态
 * 
 * @param action_name 动作名
 * @param is_input_active 输入是否处于激活状态（按下）
 * @param is_repeat_event 是否为 SDL 的按键重复事件
 * @details 根据输入状态更新动作的状态，处理按键重复事件
 */
void InputManager::updateActionStates(const std::string& action_name, bool is_input_active, bool is_repeat_event)
{
	if (auto it = action_states_.find(action_name); it == action_states_.end()) {
		spdlog::warn("输入映射警告: 未找到动作 '{}'.", action_name);
		return;
	}
	else {
		if (is_input_active) {
			if (is_repeat_event) {
				it->second = ActionState::HELD_DOWN;
			}
			else {
				it->second = ActionState::PRESSED_THIS_FRAME;
			}
		}
		else {
			it->second = ActionState::RELEASED_THIS_FRAME;
		}
	}

}

/**
 * @brief 将字符串键名转换为 SDL_Scancode
 * 
 * @param key_name 键名
 * @return SDL_Scancode 值
 * @details 将字符串形式的键名转换为对应的 SDL_Scancode 值
 */
SDL_Scancode InputManager::stringToScancode(const std::string& key_name) const
{
	return SDL_GetScancodeFromName(key_name.c_str());
}

/**
 * @brief 将字符串按钮名转换为 SDL 鼠标按钮 ID
 * 
 * @param button_name 按钮名
 * @return SDL 鼠标按钮 ID (Uint32)
 * @details 将字符串形式的鼠标按钮名转换为对应的 SDL 鼠标按钮 ID
 */
Uint32 InputManager::stringToMouseButton(const std::string& button_name) const
{
	if (button_name == "MouseLeft") return SDL_BUTTON_LEFT;
	if (button_name == "MouseMiddle") return SDL_BUTTON_MIDDLE;
	if (button_name == "MouseRight") return SDL_BUTTON_RIGHT;
	if (button_name == "MouseX1") return SDL_BUTTON_X1;
	if (button_name == "MouseX2") return SDL_BUTTON_X2;
	return 0;
}

} // namespace engine::input