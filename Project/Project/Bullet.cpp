#include "Bullet.h"
#include <random>

#include "Debug.h"
#include "Engine.h"

Bullet::Bullet(glm::vec2 pos, glm::vec2 _dir) : dir(_dir)
{
    transform2D.SetPosition(pos);

}

void Bullet::Init(const EngineContext& engineContext)
{
    SNAKE_LOG("Bullet initialized");
    SetMesh(engineContext, "default");
    SetMaterial(engineContext, "m_instancing");
    SetRenderLayer("Bullet");
    GetMaterial()->EnableInstancing(true, GetMesh());
    AttachAnimator(engineContext.renderManager->GetSpriteSheetByTag("animTest"), 0.08f);


    spriteAnimator->PlayClip("sidewalk");

    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_real_distribution<float> scaleDist(40.0f, 40.0f);
    float scale = scaleDist(gen);


    std::uniform_real_distribution<float> rDist(0.5f, 1.0f);  
    std::uniform_real_distribution<float> gDist(0.5f, 1.0f);  
    std::uniform_real_distribution<float> bDist(0.5f, 1.0f);  
    std::uniform_real_distribution<float> aDist(0.3f, 0.7f);
    std::uniform_real_distribution<float> rotDist(-5.f, 5.f);
    std::uniform_real_distribution<float> speedDist(100.f, 150.f);

    float a = aDist(gen);
    float r = rDist(gen);
    float g = gDist(gen);
    float b = bDist(gen);
    speed = speedDist(gen);
    color = glm::vec4(r, g, b, a);
    rotAmount = rotDist(gen);

    transform2D.SetScale(glm::vec2(scale));
    auto collider = std::make_unique<CircleCollider>(this, 1.f);
    collider->SetUseTransformScale(true);
    SetCollider(std::move(collider));
    SetCollision(engineContext.stateManager->GetCurrentState()->GetObjectManager(), "bullet", { "player"});
}

void Bullet::LateInit(const EngineContext& engineContext)
{
}

void Bullet::Update(float dt, const EngineContext& engineContext)
{
    transform2D.AddRotation(dt* rotAmount);
    transform2D.AddPosition(glm::vec2(dt*speed* dir.x, dt*speed * dir.y));
    timer += dt;
    if (timer >5.f)
        Kill();
}

void Bullet::Draw(const EngineContext& engineContext)
{
   // GetMaterial()->SetUniform("u_Color", glm::vec4(1.0, 1.0, 1.0, 1.0));
}

void Bullet::Free(const EngineContext& engineContext)
{
    SNAKE_LOG("Bullet Free Called");
}

void Bullet::LateFree(const EngineContext& engineContext)
{
    SNAKE_LOG("Bullet LateFree Called");
}
