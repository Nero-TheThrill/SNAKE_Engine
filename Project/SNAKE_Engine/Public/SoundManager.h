#pragma once

#include <string>
#include <unordered_map>
#include <vector>
#include <queue>

#include "fmod.hpp"

class SNAKE_Engine;
using SoundInstanceID = uint64_t;

/**
 * @brief Centralized audio playback and control system using FMOD.
 *
 * @details
 * The SoundManager is responsible for loading, playing, and controlling sounds within the engine.
 * It wraps FMOD functionality and provides high-level interfaces for volume control,
 * playback state management, and per-tag or per-instance sound operations.
 *
 * Sounds are loaded by tag and played with a unique SoundInstanceID, which can be reused.
 * Volume and playback state (pause, resume, stop) can be controlled globally, by tag, or by instance.
 *
 * @note This class is automatically initialized and updated by SNAKE_Engine.
 * Users typically call Play(), SetVolumeByTag(), or ControlByTag() from game logic.
 */
class SoundManager
{
    friend  SNAKE_Engine;
public:
    /**
     * @brief Loads a sound file into memory and registers it with a tag.
     *
     * @details
     * This function loads an audio file from disk into FMOD and stores it internally
     * using the provided tag as a key. The sound can be configured to loop or not.
     * The same tag can be used later with Play() to play the sound.
     *
     * @param tag Unique identifier for this sound (e.g., "explosion", "bgm").
     * @param filepath Path to the sound file (e.g., "assets/sounds/bgm.mp3").
     * @param loop Whether the sound should loop when played (default: false).
     *
     * @code
     * soundManager.LoadSound("bgm", "assets/music/theme.mp3", true);
     * @endcode
     */
    void LoadSound(const std::string& tag, const std::string& filepath, bool loop = false);

    /**
     * @brief Plays a sound by tag and returns a unique instance ID.
     *
     * @details
     * Retrieves the sound registered with the given tag and plays it immediately.
     * Optionally sets the initial volume and start time (in seconds).
     * A reusable SoundInstanceID is returned, which can be used to control or track the sound later.
     *
     * @param tag The tag of the sound to play.
     * @param volume Playback volume (0.0 to 1.0). Default is 1.0.
     * @param startTimeSec Time offset to start from in seconds. Default is 0.0.
     * @return SoundInstanceID representing this playback instance.
     *
     * @code
     * SoundInstanceID id = soundManager.Play("click", 0.5f);
     * @endcode
     */
    [[maybe_unused]] SoundInstanceID Play(const std::string& tag, float volume = 1.0f, float startTimeSec = 0.0f);

    /**
     * @brief Sets the playback volume of a specific sound instance.
     *
     * @details
     * Adjusts the volume of the sound associated with the given SoundInstanceID.
     * The volume must be between 0.0 (mute) and 1.0 (full volume).
     * If the ID is no longer active, the call is ignored.
     *
     * @param id The SoundInstanceID of the playing sound.
     * @param volume Volume level (0.0 to 1.0).
     *
     * @code
     * soundManager.SetVolumeByID(id, 0.25f);
     * @endcode
     */
    void SetVolumeByID(SoundInstanceID id, float volume);

    /**
     * @brief Sets the volume for all active sounds associated with a specific tag.
     *
     * @details
     * All currently playing instances of the specified sound tag will have their volume changed.
     * Useful for adjusting volume per category (e.g., background music vs sound effects).
     *
     * @param tag The tag of the sound to adjust.
     * @param volume Volume level (0.0 to 1.0).
     *
     * @code
     * soundManager.SetVolumeByTag("bgm", 0.5f);
     * @endcode
     */
    void SetVolumeByTag(const std::string& tag, float volume);

    /**
     * @brief Sets the volume for all currently playing sounds.
     *
     * @details
     * Applies a global volume change to all sound instances, regardless of tag or origin.
     * Can be used for master volume control or mute toggle.
     *
     * @param volume Volume level (0.0 to 1.0).
     *
     * @code
     * soundManager.SetVolumeAll(0.0f); // mute everything
     * @endcode
     */
    void SetVolumeAll(float volume);

