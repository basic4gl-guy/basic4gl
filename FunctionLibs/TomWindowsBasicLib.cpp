//---------------------------------------------------------------------------
// Created 27-Sep-2003: Thomas Mulgrew (tmulgrew@slingshot.co.nz)
// Copyright (C) Thomas Mulgrew
#pragma hdrstop

#include "TomWindowsBasicLib.h"
#include <windows.h>
#include <fstream>
//#include <sapi.h>
//---------------------------------------------------------------------------

#ifndef _MSC_VER
#pragma package(smart_init)
#endif

/*
#include <atlbase.h>
//You may derive a class from CComModule and use it if you want to override something,
//but do not change the name of _Module
extern CComModule _Module;
#include <atlcom.h>
#ifdef _MSC_VER
#else
#include <vcl.h>
#include "SpeechLib_TLB.h"
#endif*/

// Globals
static FileOpener *files = NULL;
static OpenGLWindowManager *windowManager = NULL;
static LARGE_INTEGER performanceFreq;

////////////////////////////////////////////////////////////////////////////////
// WindowsWav
//
// Represents a loaded windows .wav file.
class WindowsWav {
    char *sound;
    int len;
public:
    WindowsWav (std::string filename);
    ~WindowsWav () {
        if (sound != NULL)
            delete sound;
    }
    bool Loaded ()  { return sound != NULL; }
    void Play ()    { PlaySound (sound, NULL, SND_MEMORY | SND_ASYNC | SND_NODEFAULT); }
};

WindowsWav::WindowsWav (std::string filename) {
    sound   = NULL;
    len     = 0;

    // Attempt to load sound file
    GenericIStream *file = files->OpenRead(filename.c_str (), false, len);

    if (file != NULL && !file->fail ()) {
        if (!file->fail () && len > 0 && len < 0xa00000) {       // File must be 10meg or less in size

            // Allocate data storage
            sound = new char [len];

            // Read in data
            file->read (sound, len);
        }
    }
    if (file != NULL)
        delete file;
}

////////////////////////////////////////////////////////////////////////////////
// WindowsWavStore
//
// Used to track WindowsWavObjects
class WindowsWavStore : public vmResourceStore<WindowsWav *> {
protected:
    virtual void DeleteElement (int index);
public:
    WindowsWavStore () : vmResourceStore<WindowsWav *> (NULL) { ; }
};

void WindowsWavStore::DeleteElement (int index) {
	delete Value (index);
}

WindowsWavStore wavFiles;

////////////////////////////////////////////////////////////////////////////////
// Speech
/*
extern CComModule _Module;
Speechlib_tlb::ISpVoice *voice = NULL;
bool comActive = false, comTried = false, voiceTried = false;

bool InitCom () {
    if (!comTried) {
        // Initialise com
        comActive = !FAILED(::CoInitialize(NULL));
        comTried = true;
    }
    return comActive;
}

bool InitVoice () {
    if (InitCom ()) {
        if (!voiceTried) {
            CoCreateInstance(Speechlib_tlb::CLSID_SpVoice, NULL, CLSCTX_ALL, Speechlib_tlb::IID_ISpVoice, (void **)&voice);
            voiceTried = true;
        }
        return voice != NULL;
    }
    else
        return false;
} */

////////////////////////////////////////////////////////////////////////////////
// Performance counter
int PerformanceCounter() {
    if (performanceFreq.QuadPart == 0)       // No performance counter?
        return GetTickCount();      // Degrade to tick counter
    else {
        LARGE_INTEGER counter;
        QueryPerformanceCounter(&counter);
        return (counter.QuadPart * 1000) / performanceFreq.QuadPart;
    }
}

////////////////////////////////////////////////////////////////////////////////
// Function wrappers

