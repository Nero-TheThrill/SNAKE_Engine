#include "MainMenu.h"
#include <iostream>

#include "Debug.h"
#include "Level1.h"

#include "Engine.h"

void MainMenu::Load(const EngineContext& engineContext)
{
    SNAKE_LOG("[MainMenu] load called");


    engineContext.renderManager->RegisterMesh("default", std::vector<float>{
        -0.5f, -0.5f, 0.f, 0.f, 0.f, // vertex 0
            0.5f, -0.5f, 0.f, 1.f, 0.f, // vertex 1
            0.5f, 0.5f, 0.f, 1.f, 1.f, // vertex 2
            -0.5f, 0.5f, 0.f, 0.f, 1.f  // vertex 3
    }, std::vector<unsigned int>{0, 1, 2,
        2, 3, 0});

    static std::vector<float> starVertices = {
        // pos         // uv (mapped to [0, 1])
         0.0f,  0.5f, 0.0f,   0.5f, 1.0f,  // top
         0.2f,  0.1f, 0.0f,   0.7f, 0.6f,
         0.5f,  0.1f, 0.0f,   1.0f, 0.6f,
         0.3f, -0.1f, 0.0f,   0.8f, 0.4f,
         0.4f, -0.5f, 0.0f,   0.9f, 0.0f,
         0.0f, -0.2f, 0.0f,   0.5f, 0.3f,
        -0.4f, -0.5f, 0.0f,   0.1f, 0.0f,
        -0.3f, -0.1f, 0.0f,   0.2f, 0.4f,
        -0.5f,  0.1f, 0.0f,   0.0f, 0.6f,
        -0.2f,  0.1f, 0.0f,   0.3f, 0.6f
    };

    static std::vector<unsigned int> starIndices = {
        0, 1, 9,    // top -> inner right -> inner left
        1, 2, 3,
        1, 3, 5,
        3, 4, 5,
        5, 6, 7,
        5, 7, 9,
        7, 8, 9,
        9, 0, 1
    };

    engineContext.renderManager->RegisterMesh("star", starVertices, starIndices);

    TextureSettings ts;
    engineContext.renderManager->RegisterTexture("default", "Textures/Default.jpg", ts);
    engineContext.renderManager->RegisterTexture("uvchecker", "Textures/uvchecker.jpg", ts);
    engineContext.renderManager->RegisterShader("s_default", { {ShaderStage::Vertex,"Shaders/Default.vert"},{ShaderStage::Fragment,"Shaders/Default.frag"} });
    engineContext.renderManager->RegisterShader("s_instancing", { {ShaderStage::Vertex,"Shaders/Instancing.vert"},{ShaderStage::Fragment,"Shaders/Instancing.frag"} });
    engineContext.renderManager->RegisterMaterial("m_default", "s_default", { std::pair<std::string, std::string>("u_Texture","uvchecker") });
    engineContext.renderManager->RegisterMaterial("m_instancing", "s_instancing", { std::pair<std::string, std::string>("u_Texture","default") });
}

void MainMenu::Init(const EngineContext& engineContext)
{
    SNAKE_LOG("[MainMenu] init called");
    objectManager.AddObject(std::make_unique<Player>(), "mainmenu player");

    auto minimapCam = std::make_unique<Camera2D>(engineContext.windowManager->GetWidth(), engineContext.windowManager->GetHeight());
    minimapCam->SetZoom(0.05f);
    cameraManager.RegisterCamera("minimap", std::move(minimapCam));

    cameraManager.SetActiveCamera("minimap");
}

void MainMenu::LateInit(const EngineContext& engineContext)
{
}

void MainMenu::Update(float dt, const EngineContext& engineContext)
{
    if (engineContext.inputManager->IsKeyPressed(KEY_N))
    {
        SNAKE_LOG("[MainMenu] key n pressed , move to mainmenu");
        engineContext.stateManager->ChangeState(std::make_unique<Level1>());
    }
    if (engineContext.inputManager->IsKeyPressed(KEY_ESCAPE))
    {
        engineContext.engine->RequestQuit();
    }
    if (engineContext.inputManager->IsKeyPressed(KEY_Q))
    {
        cameraManager.SetActiveCamera("minimap");
    }
    if (engineContext.inputManager->IsKeyPressed(KEY_E))
    {
        cameraManager.SetActiveCamera("main");
    }
}

void MainMenu::LateUpdate(float dt, const EngineContext& engineContext)
{
    GameState::LateUpdate(dt, engineContext);
}

void MainMenu::Draw(const EngineContext& engineContext)
{
    auto& rm = *engineContext.renderManager;

    rm.SetViewport(0, 0, engineContext.windowManager->GetWidth(), engineContext.windowManager->GetHeight());
    cameraManager.SetActiveCamera("main");
    objectManager.DrawAll(engineContext, cameraManager.GetActiveCamera());

    rm.ClearBackground(10, 10, 200, 200, glm::vec4(0.3, 0.3, 1, 0));
    rm.SetViewport(10, 10, 200, 200);

    cameraManager.SetActiveCamera("minimap");
    cameraManager.SetScreenSize("minimap", 200, 200);
    objectManager.DrawObjectsWithTag(engineContext, cameraManager.GetActiveCamera(), "bullet");
}


void MainMenu::Free(const EngineContext& engineContext)
{
    SNAKE_LOG("[MainMenu] free called");
}

void MainMenu::Unload(const EngineContext& engineContext)
{
    SNAKE_LOG("[MainMenu] unload called");
}
