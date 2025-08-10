#pragma once
#include "Enemy.h"
#include"GameState.h"
#include "Player.h"

class TextObject;

class MainMenu :public GameState
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
    GameObject* startButton = nullptr, * quitButton = nullptr, *player;
    TextObject* startText = nullptr, * quitText = nullptr, * bulletCountText = nullptr;
    Camera2D* miniCam = nullptr;
};

