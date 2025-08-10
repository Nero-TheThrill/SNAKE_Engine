#pragma once
#include "glm.hpp"

class Camera2D
{
public:
    Camera2D(int screenWidth = 800, int screenHeight = 600);

    void SetScreenSize(int width, int height);
    [[nodiscard]] int GetScreenWidth() const { return screenWidth; }
    [[nodiscard]] int GetScreenHeight() const { return screenHeight; }

    void SetPosition(const glm::vec2& pos);
    void AddPosition(const glm::vec2& pos);
    [[nodiscard]] const glm::vec2& GetPosition() const;

    void SetZoom(float z);
    [[nodiscard]] float GetZoom() const;

    [[nodiscard]] glm::mat4 GetViewMatrix() const;

    [[nodiscard]] bool IsInView(const glm::vec2& pos, float radius, glm::vec2 viewportSize) const;

private:
    glm::vec2 position = glm::vec2(0.0f);
    float zoom = 1.0f;
    int screenWidth = 800;
    int screenHeight = 600;
};
