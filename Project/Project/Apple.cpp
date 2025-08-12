#include "Apple.h"


Apple::Apple(Object* dependant_, int value_) : dependant(dependant_), value(value_)
{
}

void Apple::Init(const EngineContext& engineContext)
{
    this->engineContext = &engineContext;
    SetMesh(engineContext, "default");
    SetSelected(false);
    SetCollider(std::make_unique<AABBCollider>(this, glm::vec2(0.9, 0.9))); 
    SetCollision(engineContext.stateManager->GetCurrentState()->GetObjectManager(), "apple", { "player_selection" });
    vel = { 0,0 };
}

void Apple::LateInit(const EngineContext& engineContext)
{
}

void Apple::Update(float dt, const EngineContext& engineContext)
{
    SetSelected(false);
    if (dead_timer.IsStarted())
    {
        glm::vec2 prev = GetTransform2D().GetPosition();
        vel.y += -980.f *1.5f * dt;
        GetTransform2D().SetPosition(prev + vel * dt);
        dependant->GetTransform2D().SetPosition(GetTransform2D().GetPosition());
        dead_timer.Update(dt);

        if (dead_timer.IsTimedOut())
        {
            Kill();
        }
    }
}

void Apple::Draw(const EngineContext& engineContext)
{
}

void Apple::Free(const EngineContext& engineContext)
{
    dependant->Kill();
}

void Apple::LateFree(const EngineContext& engineContext)
{
}

void Apple::OnCollision(Object* other)
{
    if (dead_timer.IsStarted()) return;

    if (other->GetTag() == "player_controller")
    {
        SetSelected(true);
    }
}

const int& Apple::GetValue() const
{
    return value;
}

void Apple::SetSelected(bool bSelected)
{
    if (bSelected)
    {
        SetMaterial(*engineContext, "m_apple_highlighted");
    }
    else
    {
        SetMaterial(*engineContext, "m_apple");
    }
}

void Apple::SetVelocityAndStartDeadTimer(const glm::vec2& vel)
{
    if (dead_timer.IsStarted())
        return;
    this->vel = vel;
    dead_timer.Start(2.0f);
    collider.reset();
}
