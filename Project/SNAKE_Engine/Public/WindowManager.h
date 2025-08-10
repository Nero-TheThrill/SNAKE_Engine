#pragma once
#include <string>
#include "vec4.hpp"
class SNAKE_Engine;
struct GLFWwindow;
struct EngineContext;

/**
 * @brief Manages the GLFW window: creation, size/title, clear color, fullscreen, per-frame ops.
 *
 * @details
 * - Owns the GLFWwindow* handle and caches width/height.
 * - Initialized via Init(); sets GL 4.6 core profile hints, creates the window, makes the context current,
 *   loads GL with glad, enables OpenGL debug output, installs a debug message callback, and sets the initial viewport.
 * - Sets the window user pointer to SNAKE_Engine and registers callbacks:
 *   - framebuffer_size_callback: calls glViewport, updates SetWidth/SetHeight, and forwards the new size to
 *     the current state's CameraManager::SetScreenSizeForAll(width, height) through StateManager.
 *   - glfwSetScrollCallback/glfwSetKeyCallback/glfwSetMouseButtonCallback: forward to
 *     InputManager::AddScroll / OnKey / OnMouseButton.
 * - Typical per-frame calls by the engine: PollEvents(), ClearScreen(), SwapBuffers().
 */
class WindowManager
{
    friend SNAKE_Engine;
    friend void framebuffer_size_callback(GLFWwindow*, int, int);
public:
    /**
     * @brief Constructs with default size (800x600) and backgroundColor (0.4,0.4,0.4,1).
     */
    WindowManager() :window(nullptr), windowWidth(800), windowHeight(600), backgroundColor(0.4, 0.4, 0.4, 1) {}

    /**
     * @brief Returns the underlying GLFWwindow handle (nullptr if not initialized).
     */
    [[nodiscard]] GLFWwindow* GetHandle() const { return window; }

    /**
     * @brief Returns the cached window width (pixels).
     */
    [[nodiscard]] int GetWidth() const { return windowWidth; }

    /**
     * @brief Returns the cached window height (pixels).
     */
    [[nodiscard]] int GetHeight() const { return windowHeight; }

    /**
     * @brief Programmatically resize the window.
     *
     * @details
     * Calls glfwSetWindowSize(window, width, height) and updates internal width/height.
     *
     * @param width New width in pixels.
     * @param height New height in pixels.
     */
    void Resize(int width, int height);

    /**
     * @brief Set the window title.
     * @param title UTF-8 title string (passed to glfwSetWindowTitle).
     */
    void SetTitle(const std::string& title) const;

    /**
     * @brief Set the RGBA clear color used by ClearScreen().
     * @param color RGBA in [0,1] per channel.
     * @note ClearScreen() applies this via glClearColor.
     */
    void SetBackgroundColor(glm::vec4 color) { backgroundColor = color; }

    /**
     * @brief Toggle fullscreen on/off for the primary monitor.
     *
     * @details
     * - Saves windowed position/size on enter, restores them on exit.
     * - Uses glfwSetWindowMonitor(...) and updates isFullscreen.
     * - Updates cached width/height to match the new mode.
     *
     * @param enable true to enter fullscreen; false to return to windowed.
     */
    void SetFullScreen(bool enable);

    /**
     * @brief Returns whether the window is currently fullscreen.
     */
    bool IsFullScreen() const { return isFullscreen; }

private:
    /**
     * @brief Initialize GLFW window and GL context.
     *
     * @details
     * - glfwInit + version/profile hints (4.6 core), glfwCreateWindow("SNAKE ENGINE", ...)
     * - glfwMakeContextCurrent, gladLoadGL(glfwGetProcAddress)
     * - Enable GL debug output and set a debug callback (filters notifications/other)
     * - glViewport(0,0,width,height)
     * - glfwSetWindowUserPointer(window, &engine)
     * - Register callbacks: framebuffer size / scroll / key / mouse button / window pos
     *
     * @param _windowWidth Initial width in pixels.
     * @param _windowHeight Initial height in pixels.
     * @param engine Engine instance stored as the window user pointer.
     * @return true on success; false on failure (also logs errors).
     * @note Internal. Called by SNAKE_Engine during initialization.
     */
    bool Init(int _windowWidth, int _windowHeight, SNAKE_Engine& engine);

    /**
     * @brief Internal setter used by framebuffer_size_callback.
     * @param width New width (pixels).
     * @note Internal use only.
     */
    void SetWidth(int width) { this->windowWidth = width; }

    /**
     * @brief Internal setter used by framebuffer_size_callback.
     * @param height New height (pixels).
     * @note Internal use only.
     */
    void SetHeight(int height) { this->windowHeight = height; }

    /**
     * @brief Swap front/back buffers (glfwSwapBuffers).
     * @note Internal. Called each frame by SNAKE_Engine.
     */
    void SwapBuffers() const;

    /**
     * @brief Clear the color buffer using backgroundColor (glClearColor + glClear(GL_COLOR_BUFFER_BIT)).
     * @note Internal. Called each frame by SNAKE_Engine.
     */
    void ClearScreen() const;

    /**
     * @brief Poll window system events (glfwPollEvents).
     * @note Internal. Called each frame by SNAKE_Engine.
     */
    void PollEvents() const;

    /**
     * @brief Destroy the window (glfwDestroyWindow).
     * @note Internal. Called by SNAKE_Engine during shutdown.
     */
    void Free() const;

    GLFWwindow* window;
    int windowWidth;
    int windowHeight;
    glm::vec4 backgroundColor;

    bool isFullscreen = false;
    int windowedPosX = 100, windowedPosY = 100;
    int windowedWidth = 800, windowedHeight = 600;
};
