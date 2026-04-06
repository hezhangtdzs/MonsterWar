#include "audio_system.h"
#include <entt/entt.hpp>
#include "../audio/audio_locator.h"
#include "../component/audio_component.h"
#include <spdlog/spdlog.h>
using namespace entt::literals;
engine::system::AudioSystem::AudioSystem(entt::registry& registry, entt::dispatcher& dispatcher)
	:registry_(registry), dispatcher_(dispatcher)
{
	dispatcher_.sink<engine::utils::PlaySoundEvent>().connect<&AudioSystem::onPlayAnimationEvent>(this);
}

engine::system::AudioSystem::~AudioSystem() = default;

void engine::system::AudioSystem::onPlayAnimationEvent(const engine::utils::PlaySoundEvent& event)
{
	if (event.entity_ == entt::null) {
		spdlog::info("播放音效事件: sound_id={:x}", event.sound_id_);
		engine::audio::AudioLocator::get().playSound(event.sound_id_);
       return;
	}

	if (!registry_.valid(event.entity_)) {
		spdlog::warn("播放音效事件目标实体已失效: entity={}, sound_id={:x}", entt::to_integral(event.entity_), event.sound_id_);
		engine::audio::AudioLocator::get().playSound(event.sound_id_);
		return;
	}

	if (auto audio_comp = registry_.try_get<engine::component::AudioComponent>(event.entity_)) {
		auto it = audio_comp->action_sounds_.find(event.sound_id_);
		if (it != audio_comp->action_sounds_.end()) {
			spdlog::info("播放音效事件: entity={}, sound_id={:x}", entt::to_integral(event.entity_), it->second);
			engine::audio::AudioLocator::get().playSound(it->second);
		} else {
			spdlog::warn("实体 {} 没有对应动作的音效: sound_id={:x}", entt::to_integral(event.entity_), event.sound_id_);
			engine::audio::AudioLocator::get().playSound(event.sound_id_);
		}
   } else {
		spdlog::info("播放音效事件: entity={} 没有 AudioComponent，sound_id={:x}", entt::to_integral(event.entity_), event.sound_id_);
		engine::audio::AudioLocator::get().playSound(event.sound_id_);
	}
}
