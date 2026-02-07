/**
 * @file audio_locator.h
 * @brief 定义 AudioLocator 类，提供音频服务的全局访问点。
 *
 * @details
 * AudioLocator 实现了服务定位器模式，允许全局访问音频系统而无需传递引用。
 * 这种模式解耦了音频系统的具体实现和使用者，便于测试和替换实现。
 *
 * @par 使用示例
 * @code
 * // 初始化时注册音频服务
 * auto audio_player = std::make_unique<AudioPlayer>(resource_manager, config);
 * AudioLocator::provide(audio_player.get());
 *
 * // 在任何地方获取音频服务
 * AudioLocator::get().playSound("explosion.wav");
 * @endcode
 *
 * @see engine::audio::IAudioPlayer 音频播放器接口
 * @see engine::audio::NullAudioPlayer 默认空实现
 */

#pragma once
#include "iaudio_player.h"

namespace engine::audio {

	/**
	 * @class AudioLocator
	 * @brief 音频播放器服务定位器。
	 * @details 提供全局访问音频系统的能力，解耦具体实现和使用。
	 *
	 * 该类使用静态成员存储服务实例，确保全局可访问。
	 * 如果未注册服务，默认返回 NullAudioPlayer 实例，避免空指针问题。
	 */
	class AudioLocator final {
	private:
		static IAudioPlayer* service_;      ///< 当前注册的音频服务
		static NullAudioPlayer null_service_; ///< 备用空服务

	public:
		AudioLocator() = delete;

		/**
		 * @brief 获取当前音频服务。
		 * @return IAudioPlayer& 音频服务引用。如果未注册服务，则返回空服务。
		 */
		static IAudioPlayer& get() {
			return *service_;
		}

		/**
		 * @brief 注册音频服务。
		 * @param service 指向音频服务实例的指针。若传入 nullptr，则恢复为空服务。
		 */
		static void provide(IAudioPlayer* service) {
			if (service == nullptr) {
				service_ = &null_service_;
			} else {
				service_ = service;
			}
		}
	};

} // namespace engine::audio
