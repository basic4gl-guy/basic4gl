#include "GlfwKeyboard.h"
#include <windows.h>				// For VK_ scan key codes

using namespace std;

///////////////////////////////////////////////////////////////////////////////////////////////////
// Global keyboard object (so callbacks can find it)

GlfwKeyboard* keyboard = nullptr;

///////////////////////////////////////////////////////////////////////////////////////////////////
// GLFW Callbacks

void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (keyboard != nullptr)
	{
		keyboard->HandleKeypress(window, key, scancode, action, mods);
	}
}

void CharCallback(GLFWwindow* window, unsigned int codepoint)
{
	if (keyboard != nullptr)
	{
		keyboard->HandleChar(codepoint);
	}

}

///////////////////////////////////////////////////////////////////////////////////////////////////
// GlfwKeyboard methods

GlfwKeyboard::GlfwKeyboard(GlfwWindowManager& windowManager) : OpenGLKeyboard(windowManager)
{
	// Register this as the global "keyboard" object (so callbacks can find us)
	keyboard = this;

	// Hookup keyboard event when window is created
	windowManager.SubscribeWindowCreated([&windowManager]()
	{
		glfwSetKeyCallback(windowManager.GetGlfwWindow(), KeyCallback);
		glfwSetCharCallback(windowManager.GetGlfwWindow(), CharCallback);
	});
}


GlfwKeyboard::~GlfwKeyboard()
{
	// Unregister this as the global "keyboard" object
	keyboard = nullptr;
}

int ScanKeyFromGlfwKey(int key, bool ignoreLeftRight)
{
	switch (key)
	{
	case GLFW_KEY_BACKSPACE: return VK_BACK;
	case GLFW_KEY_TAB: return VK_TAB;
	case GLFW_KEY_ENTER: return VK_RETURN;
	case GLFW_KEY_PAUSE: return VK_PAUSE;
	case GLFW_KEY_CAPS_LOCK: return VK_CAPITAL;
	case GLFW_KEY_ESCAPE: return VK_ESCAPE;
	case GLFW_KEY_SPACE: return VK_SPACE;
	case GLFW_KEY_PAGE_UP: return VK_PRIOR;
	case GLFW_KEY_PAGE_DOWN: return VK_NEXT;
	case GLFW_KEY_END: return VK_END;
	case GLFW_KEY_HOME: return VK_HOME;
	case GLFW_KEY_LEFT: return VK_LEFT;
	case GLFW_KEY_UP: return VK_UP;
	case GLFW_KEY_RIGHT: return VK_RIGHT;
	case GLFW_KEY_DOWN: return VK_DOWN;
	case GLFW_KEY_PRINT_SCREEN: return VK_PRINT;
	case GLFW_KEY_INSERT: return VK_INSERT;
	case GLFW_KEY_DELETE: return VK_DELETE;
	case GLFW_KEY_KP_0: return VK_NUMPAD0;
	case GLFW_KEY_KP_1: return VK_NUMPAD1;
	case GLFW_KEY_KP_2: return VK_NUMPAD2;
	case GLFW_KEY_KP_3: return VK_NUMPAD3;
	case GLFW_KEY_KP_4: return VK_NUMPAD4;
	case GLFW_KEY_KP_5: return VK_NUMPAD5;
	case GLFW_KEY_KP_6: return VK_NUMPAD6;
	case GLFW_KEY_KP_7: return VK_NUMPAD7;
	case GLFW_KEY_KP_8: return VK_NUMPAD8;
	case GLFW_KEY_KP_9: return VK_NUMPAD9;
	case GLFW_KEY_KP_MULTIPLY: return VK_MULTIPLY;
	case GLFW_KEY_KP_ADD: return VK_ADD;
	case GLFW_KEY_KP_SUBTRACT: return VK_SUBTRACT;
	case GLFW_KEY_KP_DECIMAL: return VK_DECIMAL;
	case GLFW_KEY_KP_DIVIDE: return VK_DIVIDE;
	case GLFW_KEY_F1: return VK_F1;
	case GLFW_KEY_F2: return VK_F2;
	case GLFW_KEY_F3: return VK_F3;
	case GLFW_KEY_F4: return VK_F4;
	case GLFW_KEY_F5: return VK_F5;
	case GLFW_KEY_F6: return VK_F6;
	case GLFW_KEY_F7: return VK_F7;
	case GLFW_KEY_F8: return VK_F8;
	case GLFW_KEY_F9: return VK_F9;
	case GLFW_KEY_F10: return VK_F10;
	case GLFW_KEY_F11: return VK_F11;
	case GLFW_KEY_F12: return VK_F12;
	case GLFW_KEY_LEFT_SHIFT: return ignoreLeftRight ? VK_SHIFT : VK_LSHIFT;
	case GLFW_KEY_RIGHT_SHIFT: return ignoreLeftRight ? VK_SHIFT : VK_RSHIFT;
	case GLFW_KEY_LEFT_CONTROL: return ignoreLeftRight ? VK_CONTROL : VK_LCONTROL;
	case GLFW_KEY_RIGHT_CONTROL: return ignoreLeftRight ? VK_CONTROL : VK_RCONTROL;
	default:
		// Return none if unrecognised
		return 0;
	}
}

unsigned char CharFromGlfwKey(int key)
{
	// Return key if in printable range.
	if (key >= GLFW_KEY_SPACE && key <= GLFW_KEY_GRAVE_ACCENT) return key;

	// Special case! Return key (13) is allowed as well, for backward compatibility with previous Basic4GL versions
	if (key == GLFW_KEY_ENTER || key == GLFW_KEY_KP_ENTER) return 13;

	// Don't treat any other scan keys as characters
	return 0;
}

void GlfwKeyboard::HandleKeypress(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	// Special case: ESCAPE key closes window
	if (key == GLFW_KEY_ESCAPE)
		glfwSetWindowShouldClose(window, 1);

	// Push Windows scancode to queue.
	int scankey = ScanKeyFromGlfwKey(key, false);
	int scanKeyForBuffer = ScanKeyFromGlfwKey(key, true);
	unsigned char c = CharFromGlfwKey(key);
	if (scankey == 0 && c == 0) return;				// Unknown scan key
	switch (action)
	{
	case GLFW_PRESS:
	case GLFW_REPEAT:
		ScanKeyPress(scanKeyForBuffer, c);

		// Special case! Return key is also added to buffer (for compatibility with older Basic4GLs)
		if (c == 13) KeyPress(c);

		break;

	case GLFW_RELEASE:
		ScanKeyRelease(scanKeyForBuffer, c);
		break;
	}
}

void GlfwKeyboard::HandleChar(unsigned codepoint)
{
	// Process ASCII characters only
	if (codepoint < 128)
	{
		KeyPress(codepoint);
	}
}
