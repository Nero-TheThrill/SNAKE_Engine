#pragma once
#include"GameState.h"
#include "Fluid.h"
#include "FluidParticle.h"
#include <vector>

class FluidSimulation :public GameState
{
public:
    void Load(const EngineContext& engineContext) override;

    void Init(const EngineContext& engineContext) override;

    void LateInit(const EngineContext& engineContext) override;

    void Update(float dt, const EngineContext& engineContext) override;

    void LateUpdate(float dt, const EngineContext& engineContext) override;

    void Draw(const EngineContext& engineContext) override;

    void Free(const EngineContext& engineContext) override;

    void Unload(const EngineContext& engineContext) override;

private:
    Fluid sim;
    FluidParticle particle_proxy;
    float screen_width, screen_height;
    Vector2 screen_scale;
    Vector2 position_offset;
    Vector2 debug_text_init_pos;
    const float debug_text_pos_padding = 5;
    const double coeff = 128.0 / FluidRestDensity;
    std::vector<FluidParticle*> objs;
};

