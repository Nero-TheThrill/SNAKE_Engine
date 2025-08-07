#include "Engine.h"
#include "gtc/matrix_transform.hpp"

Camera2D::Camera2D(int screenWidth, int screenHeight)
    : screenWidth(screenWidth), screenHeight(screenHeight)
{
}

void Camera2D::SetScreenSize(int width, int height)
{
    screenWidth = width;
    screenHeight = height;
}

void Camera2D::SetPosition(const glm::vec2& pos)
{
    position = pos;
}

void Camera2D::AddPosition(const glm::vec2& pos)
{
    position += pos;
}

void Camera2D::SetZoom(float z)
{
    zoom = z;
}

const glm::vec2& Camera2D::GetPosition() const
{
    return position;
}

float Camera2D::GetZoom() const
{
    return zoom;
}

glm::mat4 Camera2D::GetViewMatrix() const
{
    glm::mat4 view = glm::mat4(1.0f);
    view = glm::translate(view, glm::vec3(-position, 0.0f));
    view = glm::scale(view, glm::vec3(zoom, zoom, 1.0f));
    return view;
}

bool Camera2D::IsInView(const glm::vec2& pos, float radius, glm::vec2 viewportSize) const
{
    glm::vec2 camPos = GetPosition();
    glm::vec2 halfSize = viewportSize * 0.5f;

    return !(pos.x + radius < (camPos.x - halfSize.x) / zoom ||
        pos.x - radius > (camPos.x + halfSize.x) / zoom ||
        pos.y + radius < (camPos.y - halfSize.y) / zoom ||
        pos.y - radius > (camPos.y + halfSize.y) / zoom);
}
