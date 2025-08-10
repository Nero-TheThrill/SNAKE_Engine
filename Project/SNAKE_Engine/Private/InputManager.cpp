#include "Engine.h"

#include "glfw3.h"

void InputManager::Init(GLFWwindow* _window)
{
    window = _window;
}

void InputManager::Update()
{
    previousKeyState = currentKeyState;
    previousMouseState = currentMouseState;

    scrollDeltaX = scrollAccumX; scrollAccumX = 0.0;
    scrollDeltaY = scrollAccumY; scrollAccumY = 0.0;

    currentKeyState = stagedKeyState;
    currentMouseState = stagedMouseState;

    glfwGetCursorPos(window, &mouseX, &mouseY);
}

void InputManager::Reset()
{
    previousKeyState.reset();
    previousMouseState.reset();
    currentKeyState.reset();
    currentMouseState.reset();
    stagedKeyState.reset();
    stagedMouseState.reset();
}

bool InputManager::IsKeyDown(int key) const
{
    return currentKeyState.test(key);
}

bool InputManager::IsKeyPressed(int key) const
{
    return currentKeyState.test(key) && !previousKeyState.test(key);
}

bool InputManager::IsKeyReleased(int key) const
{
    return !currentKeyState.test(key) && previousKeyState.test(key);
}

bool InputManager::IsMouseButtonDown(int button) const
{
    return currentMouseState.test(button);
}

bool InputManager::IsMouseButtonPressed(int button) const
{
    return currentMouseState.test(button) && !previousMouseState.test(button);
}

bool InputManager::IsMouseButtonReleased(int button) const
{
    return !currentMouseState.test(button) && previousMouseState.test(button);
}

double InputManager::GetMouseX() const
{
    return mouseX;
}

double InputManager::GetMouseY() const
{
    return mouseY;
}

glm::vec2 InputManager::GetMousePos() const
{
    return glm::vec2(mouseX, mouseY);
}

double InputManager::GetMouseWorldX(Camera2D* camera) const
{
    return (camera->GetPosition().x + GetMouseX() - camera->GetScreenWidth() / 2.f) / camera->GetZoom();
}

double InputManager::GetMouseWorldY(Camera2D* camera) const
{
    return (camera->GetPosition().y + camera->GetScreenHeight() / 2.f - GetMouseY()) / camera->GetZoom();
}

glm::vec2 InputManager::GetMouseWorldPos(Camera2D* camera) const
{
    return glm::vec2(GetMouseWorldX(camera), GetMouseWorldY(camera));
}

void InputManager::AddScroll(double dx, double dy)
{
    scrollAccumX += dx;
    scrollAccumY += dy;
}

glm::vec2 InputManager::GetScrollDelta() const
{
    return { scrollDeltaX, scrollDeltaY };
}

double InputManager::GetScrollXDelta() const
{
    return scrollDeltaX;
}

double InputManager::GetScrollYDelta() const
{
    return scrollDeltaY;
}

bool InputManager::IsScrolledUp() const
{
    return scrollDeltaY > 0.0;
}

bool InputManager::IsScrolledDown() const
{
    return scrollDeltaY < 0.0;
}

void InputManager::OnKey(int key, int /*sc*/, int action, int /*mods*/)
{
    if (key < 0 || key > GLFW_KEY_LAST)
        return;

    if (action == GLFW_PRESS)
        stagedKeyState.set(key, true);
    else if (action == GLFW_RELEASE)
        stagedKeyState.set(key, false);
    else if (action == GLFW_REPEAT)
    {
        if (!stagedKeyState.test(key))
            stagedKeyState.set(key, true);
    }
}

void InputManager::OnMouseButton(int button, int action, int /*mods*/)
{
    if (button < 0 || button > GLFW_MOUSE_BUTTON_LAST)
        return;

    if (action == GLFW_PRESS)
        stagedMouseState.set(button, true);
    else if (action == GLFW_RELEASE)
        stagedMouseState.set(button, false);
}