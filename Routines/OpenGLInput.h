/*	Created 17-Apr-2016: Thomas Mulgrew (tmulgrew@slingshot.co.nz)

	Abstract base classes for OpenGL window keyboard and mouse routines.
*/

#pragma once
#include <queue>
#include <functional>
#include "OpenGLWindowManager.h"

#ifndef byte 
typedef unsigned char byte;
#endif

// Base class for keyboard access.
// Descendent classes must hookup actual keyboard events and update buffers accordingly.
class OpenGLKeyboard
{
	struct KeyPressedCallback
	{
		int unsubscribeHandle;
		std::function<void(bool)> callback;			// Parameter is whether key is scankey (true) or ASCII character (false)
		KeyPressedCallback(int unsubscribeHandle, const std::function<void(bool)>& callback)
			: unsubscribeHandle(unsubscribeHandle),
			callback(callback)
		{
		}
	};

	/// Callback functions for when key pressed.
	/// Parameter is whether key is scankey (true) or ASCII character (false)
	std::vector<KeyPressedCallback> keyPressedCallbacks;
	int unsubscribeHandleGenerator;
	bool isPausePressed;

	void NotifySubscribers(bool isScanKey);
	void ProcessScanKey(int key, byte keybit);

protected:
	std::queue<int> scanKeyBuffer;
	std::queue<char> charBuffer;
	byte scanKeyMasks[256];
	byte charMasks[128];

	OpenGLKeyboard(OpenGLWindowManager& windowManager);

	// Process keypresses, update bufferes and notify subscribers.
	// Descendent classes should call these in response to keyboard events.
	void ScanKeyPress(int key, char c);
	void ScanKeyRelease(int key, char c);
	void KeyPress(char c);

public:

	// Keyboard buffers
	char GetNextKey();
	int GetNextScanKey();
	void ClearKeyBuffers();
	void ClearKeyState();

	// Key state
	bool IsKeyDown(char key);
	bool IsScanKeyDown(int scanKey);
	bool IsPausePressed();

	// Key notification
	int SubscribeKeyPressed(std::function<void(bool)> callback);
	void UnsubscribeKeyPressed(int unsubscribeHandle);

	// Fake keypresses (e.g. for JoyKeys())
	void FakeKeyState(int scanKey, char c, byte keybit, bool isDown);
};

class OpenGLMouse
{
public:
	virtual ~OpenGLMouse() {};
	virtual float GetX() = 0;
	virtual float GetY() = 0;
	virtual float GetXD() = 0;
	virtual float GetYD() = 0;
	virtual bool GetButton(int index) = 0;
	virtual int GetWheelDelta() = 0;
};
