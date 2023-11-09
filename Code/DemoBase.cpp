#include "DemoBase.h"
#include "imgui/imgui.h"
#include "FixedFunctionPrimitives.h"

DemoBase::DemoBase() {
	displayHelp = false;
	isMouseLeftPressed = false;
	isMouseRightPressed = false;
	isMouseMiddlePressed = false;
	mouseMovement = vec2(0.0f, 0.0f);
	mousePosition = vec2(0.0f, 0.0f);
	screenSize = vec2(0.0f, 0.0f);
	configureHelpWindow = true;
}

void DemoBase::InitializeDemo(int width, int height) {
	ResizeDemo(width, height);
}

void DemoBase::ResizeDemo(int width, int height) {
	camera.SetPerspective(60.0f, (float)width / (float)height, 0.01f, 1000.0f);
	screenSize = vec2(width, height);
}

void DemoBase::UpdateDemo(float deltaTime) {
	if (isMouseRightPressed) {
		camera.RotateView(mouseMovement, deltaTime);
	}
	else if (isMouseMiddlePressed) {
		camera.Zoom(mouseMovement.y, deltaTime);
	}
	else if (isMouseLeftPressed) {
		camera.Pan(mouseMovement, deltaTime);
	}

	// Update the camera position
	camera.UpdateCamera(deltaTime);
}

void DemoBase::ImGUIUpdate() {
	if (displayHelp) {
		if (configureHelpWindow) {
			configureHelpWindow = false;

			ImGui::SetNextWindowPos(ImVec2(400, 90));
			ImGui::SetNextWindowSize(ImVec2(370, 100));
		}
		ImGui::Begin("How to Navigate", &displayHelp, ImGuiWindowFlags_NoResize);
		ImGui::Text("Navigation requires a 3-button mouse");
		ImGui::Text("Left mouse button: Pan");
		ImGui::Text("Middle mouse button: Zoom");
		ImGui::Text("Right mouse button: Rotate");
		ImGui::End();
	}
}

void DemoBase::RenderDemo() {
	FixedFunctionRenderOrigin();
}

void DemoBase::SetMouseInputState(bool leftMouse, bool middleMouse, bool rightMouse, const vec2& mouseDelta, const vec2& mousePosition) {
	isMouseLeftPressed = leftMouse;
	isMouseMiddlePressed = middleMouse;
	isMouseRightPressed = rightMouse;
	mouseMovement = mouseDelta;
	mousePosition = mousePosition;
}