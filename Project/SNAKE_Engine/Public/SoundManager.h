#pragma once

#include <string>
#include <unordered_map>
#include <vector>
#include <queue>

#include "fmod.hpp"

class SNAKE_Engine;
using SoundInstanceID = uint64_t;

/**
 * @brief Loads, plays, and controls sounds; tracks channels and reusable instance IDs.
 *
 * @details
 * - Public API:
 *   - LoadSound(tag, filepath, loop): loads a sound and stores it under @p tag.
 *   - Play(tag, volume, startTimeSec): starts playback; returns a reusable SoundInstanceID.
 *   - SetVolumeByID / SetVolumeByTag / SetVolumeAll: volume control.
 *   - ControlByID / ControlByTag / ControlAll: Pause, Resume, Stop.
 * - Engine integration:
 *   - SNAKE_Engine calls Init() during startup, Update() each frame, and Free() on shutdown.
 * - Internals:
 *   - activeChannels[tag] keeps FMOD::Channel* for currently playing sounds by tag.
 *   - instanceMap maps SoundInstanceID -> FMOD::Channel*.
 *   - Update() calls Cleanup() to remove finished channels and recycle IDs via reusableIDs.
 */
class SoundManager
{
    friend  SNAKE_Engine;
public:
    /**
     * @brief Loads a sound file and registers it with a tag.
     * @param tag Logical name used to reference the sound.
     * @param filepath Path to the audio file.
     * @param loop If true, plays in loop mode.
     */
    void LoadSound(const std::string& tag, const std::string& filepath, bool loop = false);

    /**
     * @brief Plays a sound by tag and returns a SoundInstanceID.
     *
     * @details
     * - Starts paused, applies @p startTimeSec (ms) if > 0, sets volume, then unpauses.
     * - On success: pushes the channel into activeChannels[tag], assigns an ID from GenerateID(), stores in instanceMap, and returns it.
     * - On failure: returns 0.
     *
     * @param tag Tag of a previously loaded sound.
     * @param volume Volume in [0,1].
     * @param startTimeSec Start offset in seconds.
     * @return SoundInstanceID (0 if play failed).
     *
     * @code
     * SoundInstanceID id = soundManager.Play("laser", 0.8f);
     * soundManager.SetVolumeByID(id, 0.5f);
     * soundManager.ControlByID(SoundManager::SoundControlType::Pause, id);
     * @endcode
     */
    [[maybe_unused]] SoundInstanceID Play(const std::string& tag, float volume = 1.0f, float startTimeSec = 0.0f);

    /**
     * @brief Sets volume for a specific playing instance.
     * @param id SoundInstanceID returned by Play().
     * @param volume Volume in [0,1].
     */
    void SetVolumeByID(SoundInstanceID id, float volume);

    /**
     * @brief Sets volume for all currently playing instances of a tag.
     * @param tag Tag used with LoadSound().
     * @param volume Volume in [0,1].
     */
    void SetVolumeByTag(const std::string& tag, float volume);

    /**
     * @brief Sets volume for all currently playing sounds.
     * @param volume Volume in [0,1].
     */
    void SetVolumeAll(float volume);

    /**
     * @brief Simple playback controls used by Control* methods.
     */
    enum class SoundControlType { Pause, Resume, Stop };

    /**
     * @brief Controls a specific instance by ID.
     * @details Pause/Resume toggle paused state. Stop stops the channel, erases it from instanceMap, and recycles the ID.
     * @param control Pause / Resume / Stop.
     * @param id Target instance ID.
     */
    void ControlByID(SoundControlType control, SoundInstanceID id);

    /**
     * @brief Controls all currently playing instances under a tag.
     * @details Stop also clears activeChannels[tag].
     * @param control Pause / Resume / Stop.
     * @param tag Target tag.
     */
    void ControlByTag(SoundControlType control, const std::string& tag);

    /**
     * @brief Controls all currently playing sounds.
     * @details Iterates all tags and forwards to ControlByTag().
     * @param control Pause / Resume / Stop.
     */
    void ControlAll(SoundControlType control);



private:
    /**
     * @brief Constructs with FMOD system pointer null and ID counter set to 1.
     * @note Internal. Constructed and owned by SNAKE_Engine.
     */
    SoundManager();

    /**
     * @brief Initializes FMOD system.
     * @details Creates the system, configures software channels, and initializes it.
     * @note Internal. Called by SNAKE_Engine at startup.
     */
    void Init();

    /**
     * @brief Per-frame maintenance.
     * @details Calls Cleanup() to remove finished channels and recycle IDs.
     * @note Internal. Called by SNAKE_Engine each frame.
     */
    void Update();

    /**
     * @brief Stops/reclaims finished instances and prunes dead channels.
     * @details
     * - For instanceMap: stops channels that are invalid/not playing, collects their IDs to remove, and recycles IDs to reusableIDs.
     * - For activeChannels[tag]: erases channels that are invalid/not playing.
     * @note Internal.
     */
    void Cleanup();

    /**
     * @brief Releases sounds and FMOD system; clears all maps and ID pools.
     * @note Internal. Called by SNAKE_Engine on shutdown.
     */
    void Free();

    FMOD::System* system;
    std::unordered_map<std::string, FMOD::Sound*> sounds; ///< Loaded sounds indexed by tag.
    std::unordered_map<std::string, std::vector<FMOD::Channel*>> activeChannels; ///< Playing channels per tag.
    std::unordered_map<SoundInstanceID, FMOD::Channel*> instanceMap; ///< ID ¡æ channel mapping.

    std::queue<SoundInstanceID> reusableIDs; ///< Pool of IDs recycled after Stop/cleanup.
    SoundInstanceID nextInstanceID;          ///< Monotonic counter used when no recycled ID is available.

    /**
     * @brief Returns a recycled ID if available; otherwise allocates a new one.
     * @note Internal. Used by Play().
     */
    SoundInstanceID GenerateID();
};
