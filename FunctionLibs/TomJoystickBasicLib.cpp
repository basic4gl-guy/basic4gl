//---------------------------------------------------------------------------


#pragma hdrstop

#include "TomJoystickBasicLib.h"
#include <windows.h>

//---------------------------------------------------------------------------

#ifndef _MSC_VER
#pragma package(smart_init)
#endif

#define DEFAULT_JOY_THRESHHOLD 0x4000
#define JOY_BUTTONS 4

JOYINFO joyInfo;            // Current joystick state
bool autoPoll = true;		// When true, joystick is automatically Polled before any function call
bool initialised = false;
int threshHold = DEFAULT_JOY_THRESHHOLD;    // Number of units the joystick must be moved
                                            // in order to be considered pointing up, down, left or right
UINT buttons[JOY_BUTTONS] = {
                    JOY_BUTTON1,
                    JOY_BUTTON2,
                    JOY_BUTTON3,
                    JOY_BUTTON4 };

static OpenGLKeyboard* keyboard = nullptr;

inline void ClearJoyInfo () {
    memset (&joyInfo, 0, sizeof (joyInfo));
    joyInfo.wXpos = 0x8000;
    joyInfo.wYpos = 0x8000;
}
static void Init (TomVM& vm) {

	// This function is called everytime a Basic4GL program starts.
	// Reset joystick related state.
	autoPoll = true;		// Automatically Poll joystick whenever it is accessed
    ClearJoyInfo ();
    threshHold = DEFAULT_JOY_THRESHHOLD;
}
inline void PollJoystick () {

    // Read joystick position
    if (joyGetPos (JOYSTICKID1, &joyInfo) != JOYERR_NOERROR)
        ClearJoyInfo ();
}

inline void AutoPoll () {
    if (autoPoll)
        PollJoystick ();
}

void WrapUpdateJoystick (TomVM& vm) {
    autoPoll = false;       // Explicitly polling the joystick disables automatic polling
    PollJoystick ();
}

inline int JoyX () {
    return (int) joyInfo.wXpos - 0x8000;
}
inline int JoyY () {
    return (int) joyInfo.wYpos - 0x8000;
}
void WrapJoyLeft (TomVM& vm) {
    AutoPoll ();
    vm.Reg ().IntVal () = JoyX () <= -threshHold ? -1 : 0;
}
void WrapJoyRight (TomVM& vm) {
    AutoPoll ();
    vm.Reg ().IntVal () = JoyX () >= threshHold ? -1 : 0;
}
void WrapJoyUp (TomVM& vm) {
    AutoPoll ();
    vm.Reg ().IntVal () = JoyY () <= -threshHold ? -1 : 0;
}
void WrapJoyDown (TomVM& vm) {
    AutoPoll ();
    vm.Reg ().IntVal () = JoyY () >= threshHold ? -1 : 0;
}
void WrapJoyButton0 (TomVM& vm) {
    AutoPoll ();
    vm.Reg ().IntVal () = joyInfo.wButtons & buttons [0] ? -1 : 0;
}
void WrapJoyButton1 (TomVM& vm) {
    AutoPoll ();
    vm.Reg ().IntVal () = joyInfo.wButtons & buttons [1] ? -1 : 0;
}
void WrapJoyButton2 (TomVM& vm) {
    AutoPoll ();
    vm.Reg ().IntVal () = joyInfo.wButtons & buttons [2] ? -1 : 0;
}
void WrapJoyButton3 (TomVM& vm) {
    AutoPoll ();
    vm.Reg ().IntVal () = joyInfo.wButtons & buttons [3] ? -1 : 0;
}
void WrapJoyX (TomVM& vm) {
    AutoPoll ();
    vm.Reg ().IntVal () = JoyX ();
}
void WrapJoyY (TomVM& vm) {
    AutoPoll ();
    vm.Reg ().IntVal () = JoyY ();
}
void WrapJoyButton (TomVM& vm) {
    AutoPoll ();
    int index = vm.GetIntParam (1);
    if (index >= 0 && index < JOY_BUTTONS)
        vm.Reg ().IntVal () = joyInfo.wButtons & buttons [index] ? -1 : 0;
}
void WrapJoyKeys (TomVM& vm) {
    AutoPoll ();

    // Create fake keypresses based on the joystick state
    // Axis movement translates to cursor keys
    // Fire button 1 translates to space bar
    // Fire button 2 translates to control key (Ctrl)
	keyboard->FakeKeyState(VK_LEFT, 0, 2, JoyX() < -threshHold);
	keyboard->FakeKeyState(VK_RIGHT, 0, 2, JoyX() >  threshHold);
	keyboard->FakeKeyState(VK_UP, 0, 2, JoyY() < -threshHold);
	keyboard->FakeKeyState(VK_DOWN, 0, 2, JoyY() >  threshHold);
	keyboard->FakeKeyState(VK_SPACE, ' ', 2, joyInfo.wButtons & buttons[0]);
	keyboard->FakeKeyState(VK_CONTROL, 0, 2, joyInfo.wButtons & buttons[1]);
}

void InitTomJoystickBasicLib(TomBasicCompiler& comp, OpenGLKeyboard* keyboard) {

    // Store pointer to keyboard
	::keyboard = keyboard;

	// Init function
    comp.VM().AddInitFunc (Init);		// This function will be called before Basic4GL runs any program

	 // Register functions
	comp.AddFunction ("UpdateJoystick", WrapUpdateJoystick, compParamTypeList (),true, false,VTP_INT);
   	comp.AddFunction ("Joy_Left",       WrapJoyLeft,        compParamTypeList (),true, true, VTP_INT);
	comp.AddFunction ("Joy_Right",      WrapJoyRight,       compParamTypeList (),true, true, VTP_INT);
	comp.AddFunction ("Joy_Up",         WrapJoyUp,          compParamTypeList (),true, true, VTP_INT);
	comp.AddFunction ("Joy_Down",       WrapJoyDown,        compParamTypeList (),true, true, VTP_INT);
	comp.AddFunction ("Joy_0",          WrapJoyButton0,     compParamTypeList (),true, true, VTP_INT);
	comp.AddFunction ("Joy_1",          WrapJoyButton1,     compParamTypeList (),true, true, VTP_INT);
	comp.AddFunction ("Joy_2",          WrapJoyButton2,     compParamTypeList (),true, true, VTP_INT);
	comp.AddFunction ("Joy_3",          WrapJoyButton3,     compParamTypeList (),true, true, VTP_INT);
	comp.AddFunction ("Joy_X",			WrapJoyX,		    compParamTypeList (),true, true, VTP_INT);
	comp.AddFunction ("Joy_Y",			WrapJoyY,	        compParamTypeList (),true, true, VTP_INT);
	comp.AddFunction ("Joy_Button",		WrapJoyButton,	    compParamTypeList () << VTP_INT, true, true, VTP_INT);
    comp.AddFunction ("Joy_Keys",       WrapJoyKeys,        compParamTypeList (),true, false, VTP_INT);
}
