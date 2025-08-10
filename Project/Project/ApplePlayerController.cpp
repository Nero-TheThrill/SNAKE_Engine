#include "ApplePlayerController.h"
#include "Apple.h"
#include <random>

glm::vec2 GetRandomUnitVectorRanged(float min_radian, float max_radian)
{
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_real_distribution<float> dist(min_radian, max_radian); // 60~120

    float angle = dist(gen);
    return { std::cos(angle), std::sin(angle) };
}

ApplePlayerController::ApplePlayerController() :
    funcs{ nullptr }, startPoint{0,0}
{
    /*
    +----------------+----------------+----------------+----------------+
    | prev \ current |   Pressed      |     Down       |   Released     |
    +----------------+----------------+----------------+----------------+
    | Pressed        |      N/A       | dragging       | small chance   |
    | Down           |      N/A       | dragging       | collision check|
    | Released       | start rect     | N/A            | do nothing     |
    +----------------+----------------+----------------+----------------+
    */
    funcs[Pressed][Pressed] = &ApplePlayerController::ShouldNotBeReached;
    funcs[Pressed][Down] = &ApplePlayerController::OnDragging;
    funcs[Pressed][Release] = &ApplePlayerController::DoNothing;
    funcs[Down][Pressed] = &ApplePlayerController::ShouldNotBeReached;
    funcs[Down][Down] = &ApplePlayerController::OnDragging;
    funcs[Down][Release] = &ApplePlayerController::EndDragging;
    funcs[Release][Pressed] = &ApplePlayerController::StartDragging;
    funcs[Release][Down] = &ApplePlayerController::ShouldNotBeReached;
    funcs[Release][Release] = &ApplePlayerController::DoNothing;
}

void ApplePlayerController::Init(const EngineContext& engineContext)
{
    SetMesh(engineContext, "default");
    SetMaterial(engineContext, "m_selection_box");
    SetRenderLayer("UI");
    SetVisibility(false);
    SetCollider(std::make_unique<AABBCollider>(this, glm::vec2(1, 1)));
    SetCollision(engineContext.stateManager->GetCurrentState()->GetObjectManager(), "player_selection", { "apple" });

    GetTransform2D().SetPosition({ -1000, -1000 });
    GetTransform2D().SetScale({ 0.0,0.0 });
}

void ApplePlayerController::LateInit(const EngineContext& engineContext)
{

}

void ApplePlayerController::Update(float dt, const EngineContext& engineContext)
{
    CheckSelectedApples(engineContext);

    prevState = currentState;
    if (engineContext.inputManager->IsMouseButtonDown(MOUSE_BUTTON_LEFT))
        currentState = Down;
    if (engineContext.inputManager->IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
        currentState = Pressed;
    if (engineContext.inputManager->IsMouseButtonReleased(MOUSE_BUTTON_LEFT))
        currentState = Release;
    
    (this->*funcs[prevState][currentState])(engineContext);
}

void ApplePlayerController::Draw(const EngineContext& engineContext)
{

}

void ApplePlayerController::Free(const EngineContext& engineContext)
{
}

void ApplePlayerController::LateFree(const EngineContext& engineContext)
{
}

void ApplePlayerController::OnCollision(Object* other)
{
    if (!checkApples) return;
    
    selectedObjects.push_back(other);
}

void ApplePlayerController::StartDragging(const EngineContext& engineContext)
{
    SNAKE_LOG("[ApplePlayerController]  StartDragging");
    SetVisibility(true);
    startPoint = {
        engineContext.inputManager->GetMouseX(),
        engineContext.inputManager->GetMouseY()
    };
    startPoint = ConvertScreenToCamera(engineContext.stateManager->GetCurrentState()->GetActiveCamera(), startPoint);
    GetTransform2D().SetScale({ 0.0,0.0 });
}

void ApplePlayerController::EndDragging(const EngineContext& engineContext)
{
    SNAKE_LOG("[ApplePlayerController]  EndDragging");
    SetVisibility(false);
    checkApples = true;
    selectedObjects.clear();
}

void ApplePlayerController::DoNothing(const EngineContext& engineContext)
{
}

void ApplePlayerController::OnDragging(const EngineContext& engineContext)
{
    glm::vec2 end_point = {
        engineContext.inputManager->GetMouseX(),
        engineContext.inputManager->GetMouseY()
    };
    end_point = ConvertScreenToCamera(engineContext.stateManager->GetCurrentState()->GetActiveCamera(), end_point);
    GetTransform2D().SetPosition((end_point + startPoint) * 0.5f);
    GetTransform2D().SetScale(glm::abs(end_point - startPoint));
}

void ApplePlayerController::ShouldNotBeReached(const EngineContext& engineContext)
{
    SNAKE_LOG("[ApplePlayerController]  ShouldNotBeReached");
}

void ApplePlayerController::CheckSelectedApples(const EngineContext& engineContext)
{
    if (!checkApples) return;
    
    int sum = 0;
    for (Object* obj : selectedObjects)
    {
        Apple* apple = (Apple*)obj;
        apple->SetSelected(false);
        sum += apple->GetValue();
    }

    if (sum == 10)
    {
        for (Object* obj : selectedObjects)
        {
            Apple* apple = (Apple*)obj;
            
            glm::vec2 vel = GetRandomUnitVectorRanged(1.0472f, 2.0944f) * glm::vec2{ 333 };
            if (apple->GetTransform2D().GetPosition().x > GetTransform2D().GetPosition().x)
            {
                vel.x = -vel.x;
            }

            apple->SetVelocityAndStartDeadTimer(vel);
        }
        
        score += selectedObjects.size();
    }

    selectedObjects.clear();
    checkApples = false;
    GetTransform2D().SetPosition({ -1000, -1000 });
}

glm::vec2 ApplePlayerController::ConvertScreenToCamera(Camera2D* cam, const glm::vec2& screen_pos)
{
    return cam->GetPosition() + glm::vec2{ screen_pos.x - cam->GetScreenWidth() / 2, cam->GetScreenHeight() / 2 - screen_pos.y } / cam->GetZoom();
}

int ApplePlayerController::GetScore() const
{
    return score;
}
