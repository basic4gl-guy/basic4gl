#include "Content2DManager.h"

void Content2DManager::SaveGLState()
{
	// Preserve OpenGL state
	glPushAttrib(GL_ALL_ATTRIB_BITS);
	glMatrixMode(GL_TEXTURE);
	glPushMatrix();
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();

	// Setup state for 2D rendering
	glDisable(GL_FOG);
	glDisable(GL_BLEND);
	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_LIGHTING);
	glDisable(GL_SCISSOR_TEST);
	glDisable(GL_STENCIL_TEST);

	glEnable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void Content2DManager::RestoreGLState()
{
	// Restore preserved OpenGL state
	glMatrixMode(GL_TEXTURE);
	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
	glPopAttrib();
}

void Content2DManager::InternalDraw(int bitFlags)
{
	// Draw content
	SaveGLState();
	for (auto c : content)
	{
		if (c.bitFlag & bitFlags)
		{
			c.drawCallback();
		}
	}
	RestoreGLState();
}

Content2DManager::Content2DManager(OpenGLWindowManager& windowManager): 
	windowManager(windowManager)
{
	Reset();
}

Content2DManager::~Content2DManager()
{
}

void Content2DManager::Reset()
{
	// Set default
	drawMode = TextMode::TEXT_SIMPLE;
}

void Content2DManager::AddContent(float z, int bitFlag, std::function<void()> drawCallback)
{
	// Find sorted insert position
	auto i = content.begin();
	while (i != content.end() && i->z < z) ++i;

	// Insert content
	content.insert(i, Content2D(z, bitFlag, drawCallback));
}

void Content2DManager::Draw(int bitFlags)
{
	if (drawMode == TextMode::TEXT_SIMPLE || drawMode == TextMode::TEXT_BUFFERED)
	{
		FullRedraw(bitFlags);
	}
	else if (drawMode == TextMode::TEXT_OVERLAID)
	{
		InternalDraw(bitFlags);
	}
}

void Content2DManager::FullRedraw(int bitFlags)
{
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
	InternalDraw(bitFlags);
	windowManager.SwapBuffers();
}
