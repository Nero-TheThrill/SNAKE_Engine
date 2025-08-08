#include "Player.h"
#include <random>

#include "Bullet.h"
#include "Bullet1.h"
#include "Button.h"
#include "Debug.h"
#include "Engine.h"
#include"Enemy.h"

void Player::Init(const EngineContext& engineContext)
{
    transform2D.SetPosition(glm::vec2(0, 0));
    transform2D.SetScale(glm::vec2(50.f));

    SetMesh(engineContext.renderManager->GetMeshByTag("default"));
    SetMaterial(engineContext, "m_animation");
    SpriteSheet* sheet = engineContext.renderManager->GetSpriteSheetByTag("animTest");
    sheet->AddClip("sidewalk", { 0,1,2,3,4,5,6,7,8 }, 0.08f, true);
    sheet->AddClip("frontwalk", { 86,87,88,89,90,91 }, 0.08f, true);
    sheet->AddClip("backwalk", { 80,81,82,83,84,85 }, 0.08f, true);
    sheet->AddClip("idle", { 9 }, 0.08f, false);
    AttachAnimator(sheet, 0.08f);
    spriteAnimator->PlayClip("idle");

    auto collider = std::make_unique<AABBCollider>(this, glm::vec2(1.0, 1.0));
    collider->SetUseTransformScale(true);
    SetCollider(std::move(collider));
    SetCollision(engineContext.stateManager->GetCurrentState()->GetObjectManager(), "player", { "bullet","enemy", "button" });
}

void Player::LateInit(const EngineContext& engineContext)
{

}

void Player::Update(float dt, const EngineContext& engineContext)
{
    checkIdle = true;
    if (engineContext.inputManager->IsKeyDown(KEY_W))
    {
        checkIdle = false;
        transform2D.AddPosition(glm::vec2(0, 150 * dt));
    }
    if (engineContext.inputManager->IsKeyDown(KEY_A))
    {
        checkIdle = false;
        transform2D.AddPosition(glm::vec2(-150 * dt, 0));
    }
    if (engineContext.inputManager->IsKeyDown(KEY_S))
    {
        checkIdle = false;
        transform2D.AddPosition(glm::vec2(0, -150 * dt));
    }
    if (engineContext.inputManager->IsKeyDown(KEY_D))
    {
        checkIdle = false;
        transform2D.AddPosition(glm::vec2(150 * dt, 0));
    }

    if (engineContext.inputManager->IsKeyPressed(KEY_W))
    {
        spriteAnimator->PlayClip("backwalk");
    }
    if (engineContext.inputManager->IsKeyPressed(KEY_A))
    {
        SetFlipUV_X(true);
        spriteAnimator->PlayClip("sidewalk");
    }
    if (engineContext.inputManager->IsKeyPressed(KEY_S))
    {
        spriteAnimator->PlayClip("frontwalk");
    }
    if (engineContext.inputManager->IsKeyPressed(KEY_D))
    {
        SetFlipUV_X(false);
        spriteAnimator->PlayClip("sidewalk");
    }

    if (checkIdle)
    {
        spriteAnimator->PlayClip("idle");
    }
    if (engineContext.inputManager->IsKeyPressed(KEY_SPACE))
    {
        SNAKE_LOG("player shot the bullet");

        static std::random_device rd;
        static std::mt19937 gen(rd());
        static std::uniform_real_distribution<float> angleDist(0.0f, 2.0f * glm::pi<float>());

        float angle = angleDist(gen);
        std::unique_ptr<Bullet> b = std::make_unique<Bullet>(GetWorldPosition(), glm::vec2(std::cos(angle), std::sin(angle)));
        engineContext.stateManager->GetCurrentState()->GetObjectManager().AddObject(std::move(b), "bullet");
    }


    if (engineContext.inputManager->IsKeyDown(KEY_Z))
    {
        SNAKE_LOG("player shot the bullet");

        static std::random_device rd;
        static std::mt19937 gen(rd());
        static std::uniform_real_distribution<float> angleDist(0.0f, 2.0f * glm::pi<float>());

        for (int i = 0; i < 100; i++)
        {
            float angle = angleDist(gen);
            std::unique_ptr<Bullet1> b = std::make_unique<Bullet1>(transform2D.GetPosition(), glm::vec2(std::cos(angle), std::sin(angle)));
            engineContext.stateManager->GetCurrentState()->GetObjectManager().AddObject(std::move(b), "111");
        }
    }
}

void Player::Draw(const EngineContext& engineContext)
{
    //GetMaterial()->SetUniform("u_Color", glm::vec4(1.0, 1.0, 1.0, 1.0));
}

void Player::Free(const EngineContext& engineContext)
{
    SNAKE_LOG("Player Free Called");
}

void Player::LateFree(const EngineContext& engineContext)
{
    SNAKE_LOG("Player LateFree Called");
}

void Player::OnCollision(Object* other)
{
    if (other->GetTag() == "enemyBullet")
    {
        other->Kill();
    }
    if (other->GetTag() == "enemy" && this < other)
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
            if (!static_cast<Enemy*>(other)->CheckIdle())
                other->GetTransform2D().AddPosition(-correction);
            if (!checkIdle)
                GetTransform2D().AddPosition(correction);
				
        }
    }
    if (other->GetTag() == "StartButton" || other->GetTag() == "QuitButton")
    {
        other->SetColor({ 0.3,0.3,0.3,1.0 });
    }
}

bool Player::CheckIdle()
{
    return checkIdle;
}
