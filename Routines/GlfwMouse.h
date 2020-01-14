/*	Created 17-Apr-2016: Tom Mulgrew (tmulgrew@slingshot.co.nz)

	GLFW mouse implementation
*/

#pragma once
#include "OpenGLInput.h"
#include "GlfwWindowManager.h"

class GlfwMouse : public OpenGLMouse
{
	GLFWwindow* window;
	int cursorInputMode;

	double x, y;
	double prevX, prevY;
	double prevScrollY;
	int wheelDelta;
	bool isCursorVisible;

	bool SetCursorInputMode(int mode);
	void PositionMode();
	void OffsetMode();

public:
	explicit GlfwMouse(GlfwWindowManager& windowManager);
	virtual ~GlfwMouse();

	float GetX() override;
	float GetY() override;
	float GetXD() override;
	float GetYD() override;
	bool GetButton(int index) override;
	int GetWheelDelta() override;

	// Mouse scroll callback function
	void CursorPosCallback(double xpos, double ypos);
	void ScrollCallback(double xoffset, double yoffset);
};