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
#include <entt/entt.hpp>
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
		          ///< 动作未激活
		PRESSED, ///< 动作在本帧刚刚被按下
		HELD,          ///< 动作被持续按下
		RELEASED, ///< 动作在本帧刚刚被释放
		INACTIVE, ///< 动作未激活
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
		entt::dispatcher* dispatcher_; ///< 事件分发器
		
		SDL_Renderer* sdl_renderer_; ///< SDL 渲染器，用于坐标转换

		/**
		 * @brief 动作对应的实体列表
		 * 每个动作名称映射到一个包含三个信号槽的数组，分别对应按下、持续按下和释放事件。
		 * 这些信号槽用于注册实体对特定动作事件的响应。
		 * @see entt::sigh
		 */
		std::unordered_map<std::string,std::array<entt::sigh<bool()>,3>> action_entities_; ///< 动作对应的实体列表

		/**
		 * @brief 输入映射表。
		 * 键可以是 SDL_Scancode (键盘) 或 Uint32 (鼠标按钮)。
		 * 值是关联到该输入的动作名称列表。
		 */
		std::unordered_map<std::variant<SDL_Scancode, Uint32>, std::vector<std::string>> input_to_action_;
		

		/**
		 * @brief 动作当前的状态表
		 */
		std::unordered_map<std::string, ActionState> action_states_;

		glm::vec2 logical_mouse_position_; ///< 逻辑渲染坐标系下的鼠标位置
		glm::vec2 mouse_position_; ///< 窗口坐标系下的鼠标位置
	public:
		/**
		 * @brief 构造函数
		 * @param sdl_renderer SDL 渲染器指针
		 * @param config 配置对象，用于加载输入映射
		 */
		InputManager(SDL_Renderer* sdl_renderer, entt::dispatcher* dispatcher, const engine::core::Config* config);
		
		/**
		 * @brief 更新输入状态。
		 * 每一帧调用一次，处理事件并更新动作状态。
		 */
		void Update();

		/**
		 * @brief 为特定动作注册实体响应函数
		 * 
		 * @param action_name 动作名称
		 * @param state 动作状态（按下、持续按下或释放）
		 * @return 实体响应函数的信号槽
		 */
		entt::sink<entt::sigh<bool()>> onAction(const std::string& action_name, ActionState state = ActionState::PRESSED);

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

		void quit();

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