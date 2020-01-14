/*	ContentManager2D.h
	Copyright (C) Tom Mulgrew, 2016 (tmulgrew@slingshot.co.nz)

	Orchestrates drawing 2D content, such as text and sprites.
*/

#pragma once
#include "glheaders.h"
#include "OpenGLWindowManager.h"
#include "Basic4GLOpenGLObjects.h"
#include <vector>
#include <functional>

class Content2DManager
{
	// 2D content to draw
	struct Content2D
	{
		float z;
		int bitFlag;
		std::function<void()> drawCallback;

		Content2D(float z, int bitFlag, const std::function<void()>& drawCallback)
			: z(z), bitFlag(bitFlag), drawCallback(drawCallback)
		{
		}
	};

	OpenGLWindowManager& windowManager;
	TextMode drawMode;
	std::vector<Content2D> content;

	void SaveGLState();
	void RestoreGLState();
	void InternalDraw(int bitFlags);
public:
	explicit Content2DManager(OpenGLWindowManager& windowManager);
	virtual ~Content2DManager();

	/// Reset to defaults
	void Reset();

	/// Register content to draw
	void AddContent(float z, int bitFlag, std::function<void()> drawCallback);

	// Properties
	TextMode GetDrawMode() const { return drawMode; }
	void SetDrawMode(TextMode value)  { drawMode = value; }
	OpenGLWindowManager& GetWindowManager() const { return windowManager; }

	/// Explicitly redraw
	void Draw(int bitFlags = 0xffffffff);
	void FullRedraw(int bitFlags = 0xffffffff);

	/// Notify of 2D change made
	void ChangeMade()
	{
		// Redraw automatically if in simple mode
		if (drawMode == TextMode::TEXT_SIMPLE)
		{
			Draw();
		}
	}
};

