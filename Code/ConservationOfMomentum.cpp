#include "MomentumConservationSimulation.h"
#include "GraphicsLibrary.h"
#include "glad/glad.h"
#include "imgui/imgui.h"
#include "imgui/ImGuizmo.h"
#include <iostream>

void MomentumConservationSimulation::Initialize(int screenWidth, int screenHeight) {
    DemoBase::Initialize(screenWidth, screenHeight);

    physicsSystem.RenderRandomColors = true;
    physicsSystem.ImpulseResolutionIterations = 20;

    configureImguiWindow = true;

    glPointSize(5.0f);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);

    float lightPosition[] = { 0.5f, 1.0f, -1.5f, 0.0f };
    glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);

    camera.SetTarget(vec3(3.75622f, 2.98255f, 0.0f));
    camera.SetZoom(12.0f);
    camera.SetRotation(vec2(-67.9312f, 19.8f));

    ResetSimulation();
}

void MomentumConservationSimulation::ResetSimulation() {
    physicsSystem.ClearDynamicBodies();
    physicsSystem.ClearCollisionConstraints();

    objects.clear();
    objects.resize(5);

    for (int i = 0; i < 5; ++i) {
        objects[i].type = RIGID_BODY_TYPE_SPHERE;
        float position = (float)i * (objects[i].sphere.radius + 0.00001f) * 2.0f;
        if (i == 4) {
            position = (float)i * (objects[i].sphere.radius + 0.00001f) * 3.0f;
        }
        objects[i].position = vec3(position, 1.16, 0.0f);
        physicsSystem.AddDynamicBody(&objects[i]);
    }
    vec3 impulse = vec3(-objects[4].position.x, 0.0f, 0.0f);
    Normalize(impulse);
    objects[4].ApplyLinearImpulse(impulse * 20.0f);

    groundBox = RigidBodyVolume(RIGID_BODY_TYPE_BOX);
    groundBox.box.size = vec3(15.0f, 0.15f, 15.0f);
    groundBox.mass = 0.0f;

    physicsSystem.AddDynamicBody(&groundBox);
}

float MomentumConservationSimulation::GenerateRandomFloat(float minValue, float maxValue) {
    if (maxValue < minValue) {
        float temp = minValue;
        minValue = maxValue;
        maxValue = temp;
    }

    float random = ((float)rand()) / (float)RAND_MAX;

    float range = maxValue - minValue;
    return (random * range) + minValue;
}

vec3 MomentumConservationSimulation::GenerateRandomVec3(const vec3& minValue, const vec3& maxValue) {
    vec3 result;
    result.x = GenerateRandomFloat(minValue.x, maxValue.x);
    result.y = GenerateRandomFloat(minValue.y, maxValue.y);
    result.z = GenerateRandomFloat(minValue.z, maxValue.z);
    return result;
}

void MomentumConservationSimulation::ImGUI() {
    DemoBase::ImGUI();

    if (configureImguiWindow) {
        configureImguiWindow = false;
        ImGui::SetNextWindowPos(ImVec2(400, 10));
        ImGui::SetNextWindowSize(ImVec2(370, 100));
    }

    ImGui::Begin("Conservation Demo", 0, ImGuiWindowFlags_NoResize);

    if (ImGui::Button("Reset")) {
        ResetSimulation();
    }

    ImGui::End();
}

void MomentumConservationSimulation::Render() {
    DemoBase::Render();

    float lightPosition[] = { 0.0f, 1.0f, 0.0f, 0.0f };
    glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);

    physicsSystem.RenderScene();
}

void MomentumConservationSimulation::Update(float deltaTime) {
    DemoBase::Update(deltaTime);

    physicsSystem.UpdateSimulation(deltaTime);
}
