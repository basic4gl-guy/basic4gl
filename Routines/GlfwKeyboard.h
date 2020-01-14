/*	Created 17-Apr-2016: Tom Mulgrew (tmulgrew@slingshot.co.nz)
	
	GLFW Keyboard implementation
*/

#pragma once
#include "OpenGLInput.h"
#include "GlfwWindowManager.h"

/// Adapts the GLFW keyboard events to the IKeyboard interface.
/// Keyboard behaviour is slightly different to Basic4GL:
/// * GetNextKey() will only ever return a printable key, unlike in Basic4GL, 
///   where it also includes ENTER(chr$(13)) and BACKSPACE (chr$(8)) characters.
/// * IsKeyDown() will always return false for non printable characters, 
///   unlike in Basic4GL in which KeyDown(c$) is equivalent to ScanKeyDown(asc(c$))
/// * IsScanKeyDown() and GetNextScanKey() only recognise scan codes that have a 
///   corresponding BASIC VK_xxx constant.
///	  In particular, there are no VK_A-VK_Z constants, so these scan keys are not
///   returned.
/// * IsScanKeyDown() correctly handles VK_LSHIFT,VK_RSHIFT,VK_LCONTROL,VK_RCONTROL
///   unlike Basic4GL which always returns false for these.
///	  (GetNextScanKey() only ever returns VK_SHIFT/VK_CONTROL when Shift/Control 
///   pressed however.)
class GlfwKeyboard : public OpenGLKeyboard
{

public:
	GlfwKeyboard(GlfwWindowManager& windowManager);
	virtual ~GlfwKeyboard();

	// Keyboard handlers
	void HandleKeypress(GLFWwindow* window, int key, int scancode, int action, int mods);
	void HandleChar(unsigned codepoint);
};
