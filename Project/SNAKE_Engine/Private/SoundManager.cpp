#include "Engine.h"

#include <algorithm>
#include "fmod_errors.h"

SoundManager::SoundManager() : system(nullptr), nextInstanceID(1) {}

void SoundManager::Init()
{
    FMOD_RESULT result = FMOD::System_Create(&system);
    if (result != FMOD_OK || !system)
    {
        SNAKE_ERR("FMOD::System_Create failed: " << FMOD_ErrorString(result));
        return;
    }
    result = system->setSoftwareChannels(128);
    if (result != FMOD_OK)
    {
        SNAKE_ERR("setSoftwareChannels failed: " << FMOD_ErrorString(result));
        return;
    }
    result = system->init(128, FMOD_INIT_NORMAL, nullptr);
    if (result != FMOD_OK)
    {
        SNAKE_ERR("FMOD system init failed: " << FMOD_ErrorString(result));
        return;
    }
}

void SoundManager::Free()
{
    for (auto& [tag, sound] : sounds)
        sound->release();

    sounds.clear();
    activeChannels.clear();
    instanceMap.clear();
    while (!reusableIDs.empty()) reusableIDs.pop();
    nextInstanceID = 1;

    if (system)
    {
        system->close();
        system->release();
        system = nullptr;
    }
}

void SoundManager::LoadSound(const std::string& tag, const std::string& filepath, bool loop)
{
    FMOD_MODE mode = FMOD_DEFAULT | FMOD_2D | FMOD_CREATESAMPLE;
    mode |= loop ? FMOD_LOOP_NORMAL : FMOD_LOOP_OFF;

    FMOD::Sound* sound = nullptr;
    if (system->createSound(filepath.c_str(), mode, nullptr, &sound) != FMOD_OK)
    {
        SNAKE_ERR("Failed to load sound : " << filepath);
        return;
    }

    sounds[tag] = sound;
}

SoundInstanceID SoundManager::GenerateID()
{
    if (!reusableIDs.empty())
    {
        SoundInstanceID id = reusableIDs.front();
        reusableIDs.pop();
        return id;
    }
    return nextInstanceID++;
}

SoundInstanceID SoundManager::Play(const std::string& tag, float volume, float startTimeSec)
{
    if (!system || sounds.find(tag) == sounds.end()) 
        return 0;

    FMOD::Channel* channel = nullptr;
    if (system->playSound(sounds[tag], nullptr, true, &channel) != FMOD_OK)
        return 0;

    if (channel)
    {
        if (startTimeSec > 0.0f)
        {
            channel->setPosition(static_cast<unsigned int>(startTimeSec * 1000.0f), FMOD_TIMEUNIT_MS);
        }

        channel->setVolume(volume);
        channel->setPaused(false);

        activeChannels[tag].push_back(channel);
        SoundInstanceID id = GenerateID();
        instanceMap[id] = channel;
        return id;
    }
    return 0;
}

void SoundManager::SetVolumeByID(SoundInstanceID id, float volume)
{
    auto it = instanceMap.find(id);
    if (it != instanceMap.end() && it->second)
        it->second->setVolume(volume);
}

void SoundManager::SetVolumeByTag(const std::string& tag, float volume)
{
    for (FMOD::Channel* ch : activeChannels[tag])
    {
        if (ch) 
            ch->setVolume(volume);
    }
}

void SoundManager::SetVolumeAll(float volume)
{
    for (const auto& [tag, _] : activeChannels)
    {
        SetVolumeByTag(tag, volume);
    }
}



void SoundManager::ControlByID(SoundControlType control, SoundInstanceID id)
{
    auto it = instanceMap.find(id);
    if (it == instanceMap.end() || !it->second)
        return;

    FMOD::Channel* ch = it->second;
    switch (control)
    {
    case SoundControlType::Pause:
        ch->setPaused(true);
        break;
    case SoundControlType::Resume:
        ch->setPaused(false);
        break;
    case SoundControlType::Stop:
        ch->stop();
        instanceMap.erase(it);
        reusableIDs.push(id);
        break;
    }
}

void SoundManager::ControlByTag(SoundControlType control, const std::string& tag)
{
    for (FMOD::Channel* ch : activeChannels[tag])
    {
        if (!ch) continue;

        switch (control)
        {
        case SoundControlType::Pause:
            ch->setPaused(true);
            break;
        case SoundControlType::Resume:
            ch->setPaused(false);
            break;
        case SoundControlType::Stop:
            ch->stop();
            break;
        }
    }

    if (control == SoundControlType::Stop)
        activeChannels[tag].clear();
}

void SoundManager::ControlAll(SoundControlType control)
{
    for (const auto& [tag, _] : activeChannels)
    {
        ControlByTag(control, tag);
    }
}

void SoundManager::Update()
{
    Cleanup();
}

void SoundManager::Cleanup()
{
    std::vector<SoundInstanceID> toRemove;

    for (auto& [id, ch] : instanceMap)
    {
        bool playing = false;
        if (!ch || ch->isPlaying(&playing) != FMOD_OK || !playing)
        {
            if (ch) 
                ch->stop();
            toRemove.push_back(id);
        }
    }

    for (SoundInstanceID id : toRemove)
    {
        instanceMap.erase(id);
        reusableIDs.push(id);
    }

    for (auto& [tag, vec] : activeChannels)
    {
        vec.erase(std::remove_if(vec.begin(), vec.end(), [](FMOD::Channel* ch)
            {
                bool playing = false;
                return !ch || ch->isPlaying(&playing) != FMOD_OK || !playing;
            }), vec.end());
    }
}