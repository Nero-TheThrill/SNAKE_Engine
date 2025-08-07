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

    for (int key = 0; key <= GLFW_KEY_LAST; key++)
    {
        currentKeyState.set(key, glfwGetKey(window, key) == GLFW_PRESS);
    }
    for (int button = 0; button <= GLFW_MOUSE_BUTTON_LAST; button++)
    {
        currentMouseState.set(button, glfwGetMouseButton(window, button) == GLFW_PRESS);
    }
    glfwGetCursorPos(window, &mouseX, &mouseY);
}

void InputManager::Reset()
{
    previousKeyState = currentKeyState;
    previousMouseState = currentMouseState;
    currentKeyState.reset();
    currentMouseState.reset();
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