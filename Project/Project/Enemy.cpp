#include "Enemy.h"

#include <random>

#include "Bullet.h"
#include "Debug.h"
#include "Engine.h"
#include "Player.h"

Enemy::Enemy(glm::vec2 pos)
{
    transform2D.SetPosition(pos);
    transform2D.SetScale(glm::vec2(50.f));
}

void Enemy::Init(const EngineContext& engineContext)
{
    SetMesh(engineContext, "default");
    SetMaterial(engineContext, "m_animation");
    SpriteSheet* sheet = engineContext.renderManager->GetSpriteSheetByTag("animTest");
    sheet->AddClip("sidewalk", { 0,1,2,3,4,5,6,7,8 }, 0.08f, true);
    sheet->AddClip("frontwalk", { 86,87,88,89,90,91 }, 0.08f, true);
    sheet->AddClip("backwalk", { 80,81,82,83,84,85 }, 0.08f, true);
    sheet->AddClip("idle", { 9 }, 0.08f, false);
    AttachAnimator(sheet, 0.08f);
    spriteAnimator->PlayClip("idle");

    SetRenderLayer("Penguin");
    SetColor({ 0.6,0.2,0.2,1 });


    auto collider = std::make_unique<AABBCollider>(this, glm::vec2(1.0, 1.0));
    collider->SetUseTransformScale(true);
    SetCollider(std::move(collider));
    SetCollision(engineContext.stateManager->GetCurrentState()->GetObjectManager(), "enemy", { "player" });
}

void Enemy::LateInit(const EngineContext& engineContext)
{

}

void Enemy::Update(float dt, const EngineContext& engineContext)
{
    checkIdle = true;
    if (engineContext.inputManager->IsKeyDown(KEY_UP))
    {
        checkIdle = false;
        transform2D.AddPosition(glm::vec2(0, 150 * dt));
    }
    if (engineContext.inputManager->IsKeyDown(KEY_LEFT))
    {
        checkIdle = false;
        transform2D.AddPosition(glm::vec2(-150 * dt, 0));
    }
    if (engineContext.inputManager->IsKeyDown(KEY_DOWN))
    {
        checkIdle = false;
        transform2D.AddPosition(glm::vec2(0, -150 * dt));
    }
    if (engineContext.inputManager->IsKeyDown(KEY_RIGHT))
    {
        checkIdle = false;
        transform2D.AddPosition(glm::vec2(150 * dt, 0));
    }

    if (engineContext.inputManager->IsKeyPressed(KEY_UP))
    {
        spriteAnimator->PlayClip("backwalk");
    }
    if (engineContext.inputManager->IsKeyPressed(KEY_LEFT))
    {
        SetFlipUV_X(true);
        spriteAnimator->PlayClip("sidewalk");
    }
    if (engineContext.inputManager->IsKeyPressed(KEY_DOWN))
    {
        spriteAnimator->PlayClip("frontwalk");
    }
    if (engineContext.inputManager->IsKeyPressed(KEY_RIGHT))
    {
        SetFlipUV_X(false);
        spriteAnimator->PlayClip("sidewalk");
    }

    if (checkIdle)
    {
        spriteAnimator->PlayClip("idle");
    }

    if (engineContext.inputManager->IsKeyDown(KEY_RIGHT_CONTROL))
    {
        SNAKE_LOG("enemy shot the bullet");

        static std::random_device rd;
        static std::mt19937 gen(rd());
        static std::uniform_real_distribution<float> angleDist(0.0f, 2.0f * glm::pi<float>());

        for (int i = 0; i < 10; i++)
        {
            float angle = angleDist(gen);
            std::unique_ptr<Bullet> b = std::make_unique<Bullet>(transform2D.GetPosition(), glm::vec2(std::cos(angle), std::sin(angle)));
            engineContext.stateManager->GetCurrentState()->GetObjectManager().AddObject(std::move(b), "enemyBullet");
        }
    }
}

void Enemy::Draw(const EngineContext& engineContext)
{
    /*GetMaterial()->SetUniform("u_Color", glm::vec4(1.0, 0.0, 1.0, 1.0));
    GetMaterial()->SetUniform("u_Model", transform2D.GetMatrix());*/
}

void Enemy::Free(const EngineContext& engineContext)
{
    SNAKE_LOG("Player Free Called");
}

void Enemy::LateFree(const EngineContext& engineContext)
{
    SNAKE_LOG("Player LateFree Called");
}

void Enemy::OnCollision(Object* other)
{
    if (other->GetTag() == "player" && this < other)
    {
        glm::vec2 halfSize = GetWorldScale() / glm::vec2(2);
        glm::vec2 otherHalfSize = other->GetWorldScale() / glm::vec2(2);

        glm::vec2 center = GetWorldPosition() + halfSize;
        glm::vec2 otherCenter = other->GetWorldPosition() + otherHalfSize;

        glm::vec2 delta = center - otherCenter;
        glm::vec2 overlap = halfSize + otherHalfSize - glm::abs(delta);

        if (overlap.x > 0 && overlap.y > 0)
        {
            glm::vec2 correction = { 0, 0 };

            if (overlap.x < overlap.y)
            {
                correction.x = (delta.x > 0 ? overlap.x : -overlap.x) * 0.5f;
            }
            else
            {
                correction.y = (delta.y > 0 ? overlap.y : -overlap.y) * 0.5f;
            }

            if (!checkIdle)
                GetTransform2D().AddPosition(correction);
            if (!static_cast<Player*>(other)->CheckIdle())
                other->GetTransform2D().AddPosition(-correction);
        }
    }
}

bool Enemy::CheckIdle()
{
    return checkIdle;
}
