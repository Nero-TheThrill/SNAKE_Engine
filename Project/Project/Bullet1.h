#pragma once
#include "ObjectManager.h"
#include "Engine.h"

class Bullet1 : public GameObject
{
public:
    Bullet1(glm::vec2 _pos, glm::vec2 _dir);
    void Init(const EngineContext& engineContext) override;
    void LateInit(const EngineContext& engineContext) override;
    void Update(float dt, const EngineContext& engineContext) override;
    void Draw(const EngineContext& engineContext) override;
    void Free(const EngineContext& engineContext) override;
    void LateFree(const EngineContext& engineContext) override;
private:
    glm::vec2 dir;
    float timer = 0;
    float rotAmount = 0;
    float speed = 1;
};

