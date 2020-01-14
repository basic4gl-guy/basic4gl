#include "GlfwWindowManager.h"
#include <cstdlib>

// Native window handle required to set focus (in windowed mode)
#define GLFW_EXPOSE_NATIVE_WIN32
#define GLFW_EXPOSE_NATIVE_WGL
#include <glfw/glfw3native.h>

void GlfwWindowManager::InternalDestroyWindow()
{
	assert(window);
	glfwDestroyWindow(window);
	window = nullptr;
}

void GlfwWindowManager::InternalCreateWindow(OpenGLWindowParams params)
{
	assert(window == nullptr);

	// Some params must specified as "hints"
	// Note: Must specify them even if set to defaults, because user can change them multiple
	// times within a single session.
	glfwWindowHint(GLFW_RESIZABLE, params.isResizable ? 1 : 0);
	glfwWindowHint(GLFW_DECORATED, params.isBordered ? 1 : 0);
	glfwWindowHint(GLFW_STENCIL_BITS, params.isStencilBufferRequired ? 8 : 0);
	if (params.isFullscreen && params.bpp == 16)
	{
		glfwWindowHint(GLFW_RED_BITS, 5);
		glfwWindowHint(GLFW_GREEN_BITS, 6);
		glfwWindowHint(GLFW_BLUE_BITS, 5);
	}
	else if (params.isFullscreen && params.bpp == 32)
	{
		glfwWindowHint(GLFW_RED_BITS, 8);
		glfwWindowHint(GLFW_GREEN_BITS, 8);
		glfwWindowHint(GLFW_BLUE_BITS, 8);
	}
	else
	{
		glfwWindowHint(GLFW_RED_BITS, GLFW_DONT_CARE);
		glfwWindowHint(GLFW_GREEN_BITS, GLFW_DONT_CARE);
		glfwWindowHint(GLFW_BLUE_BITS, GLFW_DONT_CARE);
	}

	// Calculate actual dimensions
	int width = params.width;
	int height = params.height;
	if (width == 0 || height == 0)
	{
		// Use desktop resolution
		auto* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
		width = mode->width;
		height = mode->height;
	}

	// Create the window
	window = glfwCreateWindow(
		width,
		height,
		params.title.c_str(),
		params.isFullscreen ? glfwGetPrimaryMonitor() : nullptr,
		nullptr);

	if (window == nullptr) {
		SetError("Error creating window");
	}

	// Make OpenGL context current
	glfwMakeContextCurrent(window);

	// Load OpenGL extensions
	if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress))) {
		SetError("gladLoadGLLoader() failed");
	}
}

void GlfwWindowManager::InternalActivateWindow()
{
	assert(window);

	if (activeParams.isFullscreen)
	{
		glfwRestoreWindow(window);
	}

	// Give window focus.
	// Unfortunately glfw doesn't have a corresponding function, so we have to use Win32
	// platform specific code.
	HWND handle = glfwGetWin32Window(window);
	SetForegroundWindow(handle);						// Slightly Higher Priority
	SetFocus(handle);									// Sets Keyboard Focus To The Window
}

void GlfwWindowManager::InternalDeactivateWindow()
{
	assert(window);

	if (activeParams.isFullscreen)
	{
		// Switch away from fullscreen window by minimising it
		glfwIconifyWindow(window);
		//HWND handle = glfwGetWin32Window(window);
		//ShowWindow(handle, SW_HIDE); 		    			// Hide the window
		//ChangeDisplaySettings(nullptr, 0);
	}
}

bool GlfwWindowManager::InternalIsCloseRequested()
{
	assert(window);
	return glfwWindowShouldClose(window);
}

void GlfwWindowManager::InternalSwapBuffers()
{
	assert(window);
	glfwSwapBuffers(window);
}

int GlfwWindowManager::GetScreenWidth()
{
	auto* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
	return mode->width;
}

int GlfwWindowManager::GetScreenHeight()
{
	auto* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
	return mode->height;
}

GlfwWindowManager::GlfwWindowManager() : window(nullptr)
{
	if (!glfwInit())
	{
		HasErrorState::SetError("glfwInit() failed");
		return;
	}

	atexit(glfwTerminate);
}

GlfwWindowManager::~GlfwWindowManager()
{
	if (window) {
		GlfwWindowManager::InternalDestroyWindow();
	}
}

