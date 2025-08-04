#pragma once
/**
 * @brief Utility class for tracking delta time and calculating FPS.
 *
 * @details
 * EngineTimer provides timing utilities for the engine's main loop.
 * It measures time between frames (delta time), tracks frame count,
 * and determines when to update FPS metrics (typically every 0.3 seconds).
 *
 * The timer should be started once at the beginning of the game loop using Start(),
 * and Tick() should be called once per frame to get delta time.
 */
struct EngineTimer
{
	/**
     * @brief Initializes the timer and resets FPS counters.
     */
    void Start();

    /**
    * @brief Returns the elapsed time (in seconds) since the last frame.
    *
    * @return Delta time in seconds.
    */
    [[nodiscard]] float Tick();

    /**
	  * @brief Determines if it's time to update the FPS value.
	  *
	  * @details
	  * Returns true approximately every 0.3 seconds and writes the
	  * computed FPS into the outFPS parameter.
	  *
	  * @param outFPS Reference to store the calculated FPS.
	  * @return true if FPS should be updated, false otherwise.
	  */
    [[nodiscard]] bool ShouldUpdateFPS(float& outFPS);

    float lastTime = 0.0f;
    float fpsTimer = 0.0f;
    int frameCount = 0;
};
