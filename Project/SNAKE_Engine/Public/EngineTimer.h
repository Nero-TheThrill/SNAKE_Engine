#pragma once

struct EngineTimer
{
    /**
     * @brief Initializes the internal timing state.
     * @details Resets time markers and counters used by Tick() / ShouldUpdateFPS().
     */
    void Start();

    /**
     * @brief Returns the elapsed time since the previous Tick() (or Start()) in seconds.
     * @details Call once per frame to obtain delta time and advance the internal reference time.
     */
    [[nodiscard]] float Tick();

    /**
     * @brief Reports frames-per-second when an update is due.
     * @details
     * If an FPS update should occur this frame, writes the computed FPS to @p outFPS and returns true.
     * Otherwise returns false without modifying @p outFPS.
     * The decision is based on the struct's internal accumulators.
     *
     * @param outFPS Output variable that receives the current FPS when available.
     * @return true if @p outFPS was updated this frame; false otherwise.
     */
    [[nodiscard]] bool ShouldUpdateFPS(float& outFPS);

    /** @brief Time of the last Tick()/Start() sampling (seconds). */
    float lastTime = 0.0f;
    /** @brief Accumulated elapsed time since the last FPS report (seconds). */
    float fpsTimer = 0.0f;
    /** @brief Number of frames counted since the last FPS report. */
    int frameCount = 0;
};
