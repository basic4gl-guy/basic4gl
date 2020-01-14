#include "OpenGLWindowManager.h"
#include "glheaders.h"

///////////////////////////////////////////////////////////////////////////////
// OpenGLWindowParams

OpenGLWindowParams::OpenGLWindowParams()
	: isFullscreen(false),
	width(1024),
	height(768),
	bpp(0),
	isBordered(true),
	isResizable(false),
	isStencilBufferRequired(false),
	title("OpenGL")
{
}

///////////////////////////////////////////////////////////////////////////////
// OpenGLWindowManager

void OpenGLWindowManager::SetOpenGLDefaults()
{
	// Setup a default viewport
	glViewport(0, 0, GetWindowWidth(), GetWindowHeight());	// Reset The Current Viewport

	// Set some default OpenGL matrices. Basic 3D perspective projection
	glMatrixMode(GL_PROJECTION);							// Select The Projection Matrix
	glLoadIdentity();										// Reset The Projection Matrix
	gluPerspective(
		60.0, 
		(double)GetWindowWidth() / (double)GetWindowHeight(), 
		1.0, 
		1000.0);

	glMatrixMode(GL_MODELVIEW);							// Select The Modelview Matrix
	glLoadIdentity();									// Reset The Modelview Matrix
	try { glShadeModel(GL_SMOOTH); }
	catch (...) { ; }
	try { glClearColor(0.0f, 0.0f, 0.0f, 0.5f); }
	catch (...) { ; }
	try { glClearDepth(1.0f); }
	catch (...) { ; }
	try { glEnable(GL_DEPTH_TEST); }
	catch (...) { ; }
	try { glDepthFunc(GL_LEQUAL); }
	catch (...) { ; }
	try { glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST); }
	catch (...) { ; }
	try { glDisable(GL_TEXTURE_2D); }
	catch (...) { ; }

	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
	SwapBuffers();
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
}

OpenGLWindowManager::OpenGLWindowManager() : isWindowCreated(false), isWindowShowing(false)
{
}

OpenGLWindowManager::~OpenGLWindowManager()
{
}

void OpenGLWindowManager::RecreateWindow()
{
	// Destroy any existing window first
	DestroyWindow();

	// Create new window
	ClearError();
	InternalCreateWindow(pendingParams);

	if (Error()) return;

	isWindowCreated = true;
	activeParams = pendingParams;
	isWindowShowing = true;
	SetOpenGLDefaults();

	// Call window created callbacks
	for (auto callback : windowCreatedCallbacks)
	{
		callback();
	}
}

void OpenGLWindowManager::ActivateWindow()
{
	if (isWindowCreated && !isWindowShowing)
	{
		InternalActivateWindow();
		isWindowShowing = true;
	}
}

void OpenGLWindowManager::DeactivateWindow()
{
	if (isWindowCreated && isWindowShowing)
	{
		InternalDeactivateWindow();
		isWindowShowing = false;
	}
}

void OpenGLWindowManager::DestroyWindow()
{
	if (isWindowCreated)
	{
		// Call before window destroyed callbacks
		for (auto callback : beforeDestroyWindowCallbacks)
		{
			callback();
		}

		InternalDestroyWindow();
		isWindowCreated = false;
	}
}

bool OpenGLWindowManager::IsCloseRequested()
{
	return isWindowCreated && InternalIsCloseRequested();
}

void OpenGLWindowManager::SwapBuffers()
{
	if (isWindowCreated)
	{
		InternalSwapBuffers();
	}
}

int OpenGLWindowManager::GetWindowWidth()
{
	int width = activeParams.width;
	if (width == 0) width = GetScreenWidth();
	return width;
}

int OpenGLWindowManager::GetWindowHeight()
{
	int height = activeParams.height;
	if (height == 0) height = GetScreenHeight();
	return height;
}

int OpenGLWindowManager::GetPendingWindowWidth()
{
	int width = pendingParams.width;
	if (width == 0) width = GetScreenWidth();
	return width;
}

int OpenGLWindowManager::GetPendingWindowHeight()
{
	int height = pendingParams.height;
	if (height == 0) height = GetScreenHeight();
	return height;
}
