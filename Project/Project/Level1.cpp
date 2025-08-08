#include "Level1.h"
#include <iostream>
#include "Debug.h"
#include "MainMenu.h"
#include <random>

#include "Engine.h"
#include "TextObject.h"
#include "Apple.h"
#include "ApplePlayerController.h"

void Level1::Load(const EngineContext& engineContext)
{
    SNAKE_LOG("[Level1] load called");

    engineContext.renderManager->RegisterTexture("t_apple", "Textures/apple.png");
    engineContext.renderManager->RegisterTexture("t_apple_selected", "Textures/apple_highlighted.png");
    engineContext.renderManager->RegisterTexture("t_background", "Textures/tiled_pattern_800x480.png");
    engineContext.renderManager->RegisterTexture("t_selection_box", "Textures/TransparentSquare.png");
    engineContext.renderManager->RegisterTexture("t_border", "Textures/SquareBorder.png");
    engineContext.renderManager->RegisterTexture("t_fill", "Textures/Square.png");
    engineContext.renderManager->RegisterMaterial("m_apple", "s_default1", { std::pair<std::string, std::string>("u_Texture","t_apple") });
    engineContext.renderManager->RegisterMaterial("m_apple_highlighted", "s_default1", { std::pair<std::string, std::string>("u_Texture","t_apple_selected") });
    engineContext.renderManager->RegisterMaterial("m_background", "s_default1", { std::pair<std::string, std::string>("u_Texture","t_background") });
    engineContext.renderManager->RegisterMaterial("m_selection_box", "s_default1", { std::pair<std::string, std::string>("u_Texture","t_selection_box") });
    engineContext.renderManager->RegisterMaterial("m_border", "s_default1", { std::pair<std::string, std::string>("u_Texture","t_border") });
    engineContext.renderManager->RegisterMaterial("m_fill", "s_default1", { std::pair<std::string, std::string>("u_Texture","t_fill") });

    engineContext.engine->RenderDebugDraws(false);
}

void Level1::Init(const EngineContext& engineContext)
{
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<int> dist(1, 9);

    SNAKE_LOG("[Level1] init called");

    auto font = engineContext.renderManager->GetFontByTag("default");

    cameraManager.GetActiveCamera()->SetPosition(
        {
            engineContext.windowManager->GetWidth() * 0.5f - marginX * multiplier,
            engineContext.windowManager->GetHeight() * 0.5f - marginY * multiplier
        });
    cameraManager.GetActiveCamera()->SetZoom(1.0f);

    engineContext.soundManager->Play("bgm", 1, 20);

    auto* backgroundObj = objectManager.AddObject(std::make_unique<GameObject>(), "background");
    backgroundObj->SetMesh(engineContext, "default");
    backgroundObj->SetMaterial(engineContext, "m_background");
    backgroundObj->GetTransform2D().SetScale({ engineContext.windowManager->GetWidth(), engineContext.windowManager->GetHeight() });
    backgroundObj->SetIgnoreCamera(true, cameraManager.GetActiveCamera());
    backgroundObj->SetRenderLayer("Game.Background");

    auto* scoreUIObj = objectManager.AddObject(std::make_unique<GameObject>(), "score_ui");
    scoreUIObj->SetMesh(engineContext, "default");
    scoreUIObj->SetMaterial(engineContext, "m_apple");
    scoreUIObj->GetTransform2D().SetPosition({ 0, 100 });
    scoreUIObj->GetTransform2D().SetScale({ 300, 300 });
    scoreUIObj->SetIgnoreCamera(true, cameraManager.GetActiveCamera());
    scoreUIObj->SetRenderLayer("UI.Pause");
    scoreUIObj->SetVisibility(false);

    scoreUIText = new TextObject(engineContext.renderManager->GetFontByTag("default"), std::to_string(0), TextAlignH::Center, TextAlignV::Middle);
    objectManager.AddObject(std::unique_ptr<TextObject>(scoreUIText), "scoreUIText");
    scoreUIText->GetTransform2D().SetPosition(scoreUIObj->GetTransform2D().GetPosition());
    scoreUIText->GetTransform2D().SetScale({ 3, 3 });
    scoreUIText->SetIgnoreCamera(true, cameraManager.GetActiveCamera());
    scoreUIText->SetRenderLayer("UI.Pause.Text");
    scoreUIText->SetVisibility(false);

    restartUIText = new TextObject(engineContext.renderManager->GetFontByTag("default"), "press R to restart\npress N to go to mainmenu", TextAlignH::Center, TextAlignV::Middle);
    objectManager.AddObject(std::unique_ptr<TextObject>(restartUIText), "restartUIText");
    restartUIText->GetTransform2D().SetPosition(scoreUIObj->GetTransform2D().GetPosition() + glm::vec2(0, -300));
    restartUIText->GetTransform2D().SetScale({ 0.9,0.9 });
    restartUIText->SetColor({ 0.4,0.4,0.4,1 });
    restartUIText->SetIgnoreCamera(true, cameraManager.GetActiveCamera());
    restartUIText->SetRenderLayer("UI.Pause.Text");
    restartUIText->SetVisibility(false);

    objectManager.AddObject(std::make_unique<ApplePlayerController>(), "player_controller");

    for (int row = 0; row < rows; ++row)
    {
        for (int col = 0; col < cols; ++col)
        {
            glm::vec2 pos = { col * (spacingX + appleSizeX) * multiplier, row * (spacingY + appleSizeY) * multiplier };
            int value = dist(gen);

            auto text = new TextObject(engineContext.renderManager->GetFontByTag("default"), std::to_string(value), TextAlignH::Center, TextAlignV::Middle);
            objectManager.AddObject(std::unique_ptr<TextObject>(text), "apple_text");
            text->GetTransform2D().SetPosition(pos);
            text->GetTransform2D().SetScale({ 0.5,0.5 });
            text->SetRenderLayer("UI");

            Apple* apple = (Apple*)objectManager.AddObject(std::make_unique<Apple>(text, value), "apple");
            apple->GetTransform2D().SetPosition({pos});
            apple->GetTransform2D().SetScale({ appleSizeX, appleSizeY });
            apple->SetRenderLayer("Game");
        }
    }

    fillInitialScaleY = engineContext.windowManager->GetHeight() * 0.8f;
    glm::vec2 pos = { cols * (spacingX + appleSizeX) * multiplier, fillInitialScaleY * 0.5f };

    auto timerBarBackground = objectManager.AddObject(std::make_unique<GameObject>(), "timerBarBackground");
    timerBarBackground->SetMesh(engineContext, "default");
    timerBarBackground->SetMaterial(engineContext, "m_border");
    timerBarBackground->SetColor({ 0.0f, 1.0f, 0.12f, 1.0f });
    timerBarBackground->GetTransform2D().SetPosition(pos);
    timerBarBackground->GetTransform2D().SetScale({ appleSizeX, fillInitialScaleY });
    timerBarBackground->SetRenderLayer("Game");

    timerBarFill = objectManager.AddObject(std::make_unique<GameObject>(), "timerBarFill");
    timerBarFill->SetMesh(engineContext, "default");
    timerBarFill->SetMaterial(engineContext, "m_fill");
    timerBarFill->SetColor({ 0.0f, 1.0f, 0.12f, 1.0f });
    timerBarFill->GetTransform2D().SetPosition(pos);
    timerBarFill->GetTransform2D().SetScale({ appleSizeX, fillInitialScaleY });
    timerBarFill->SetRenderLayer("Game");

    gameTimer.Start(120);
    dokidoki.Start(0);
}

