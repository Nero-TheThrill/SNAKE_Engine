#include "Engine.h"

CameraManager::CameraManager()
{
    auto defaultCam = std::make_unique<Camera2D>(1, 1);
    defaultCam->SetZoom(1.0f);
    cameraMap["main"] = std::move(defaultCam);
    activeCameraTag = "main";
}
