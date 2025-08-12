#pragma once

#include "GameState.h"
#include "Player.h"
#include "TextObject.h"
#include "Enemy.h"
#include "Timer.h"

class Level1 : public GameState
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
    void HandleStateInput(const EngineContext& engineContext);
    void HandleSoundInput(const EngineContext& engineContext);

    float multiplier = 1.5f;
   
    const int rows = 10;
    const int cols = 17;

    const int marginX = 45;
    const int marginY = 40;

    const int spacingX = 10;
    const int spacingY = 8;

    const int appleSizeX = 32;
    const int appleSizeY = 35;

    TextObject* scoreUIText = nullptr;
    TextObject* restartUIText = nullptr;
    Timer gameTimer;
    Timer dokidoki;
    Object* timerBarFill = nullptr;
    float fillInitialScaleY; 


};
