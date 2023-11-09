#include "DemoWindow.h"
#include "imgui/imgui.h"
#include "RaycastDemo.h"
#include "CollisionFeature.h"
#include "LinearImpulse.h"
#include "ConservationOfMomentum.h"
#include "SimpleSprings.h"

#include <cstdlib>

// Global instance of the IWindow singleton.
static DemoWindow g_AppWindow("Physics Playground", 800, 600);

void DemoWindow::Initialize() {
    GLWindow::Initialize();

    m_previousMousePosition = vec2(0, 0);
    m_selectedDemoIndex = -1;
    m_currentDemo = nullptr;
    initializedImGui = true;
    selectAllDemos = false;
}

DemoWindow::~DemoWindow() {
    TerminateDemo();
}

void DemoWindow::Resize(int width, int height) {
    GLWindow::Resize(width, height);

    if (m_currentDemo != nullptr) {
        m_currentDemo->ResizeDemo(width, height);
    }
    ApplyDemoCameraSettings();
}

void DemoWindow::Render() {
    GLWindow::Render();

    if (m_currentDemo != nullptr) {
        mat4 viewMatrix = m_currentDemo->camera.GetViewMatrix();
        SetOpenGLModelViewMatrix(viewMatrix.asArray);

        m_currentDemo->RenderDemo();
    }
}

void DemoWindow::ApplyDemoCameraSettings() {
    if (m_currentDemo == nullptr) {
        return;
    }

    mat4 projectionMatrix = m_currentDemo->camera.GetProjectionMatrix();
    mat4 viewMatrix = m_currentDemo->camera.GetViewMatrix();

    SetOpenGLProjectionMatrix(projectionMatrix.asArray);
    SetOpenGLModelViewMatrix(viewMatrix.asArray);
}

void DemoWindow::Update(float deltaTime) {
    GLWindow::Update(deltaTime);

    if (initializedImGui) {
        ImGui::SetNextWindowPos(ImVec2(10, 10));
        initializedImGui = false;
    }

    ImGui::SetNextWindowSize(ImVec2(370, (selectAllDemos) ? 145 : 75));
    ImGui::Begin("Physics Demos", 0, ImGuiWindowFlags_NoResize);
    ImGui::Text("Average frame time: %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

    if (selectAllDemos) {
        const char* demoItems[] = {
            "Raycast Demo", "Collision Features", "Linear Impulse", "Conservation Of Momentum", "Simple Springs",
        };

        int previousDemoIndex = m_selectedDemoIndex;
        ImGui::PushItemWidth(350);
        ImGui::ListBox("", &m_selectedDemoIndex, demoItems, 9, 5);

        if (m_selectedDemoIndex != previousDemoIndex) {
            TerminateDemo();

            switch (m_selectedDemoIndex) {
                case 0: m_currentDemo = new RaycastDemo(); break;
                case 1: m_currentDemo = new CollisionFeature(); break;
                case 2: m_currentDemo = new LinearImpulse(); break;
                case 3: m_currentDemo = new ConservationOfMomentum(); break;
                case 4: m_currentDemo = new SimpleSprings(); break;
            }

            m_currentDemo->InitializeDemo(GetWindowWidth(), GetWindowHeight());
            ApplyDemoCameraSettings();
        }
    }

    ImGui::End();

    if (m_currentDemo != nullptr) {
        m_currentDemo->ImGuiUpdate();
    }
}

void DemoWindow::FixedUpdate(float deltaTime) {
    GLWindow::FixedUpdate(deltaTime);

    bool isLeftMouseButtonPressed = MouseButtonPressed(MOUSE_LEFT);
    bool isMiddleMouseButtonPressed = MouseButtonPressed(MOUSE_MIDDLE);
    bool isRightMouseButtonPressed = MouseButtonPressed(MOUSE_RIGHT);

    vec2 currentMousePosition = GetMousePosition();
    vec2 mousePositionDelta = currentMousePosition - m_previousMousePosition;
    mousePositionDelta.x /= (float)GetWindowWidth();
    mousePositionDelta.y /= (float)GetWindowHeight();

    if (m_currentDemo != nullptr) {
        m_currentDemo->SetMouseState(
            isLeftMouseButtonPressed, isMiddleMouseButtonPressed, isRightMouseButtonPressed,
            mousePositionDelta, currentMousePosition
        );
        m_currentDemo->UpdateDemo(deltaTime);
    }

    m_previousMousePosition = currentMousePosition;
}

int main() {
    return g_AppWindow.Run();
}