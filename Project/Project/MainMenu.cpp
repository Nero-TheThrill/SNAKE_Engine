#include "MainMenu.h"
#include <iostream>
#include "Button.h"
#include "Debug.h"
#include "Level1.h"

void MainMenu::Load(const EngineContext& engineContext)
{
    SNAKE_LOG("[MainMenu] load called");
}

void MainMenu::Init(const EngineContext& engineContext)
{
    SNAKE_LOG("[MainMenu] init called");


    objectManager.AddObject(std::make_unique<Player>(), "player")->SetRenderLayer(engineContext, "Penguin");
    objectManager.AddObject(std::make_unique<Enemy>(glm::vec2(200,0)), "enemy");


    startText = static_cast<TextObject*>(objectManager.AddObject(std::make_unique<TextObject>(engineContext.renderManager->GetFontByTag("default"),"START",TextAlignH::Center, TextAlignV::Middle), "StartText"));
    startText->GetTransform2D().SetPosition({ 0,100 });
    startText->SetIgnoreCamera(true, cameraManager.GetActiveCamera());
    startText->SetRenderLayer(engineContext, "UI");

    startButton = static_cast<GameObject*>(objectManager.AddObject(std::make_unique<Button>(), "StartButton"));
    startButton->GetTransform2D().SetPosition({ startText->GetWorldPosition() });
    startButton->GetTransform2D().SetScale({ startText->GetWorldScale()*1.5f });

    quitText = static_cast<TextObject*>(objectManager.AddObject(std::make_unique<TextObject>(engineContext.renderManager->GetFontByTag("default"), "QUIT", TextAlignH::Center, TextAlignV::Middle), "QuitText"));
    quitText->GetTransform2D().SetPosition({ 0,-100 });
    quitText->SetIgnoreCamera(true, cameraManager.GetActiveCamera());
    quitText->SetRenderLayer(engineContext, "UI");

    quitButton = static_cast<GameObject*>(objectManager.AddObject(std::make_unique<Button>(), "QuitButton"));
    quitButton->GetTransform2D().SetPosition({ quitText->GetWorldPosition() });
    quitButton->GetTransform2D().SetScale({ quitText->GetWorldScale() * 1.5f });


    bulletCountText = static_cast<TextObject*>(objectManager.AddObject(
        std::make_unique<TextObject>(engineContext.renderManager->GetFontByTag("default"), "0", TextAlignH::Center, TextAlignV::Middle), "text"));
    bulletCountText->GetTransform2D().SetScale({ 0.5, 0.5 });
    bulletCountText->SetRenderLayer(engineContext, "UI.Penguin");
}

void MainMenu::LateInit(const EngineContext& engineContext)
{
}

void MainMenu::Update(float dt, const EngineContext& engineContext)
{
    if (engineContext.inputManager->IsKeyReleased(KEY_N))
    {
        engineContext.stateManager->ChangeState(std::make_unique<Level1>());
    }
    if (engineContext.inputManager->IsKeyPressed(KEY_ESCAPE))
    {
        engineContext.engine->RequestQuit();
    }

    if (engineContext.inputManager->IsKeyPressed(KEY_3))
    {
        engineContext.engine->RenderDebugDraws(true);
    }
    if (engineContext.inputManager->IsKeyPressed(KEY_4))
    {
        engineContext.engine->RenderDebugDraws(false);
    }

    if (startButton->GetColor() == glm::vec4(0.3, 0.3, 0.3, 1.0))
    {
        if (engineContext.inputManager->IsKeyPressed(KEY_SPACE))
        {
            engineContext.stateManager->ChangeState(std::make_unique<Level1>());
        }
    }
    if (quitButton->GetColor() == glm::vec4(0.3, 0.3, 0.3, 1.0))
    {
        if (engineContext.inputManager->IsKeyPressed(KEY_SPACE))
        {
            engineContext.engine->RequestQuit();
        }
    }

    if (startButton->GetCollider()->CheckPointCollision(engineContext.inputManager->GetMouseWorldPos(cameraManager.GetActiveCamera())))
    {
        startButton->SetColor({ 0.3,0.3,0.3,1.0 });
        startText->SetColor({ 0.3,0.3,0.3,1.0 });
        if (engineContext.inputManager->IsMouseButtonReleased(MOUSE_BUTTON_LEFT))
        {
            engineContext.stateManager->ChangeState(std::make_unique<Level1>());
        }
    }
    else
    {
        startButton->SetColor({ 1.0,1.0,1.0,1.0});
        startText->SetColor({ 1.0,1.0,1.0,1.0 });
    }

    if (quitButton->GetCollider()->CheckPointCollision(engineContext.inputManager->GetMouseWorldPos(cameraManager.GetActiveCamera())))
    {
        quitButton->SetColor({ 0.3,0.3,0.3,1.0 });
        quitText->SetColor({ 0.3,0.3,0.3,1.0 });
        if (engineContext.inputManager->IsMouseButtonReleased(MOUSE_BUTTON_LEFT))
        {
            engineContext.engine->RequestQuit();
        }
    }
    else
    {
        quitButton->SetColor({ 1.0,1.0,1.0,1.0 });
        quitText->SetColor({ 1.0,1.0,1.0,1.0 });
    }

    std::vector<Object*> bullets;
    objectManager.FindByTag("bullet", bullets);
    for (auto* bullet : bullets)
    {
        engineContext.renderManager->DrawDebugLine(
            bullet->GetTransform2D().GetPosition(),
            objectManager.FindByTag("player")->GetWorldPosition(),
            cameraManager.GetActiveCamera());
    }

    objectManager.FindByTag("enemyBullet", bullets);
    bulletCountText->SetText(std::to_string(bullets.size()));
    bulletCountText->GetTransform2D().SetPosition(objectManager.FindByTag("player")->GetTransform2D().GetPosition() + glm::vec2(0, 50));


    objectManager.UpdateAll(dt, engineContext);
}

void MainMenu::LateUpdate(float dt, const EngineContext& engineContext)
{
}

void MainMenu::Draw(const EngineContext& engineContext)
{
    engineContext.renderManager->ClearBackground(0, 0, engineContext.windowManager->GetWidth(), engineContext.windowManager->GetHeight(), { 0.2,0.2,0.5,1 });
    objectManager.DrawAll(engineContext, cameraManager.GetActiveCamera());
}


void MainMenu::Free(const EngineContext& engineContext)
{
    SNAKE_LOG("[MainMenu] free called");
}

void MainMenu::Unload(const EngineContext& engineContext)
{
    SNAKE_LOG("[MainMenu] unload called");
}
