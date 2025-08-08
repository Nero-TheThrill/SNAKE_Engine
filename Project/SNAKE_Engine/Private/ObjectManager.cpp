#include "Engine.h"

#include <cassert>
#include <algorithm>
#include <unordered_set>

Object* ObjectManager::AddObject(std::unique_ptr<Object> obj, const std::string& tag)
{
    assert(obj != nullptr && "Cannot add null object");

    obj->SetTag(tag);

    if (!tag.empty())
    {
        if (objectMap.find(tag) != objectMap.end())
            SNAKE_WRN("Duplicate Object ID");

        Object* rawPointer = obj.get();
        objectMap[tag] = rawPointer;
    }

    Object* returnVal = obj.get();
    rawPtrObjects.push_back(obj.get());
    pendingObjects.push_back(std::move(obj));
    return returnVal;
}

void ObjectManager::InitAll(const EngineContext& engineContext)
{
    for (const auto& obj : objects)
        obj->Init(engineContext);

    for (const auto& obj : objects)
        obj->LateInit(engineContext);
}

void ObjectManager::UpdateAll(float dt, const EngineContext& engineContext)
{
    for (const auto& obj : objects)
    {
        if (obj->IsAlive())
        {
            if (obj->GetType()==ObjectType::TEXT)
            {
                static_cast<TextObject*>(obj.get())->CheckFontAtlasAndMeshUpdate();
            }
            obj->Update(dt, engineContext);
            if (obj->HasAnimation())
                obj->GetAnimator()->Update(dt);
            if (Collider* col = obj->GetCollider())
                col->SyncWithTransformScale();
        }
    }

    EraseDeadObjects(engineContext);
    AddAllPendingObjects(engineContext);
}

void ObjectManager::AddAllPendingObjects(const EngineContext& engineContext)
{
    std::vector<std::unique_ptr<Object>> tmp;
    std::swap(tmp, pendingObjects);

    for (auto& obj : tmp)
        obj->Init(engineContext);

    for (auto& obj : tmp)
    {
        obj->LateInit(engineContext);
        objects.push_back(std::move(obj));
    }
}

void ObjectManager::EraseDeadObjects(const EngineContext& engineContext)
{
    std::vector<Object*> deadObjects;
    for (const auto& obj : objects)
    {
        if (!obj->IsAlive())
            deadObjects.push_back(obj.get());
    }

    for (auto& obj : deadObjects)
        obj->Free(engineContext);

    for (auto& obj : deadObjects)
    {
        obj->LateFree(engineContext);
        objectMap.erase(obj->GetTag());
        rawPtrObjects.erase(std::remove(rawPtrObjects.begin(), rawPtrObjects.end(), obj), rawPtrObjects.end());
    }

    objects.erase(
        std::remove_if(objects.begin(), objects.end(),
            [&](const std::unique_ptr<Object>& obj)
            {
                return std::find(deadObjects.begin(), deadObjects.end(), obj.get()) != deadObjects.end();
            }),
        objects.end());
}

void ObjectManager::DrawAll(const EngineContext& engineContext)
{
    engineContext.renderManager->Submit(rawPtrObjects, engineContext);
}

void ObjectManager::DrawObjects(const EngineContext& engineContext, const std::vector<Object*>& objects)
{
    engineContext.renderManager->Submit(objects, engineContext);
}

void ObjectManager::DrawObjectsWithTag(const EngineContext& engineContext, const std::string& tag)
{
    std::vector<Object*> filteredObjects;
    FindByTag(tag, filteredObjects);
    engineContext.renderManager->Submit(filteredObjects, engineContext);
}

void ObjectManager::FreeAll(const EngineContext& engineContext)
{
    for (const auto& obj : objects)
        obj->Free(engineContext);

    for (const auto& obj : objects)
        obj->LateFree(engineContext);

    objects.clear();
    objectMap.clear();
    rawPtrObjects.clear();
}

Object* ObjectManager::FindByTag(const std::string& tag) const
{
    auto it = objectMap.find(tag);
    if (it != objectMap.end() && it->second->IsAlive())
        return it->second;
    return nullptr;
}

void ObjectManager::FindByTag(const std::string& tag, std::vector<Object*>& result)
{
    for (Object* obj : rawPtrObjects)
    {
        if (obj && obj->IsAlive() && obj->GetTag() == tag)
            result.push_back(obj);
    }
}
void ObjectManager::CheckCollision()
{
    std::unordered_set<uint64_t> checkedPairs;
    checkedPairs.reserve(2000);

    broadPhaseGrid.Clear();

    for (Object* obj : rawPtrObjects)
    {
        Collider* collider = obj->GetCollider();
        if (collider)
            broadPhaseGrid.Insert(obj);
    }

    broadPhaseGrid.ComputeCollisions([&](Object* a, Object* b)
        {
            if ((a->GetCollisionMask() & b->GetCollisionCategory()) == 0 ||
                (b->GetCollisionMask() & a->GetCollisionCategory()) == 0)
                return;

            if (a > b) std::swap(a, b);
            uint64_t key = std::hash<Object*>()(a) ^ (std::hash<Object*>()(b) << 1);

            if (checkedPairs.find(key) != checkedPairs.end())
                return;

            checkedPairs.insert(key);

            if (a->GetCollider()->CheckCollision(b->GetCollider()))
            {
                a->OnCollision(b);
                b->OnCollision(a);
            }
        });
}

void ObjectManager::DrawColliderDebug(RenderManager* rm, Camera2D* cam)
{
    for (auto* obj : rawPtrObjects)
    {
        if (!obj->IsAlive() || !obj->IsVisible()) continue;

        if (Collider* col = obj->GetCollider())
        {
            col->DrawDebug(rm, cam);
        }
    }
}