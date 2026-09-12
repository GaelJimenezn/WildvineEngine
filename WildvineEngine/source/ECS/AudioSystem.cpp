/**
 * @file AudioSystem.cpp
 * @brief Implementa el audio espacial mediante DirectXTK.
 */
#include "Utilities/AudioSystem.h"

#include <algorithm>
#include <cmath>
#include <exception>
#include <string>
#include <unordered_map>

#include <Audio.h>

#include "ECS/AudioSourceComponent.h"
#include "ECS/Transform.h"
#include "EngineUtilities/Utilities/Camera.h"
#include "Logger.h"

namespace {

std::wstring
toWideString(const std::string &value) {
  return std::wstring(value.begin(), value.end());
}

DirectX::XMFLOAT3
toAudioFloat3(const EU::Vector3 &value) {
  return DirectX::XMFLOAT3(value.x, value.z, value.y);
}

} // namespace

struct

    AudioSystem::State {
  struct
  SourceState {
    std::unique_ptr<DirectX::SoundEffect> effect;
    std::unique_ptr<DirectX::SoundEffectInstance> instance;
    std::string loadedPath;
  };

  std::unique_ptr<DirectX::AudioEngine> engine;
  DirectX::AudioListener listener;
  std::unordered_map<AudioSourceComponent *, SourceState> sources;
  bool paused = false;
};

AudioSystem::AudioSystem() = default;

AudioSystem::~AudioSystem() {
  destroy();
}

bool
AudioSystem::init() {
  if (m_state && m_state->engine) {
    return true;
  }

  try {
    m_state = std::make_unique<State>();
    m_state->engine = std::make_unique<DirectX::AudioEngine>();
    MESSAGE("AudioSystem", "init", "DirectXTK AudioEngine inicializado");
    return true;
  } catch (const std::exception &exception) {
    m_state.reset();
    ERROR("AudioSystem", "init", exception.what());
    return false;
  }
}

void
AudioSystem::update(const Camera &camera) {
  if (!m_state || !m_state->engine || m_state->paused) {
    return;
  }

  if (!m_state->engine->Update()) {
    return;
  }

  const EU::Vector3 position = camera.getPosition();
  m_state->listener.SetPosition(toAudioFloat3(position));
  m_state->listener.SetOrientation(toAudioFloat3(camera.GetForward()),
                                   toAudioFloat3(camera.GetUp()));

  for (AudioSourceComponent *source : m_sources) {
    if (!source || source->m_audioPath.empty()) {
      continue;
    }

    State::SourceState &state = m_state->sources[source];
    if (source->m_assetDirty || state.loadedPath != source->m_audioPath) {
      state.instance.reset();
      state.effect.reset();

      try {
        const std::wstring path = toWideString(source->m_audioPath);
        state.effect =
            std::make_unique<DirectX::SoundEffect>(m_state->engine.get(), path.c_str());
        const DirectX::SOUND_EFFECT_INSTANCE_FLAGS instanceFlags =
            source->m_spatial ? DirectX::SoundEffectInstance_Use3D
                              : DirectX::SoundEffectInstance_Default;
        state.instance = state.effect->CreateInstance(instanceFlags);
        state.loadedPath = source->m_audioPath;
        source->m_assetDirty = false;
      } catch (const std::exception &exception) {
        ERROR("AudioSystem", "update", exception.what());
        state.loadedPath.clear();
        source->m_assetDirty = false;
        continue;
      }
    }

    if (!state.instance) {
      continue;
    }

    float outputVolume = source->m_muted ? 0.0f : source->m_volume;
    if (source->m_spatial && source->m_transform) {
      const EU::Vector3 emitterPosition = source->m_transform->getPosition();
      const float deltaX = emitterPosition.x - position.x;
      const float deltaY = emitterPosition.y - position.y;
      const float deltaZ = emitterPosition.z - position.z;
      const float distance =
          std::sqrt(deltaX * deltaX + deltaY * deltaY + deltaZ * deltaZ);
      if (distance >= source->m_maxDistance) {
        outputVolume = 0.0f;
      } else if (distance > source->m_minDistance) {
        const float range = source->m_maxDistance - source->m_minDistance;
        const float attenuation = 1.0f - ((distance - source->m_minDistance) / range);
        outputVolume *= attenuation;
      }

      DirectX::AudioEmitter emitter;
      emitter.SetPosition(toAudioFloat3(emitterPosition));
      emitter.EnableDefaultCurves();
      emitter.InnerRadius = source->m_minDistance;
      emitter.CurveDistanceScaler = 1.0f / source->m_maxDistance;
      state.instance->Apply3D(m_state->listener, emitter);
    }

    state.instance->SetVolume(outputVolume);
    state.instance->SetPitch(source->m_pitch);

    if (source->m_stopRequested) {
      state.instance->Stop();
      source->m_stopRequested = false;
    }

    if (source->m_playRequested) {
      state.instance->Play(source->m_loop);
      source->m_playRequested = false;
    }
  }
}

void
AudioSystem::destroy() {
  if (!m_state) {
    return;
  }

  for (auto &entry : m_state->sources) {
    if (entry.second.instance) {
      entry.second.instance->Stop();
    }
  }

  m_state.reset();
  m_sources.clear();
}

void
AudioSystem::registerSource(AudioSourceComponent *source) {
  if (!source ||
      std::find(m_sources.begin(), m_sources.end(), source) != m_sources.end()) {
    return;
  }

  m_sources.push_back(source);
}

void
AudioSystem::unregisterSource(AudioSourceComponent *source) {
  m_sources.erase(std::remove(m_sources.begin(), m_sources.end(), source),
                  m_sources.end());

  if (m_state) {
    m_state->sources.erase(source);
  }
}

void
AudioSystem::setMasterVolume(float volume) {
  if (!m_state || !m_state->engine) {
    return;
  }

  const float clampedVolume = volume < 0.0f ? 0.0f : (volume > 1.0f ? 1.0f : volume);
  m_state->engine->SetMasterVolume(clampedVolume);
}

void
AudioSystem::pause() {
  if (!m_state || !m_state->engine || m_state->paused) {
    return;
  }

  m_state->engine->Suspend();
  m_state->paused = true;
}

void
AudioSystem::resume() {
  if (!m_state || !m_state->engine || !m_state->paused) {
    return;
  }

  m_state->engine->Resume();
  m_state->paused = false;
}

void
AudioSystem::stopAll() {
  if (!m_state || !m_state->engine) {
    return;
  }

  if (m_state->paused) {
    m_state->engine->Resume();
    m_state->paused = false;
  }

  for (auto &entry : m_state->sources) {
    if (entry.second.instance) {
      entry.second.instance->Stop();
    }
  }

  for (AudioSourceComponent *source : m_sources) {
    if (!source) {
      continue;
    }

    source->m_playRequested = false;
    source->m_stopRequested = false;
  }
}

bool
AudioSystem::isPaused() const {
  return m_state && m_state->paused;
}

bool
AudioSystem::isReady() const {
  return m_state && m_state->engine;
}