void WrapBeep (TomVM& vm)           { MessageBeep (MB_ICONASTERISK); }
void WrapSleep (TomVM& vm)          {
    int msec = vm.GetIntParam (1);
    if (msec > 5000)
        msec = 5000;
    if (msec > 0)
        Sleep (msec);
}
void WrapTickCount (TomVM& vm)      { vm.Reg ().IntVal () = GetTickCount (); }
void WrapPerformanceCounter(TomVM& vm) {
    vm.Reg().IntVal() = PerformanceCounter();
}
/*void WrapLoadSound (TomVM& vm) {

    // Load wav file
    WindowsWav *wav = new WindowsWav (vm.GetStringParam (1));

    // If successfully loaded, allocate and return handle (guaranteed to be not zero)
    if (wav->Loaded ())
        vm.Reg ().IntVal () = wavFiles.Alloc (wav);
    else {
        // Otherwise delete it and return 0
        delete wav;
        vm.Reg ().IntVal () = 0;
    }
}
void WrapDeleteSound (TomVM& vm) {
    int handle = vm.GetIntParam (1);
    if (handle > 0 && wavFiles.IndexStored (handle))
        wavFiles.Free (handle);
}
void WrapPlaySound (TomVM& vm) {
    int handle = vm.GetIntParam (1);
    if (handle > 0 && wavFiles.IndexStored (handle))
        wavFiles.Value (handle)->Play ();
}*/
/*void WrapCanTalk (TomVM& vm) {
    vm.Reg ().IntVal () = InitVoice () ? -1 : 0;
}
void Say (std::string text, bool wait) {
    if (InitVoice ()) {

        // Truncate text to 4095 characters if necessary
        if (text.length () > 4095)
            text = text.substr (0, 4095);

        // Convert text to wide string format
        wchar_t buffer[8192];
        MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, text.c_str (), -1, buffer, 8192);

        // Say text
        voice->Speak(buffer, wait ? 0 : SPF_ASYNC, NULL);
    }
}
void WrapSay (TomVM& vm) {
    Say (vm.GetStringParam (1), false);
}
void WrapSayAndWait (TomVM& vm) {
    Say (vm.GetStringParam (1), true);
}*/

void WrapDesktopWidth(TomVM& vm) {
    vm.Reg().IntVal() = GetSystemMetrics(SM_CXSCREEN);
}

void WrapDesktopHeight(TomVM& vm) {
    vm.Reg().IntVal() = GetSystemMetrics(SM_CYSCREEN);
}

void WrapIsWindowClosing(TomVM& vm)
{
	vm.Reg().IntVal() = windowManager->IsCloseRequested() ? -1 : 0;
}

////////////////////////////////////////////////////////////////////////////////
// Initialisation

void InitTomWindowsBasicLib (TomBasicCompiler& comp, FileOpener *_files, OpenGLWindowManager* _windowManager) {

    files = _files;
	windowManager = _windowManager;

    LARGE_INTEGER freq;
    if (QueryPerformanceFrequency(&freq))
        performanceFreq = freq;
    else
        performanceFreq.QuadPart = 0;

    // Register resources
    comp.VM().AddResources (wavFiles);

    ///////////////////////
    // Register constants

    // Regular constants

    // Mathematics constants

    ///////////////////////
    // Register functions
    comp.AddFunction ("beep",               WrapBeep,               compParamTypeList (),                                       true,   false,   VTP_INT, true);
    comp.AddFunction ("sleep",              WrapSleep,              compParamTypeList () << VTP_INT,                            true,   false,   VTP_INT, true);
    comp.AddFunction ("tickcount",          WrapTickCount,          compParamTypeList (),                                       true,   true,    VTP_INT);
    comp.AddFunction ("performancecounter", WrapPerformanceCounter, compParamTypeList (),                                       true,   true,    VTP_INT);
//    comp.AddFunction ("loadsound",          WrapLoadSound,          compParamTypeList () << VTP_STRING,                         true,   true,    VTP_INT, true);
//    comp.AddFunction ("deletesound",        WrapDeleteSound,        compParamTypeList () << VTP_INT,                            true,   false,   VTP_INT);
//    comp.AddFunction ("playsound",          WrapPlaySound,          compParamTypeList () << VTP_INT,                            true,   false,   VTP_INT);
/*    comp.AddFunction ("cantalk",    WrapCanTalk,    compParamTypeList (),                                       true,   true,    VTP_INT);
    comp.AddFunction ("say",        WrapSay,        compParamTypeList () << VTP_STRING,                         false,  false,   VTP_INT);
    comp.AddFunction ("sayandwait", WrapSayAndWait, compParamTypeList () << VTP_STRING,                         false,  false,   VTP_INT, true);*/
    comp.AddFunction("DesktopWidth",        WrapDesktopWidth,       compParamTypeList(),                                        true,   true,    VTP_INT);
    comp.AddFunction("DesktopHeight",       WrapDesktopHeight,      compParamTypeList(),                                        true,   true,    VTP_INT);
	comp.AddFunction("iswindowclosing",     WrapIsWindowClosing,	compParamTypeList(),										true,   true,    VTP_INT);
}

void ShutDownTomWindowsBasicLib () {
/*
    // Delete voice
    if (voice != NULL) {
        voice->Release ();
        voice = NULL;
        voiceTried = false;
    }

    // Clean up com
    if (comActive) {
        ::CoUninitialize();
        comActive = false;
        comTried = false;
    }*/
}
