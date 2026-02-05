/**
 * @file audio_locator.cpp
 * @brief AudioLocator 类的实现，提供全局音频服务的访问。
 */

#include "audio_locator.h"

namespace engine::audio {

	NullAudioPlayer AudioLocator::null_service_;  ///< 空服务对象，作为默认音频服务
	IAudioPlayer* AudioLocator::service_ = &AudioLocator::null_service_;  ///< 当前音频服务指针，默认指向空服务

} // namespace engine::audio
