#include "OpenGLInput.h"
#include <windows.h>

using namespace std;

OpenGLKeyboard::OpenGLKeyboard(OpenGLWindowManager& windowManager) : unsubscribeHandleGenerator(0), isPausePressed(false)
{
	ClearKeyState();
	windowManager.SubscribeWindowCreated([this]()
	{
		ClearKeyBuffers();
		ClearKeyState();
	});
}

void OpenGLKeyboard::NotifySubscribers(bool isScanKey)
{
	// Iterate a COPY of the subscriber set, in case the subscriber unsubscribes 
	// during processing of the notification. (E.g. Input$ BASIC command handler 
	// completes when ENTER key is pressed).
	vector<KeyPressedCallback> callbackCopy(keyPressedCallbacks);
	for (auto c : callbackCopy)
	{
		c.callback(isScanKey);
	}
}

void OpenGLKeyboard::ProcessScanKey(int key, byte keybit)
{
	scanKeyMasks[key] |= keybit;

	// Add to buffer
	if (scanKeyBuffer.size() < 16)
		scanKeyBuffer.push(key);

	// Notify subscribers of scan key press
	NotifySubscribers(true);
}

void OpenGLKeyboard::ScanKeyPress(int key, char c)
{
	// Special case: Detect PAUSE
	if (key == VK_PAUSE)
		isPausePressed = true;

	// Buffer scan key and mark as down
	if (key != 0) {
		ProcessScanKey(key, 1);
	}

	// Mark character as down
	if (c != 0) {
		charMasks[c] |= 1;
	}
}

void OpenGLKeyboard::ScanKeyRelease(int key, char c)
{
	// Mark scan key as up
	if (key != 0) {
		scanKeyMasks[key] &= ~1;
	}

	// Mark character as up
	if (c != 0) {
		charMasks[c] &= ~1;
	}
}

void OpenGLKeyboard::KeyPress(char c)
{
	if (charBuffer.size() < 16)
		charBuffer.push(c);

	// Notify subscribers of keypress
	NotifySubscribers(false);
}

char OpenGLKeyboard::GetNextKey()
{
	char c = 0;
	if (!charBuffer.empty())
	{
		c = charBuffer.front();
		charBuffer.pop();
	}

	return c;
}

int OpenGLKeyboard::GetNextScanKey()
{
	int key = 0;
	if (!scanKeyBuffer.empty())
	{
		key = scanKeyBuffer.front();
		scanKeyBuffer.pop();
	}

	return key;
}

void OpenGLKeyboard::ClearKeyBuffers()
{
	// There does not appear to be a queue::clear() method..?
	while (!charBuffer.empty()) charBuffer.pop();
	while (!scanKeyBuffer.empty()) scanKeyBuffer.pop();
}

void OpenGLKeyboard::ClearKeyState()
{
	for (int i = 0; i < 256; i++) scanKeyMasks[i] = 0;
	for (int i = 0; i < 128; i++) charMasks[i] = 0;
}

bool OpenGLKeyboard::IsKeyDown(char key)
{
	if (key >= 0 && key <= 127)
		return charMasks[key] != 0;
	return false;
}

bool OpenGLKeyboard::IsScanKeyDown(int scanKey)
{
	if (scanKey >= 0 && scanKey <= 255)
		return scanKeyMasks[scanKey] != 0;
	return false;
}

bool OpenGLKeyboard::IsPausePressed()
{
	bool result = isPausePressed;
	isPausePressed = false;				// Reset flag on read
	return result;
}

int OpenGLKeyboard::SubscribeKeyPressed(std::function<void(bool)> callback)
{
	int unsubscribeHandle = ++unsubscribeHandleGenerator;
	keyPressedCallbacks.push_back(KeyPressedCallback(unsubscribeHandle, callback));
	return unsubscribeHandle;
}

void OpenGLKeyboard::UnsubscribeKeyPressed(int unsubscribeHandle)
{
	auto i = keyPressedCallbacks.begin();
	while (i != keyPressedCallbacks.end())
	{
		if (i->unsubscribeHandle == unsubscribeHandle)
			i = keyPressedCallbacks.erase(i);
		else
			++i;
	}
}

void OpenGLKeyboard::FakeKeyState(int scanKey, char c, byte keybit, bool isDown)
{
	if (scanKey != 0)
	{
		// Set/clear corresponding bit
		if (isDown) {
			if (!(scanKeyMasks[scanKey] & keybit))
				ProcessScanKey(scanKey, keybit);
		}
		else
			scanKeyMasks[scanKey] &= ~keybit;
	}

	if (c != 0)
	{
		if (isDown) {
			bool wasPressed = charMasks[c] & keybit;
			charMasks[c] |= keybit;

			// Add to char buffer if wasn't previously pressed.
			// This also triggers notifications.
			if (!wasPressed)
				KeyPress(c);
		}
		else
			charMasks[c] &= ~keybit;
	}
}
