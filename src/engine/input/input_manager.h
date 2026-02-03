#pragma once
/**
 * @file input_manager.h
 * @brief 输入管理器，负责处理键盘、鼠标事件并将其映射为游戏动作
 */

#include <unordered_map>
#include <string>
#include <SDL3/SDL_render.h>
#include <glm/vec2.hpp>
#include <vector>
#include <variant>

namespace engine::core
{
	class Config;
};

/**
 * @namespace engine::input
 * @brief 输入处理相关的命名空间
 */
namespace engine::input
{
	/**
	 * @enum ActionState
	 * @brief 动作的状态枚举
	 */
	enum class ActionState {
		INACTIVE,           ///< 动作未激活
		PRESSED_THIS_FRAME, ///< 动作在本帧刚刚被按下
		HELD_DOWN,          ///< 动作被持续按下
		RELEASED_THIS_FRAME ///< 动作在本帧刚刚被释放
	};

	/**
	 * @class InputManager
	 * @brief 负责事件循环轮询、输入状态维护及动作映射的类
	 * 
	 * 该类通过 SDL_PollEvent 获取原始输入事件，并将其转化为配置中定义的抽象动作。
	 */
	class InputManager final
	{
	private:
		SDL_Renderer* sdl_renderer_; ///< SDL 渲染器，用于坐标转换

		/**
		 * @brief 输入映射表。
		 * 键可以是 SDL_Scancode (键盘) 或 Uint32 (鼠标按钮)。
		 * 值是关联到该输入的动作名称列表。
		 */
		std::unordered_map<std::variant<SDL_Scancode, Uint32>, std::vector<std::string>> input_to_action_;
		
		/**
		 * @brief 动作名到按键名称的映射。
		 * 从配置中加载，用于初始化。
		 */
		std::unordered_map<std::string, std::vector<std::string>> actions_to_keyname_;

		/**
		 * @brief 动作当前的状态表
		 */
		std::unordered_map<std::string, ActionState> action_states_;

		bool should_quit_ = false; ///< 是否收到退出信号
		glm::vec2 mouse_position_; ///< 窗口坐标系下的鼠标位置
	public:
		/**
		 * @brief 构造函数
		 * @param sdl_renderer SDL 渲染器指针
		 * @param config 配置对象，用于加载输入映射
		 */
		InputManager(SDL_Renderer* sdl_renderer, const engine::core::Config* config);
		
		/**
		 * @brief 更新输入状态。
		 * 每一帧调用一次，处理事件并更新动作状态。
		 */
		void Update();

		/**
		 * @brief 检查动作是否处于按下状态（包括刚按下和持续按下）
		 * @param action_name 动作名称
		 * @return 是否按下
		 */
		bool isActionDown(const std::string& action_name) const;

		/**
		 * @brief 检查动作是否在本帧被按下
		 * @param action_name 动作名称
		 * @return 是否刚刚按下
		 */
		bool isActionPressed(const std::string& action_name) const;

		/**
		 * @brief 检查动作是否在本帧被释放
		 * @param action_name 动作名称
		 * @return 是否刚刚释放
		 */
		bool isActionReleased(const std::string& action_name) const;

		/**
		 * @brief 获取当前鼠标在窗口坐标系下的位置
		 * @return 鼠标位置 (x, y)
		 */
		glm::vec2 getMousePosition() const;

		/**
		 * @brief 是否应该退出程序
		 * @return 是否退出
		 */
		bool shouldQuit() const;

		/**
		 * @brief 设置退出信号
		 * @param value 退出值
		 */
		void setShouldQuit(bool value);

		/**
		 * @brief 获取鼠标在逻辑渲染坐标系下的位置。
		 * 会考虑 SDL 的逻辑大小缩放。
		 * @return 逻辑鼠标位置
		 */
		glm::vec2 getLogicalMousePosition() const;

	private:
		/**
		 * @brief 处理单个 SDL 事件
		 * @param event SDL 事件结构体
		 */
		void processEvent(const SDL_Event& event);

		/**
		 * @brief 从配置文件初始化输入映射
		 * @param config 配置对象指针
		 */
		void initializeMapFromConfig(const engine::core::Config* config);

		/**
		 * @brief 更新特定动作的状态
		 * @param action_name 动作名
		 * @param is_input_active 输入是否处于激活状态（按下）
		 * @param is_repeat_event 是否为 SDL 的按键重复事件
		 */
		void updateActionStates(const std::string& action_name,bool is_input_active,bool is_repeat_event);

		/**
		 * @brief 将字符串键名转换为 SDL_Scancode
		 * @param key_name 键名
		 * @return SDL_Scancode 值
		 */
		SDL_Scancode stringToScancode(const std::string& key_name) const;

		/**
		 * @brief 将字符串按钮名转换为 SDL 鼠标按钮 ID
		 * @param button_name 按钮名
		 * @return SDL 鼠标按钮 ID (Uint32)
		 */
		Uint32 stringToMouseButton(const std::string& button_name) const;

	};
}