#include "Bullet1.h"
#include <random>

#include "Debug.h"
#include "Engine.h"

Bullet1::Bullet1(glm::vec2 pos, glm::vec2 _dir) : dir(_dir)
{
    transform2D.SetPosition(pos);

}

void Bullet1::Init(const EngineContext& engineContext)
{
    SNAKE_LOG("Bullet initialized");
    SetMesh(engineContext, "default");
    SetMaterial(engineContext, "m_instancing1");
    SetRenderLayer("Bullet");
    GetMaterial()->EnableInstancing(true, GetMesh());


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
}

void Bullet1::LateInit(const EngineContext& engineContext)
{
}

void Bullet1::Update(float dt, const EngineContext& engineContext)
{
    transform2D.AddRotation(dt* rotAmount);
    transform2D.AddPosition(glm::vec2(dt*speed* dir.x, dt*speed * dir.y));
    timer += dt;
    if (timer >5.f)
        Kill();
}

void Bullet1::Draw(const EngineContext& engineContext)
{
   // GetMaterial()->SetUniform("u_Color", glm::vec4(1.0, 1.0, 1.0, 1.0));
}

void Bullet1::Free(const EngineContext& engineContext)
{
    SNAKE_LOG("Bullet Free Called");
}

void Bullet1::LateFree(const EngineContext& engineContext)
{
    SNAKE_LOG("Bullet LateFree Called");
}
