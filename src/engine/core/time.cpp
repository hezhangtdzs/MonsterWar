#include "time.h"
#include <spdlog/spdlog.h>
#include <SDL3/SDL_timer.h>

/**
 * @brief 构造函数，初始化计时数据。
 */
engine::core::Time::Time()
{
	last_time_ =  SDL_GetTicksNS();
	frame_start_time_ = last_time_;
	spdlog::trace("Time 初始化。Last time : {}", last_time_);
}

/**
 * @brief 在主循环每帧开始时更新时间状态。
 * 计算自上一帧以来的 delta time，并处理帧率限制。
 */
void engine::core::Time::update()
{
	frame_start_time_ = SDL_GetTicksNS();
	auto current_delta_time = static_cast<double>(frame_start_time_ - last_time_) / 1e9;
	if (target_fps_ > 0)
	{
		limitFrameRate(current_delta_time);
	}
	else {
		delta_time_ = current_delta_time;
	}
	last_time_ = SDL_GetTicksNS();

}

/**
 * @brief 设置全局时间缩放系数。
 * @param scale 缩放倍率（1.0 为正常速度）。
 */
void engine::core::Time::setTimeScale(double scale)
{
	if (scale <= 0.0) {
		spdlog::warn("Time Scale 必须为正数。Setting to 1.0.");
		time_scale_ = 1.0;
	}
	else {
		time_scale_ = scale;
	}
}

/**
 * @brief 设置游戏运行的目标 FPS。
 * @param fps 目标帧率（如 60），设为 0 表示不限制。
 */
void engine::core::Time::setTargetFPS(int fps)
{
	if (fps < 0) {
		spdlog::warn("Target FPS 不能为负。Setting to 0 (unlimited).");
		target_fps_ = 0;
	}
	else {
		target_fps_ = fps;
	}

	if (target_fps_ > 0) {
		target_frame_time_ = 1.0 / static_cast<double>(target_fps_);
		spdlog::info("Target FPS 设置为: {} (Frame time: {:.6f}s)", target_fps_, target_frame_time_);
	}
	else {
		target_frame_time_ = 0.0;
		spdlog::info("Target FPS 设置为: Unlimited");
	}
}

/**
 * @brief 内部辅助方法，若实际运行速度过快则强制延迟以维持目标帧率。
 * @param current_delta_time 当前帧处理完毕实际耗费的时间。
 */
void engine::core::Time::limitFrameRate(float current_delta_time)
{
	if (current_delta_time < target_frame_time_) {
		double time_to_wait = target_frame_time_ - current_delta_time;
		Uint64 ns_to_wait = static_cast<Uint64>(time_to_wait * 1e9);
		SDL_DelayNS(ns_to_wait);
		delta_time_ = static_cast<double>(SDL_GetTicksNS() - last_time_) / 1e9;
	}else{
		delta_time_ = current_delta_time;
	}
}
