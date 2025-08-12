#pragma once

#include <string>
#include <unordered_map>
#include <vector>
#include <queue>
#include <cstdint>

struct ma_engine;
struct ma_sound;

class SNAKE_Engine;
using SoundInstanceID = uint64_t;

class SoundManager
{
    friend SNAKE_Engine;

public:
    SoundManager();
    ~SoundManager();

    SoundManager(const SoundManager&) = delete;
    SoundManager& operator=(const SoundManager&) = delete;

    void LoadSound(const std::string& tag, const std::string& filepath, bool loop = false);

    [[maybe_unused]] SoundInstanceID Play(const std::string& tag,
        float volume = 1.0f,
        float startTimeSec = 0.0f);

    void SetVolumeByID(SoundInstanceID id, float volume);
    void SetVolumeByTag(const std::string& tag, float volume);
    void SetVolumeAll(float volume);

    enum class SoundControlType { Pause, Resume, Stop };
    void ControlByID(SoundControlType control, SoundInstanceID id);
    void ControlByTag(SoundControlType control, const std::string& tag);
    void ControlAll(SoundControlType control);

private:
    bool Init();
    void Update();
    void Cleanup();
    void Free();

    SoundInstanceID GenerateID();

private:
    ma_engine* engine = nullptr;

    struct SoundInfo
    {
        std::string filepath;
        bool loop = false;
    };
    std::unordered_map<std::string, SoundInfo> sounds;

    struct SoundInstanceMA;
    std::unordered_map<std::string, std::vector<SoundInstanceMA*>> activeChannels;
    std::unordered_map<SoundInstanceID, SoundInstanceMA*> instanceMap;

    std::queue<SoundInstanceID> reusableIDs;
    SoundInstanceID nextInstanceID = 1;
};
