#pragma once
#include <string>
class SNAKE_Engine;
struct GLFWwindow;
struct EngineContext;

/**
 * @brief Manages the game window and OpenGL context.
 *
 * @details
 * WindowManager handles window creation, resizing, and OpenGL-related operations
 * such as buffer swapping, screen clearing, and event polling. It integrates with GLFW
 * and is responsible for initializing the rendering context and managing screen dimensions.
 *
 * Used internally by the engine, but also accessible for getting window size or handle.
 *
 * @note This class is automatically initialized by SNAKE_Engine.
 */
class WindowManager
{
    friend SNAKE_Engine;
    friend void framebuffer_size_callback(GLFWwindow*, int, int);
public:
    /**
	 * @brief Constructs the window manager with default size.
	 *
	 * @details
	 * Initializes the internal window pointer to null and sets default window size to 800x600.
	 */
    WindowManager() :window(nullptr), windowWidth(800), windowHeight(600) {}

    /**
	 * @brief Returns the internal GLFW window handle.
	 *
	 * @return Pointer to the GLFWwindow instance.
	 */
    [[nodiscard]] GLFWwindow* GetHandle() const { return window; }

    /**
	 * @brief Returns the current window width in pixels.
	 *
	 * @return Window width.
	 */
    [[nodiscard]] int GetWidth() const { return windowWidth; }

    /**
	 * @brief Returns the current window height in pixels.
	 *
	 * @return Window height.
	 */
    [[nodiscard]] int GetHeight() const { return windowHeight; }

    /**
	 * @brief Sets the window size to the specified dimensions.
	 *
	 * @details
	 * Updates the window size both internally and at the GLFW level.
	 *
	 * @param width New window width.
	 * @param height New window height.
	 */
    void Resize(int width, int height);

	/**
	 * @brief Sets the title of the window.
	 *
	 * @param title The new window title.
	 */
    void SetTitle(const std::string& title) const;

private:
	/**
	 * @brief Initializes the GLFW window and OpenGL context.
	 *
	 * @details
	 * Sets up GLFW, creates the window, and loads the OpenGL context using GLAD.
	 * Links the window to the engine for event callbacks.
	 *
	 * @param _windowWidth Initial window width.
	 * @param _windowHeight Initial window height.
	 * @param engine Reference to the main SNAKE_Engine instance.
	 * @return true if successful, false otherwise.
	 *
	 * @note This function is called internally by the engine.
	 */
    bool Init(int _windowWidth, int _windowHeight, SNAKE_Engine& engine);

	/**
	 * @brief Sets the internal window width.
	 *
	 * @details
	 * Used internally when the framebuffer size changes (e.g., via resize callback).
	 * Should not be called manually by users.
	 *
	 * @param width New window width in pixels.
	 *
	 * @note Internal use only.
	 */
    void SetWidth(int width) { this->windowWidth = width; }

	/**
	 * @brief Sets the internal window height.
	 *
	 * @details
	 * Used internally when the framebuffer size changes (e.g., via resize callback).
	 * Should not be called manually by users.
	 *
	 * @param height New window height in pixels.
	 *
	 * @note Internal use only.
	 */
	void SetHeight(int height) { this->windowHeight = height; }

	/**
	 * @brief Swaps the front and back buffers.
	 *
	 * @details
	 * This displays the rendered frame to the screen.
	 */
    void SwapBuffers() const;

	/**
	* @brief Clears the screen with a predefined background color.
	*
	* @note Default color is gray (0.4, 0.4, 0.4, 1.0).
	*/
    void ClearScreen() const;

	/**
	 * @brief Polls for and processes window events (e.g., input, resize).
	 */
    void PollEvents() const;

	/**
	 * @brief Destroys the window and releases GLFW resources.
	 */
    void Free() const;

    GLFWwindow* window;
    int windowWidth;
    int windowHeight;
};
