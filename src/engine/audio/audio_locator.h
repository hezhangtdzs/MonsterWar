#pragma once
#include "iaudio_player.h"

namespace engine::audio {

	/**
	 * @brief 音频播放器服务定位器。
	 * @details 提供全局访问音频系统的能力，解耦具体实现和使用。
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
