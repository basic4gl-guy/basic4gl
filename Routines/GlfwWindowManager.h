#pragma once
#include "OpenGLWindowManager.h"
#include "glheaders.h"

class GlfwWindowManager :
	public OpenGLWindowManager
{
	GLFWwindow* window;

protected:
	void InternalDestroyWindow() override;
	void InternalCreateWindow(OpenGLWindowParams params) override;
	void InternalActivateWindow() override;
	void InternalDeactivateWindow() override;
	bool InternalIsCloseRequested() override;
	void InternalSwapBuffers() override;
	int GetScreenWidth() override;
	int GetScreenHeight() override;
public:
	explicit GlfwWindowManager();
	virtual ~GlfwWindowManager();
	
	GLFWwindow* GetGlfwWindow() const { return window; }
};

