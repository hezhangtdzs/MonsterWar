#include "animation_component.h"
#include "../render/animation.h"
#include "sprite_component.h"
#include "../object/game_object.h"
#include <spdlog/spdlog.h>

/**
 * @brief 析构函数。
 */
engine::component::AnimationComponent::~AnimationComponent() = default;

/**
 * @brief 向 animations_ map容器中添加一个动画。
 * @param animation 动画对象的唯一指针
 */
void engine::component::AnimationComponent::addAnimation(std::unique_ptr<engine::render::Animation> animation)
{
	if (animation) {
		const std::string& name = animation->getName();
		animations_[name] = std::move(animation);
	}
}

/**
 * @brief 播放指定名称的动画。
 * @param name 动画名称
 * 
 * 播放过程包括：
 * 1. 查找指定名称的动画
 * 2. 如果找到且不是当前正在播放的动画，开始播放
 * 3. 重置动画计时器
 * 4. 尝试获取精灵组件
 * 5. 立即将精灵更新到第一帧
 */
void engine::component::AnimationComponent::playAnimation(const std::string& name)
{
	auto it = animations_.find(name);
	if (it == animations_.end() || !it->second) {
		spdlog::warn("未找到 GameObject '{}' 的动画 '{}'", name, owner_ ? owner_->getName() : "未知");
		return;
	}

	// 如果已经在播放相同的动画，不重新开始（注释这一段则重新开始播放）
	if (current_animation_ == it->second.get() && is_playing_) {
		return;
	}

	current_animation_ = it->second.get();
	animation_timer_ = 0.0f;
	is_playing_ = true;

	// 尝试获取精灵组件（如果 init 尚未调用）
	if (!sprite_component_ && owner_) {
		sprite_component_ = owner_->getComponent<SpriteComponent>();
	}

	// 立即将精灵更新到第一帧
	if (sprite_component_ && !current_animation_->isEmpty()) {
		const auto& first_frame = current_animation_->getFrame(0.0f);
		SDL_FRect source_rect = {
			static_cast<float>(first_frame.src_rect.x),
			static_cast<float>(first_frame.src_rect.y),
			static_cast<float>(first_frame.src_rect.w),
			static_cast<float>(first_frame.src_rect.h)
		};
		sprite_component_->setSourceRect(source_rect);
	}
}

/**
 * @brief 获取当前播放的动画名称。
 * @return 当前动画名称，如果没有动画在播放则返回空字符串
 */
std::string engine::component::AnimationComponent::getCurrentAnimationName() const
{
	if (current_animation_) {
		return current_animation_->getName();
	}
	return std::string();
}

/**
 * @brief 检查当前动画是否已播放完毕。
 * @return 动画已播放完毕返回true，否则返回false
 */
bool engine::component::AnimationComponent::isAnimationFinished() const
{
	if (current_animation_) {
		return animation_timer_ >= current_animation_->getTotalDuration();
	}
	return false;
}

/**
 * @brief 组件初始化，获取精灵组件。
 * 
 * 初始化过程包括：
 * 1. 获取所属游戏对象的 SpriteComponent
 * 2. 如果没有 SpriteComponent，记录错误日志
 */
void engine::component::AnimationComponent::init()
{
	sprite_component_ = owner_->getComponent<SpriteComponent>();
	if (!sprite_component_) {
		spdlog::error("AnimationComponent 初始化失败：所属的 GameObject '{}' 缺少 SpriteComponent 组件。", owner_->getName());
	}
}

/**
 * @brief 更新动画状态，处理帧切换和动画结束逻辑。
 * @param delta_time 自上一帧的时间间隔（秒）
 * @param context 引擎上下文
 * 
 * 更新过程包括：
 * 1. 检查是否正在播放动画
 * 2. 更新动画计时器
 * 3. 处理动画结束逻辑（循环或停止）
 * 4. 获取并设置当前帧
 * 5. 处理一键移除逻辑（如爆炸特效）
 */
void engine::component::AnimationComponent::update(float delta_time, engine::core::Context&)
{
	if (!is_playing_ || !current_animation_ || !sprite_component_) return;

	animation_timer_ += delta_time;
	float total_duration = current_animation_->getTotalDuration();

	// 1. 处理动画结束逻辑
	if (animation_timer_ >= total_duration) {
		if (current_animation_->isLooping()) {
			// 使用 fmod 保留 delta_time 的余数，解决卡顿/漂移问题
			animation_timer_ = fmod(animation_timer_, total_duration);
		} else {
			// 非循环动画：锁定在结束位置，停止播放
			animation_timer_ = total_duration;
			is_playing_ = false;
		}
	}

	// 2. 获取并设置正确的帧（此时 timer 已经被处理好，不会导致刷新第一帧）
	const auto& frame = current_animation_->getFrame(animation_timer_);

	// 统一转换为 SDL_FRect
	SDL_FRect source_rect = {
		static_cast<float>(frame.src_rect.x),
		static_cast<float>(frame.src_rect.y),
		static_cast<float>(frame.src_rect.w),
		static_cast<float>(frame.src_rect.h)
	};
	sprite_component_->setSourceRect(source_rect);

	// 3. 处理一键移除逻辑（如爆炸特效）
	if (!is_playing_ && is_one_shot_removal_) {
		owner_->setNeedRemove(true);
	}
}
