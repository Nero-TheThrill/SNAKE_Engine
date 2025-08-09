#include "gl.h"
#include "glfw3.h"
#include "Engine.h"

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
    SNAKE_Engine* snakeEngine = static_cast<SNAKE_Engine*>(glfwGetWindowUserPointer(window));
    if (snakeEngine)
    {
        snakeEngine->GetEngineContext().windowManager->SetWidth(width);
        snakeEngine->GetEngineContext().windowManager->SetHeight(height);
        auto* state = snakeEngine->GetEngineContext().stateManager->GetCurrentState();
        if (state)
        {
            state->GetCameraManager().SetScreenSizeForAll(width, height);
        }
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
    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);

    glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_NOTIFICATION,
        0, nullptr, GL_FALSE);
    glDebugMessageCallback(
        [](GLenum /*src*/, GLenum type, GLuint id, GLenum severity, GLsizei, const GLchar* msg, const void*) {
            if (severity == GL_DEBUG_SEVERITY_NOTIFICATION) return;
            if (type == GL_DEBUG_TYPE_OTHER) return;
            if (severity == GL_DEBUG_SEVERITY_HIGH) { SNAKE_ERR(std::string("[GL] ") + msg); }
            else if (severity == GL_DEBUG_SEVERITY_MEDIUM) { SNAKE_WRN(std::string("[GL] ") + msg); }
            else { SNAKE_LOG(std::string("[GL] ") + msg); }
        }, nullptr);

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
    framebuffer_size_callback(window, windowWidth, windowHeight);
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
