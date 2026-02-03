#include "animation.h"
#include <spdlog/spdlog.h>

/**
 * @brief 构造函数，创建一个新的动画实例。
 * @param name 动画名称，默认为 "default"
 * @param loop 是否循环播放，默认为 true
 */
engine::render::Animation::Animation(const std::string& name, bool loop):
	name_(name), total_duration_(0.0f), loop_(loop)
{
	spdlog::debug("动画 '{}' 创建. 是否循环: {}", name_, loop_ ? "true" : "false");
}

/**
 * @brief 向动画添加一个帧。
 * @param source_rect 帧的纹理源矩形
 * @param duration 帧的持续时间（秒）
 */
void engine::render::Animation::addFrame(const SDL_FRect& source_rect, float duration)
{
	AnimationFrame frame;
	frame.src_rect = SDL_FRect{
		static_cast<float>(source_rect.x),
		static_cast<float>(source_rect.y),
		static_cast<float>(source_rect.w),
		static_cast<float>(source_rect.h)
	};
	frame.duration = duration;
	frames_.push_back(frame);
	total_duration_ += duration;
	spdlog::debug("动画 '{}' 添加帧: src_rect=({}, {}, {}, {}), duration={}", 
		name_, frame.src_rect.x, frame.src_rect.y, frame.src_rect.w, frame.src_rect.h, duration);
}

/**
 * @brief 根据当前时间获取对应的动画帧。
 * @param time 当前时间（秒）
 * @return 对应的动画帧引用
 * @throw std::runtime_error 如果动画为空
 */
const engine::render::AnimationFrame& engine::render::Animation::getFrame(float time) const
{
	if (frames_.empty()) {
		spdlog::warn("动画 '{}' 没有帧.", name_);
		throw std::runtime_error("Animation is empty");
	}

	// 根据 loop_ 标志决定时间处理逻辑
	if (loop_) {
		time = fmod(time, total_duration_);
	} else {
		// 如果不循环，超过总时长的部分钳制在最后一帧的时间点上
		if (time >= total_duration_) {
			return frames_.back();
		}
	}

	float accumulated_time = 0.0f;

	for (const auto& frame : frames_) {
		accumulated_time += frame.duration;
		if (accumulated_time >= time) {
			return frame;
		}
	}

	// 如果没有找到合适的帧，返回最后一帧
	return frames_.back();
}
