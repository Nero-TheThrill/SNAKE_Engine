#include "ObjectManager.h"
#include "EngineContext.h"
#include <cassert>

#include <iostream>
#include "Debug.h"
#include "InstanceBatchKey.h"

void ObjectManager::AddObject(std::unique_ptr<GameObject> obj, const std::string& tag)
{
    assert(obj != nullptr && "Cannot add null object");

    obj->SetTag(tag);

    if (!tag.empty())
    {
        if (objectMap.find(tag) != objectMap.end())
            SNAKE_WRN("Duplicate GameObject ID");

        GameObject* rawPointer = obj.get();
        objectMap[tag] = rawPointer;
    }
    allRawPtrs.push_back(obj.get());
    pendingObjects.push_back(std::move(obj));
}



void ObjectManager::InitAll(const EngineContext& engineContext)
{
    for (const auto& obj : objects)
    {
        obj->Init(engineContext);
    }
    for (const auto& obj : objects)
    {
        obj->LateInit(engineContext);
    }
}


void ObjectManager::UpdateAll(float dt, const EngineContext& engineContext)
{
    for (const auto& obj : objects)
    {
        if (obj->IsAlive())
            obj->Update(dt, engineContext);
    }

    EraseDeadObjects(engineContext);
    AddAllPendingObjects(engineContext);
}

void ObjectManager::AddAllPendingObjects(const EngineContext& engineContext)
{
    std::vector<std::unique_ptr<GameObject>> tmp;
    std::swap(tmp, pendingObjects);
    for (auto& obj : tmp)
    {
        obj->Init(engineContext);
    }
    for (auto& obj : tmp)
    {
        obj->LateInit(engineContext);
        objects.push_back(std::move(obj));
    }
}

void ObjectManager::EraseDeadObjects(const EngineContext& engineContext)
{
    std::vector<GameObject*> deadObjects;
    for (const auto& obj : objects)
    {
        if (!obj->IsAlive())
        {
            deadObjects.push_back(obj.get());
        }
    }
    for (auto& obj : deadObjects)
    {
        obj->Free(engineContext);
    }
    for (auto& obj : deadObjects)
    {
        obj->LateFree(engineContext);
        objectMap.erase(obj->GetTag());
        allRawPtrs.erase(std::remove(allRawPtrs.begin(), allRawPtrs.end(), obj), allRawPtrs.end());
    }

    objects.erase(
        std::remove_if(objects.begin(), objects.end(),
            [](const std::unique_ptr<GameObject>& obj)
            {
                return !obj->IsAlive();
            }),
        objects.end());
}

void ObjectManager::DrawAll(const EngineContext& engineContext, Camera2D* camera)
{
    //std::cout << objects.size() << std::endl;
    std::vector<GameObject*> visibleObjects;
    FrustumCuller::CullVisible(*camera, allRawPtrs, visibleObjects,glm::vec2(engineContext.windowManager->GetWidth(), engineContext.windowManager->GetHeight()));
    using ShaderMap = std::map<Shader*, std::map<InstanceBatchKey, std::vector<GameObject*>>>;
    std::map<int, ShaderMap> renderMap = BuildRenderMap(visibleObjects);
    SubmitRenderMap(engineContext, camera, renderMap);

}

void ObjectManager::DrawObjects(const EngineContext& engineContext, Camera2D* camera, const std::vector<GameObject*>& gameObjects)
{
    std::vector<GameObject*> visibleObjects;
    FrustumCuller::CullVisible(*camera, gameObjects, visibleObjects, glm::vec2(engineContext.windowManager->GetWidth(), engineContext.windowManager->GetHeight()));
    std::map<int, ShaderMap> renderMap = BuildRenderMap(visibleObjects);
    SubmitRenderMap(engineContext, camera, renderMap);

}

