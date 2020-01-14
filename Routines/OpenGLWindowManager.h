#pragma once
#include <string>
#include <vector>
#include <functional>
#include <HasErrorState.h>
#include <compConstant.h>

struct OpenGLWindowParams
{
	bool isFullscreen;
	int width;
	int height;
	int bpp;
	bool isBordered;
	bool isResizable;
	bool isStencilBufferRequired;
	std::string title;

	OpenGLWindowParams();
};

/// Manages creating and recreating the OpenGL window.
/// Also manages swapping buffers and orchestrating 2D content like text and sprites.
/// Abstract base class. Must be implemented for specific OpenGL window library (e.g. GLFW)
class OpenGLWindowManager : public HasErrorState
{
	void SetOpenGLDefaults();

	std::vector<std::function<void()>> windowCreatedCallbacks;
	std::vector<std::function<void()>> beforeDestroyWindowCallbacks;

protected:
	bool isWindowCreated;
	OpenGLWindowParams activeParams;
	bool isWindowShowing;

	// Must be implemented in base class

	/// Destroy the window (if exists)
	virtual void InternalDestroyWindow() = 0;

	/// Create a new window based on the given parameters, and make active
	virtual void InternalCreateWindow(OpenGLWindowParams params) = 0;

	/// Make window active
	virtual void InternalActivateWindow() = 0;

	virtual void InternalDeactivateWindow() = 0;

	virtual bool InternalIsCloseRequested() = 0;

	virtual void InternalSwapBuffers() = 0;

	virtual int GetScreenWidth() = 0;

	virtual int GetScreenHeight() = 0;

public:
	OpenGLWindowParams pendingParams;

	explicit OpenGLWindowManager();
	virtual ~OpenGLWindowManager();

	OpenGLWindowParams const& GetActiveParams() const { return activeParams; }

	void RecreateWindow();
	void ActivateWindow();
	void DeactivateWindow();
	void DestroyWindow();
	bool IsCloseRequested();
	void SwapBuffers();
	bool IsWindowCreated() { return isWindowCreated; }

	void SubscribeWindowCreated(std::function<void()> callback)	{ windowCreatedCallbacks.push_back(callback); }
	void SubscribeBeforeDestroyWindow(std::function<void()> callback) { beforeDestroyWindowCallbacks.push_back(callback); }

	int GetWindowWidth();
	int GetWindowHeight();
	int GetPendingWindowWidth();
	int GetPendingWindowHeight();
};

