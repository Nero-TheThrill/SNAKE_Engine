#pragma once
#include <bitset>
#include "vec2.hpp"

class Camera2D;
class SNAKE_Engine;
struct GLFWwindow;

/**
 * @brief Frame-based keyboard/mouse input tracker backed by bitsets.
 *
 * @details
 * - Per-frame semantics: Update() copies staged->current, current->previous.
 *   Press/Release queries compare current vs. previous.
 * - Mouse:
 *   - Cursor position is sampled each Update() via glfwGetCursorPos().
 *   - Scroll: AddScroll() accumulates deltas; Update() publishes them as per-frame
 *     scrollDelta and resets the accumulator.
 * - World-space mouse helpers project window coordinates using Camera2D
 *   (position, screen size, zoom).
 */
class InputManager
{
    friend SNAKE_Engine;

public:

    /**
     * @brief Constructs with null window and zeroed mouse position.
     */
    InputManager() :window(nullptr), mouseX(0.0), mouseY(0.0) {}

    /**
     * @brief True while the key is held down (current frame).
     * @param key Key code (matches GLFW key codes; see InputKey).
     */
    [[nodiscard]] bool IsKeyDown(int key) const;

    /**
     * @brief True only on the frame the key transitions up->down.
     * @param key Key code.
     * @note Requires per-frame Update() for correct previous/current tracking.
     */
    [[nodiscard]] bool IsKeyPressed(int key) const;

    /**
     * @brief True only on the frame the key transitions down->up.
     * @param key Key code.
     * @note Requires per-frame Update().
     */
    [[nodiscard]] bool IsKeyReleased(int key) const;

    /**
     * @brief True while the mouse button is held down (current frame).
     * @param button Mouse button (matches GLFW button codes; see InputMouseButton).
     */
    [[nodiscard]] bool IsMouseButtonDown(int button) const;

    /**
     * @brief True only on the frame the mouse button goes up->down.
     * @param button Mouse button.
     */
    [[nodiscard]] bool IsMouseButtonPressed(int button) const;

    /**
     * @brief True only on the frame the mouse button goes down->up.
     * @param button Mouse button.
     */
    [[nodiscard]] bool IsMouseButtonReleased(int button) const;

    /**
     * @brief Mouse X in window coordinates (pixels).
     * @details Sampled in Update() via glfwGetCursorPos().
     */
    [[nodiscard]] double GetMouseX() const;

    /**
     * @brief Mouse Y in window coordinates (pixels).
     * @details Sampled in Update() via glfwGetCursorPos().
     */
    [[nodiscard]] double GetMouseY() const;

    /**
     * @brief Mouse position in window coordinates (pixels).
     */
    [[nodiscard]] glm::vec2 GetMousePos() const;

    /**
     * @brief Mouse X in world space using Camera2D.
     * @details (cam.pos.x + mouseX - cam.screenWidth/2) / cam.zoom.
     */
    [[nodiscard]] double GetMouseWorldX(Camera2D* camera) const;

    /**
     * @brief Mouse Y in world space using Camera2D.
     * @details (cam.pos.y + cam.screenHeight/2 - mouseY) / cam.zoom.
     */
    [[nodiscard]] double GetMouseWorldY(Camera2D* camera) const;

    /**
     * @brief Mouse position in world space using Camera2D.
     */
    [[nodiscard]] glm::vec2 GetMouseWorldPos(Camera2D* camera) const;

    /**
     * @brief Accumulates scroll deltas (usually from the GLFW scroll callback).
     * @param dx Horizontal scroll delta.
     * @param dy Vertical scroll delta.
     * @note Published on the next Update() as per-frame scrollDelta.
     */
    void AddScroll(double dx, double dy);

    /**
     * @brief Per-frame scroll delta (dx, dy) since the last Update().
     */
    glm::vec2 GetScrollDelta() const;

    /**
     * @brief Per-frame horizontal scroll delta since the last Update().
     */
    double GetScrollXDelta() const;

    /**
     * @brief Per-frame vertical scroll delta since the last Update().
     */
    double GetScrollYDelta() const;

    /**
     * @brief True if vertical scroll this frame is positive.
     */
    bool IsScrolledUp() const;

    /**
     * @brief True if vertical scroll this frame is negative.
     */
    bool IsScrolledDown() const;

    /**
     * @brief Key event intake (used by the GLFW key callback).
     * @param key GLFW key code; ignored if out of range (key<0 || key>GLFW_KEY_LAST).
     * @param action GLFW_PRESS, GLFW_RELEASE, or GLFW_REPEAT.
     * @details Updates staged key state; repeat sets pressed if not already set.
     */
    void OnKey(int key, int, int action, int);

    /**
     * @brief Mouse button event intake (used by the GLFW mouse button callback).
     * @param button GLFW button code; ignored if out of range.
     * @param action GLFW_PRESS or GLFW_RELEASE.
     * @details Updates staged mouse button state.
     */
    void OnMouseButton(int button, int action, int);

    /**
     * @brief Clears all key/mouse bitsets (current/previous/staged).
     * @note Does not modify mouse position or scroll values.
     */
    void Reset();

private:
    /**
     * @brief Stores the window handle for input sampling.
     * @note Internal. Called by the engine during initialization.
     */
    void Init(GLFWwindow* _window);

    /**
     * @brief Advances input state one frame.
     *
     * @details
     * - previous = current
     * - scrollDelta = scrollAccum; scrollAccum = 0
     * - current = staged
     * - samples mouseX/Y via glfwGetCursorPos(window, ...)
     *
     * @note Internal. Called once per frame by the engine.
     */
    void Update();

    GLFWwindow* window;

    static constexpr int MAX_KEYS = 349;            ///< Bitset capacity for keys.
    static constexpr int MAX_MOUSE_BUTTONS = 9;     ///< Bitset capacity for mouse buttons.

    std::bitset<MAX_KEYS> currentKeyState;
    std::bitset<MAX_KEYS> previousKeyState;
    std::bitset<MAX_MOUSE_BUTTONS> currentMouseState;
    std::bitset<MAX_MOUSE_BUTTONS> previousMouseState;

    double mouseX;
    double mouseY;

    double scrollAccumX = 0.0;
    double scrollAccumY = 0.0;
    double scrollDeltaX = 0.0;
    double scrollDeltaY = 0.0;

    std::bitset<MAX_KEYS> stagedKeyState;
    std::bitset<MAX_MOUSE_BUTTONS> stagedMouseState;

};

/**
 * @brief Key codes compatible with GLFW (pass to InputManager APIs).
 */
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

/**
 * @brief Mouse button codes compatible with GLFW.
 */
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
