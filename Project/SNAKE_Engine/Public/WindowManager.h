#pragma once
#include <string>
#include "vec4.hpp"
class SNAKE_Engine;
struct GLFWwindow;
struct EngineContext;

class WindowManager
{
    friend SNAKE_Engine;
    friend void framebuffer_size_callback(GLFWwindow*, int, int);
public:
    WindowManager() :window(nullptr), windowWidth(800), windowHeight(600), backgroundColor(0.4,0.4,0.4,1){}

    [[nodiscard]] GLFWwindow* GetHandle() const { return window; }

    [[nodiscard]] int GetWidth() const { return windowWidth; }

    [[nodiscard]] int GetHeight() const { return windowHeight; }

    void Resize(int width, int height);

    void SetTitle(const std::string& title) const;

    void SetBackgroundColor(glm::vec4 color) { backgroundColor = color; }

private:
    bool Init(int _windowWidth, int _windowHeight, SNAKE_Engine& engine);

    void SetWidth(int width) { this->windowWidth = width; }

    void SetHeight(int height) { this->windowHeight = height; }

    void SwapBuffers() const;

    void ClearScreen() const;

    void PollEvents() const;

    void Free() const;

    GLFWwindow* window;
    int windowWidth;
    int windowHeight;
    glm::vec4 backgroundColor;
};
