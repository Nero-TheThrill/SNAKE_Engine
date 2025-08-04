#pragma once
#include <bitset>
#include "vec2.hpp"

class Camera2D;
class SNAKE_Engine;
struct GLFWwindow;

/**
 * @brief Handles real-time keyboard and mouse input tracking.
 *
 * @details
 * InputManager tracks both keyboard and mouse input using fixed-size bitsets,
 * maintaining the current and previous frame states for all keys and mouse buttons.
 * This allows for precise input queries like Down, Pressed (edge trigger), and Released.
 *
 * Mouse position is tracked in screen-space coordinates, and can also be
 * transformed into world-space coordinates using a Camera2D reference.
 *
 * InputManager must be updated once per frame using Update().
 * Reset() is not called every frame-it is only called when the window size changes, such as when the framebuffer is resized.
 *
 * The system is designed around the GLFW backend and requires a GLFWwindow* handle
 * during initialization. InputManager is owned and initialized by SNAKE_Engine.
 *
 * @note This class is not intended to be created or accessed manually by users.
 */

class InputManager
{
    friend SNAKE_Engine;

public:
    /**
     * @brief Constructs an uninitialized input manager.
     *
     * @details
     * Initializes internal mouse position to (0.0, 0.0) and sets the GLFWwindow pointer to nullptr.
     * Bitsets for key and mouse state are default-initialized to all false.
     * Input polling does not work until Init() is called with a valid window handle.
     *
     * @note This constructor is only used internally by the engine.
     */
    InputManager() :window(nullptr), mouseX(0.0), mouseY(0.0) {}

    /**
     * @brief Checks if a key is currently being held down.
     *
     * @details
     * Returns true if the key was down in the most recent Update() call.
     * The key state is looked up from the currentKeyState bitset.
     *
     * @param key The GLFW key code to check.
     * @return true if the key is currently pressed, false otherwise.
     *
     * @code
     * if (input.IsKeyDown(KEY_W)) MoveUp();
     * @endcode
     */
    [[nodiscard]] bool IsKeyDown(int key) const;

    /**
     * @brief Checks if a key is currently being held down.
     *
     * @details
     * Returns true if the key was down in the most recent Update() call.
     * The key state is looked up from the currentKeyState bitset.
     *
     * @param key The GLFW key code to check.
     * @return true if the key is currently pressed, false otherwise.
     *
     * @code
     * if (input.IsKeyDown(KEY_W)) MoveUp();
     * @endcode
     */
    [[nodiscard]] bool IsKeyPressed(int key) const;

    /**
     * @brief Checks if a key was just pressed this frame.
     *
     * @details
     * Returns true only if the key was not pressed in the previous frame,
     * but is currently down in the current frame.
     *
     * Internally compares currentKeyState[key] and previousKeyState[key].
     *
     * @param key The GLFW key code to check.
     * @return true if the key was just pressed this frame, false otherwise.
     *
     * @code
     * if (input.IsKeyPressed(KEY_SPACE)) Jump();
     * @endcode
     */
    [[nodiscard]] bool IsKeyReleased(int key) const;

    /**
     * @brief Checks if a mouse button is currently being held down.
     *
     * @details
     * Looks up the button's state in the currentMouseState bitset.
     *
     * @param button The GLFW mouse button index (e.g., MOUSE_BUTTON_LEFT).
     * @return true if the button is currently pressed, false otherwise.
     */
    [[nodiscard]] bool IsMouseButtonDown(int button) const;

    /**
     * @brief Checks if a mouse button was just pressed this frame.
     *
     * @details
     * Returns true only if the button was not pressed in the previous frame,
     * but is currently down.
     *
     * Internally compares currentMouseState[button] and previousMouseState[button].
     *
     * @param button The GLFW mouse button index.
     * @return true if the button was just pressed, false otherwise.
     */
    [[nodiscard]] bool IsMouseButtonPressed(int button) const;

    /**
     * @brief Checks if a mouse button was just released this frame.
     *
     * @details
     * Returns true only if the button was pressed in the previous frame
     * and is now released in the current frame.
     *
     * Internally compares currentMouseState[button] and previousMouseState[button].
     *
     * @param button The GLFW mouse button index.
     * @return true if the button was just released, false otherwise.
     */
    [[nodiscard]] bool IsMouseButtonReleased(int button) const;

