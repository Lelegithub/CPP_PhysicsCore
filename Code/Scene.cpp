#include "Scene.h"
#include <algorithm>
#include <list>

void Scene::AddModelToScene(Model* newModel) {
    if (std::find(sceneObjects.begin(), sceneObjects.end(), newModel) != sceneObjects.end()) {
        // Avoid adding duplicates
        return;
    }
    sceneObjects.push_back(newModel);
}

void Scene::RemoveModelFromScene(Model* modelToRemove) {
    sceneObjects.erase(std::remove(sceneObjects.begin(), sceneObjects.end(), modelToRemove), sceneObjects.end());
}

void Scene::UpdateModelInScene(Model* modelToUpdate) {
    // Placeholder for updating models in the scene
}

std::vector<Model*> Scene::FindChildrenOfModel(const Model* parentModel) {
    std::vector<Model*> children;

    for (int i = 0, size = sceneObjects.size(); i < size; ++i) {
        if (sceneObjects[i] == 0 || sceneObjects[i] == parentModel) {
            continue;
        }

        Model* iterator = sceneObjects[i]->parent;
        if (iterator != 0) {
            if (iterator == parentModel) {
                children.push_back(sceneObjects[i]);
                continue;
            }
            iterator = iterator->parent;
        }
    }

    return children;
}

Model* Scene::PerformRaycast(const Ray& ray) {
    if (sceneOctree != 0) {
        return ::PerformRaycast(sceneOctree, ray);
    }

    Model* closestModel = 0;
    float closestDistance = -1;

    for (int i = 0, size = sceneObjects.size(); i < size; ++i) {
        float distance = CalculateRayModelIntersection(*sceneObjects[i], ray);
        if (closestModel == 0 && distance >= 0) {
            closestModel = sceneObjects[i];
            closestDistance = distance;
        } else if (closestModel != 0 && distance < closestDistance) {
            closestModel = sceneObjects[i];
            closestDistance = distance;
        }
    }

    return closestModel;
}

std::vector<Model*> Scene::QueryModels(const Sphere& sphere) {
    if (sceneOctree != 0) {
        return ::QueryModels(sceneOctree, sphere);
    }

    std::vector<Model*> result;
    for (int i = 0, size = sceneObjects.size(); i < size; ++i) {
        OBB bounds = GetObjectOBB(*sceneObjects[i]);
        if (CheckSphereOBBIntersection(sphere, bounds)) {
            result.push_back(sceneObjects[i]);
        }
    }
    return result;
}

std::vector<Model*> Scene::QueryModels(const AABB& aabb) {
    if (sceneOctree != 0) {
        return ::QueryModels(sceneOctree, aabb);
    }

    std::vector<Model*> result;
    for (int i = 0, size = sceneObjects.size(); i < size; ++i) {
        OBB bounds = GetObjectOBB(*sceneObjects[i]);
        if (CheckAABBOBBIntersection(aabb, bounds)) {
            result.push_back(sceneObjects[i]);
        }
    }
    return result;
}

void SplitOctree(OctreeNode* node, int depth) {
    if (depth-- <= 0) {
        return;
    }

    if (node->children == 0) {
        node->children = new OctreeNode[8];

        vec3 center = node->bounds.position;
        vec3 halfExtent = node->bounds.size * 0.5f;

        node->children[0].bounds = AABB(center + vec3(-halfExtent.x, halfExtent.y, -halfExtent.z), halfExtent);
        node->children[1].bounds = AABB(center + vec3(halfExtent.x, halfExtent.y, -halfExtent.z), halfExtent);
        node->children[2].bounds = AABB(center + vec3(-halfExtent.x, halfExtent.y, halfExtent.z), halfExtent);
        node->children[3].bounds = AABB(center + vec3(halfExtent.x, halfExtent.y, halfExtent.z), halfExtent);
        node->children[4].bounds = AABB(center + vec3(-halfExtent.x, -halfExtent.y, -halfExtent.z), halfExtent);
        node->children[5].bounds = AABB(center + vec3(halfExtent.x, -halfExtent.y, -halfExtent.z), halfExtent);
        node->children[6].bounds = AABB(center + vec3(-halfExtent.x, -halfExtent.y, halfExtent.z), halfExtent);
        node->children[7].bounds = AABB(center + vec3(halfExtent.x, -halfExtent.y, halfExtent.z), halfExtent);
    }

    if (node->children != 0 && node->models.size() > 0) {
        for (int i = 0; i < 8; ++i) {
            for (int j = 0, size = node->models.size(); j < size; ++j) {
                OBB bounds = GetObjectOBB(*node->models[j]);
                if (CheckAABBOBBIntersection(node->children[i].bounds, bounds)) {
                    node->children[i].models.push_back(node->models[j]);
                }
            }
        }
        node->models.clear();

        for (int i = 0; i < 8; ++i) {
            SplitOctree(&(node->children[i]), depth);
        }
    }
}

void InsertIntoOctree(OctreeNode* node, Model* model) {
    OBB bounds = GetObjectOBB(*model);
    if (CheckAABBOBBIntersection(node->bounds, bounds)) {
        if (node->children == 0) {
            node->models.push_back(model);
        } else {
            for (int i = 0; i < 8; ++i) {
                InsertIntoOctree(&(node->children[i]), model);
            }
        }
    }
}

void RemoveFromOctree(OctreeNode* node, Model* model) {
    if (node->children == 0) {
        std::vector<Model*>::iterator it = std::find(node->models.begin(), node->models.end(), model);
        if (it != node->models.end()) {
            node->models.erase(it);
        }
    } else {
        for (int i = 0; i < 8; ++i) {
            RemoveFromOctree(&(node->children[i]), model);
        }
    }
}

void UpdateOctree(OctreeNode* node, Model* model) {
    RemoveFromOctree(node, model);
    InsertIntoOctree(node, model);
}

Model* FindClosestModel(const std::vector<Model*>& modelSet, const Ray& ray) {
    if (modelSet.size() == 0) {
        return 0;
    }

    Model* closestModel = 0;
    float closestT = -1;

    for (int i = 0, size = modelSet.size(); i < size; ++i) {
        float thisT = CalculateRayModelIntersection(*modelSet[i], ray);

        if (thisT < 0) {
