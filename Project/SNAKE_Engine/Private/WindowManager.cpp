#include "WindowManager.h"

#include "Debug.h"
#include "gl.h"
#include "glfw3.h"
#include "SNAKE_Engine.h"
#include "GameState.h"
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
    SNAKE_Engine* snakeEngine = static_cast<SNAKE_Engine*>(glfwGetWindowUserPointer(window));
    if (snakeEngine)
    {
        snakeEngine->GetEngineContext().windowManager->SetWidth(width);
        snakeEngine->GetEngineContext().windowManager->SetHeight(height);
        snakeEngine->GetEngineContext().stateManager->GetCurrentState()->GetCameraManager().SetScreenSizeForAll(width, height);
        snakeEngine->GetEngineContext().inputManager->Reset();
        SNAKE_LOG("changed: " << snakeEngine->GetEngineContext().windowManager->GetWidth() << " " << snakeEngine->GetEngineContext().windowManager->GetHeight());
    }
}
bool WindowManager::Init(int _windowWidth, int _windowHeight, SNAKE_Engine& engine)
{

    if (!glfwInit())
    {
        SNAKE_ERR("Failed to initialize GLFW");
        return false;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    windowWidth = _windowWidth;
    windowHeight = _windowHeight;

    window = glfwCreateWindow(windowWidth, windowHeight, "SNAKE ENGINE", nullptr, nullptr);
    if (!window)
    {
        SNAKE_ERR("Failed to create GLFW window");
        glfwTerminate();
        return false;
    }

    glfwMakeContextCurrent(window);

    if (!gladLoadGL(glfwGetProcAddress))
    {
        SNAKE_ERR("Failed to initialize GLAD");
        return false;
    }

    glViewport(0, 0, windowWidth, windowHeight);
    glfwSetWindowUserPointer(window, &engine);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    return true;
}

void WindowManager::Resize(int width, int height)
{
    if (window)
    {
        glfwSetWindowSize(window, width, height);
        windowWidth = width;
        windowHeight = height;
    }
}

void WindowManager::SetTitle(const std::string& title) const
{
    glfwSetWindowTitle(window, title.c_str());
}

void WindowManager::SetFullScreen(bool enable)
{
    if (!window || isFullscreen == enable)
        return;

    GLFWmonitor* monitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* mode = glfwGetVideoMode(monitor);

    if (enable)
    {
        glfwGetWindowPos(window, &windowedPosX, &windowedPosY);
        glfwGetWindowSize(window, &windowedWidth, &windowedHeight);

        glfwSetWindowMonitor(window, monitor, 0, 0, mode->width, mode->height, mode->refreshRate);
    }
    else
    {
        glfwSetWindowMonitor(window, nullptr, windowedPosX, windowedPosY, windowedWidth, windowedHeight, 0);
    }

    isFullscreen = enable;

    windowWidth = enable ? mode->width : windowedWidth;
    windowHeight = enable ? mode->height : windowedHeight;
}
void WindowManager::Free() const
{
    glfwDestroyWindow(window);
}

void WindowManager::SwapBuffers() const
{
    glfwSwapBuffers(window);
}

void WindowManager::ClearScreen() const
{
    glClearColor(backgroundColor.r, backgroundColor.g, backgroundColor.b, backgroundColor.a);
    glClear(GL_COLOR_BUFFER_BIT);
}

void WindowManager::PollEvents() const
{
    glfwPollEvents();
}
