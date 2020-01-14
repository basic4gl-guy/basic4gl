#include "GlfwMouse.h"

// Globals
static GlfwMouse* mouse = nullptr;

// Functions

static void MouseCursorPosCallback(GLFWwindow* window, double xpos, double ypos)
{
	if (mouse)
	{
		mouse->CursorPosCallback(xpos, ypos);
	}
}

void MouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
	if (mouse)
	{
		mouse->ScrollCallback(xoffset, yoffset);
	}
}

// GlfwMouse methods

bool GlfwMouse::SetCursorInputMode(int mode)
{
	if (window && cursorInputMode != mode)
	{
		glfwSetInputMode(window, GLFW_CURSOR, mode);
		cursorInputMode = mode;
		return true;
	}

	return false;
}

void GlfwMouse::PositionMode()
{	
	// For backwards compatibility with previous Basic4GL versions, 
	// once cursor has been hidden (by executing MouseXD() or MouseYD())
	// it remains hidden, even if using regular positions again.
	SetCursorInputMode(isCursorVisible ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_HIDDEN);
}

void GlfwMouse::OffsetMode()
{
	if (SetCursorInputMode(GLFW_CURSOR_DISABLED))
	{
		// Set previous position to current position, so that initial XD and YD return 0.
		prevX = x;
		prevY = y;
		isCursorVisible = false;
	}
}

GlfwMouse::GlfwMouse(GlfwWindowManager& windowManager)
	: window(nullptr), cursorInputMode(GLFW_CURSOR_NORMAL), x(0.0), y(0.0), prevX(0.0), prevY(0.0), prevScrollY(0.0), wheelDelta(0), isCursorVisible(true)
{
	// Set global mouse pointer for callback function
	mouse = this;

	// Get window handle when window has been created
	windowManager.SubscribeWindowCreated([this,&windowManager]()
	{
		// Store window
		window = windowManager.GetGlfwWindow();

		// Reset state
		cursorInputMode = GLFW_CURSOR_NORMAL;
		x = 0.0;
		y = 0.0;
		prevX = 0.0;
		prevY = 0.0;
		prevScrollY = 0.0;
		wheelDelta = 0;
		isCursorVisible = true;

		// Hide cursor if in full screen mode
		if (windowManager.GetActiveParams().isFullscreen)
		{
			SetCursorInputMode(GLFW_CURSOR_HIDDEN);
			isCursorVisible = false;
		}

		// Subscribe to scroll events
		glfwSetScrollCallback(window, MouseScrollCallback);
		glfwSetCursorPosCallback(window, MouseCursorPosCallback);
	});
	windowManager.SubscribeBeforeDestroyWindow([this]()
	{
		SetCursorInputMode(GLFW_CURSOR_NORMAL);			// Seems to be required to prevent cursor locking to window's region in some cases
		window = nullptr;
	});
}

GlfwMouse::~GlfwMouse()
{
	if (mouse == this) 
		mouse = nullptr;
}

float GlfwMouse::GetX()
{
	if (!window) return 0.0f;
	PositionMode();
	return static_cast<float>(x);
}

float GlfwMouse::GetY()
{
	if (!window) return 0.0f;
	PositionMode();
	return static_cast<float>(y);
}

float GlfwMouse::GetXD()
{
	if (!window) return 0.0f;
	glfwPollEvents();									// Because we're using the Qt event loop, glfwPollEvents is not called normally. However it seems to be required for relative mouse input to function correctly.
	OffsetMode();
	double offsetX = x - prevX;
	prevX = x;
	return static_cast<float>(offsetX);
}

float GlfwMouse::GetYD()
{
	if (!window) return 0.0f;
	glfwPollEvents();									// Because we're using the Qt event loop, glfwPollEvents is not called normally. However it seems to be required for relative mouse input to function correctly.
	OffsetMode();
	double offsetY = y - prevY;
	prevY = y;
	return static_cast<float>(offsetY);
}

bool GlfwMouse::GetButton(int index)
{
	if (!window) return false;
	return index >= GLFW_MOUSE_BUTTON_1 && index <= GLFW_MOUSE_BUTTON_8 
		&& glfwGetMouseButton(window, index) == GLFW_PRESS;
}

int GlfwMouse::GetWheelDelta()
{
	if (!window) return 0;
	int result = wheelDelta;
	wheelDelta = 0;
	return result;
}

void GlfwMouse::CursorPosCallback(double xpos, double ypos)
{
	x = xpos;
	y = ypos;
}

void GlfwMouse::ScrollCallback(double xoffset, double yoffset)
{
	double scrollY = prevScrollY + yoffset;
	wheelDelta += floor(scrollY) - floor(prevScrollY);
	prevScrollY = scrollY;
}
