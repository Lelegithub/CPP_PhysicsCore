#ifndef _H_DEMO_BASE_REFACTORED_
#define _H_DEMO_BASE_REFACTORED_

#include "Camera.h"

class DemoBaseRefactored {
private: // Avoid copying
	DemoBaseRefactored(const DemoBaseRefactored&);
	DemoBaseRefactored& operator=(const DemoBaseRefactored&);
protected:
	bool displayHelpInfo;
	bool isLeftMouseButtonPressed;
	bool isRightMouseButtonPressed;
	bool isMiddleMouseButtonPressed;
	vec2 mouseMovementDelta;
	vec2 mouseCursorPosition;
	vec2 windowSize;
	bool configureHelpWindow;
public:
	OrbitCamera camera;

	DemoBaseRefactored();
	inline virtual ~DemoBaseRefactored() { }

	virtual void InitializeDemo(int screenWidth, int screenHeight);
	virtual void AdjustWindowSize(int screenWidth, int screenHeight);
	virtual void RenderScene();
	virtual void UpdateScene(float deltaTime);
	virtual void RenderImGUI();
	inline virtual void ShutDownDemo() { }

	void SetMouseButtonState(bool isLeftPressed, bool isMiddlePressed, bool isRightPressed, const vec2& mouseDelta, const vec2& mousePosition);
};

#endif