    /**
     * @brief Returns the current mouse X coordinate in screen space.
     *
     * @details
     * The value is updated each frame during Update() using GLFW's cursor polling.
     * The coordinate represents the horizontal pixel position from the left of the window.
     *
     * @return Mouse X position in screen space (pixels).
     */
    [[nodiscard]] double GetMouseX() const;

    /**
     * @brief Returns the current mouse X coordinate in screen space.
     *
     * @details
     * The value is updated each frame during Update() using GLFW's cursor polling.
     * The coordinate represents the horizontal pixel position from the left of the window.
     *
     * @return Mouse X position in screen space (pixels).
     */
    [[nodiscard]] double GetMouseY() const;

    /**
     * @brief Returns the current mouse Y coordinate in screen space.
     *
     * @details
     * The value is updated each frame during Update().
     * The coordinate is in pixels from the top of the window,
     * typically inverted in OpenGL compared to screen space.
     *
     * @return Mouse Y position in screen space (pixels).
     */
    [[nodiscard]] glm::vec2 GetMousePos() const;

    /**
     * @brief Returns the current mouse position as a 2D screen-space vector.
     *
     * @details
     * Combines GetMouseX() and GetMouseY() into a single glm::vec2.
     *
     * @return Mouse position in screen coordinates.
     */
    [[nodiscard]] double GetMouseWorldX(Camera2D* camera) const;

    /**
     * @brief Returns the mouse Y coordinate in world space.
     *
     * @details
     * Converts the screen-space mouse Y position to world coordinates
     * using the given Camera2D.
     *
     * @param camera Pointer to the Camera2D used for conversion.
     * @return Mouse Y position in world space.
     */
    [[nodiscard]] double GetMouseWorldY(Camera2D* camera) const;

    /**
     * @brief Returns the mouse position in world space as a 2D vector.
     *
     * @details
     * Combines GetMouseWorldX() and GetMouseWorldY() into a single vector.
     * Useful for world-space interaction (e.g., object picking).
     *
     * @param camera Pointer to the Camera2D used for conversion.
     * @return World-space mouse position (glm::vec2).
     */
    [[nodiscard]] glm::vec2 GetMouseWorldPos(Camera2D* camera) const;

    /**
     * @brief Resets input states by clearing all bitsets and positions.
     *
     * @details
     * Clears all current and previous input bitsets for keys and mouse buttons,
     * and resets mouse position to (0, 0). This function is not called every frame-
     * it is intended for one-time use when the window size changes.
     *
     * @note Called only during window resizing (e.g., framebuffer_size_callback).
     */
    void Reset();

private:
    /**
     * @brief Initializes the InputManager with a GLFW window handle.
     *
     * @details
     * This function stores the provided GLFWwindow pointer for future input queries.
     * Without calling this, Update() and all input state functions will be invalid.
     * This function is called internally by SNAKE_Engine during system setup.
     *
     * @param _window A valid GLFWwindow pointer to use for polling input.
     *
     * @note Must be called before using Update() or any input query function.
     */
    void Init(GLFWwindow* _window);

    /**
     * @brief Updates the input state by polling GLFW.
     *
     * @details
     * This function should be called once per frame to:
     * - Copy current key/mouse state to previous
     * - Query latest key/mouse input from GLFW
     * - Update currentKeyState and currentMouseState bitsets
     * - Retrieve current mouse cursor position
     *
     * This enables accurate tracking of Pressed/Released transitions.
     * Must be called before querying IsKeyPressed(), IsMouseButtonReleased(), etc.
     *
     * @note Internally called by SNAKE_Engine once per frame before logic updates.
     */
    void Update();

    GLFWwindow* window;

    static constexpr int MAX_KEYS = 349;
    static constexpr int MAX_MOUSE_BUTTONS = 9;

    std::bitset<MAX_KEYS> currentKeyState;
    std::bitset<MAX_KEYS> previousKeyState;
    std::bitset<MAX_MOUSE_BUTTONS> currentMouseState;
    std::bitset<MAX_MOUSE_BUTTONS> previousMouseState;

