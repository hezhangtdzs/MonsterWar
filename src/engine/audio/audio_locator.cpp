#include "audio_locator.h"

namespace engine::audio {

	NullAudioPlayer AudioLocator::null_service_;
	IAudioPlayer* AudioLocator::service_ = &AudioLocator::null_service_;

} // namespace engine::audio
