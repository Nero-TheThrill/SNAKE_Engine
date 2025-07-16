#include "FluidSimulation.h"
#include <iostream>
#include "Debug.h"
#include "Engine.h"

void FluidSimulation::Load(const EngineContext& engineContext)
{
    SNAKE_LOG("[FluidSimulation] load called");

    engineContext.renderManager->RegisterMesh("default", std::vector<float>{
        -0.5f, -0.5f, 0.f, 0.f, 0.f, // vertex 0
            0.5f, -0.5f, 0.f, 1.f, 0.f, // vertex 1
            0.5f, 0.5f, 0.f, 1.f, 1.f, // vertex 2
            -0.5f, 0.5f, 0.f, 0.f, 1.f  // vertex 3
    }, std::vector<unsigned int>{0, 1, 2,
        2, 3, 0});

    sim.Create(1.6f, 1.2f);
    sim.Fill(0.5f);

    TextureSettings ts;
    engineContext.renderManager->RegisterTexture("default", "Textures/circle.png", ts);
    engineContext.renderManager->RegisterShader("s_instancing", { {ShaderStage::Vertex,"Shaders/Instancing.vert"},{ShaderStage::Fragment,"Shaders/Instancing.frag"} });
    engineContext.renderManager->RegisterMaterial("m_instancing", "s_instancing", { std::pair<std::string, std::string>("u_Texture","default") });

    screen_width = 1200;
    screen_height = 800;
    engineContext.windowManager->Resize(screen_width, screen_height);

    screen_scale.x = screen_width / sim.Width();
    screen_scale.y = screen_height / sim.Height();

    position_offset.x = -screen_width * 0.5f;
    position_offset.y = -screen_height * 0.5f;
}

void FluidSimulation::Init(const EngineContext& engineContext)
{
    SNAKE_LOG("[FluidSimulation] init called");

    objs.reserve(sim.Size());

    for (unsigned int i = 0; i < sim.Size(); i++)
    {
        FluidParticle* obj = new FluidParticle;
        objectManager.AddObject(std::unique_ptr<GameObject>(obj));
        objs.push_back(obj);
    }
}

void FluidSimulation::LateInit(const EngineContext& engineContext)
{
}

void FluidSimulation::Update(float dt, const EngineContext& engineContext)
{
    if (engineContext.inputManager->IsMouseButtonReleased(MOUSE_BUTTON_LEFT))
    {
        sim.Pause(!sim.Paused());
    }

    sim.Update(FluidTimestep);
 
    for (unsigned int i = 0; i < sim.Size(); i++)
    {
        Particle* p = sim.particle_at(i);
        Vector2 pos = { p->pos.x* screen_scale.x, p->pos.y* screen_scale.y };
        pos = pos + position_offset;

        unsigned int color = (unsigned int)(p->density * coeff);
        color = Clamp(color, 0, 255);

        objs[i]->GetTransform2D().SetPosition({ pos.x, -pos.y });
        objs[i]->color.r = float(255 - color) / 255.f;
        objs[i]->color.g = objs[i]->color.r;
        objs[i]->color.a = 1.0f;
    }
}

void FluidSimulation::LateUpdate(float dt, const EngineContext& engineContext)
{
    GameState::LateUpdate(dt, engineContext);
}

void FluidSimulation::Draw(const EngineContext& engineContext)
{
    auto& rm = *engineContext.renderManager;

    objectManager.DrawAll(engineContext, cameraManager.GetActiveCamera());
}


void FluidSimulation::Free(const EngineContext& engineContext)
{
    SNAKE_LOG("[FluidSimulation] free called");
}

void FluidSimulation::Unload(const EngineContext& engineContext)
{
    SNAKE_LOG("[FluidSimulation] unload called");
}