    double mouseX;
    double mouseY;
};

enum InputKey
{
    KEY_UNKNOWN = -1,
    KEY_SPACE = 32,
    KEY_APOSTROPHE = 39,
    KEY_COMMA = 44,
    KEY_MINUS = 45,
    KEY_PERIOD = 46,
    KEY_SLASH = 47,
    KEY_0 = 48,
    KEY_1 = 49,
    KEY_2 = 50,
    KEY_3 = 51,
    KEY_4 = 52,
    KEY_5 = 53,
    KEY_6 = 54,
    KEY_7 = 55,
    KEY_8 = 56,
    KEY_9 = 57,
    KEY_SEMICOLON = 59,
    KEY_EQUAL = 61,
    KEY_A = 65,
    KEY_B = 66,
    KEY_C = 67,
    KEY_D = 68,
    KEY_E = 69,
    KEY_F = 70,
    KEY_G = 71,
    KEY_H = 72,
    KEY_I = 73,
    KEY_J = 74,
    KEY_K = 75,
    KEY_L = 76,
    KEY_M = 77,
    KEY_N = 78,
    KEY_O = 79,
    KEY_P = 80,
    KEY_Q = 81,
    KEY_R = 82,
    KEY_S = 83,
    KEY_T = 84,
    KEY_U = 85,
    KEY_V = 86,
    KEY_W = 87,
    KEY_X = 88,
    KEY_Y = 89,
    KEY_Z = 90,
    KEY_LEFT_BRACKET = 91,
    KEY_BACKSLASH = 92,
    KEY_RIGHT_BRACKET = 93,
    KEY_GRAVE_ACCENT = 96,

    KEY_ESCAPE = 256,
    KEY_ENTER = 257,
    KEY_TAB = 258,
    KEY_BACKSPACE = 259,
    KEY_INSERT = 260,
    KEY_DELETE = 261,
    KEY_RIGHT = 262,
    KEY_LEFT = 263,
    KEY_DOWN = 264,
    KEY_UP = 265,
    KEY_PAGE_UP = 266,
    KEY_PAGE_DOWN = 267,
    KEY_HOME = 268,
    KEY_END = 269,
    KEY_CAPS_LOCK = 280,
    KEY_SCROLL_LOCK = 281,
    KEY_NUM_LOCK = 282,
    KEY_PRINT_SCREEN = 283,
    KEY_PAUSE = 284,

    KEY_F1 = 290,
    KEY_F2 = 291,
    KEY_F3 = 292,
    KEY_F4 = 293,
    KEY_F5 = 294,
    KEY_F6 = 295,
    KEY_F7 = 296,
    KEY_F8 = 297,
    KEY_F9 = 298,
    KEY_F10 = 299,
    KEY_F11 = 300,
    KEY_F12 = 301,

    KEY_LEFT_SHIFT = 340,
    KEY_LEFT_CONTROL = 341,
    KEY_LEFT_ALT = 342,
    KEY_LEFT_SUPER = 343,
    KEY_RIGHT_SHIFT = 344,
    KEY_RIGHT_CONTROL = 345,
    KEY_RIGHT_ALT = 346,
    KEY_RIGHT_SUPER = 347,

    KEY_MENU = 348
};

enum InputMouseButton
{
    MOUSE_BUTTON_1 = 0,
    MOUSE_BUTTON_LEFT = MOUSE_BUTTON_1,
    MOUSE_BUTTON_2 = 1,
    MOUSE_BUTTON_RIGHT = MOUSE_BUTTON_2,
    MOUSE_BUTTON_3 = 2,
    MOUSE_BUTTON_MIDDLE = MOUSE_BUTTON_3,
    MOUSE_BUTTON_4 = 3,
    MOUSE_BUTTON_5 = 4,
    MOUSE_BUTTON_6 = 5,
    MOUSE_BUTTON_7 = 6,
    MOUSE_BUTTON_8 = 7,

    MOUSE_BUTTON_LAST = MOUSE_BUTTON_8
};
