#include "FluidParticle.h"
#include <iostream>
#include <random>

#include "Debug.h"
#include "Engine.h"


void FluidParticle::Init(const EngineContext& engineContext)
{
    transform2D.SetPosition(glm::vec2(0, 0));
    transform2D.SetScale(glm::vec2(8.f));
    mesh = engineContext.renderManager->GetMeshByTag("default");
    material = engineContext.renderManager->GetMaterialByTag("m_instancing");
    material->EnableInstancing(true, mesh);
    color = glm::vec4{ 0, 0, 1, 1 };
}

void FluidParticle::LateInit(const EngineContext& engineContext)
{

}

void FluidParticle::Update(float dt, const EngineContext& engineContext)
{
}

void FluidParticle::Draw(const EngineContext& engineContext)
{
}

void FluidParticle::Free(const EngineContext& engineContext)
{
}

void FluidParticle::LateFree(const EngineContext& engineContext)
{
}
