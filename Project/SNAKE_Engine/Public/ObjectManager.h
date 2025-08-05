#pragma once

#include <vector>
#include <unordered_map>
#include <string>
#include <memory>

#include "RenderManager.h"

class GameState;
class Object;
struct EngineContext;
class Camera2D;

class ObjectManager
{
    friend GameState;
public:
    [[maybe_unused]]Object* AddObject(std::unique_ptr<Object> obj, const std::string& tag = "");
    void InitAll(const EngineContext& engineContext);
    void UpdateAll(float dt, const EngineContext& engineContext);
    void DrawAll(const EngineContext& engineContext, Camera2D* camera);
    void DrawObjects(const EngineContext& engineContext, Camera2D* camera, const std::vector<Object*>& objects);
    void DrawObjectsWithTag(const EngineContext& engineContext, Camera2D* camera, const std::string& tag);

    void FreeAll(const EngineContext& engineContext);

    [[nodiscard]] Object* FindByTag(const std::string& tag) const;
    void FindByTag(const std::string& tag, std::vector<Object*>& result);
    void CheckCollision();
    void DrawColliderDebug(RenderManager* rm, Camera2D* cam);

    [[nodiscard]] CollisionGroupRegistry& GetCollisionGroupRegistry() { return collisionGroupRegistry; }
private:
    void AddAllPendingObjects(const EngineContext& engineContext);
    void EraseDeadObjects(const EngineContext& engineContext);

    std::vector<std::unique_ptr<Object>> objects;
    std::vector<std::unique_ptr<Object>> pendingObjects;
    std::unordered_map<std::string, Object*> objectMap;
    std::vector<Object*> rawPtrObjects;
    SpatialHashGrid broadPhaseGrid;
    CollisionGroupRegistry collisionGroupRegistry;
};