void Level1::LateInit(const EngineContext& engineContext)
{
}

void Level1::Update(float dt, const EngineContext& engineContext)
{
    HandleStateInput(engineContext);
    if (gameTimer.IsTimedOut())
    {
        ApplePlayerController* player = (ApplePlayerController*)objectManager.FindByTag("player_controller");
        player->SetVisibility(false);
        scoreUIText->SetText(std::to_string(player->GetScore()));
        scoreUIText->SetVisibility(true);
        restartUIText->SetVisibility(true);
        Object* scoreUIObj = objectManager.FindByTag("score_ui");


    	float pulse = std::sin(dokidoki.GetElapsed() * 6.0f) * 10.f + 400.0f;
    	scoreUIObj->GetTransform2D().SetScale(glm::vec2(pulse));
        scoreUIText->GetTransform2D().SetScale(glm::vec2(pulse/100.f));
    	scoreUIObj->SetVisibility(true);

        std::vector<Object*> selected_objects;
        objectManager.FindByTag("apple", selected_objects);

        
    	for (Object* obj : selected_objects)
        {
            Apple* apple = (Apple*)obj;
            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_real_distribution<float> dist(1.0472f, 2.0944f);
            std::uniform_real_distribution<float> dirDir(-1,1);
            float angle = dist(gen);
            float dir = dirDir(gen);
            glm::vec2 vel = glm::vec2( std::cos(angle), std::sin(angle) )* glm::vec2{ 333 };
            if (dir>0)
            {
                vel.x = -vel.x;
            }

            apple->SetVelocityAndStartDeadTimer(vel);
            apple->Update(dt, engineContext);
        }
        dokidoki.Update(dt);
    }
    else
    {
        float current_scale_y = fillInitialScaleY * (1 - gameTimer.GetProgressPercentage());
        timerBarFill->GetTransform2D().SetScale(
            { timerBarFill->GetTransform2D().GetScale().x, current_scale_y });
        timerBarFill->GetTransform2D().SetPosition(
            { timerBarFill->GetTransform2D().GetPosition().x, current_scale_y * 0.5f });
        gameTimer.Update(dt);
        HandleSoundInput(engineContext);
        GameState::Update(dt, engineContext);
    }
}

void Level1::HandleStateInput(const EngineContext& engineContext)
{
    if (engineContext.inputManager->IsKeyReleased(KEY_N))
    {
        engineContext.stateManager->ChangeState(std::make_unique<MainMenu>());
    }
    if (engineContext.inputManager->IsKeyPressed(KEY_ESCAPE))
    {
        engineContext.engine->RequestQuit();
    }
    if (engineContext.inputManager->IsKeyPressed(KEY_R))
    {
        Restart(engineContext);
    }
}

void Level1::HandleSoundInput(const EngineContext& engineContext)
{
    auto& input = *engineContext.inputManager;

    if (input.IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
    {
        engineContext.soundManager->Play("click1", 1.0f);
    }
    if (input.IsMouseButtonReleased(MOUSE_BUTTON_LEFT))
    {
        engineContext.soundManager->Play("click2", 1.0f);
    }
}

void Level1::LateUpdate(float dt, const EngineContext& engineContext)
{
}

void Level1::Draw(const EngineContext& engineContext)
{
    objectManager.DrawAll(engineContext);
}

void Level1::Free(const EngineContext& engineContext)
{
    engineContext.soundManager->ControlAll(SoundManager::SoundControlType::Stop);
    SNAKE_LOG("[Level1] free called");
}

void Level1::Unload(const EngineContext& engineContext)
{
    SNAKE_LOG("[Level1] unload called");
}