    /**
     * @brief Specifies control operations for sound playback.
     *
     * @details
     * Used in ControlByID, ControlByTag, and ControlAll to determine how a sound instance
     * should be affected: paused, resumed, or stopped.
     */
    enum class SoundControlType { Pause, Resume, Stop};

    /**
     * @brief Applies a control operation (pause, resume, stop) to a specific sound instance.
     *
     * @details
     * Targets the sound identified by the given SoundInstanceID.
     * Has no effect if the ID is inactive or invalid.
     *
     * @param control The control operation to apply (Pause, Resume, Stop).
     * @param id The SoundInstanceID of the target sound.
     *
     * @code
     * soundManager.ControlByID(SoundControlType::Pause, id);
     * @endcode
     */
    void ControlByID(SoundControlType control, SoundInstanceID id);

    /**
     * @brief Applies a control operation to all active instances of a tagged sound.
     *
     * @details
     * Affects all currently playing sounds that were launched with the specified tag.
     * Useful for pausing or resuming entire groups like "bgm" or "sfx".
     *
     * @param control The control operation to apply.
     * @param tag The tag identifying the sound group.
     *
     * @code
     * soundManager.ControlByTag(SoundControlType::Stop, "bgm");
     * @endcode
     */
    void ControlByTag(SoundControlType control, const std::string& tag);

    /**
     * @brief Applies a control operation to all currently playing sounds.
     *
     * @details
     * This affects every sound instance regardless of tag or origin.
     * Useful for global pause/resume or clean shutdown.
     *
     * @param control The control operation to apply.
     *
     * @code
     * soundManager.ControlAll(SoundControlType::Pause);
     * @endcode
     */
    void ControlAll(SoundControlType control);



private:
    /**
     * @brief Private constructor to prevent manual instantiation.
     *
     * @details
     * This constructor is hidden from external use to ensure that only SNAKE_Engine
     * or friend classes can create an instance. SoundManager is designed to be
     * centrally managed and should not be constructed directly by user code.
     *
     * @note Only accessible by SNAKE_Engine and used internally.
     */
    SoundManager();

    /**
     * @brief Updates the FMOD system and cleans up finished sound instances.
     *
     * @details
     * This is called once per frame to update FMOD and remove finished channels
     * from the active tracking map.
     *
     * @note Called internally by the engine per frame.
     */
    void Init();

    /**
     * @brief Updates the FMOD system and cleans up finished sound instances.
     *
     * @details
     * This is called once per frame to update FMOD and remove finished channels
     * from the active tracking map.
     *
     * @note Called internally by the engine per frame.
     */
    void Update();

    /**
     * @brief Cleans up inactive or dead channels from internal tracking maps.
     *
     * @details
     * Removes channel pointers that are no longer valid due to sound finishing or stopping.
     * Helps prevent memory growth and reuse instance IDs.
     */
    void Cleanup();

    /**
     * @brief Shuts down the FMOD system and releases all audio resources.
     *
     * @details
     * Frees all loaded sounds and active channels. Called during engine shutdown.
     *
     * @note Called automatically by SNAKE_Engine::RequestQuit().
     */
    void Free();

    /**
     * @brief Generates or recycles a unique sound instance ID.
     *
     * @details
     * If reusable IDs are available, returns one from the queue.
     * Otherwise, increments and returns a new unique ID.
     *
     * @return A valid, unique SoundInstanceID.
     */
    SoundInstanceID GenerateID();

    FMOD::System* system;
    std::unordered_map<std::string, FMOD::Sound*> sounds;
    std::unordered_map<std::string, std::vector<FMOD::Channel*>> activeChannels;
    std::unordered_map<SoundInstanceID, FMOD::Channel*> instanceMap;

    std::queue<SoundInstanceID> reusableIDs;
    SoundInstanceID nextInstanceID;

};