void ObjectManager::DrawObjectsWithTag(const EngineContext& engineContext, Camera2D* camera, const std::string& tag)
{
    std::vector<GameObject*> filteredObjects;
    for (GameObject* obj : allRawPtrs)
    {
        if (obj && obj->IsAlive() && obj->GetTag() == tag)
        {
            filteredObjects.push_back(obj);
        }
    }
    std::vector<GameObject*> visibleObjects;
    FrustumCuller::CullVisible(*camera, filteredObjects, visibleObjects, glm::vec2(engineContext.windowManager->GetWidth(), engineContext.windowManager->GetHeight()));
    std::map<int, ShaderMap> renderMap = BuildRenderMap(visibleObjects);
    SubmitRenderMap(engineContext, camera, renderMap);
}

void ObjectManager::FreeAll(const EngineContext& engineContext)
{
    for (const auto& obj : objects)
    {
        obj->Free(engineContext);
    }
    for (const auto& obj : objects)
    {
        obj->LateFree(engineContext);
    }

    objects.clear();
    objectMap.clear();
}

GameObject* ObjectManager::FindByID(const std::string& id) const
{
    auto it = objectMap.find(id);
    if (it != objectMap.end())
        return it->second;
    return nullptr;
}

RenderMap ObjectManager::BuildRenderMap(const std::vector<GameObject*>& source)
{
    std::map<int, ShaderMap> renderMap;

    for (auto* obj : source)
    {
        if (!obj || !obj->IsVisible())
            continue;

        Material* material = obj->GetMaterial();
        Mesh* mesh = obj->GetMesh();
        Shader* shader = material ? material->GetShader() : nullptr;

        if (!material || !mesh || !shader)
            continue;

        InstanceBatchKey key{ mesh, material };
        renderMap[obj->GetRenderLayer()][shader][key].push_back(obj);
    }

    return renderMap;
}

void ObjectManager::SubmitRenderMap(const EngineContext& engineContext, Camera2D* camera,
	const RenderMap& renderMap)
{
    Material* lastMaterial = nullptr;
    Shader* lastShader = nullptr;

    for (auto& [layer, shaderMap] : renderMap)
    {
        for (auto& [shader, batchMap] : shaderMap)
        {
            for (auto& [key, batch] : batchMap)
            {
                if (batch.front()->CanBeInstanced())
                {
                    engineContext.renderManager->Submit([=]() mutable {
                        std::vector<glm::mat4> transforms;
                        std::vector<glm::vec4> colors;
                        transforms.reserve(batch.size());
                        colors.reserve(batch.size());
                        for (auto* obj : batch)
                            transforms.push_back(obj->GetTransform2DMatrix());

                        for (auto* obj : batch)
                            colors.push_back(obj->GetColor());

                        Material* material = key.material;
                        Shader* currentShader = material->GetShader();

                        if (material != lastMaterial)
                        {
                            material->Bind();
                            lastMaterial = material;
                        }

                        if (currentShader != lastShader)
                        {
                            material->SetUniform("u_Projection", camera->GetProjectionMatrix());
                            lastShader = currentShader;
                        }

                        batch.front()->Draw(engineContext);
                        material->SendUniforms();

                        key.mesh->BindVAO();
                        material->UpdateInstanceBuffer(transforms);
                        material->UpdateInstanceBuffer(colors);
                        key.mesh->DrawInstanced(static_cast<GLsizei>(transforms.size()));

                        material->UnBind();
                        });
                }
                else
                {
                    for (auto* obj : batch)
                    {
                        engineContext.renderManager->Submit([=]() mutable {
                            Material* mat = key.material;
                            Shader* currentShader = mat->GetShader();

                            if (mat != lastMaterial)
                            {
                                mat->Bind();
                                lastMaterial = mat;
                            }

                            if (currentShader != lastShader)
                            {
                                mat->SetUniform("u_Projection", camera->GetProjectionMatrix());
                                lastShader = currentShader;
                            }

                            obj->Draw(engineContext);
                            mat->SendUniforms();
                            key.mesh->Draw();
                            mat->UnBind();
                            });
                    }
                }
            }
        }
    }
}
