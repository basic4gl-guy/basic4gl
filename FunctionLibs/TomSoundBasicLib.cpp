//---------------------------------------------------------------------------


#pragma hdrstop

#include "TomSoundBasicLib.h"
#include "B4GLSound.h"
#include <windows.h>

//---------------------------------------------------------------------------

#pragma package(smart_init)

////////////////////////////////////////////////////////////////////////////////
//  Globals
static HINSTANCE dll    = NULL;
static bool triedToLoad = false;
FileOpener *files       = NULL;

static B4GLSound_Init           SndInit             = NULL;
static B4GLSound_Shutdown       SndShutdown         = NULL;
static B4GLSound_Reset          SndReset            = NULL;
static B4GLSound_LoadSound      SndLoadSound        = NULL;
static B4GLSound_DeleteSound    SndDeleteSound      = NULL;
static B4GLSound_DoPlaySound    SndDoPlaySound      = NULL;
static B4GLSound_StopSounds     SndStopSounds       = NULL;
static B4GLSound_PlayMusic      SndPlayMusic        = NULL;
static B4GLSound_StopMusic      SndStopMusic        = NULL;
static B4GLSound_MusicPlaying   SndMusicPlaying     = NULL;
static B4GLSound_SetMusicVolume SndSetMusicVolume   = NULL;
static B4GLSound_StopSoundVoice SndStopSoundVoice   = NULL;
static B4GLSound_GetError       SndGetError         = NULL;

////////////////////////////////////////////////////////////////////////////////
//  Init function
static void Init (TomVM& vm) {
    if (dll != NULL)
        SndReset();
}

static bool CheckSoundEngine() {
    if (!triedToLoad) {
    
        // Try to load sound engine
        dll = LoadLibrary("B4GLSound.dll");
        if (dll != NULL) {

            // Extract function pointers
            SndInit            = (B4GLSound_Init)           GetProcAddress(dll, "Init");
            SndShutdown        = (B4GLSound_Shutdown)       GetProcAddress(dll, "Shutdown");
            SndReset           = (B4GLSound_Reset)          GetProcAddress(dll, "Reset");
            SndLoadSound       = (B4GLSound_LoadSound)      GetProcAddress(dll, "LoadSound");
            SndDeleteSound     = (B4GLSound_DeleteSound)    GetProcAddress(dll, "DeleteSound");
            SndDoPlaySound     = (B4GLSound_DoPlaySound)    GetProcAddress(dll, "DoPlaySound");
            SndStopSounds      = (B4GLSound_StopSounds)     GetProcAddress(dll, "StopSounds");
            SndPlayMusic       = (B4GLSound_PlayMusic)      GetProcAddress(dll, "PlayMusic");
            SndStopMusic       = (B4GLSound_StopMusic)      GetProcAddress(dll, "StopMusic");
            SndMusicPlaying    = (B4GLSound_MusicPlaying)   GetProcAddress(dll, "MusicPlaying");
            SndSetMusicVolume  = (B4GLSound_SetMusicVolume) GetProcAddress(dll, "SetMusicVolume");
            SndStopSoundVoice  = (B4GLSound_StopSoundVoice) GetProcAddress(dll, "StopSoundVoice");
            SndGetError        = (B4GLSound_GetError)       GetProcAddress(dll, "GetError");

            // Verify them
            if (
                SndInit             != NULL &&
                SndShutdown         != NULL &&
                SndReset            != NULL &&
                SndLoadSound        != NULL &&
                SndDeleteSound      != NULL &&
                SndDoPlaySound      != NULL &&
                SndStopSounds       != NULL &&
                SndPlayMusic        != NULL &&
                SndStopMusic        != NULL &&
                SndMusicPlaying     != NULL &&
                SndSetMusicVolume   != NULL &&
                SndStopSoundVoice   != NULL &&
                SndGetError         != NULL)

                // Initialise sound library
                SndInit(10);
            else {

                // Failed to load all functions.
                FreeLibrary(dll);
                dll = NULL;
            }
        }
        triedToLoad = true;
    }
    
    return dll != NULL;
}

////////////////////////////////////////////////////////////////////////////////
//  SoundStore
//
/// Stores sound objects as returned from the DLL.
class SoundStore : public vmResourceStore<void*> {
protected:
    virtual void DeleteElement(int index);
public:
    SoundStore() : vmResourceStore<void*>(NULL) { ; }
};

void SoundStore::DeleteElement(int index) {
    if (dll != NULL)
        SndDeleteSound(Value(index));
}

SoundStore sounds;

////////////////////////////////////////////////////////////////////////////////
//  Runtime function wrappers
void WrapLoadSound(TomVM& vm) {
    if (CheckSoundEngine()) {

        // Load sound file
        std::string filename = files->FilenameForRead(vm.GetStringParam(1), false);
        void *sound = SndLoadSound((char *) filename.c_str());
        if (sound != NULL)
            vm.Reg().IntVal() = sounds.Alloc(sound);
        else
            vm.Reg().IntVal() = 0;
    }
    else
        vm.Reg().IntVal() = 0;
}

void WrapDeleteSound(TomVM& vm) {
    if (CheckSoundEngine()) {
        int handle = vm.GetIntParam(1);
        if (handle > 0 && sounds.IndexStored(handle))
            sounds.Free(handle);
    }
}

void WrapPlaySound(TomVM& vm) {
    if (CheckSoundEngine()) {
        int handle = vm.GetIntParam(1);
        if (handle > 0 && sounds.IndexStored(handle))
            vm.Reg().IntVal() = SndDoPlaySound(sounds.Value(handle), 1, false);
        else
            vm.Reg().IntVal() = -1;
    }
    else
        vm.Reg().IntVal() = -1;
}

void WrapPlaySound2(TomVM& vm) {
    if (CheckSoundEngine()) {
        int handle = vm.GetIntParam(3);
        if (handle > 0 && sounds.IndexStored(handle))
            vm.Reg().IntVal() = SndDoPlaySound(sounds.Value(handle), vm.GetRealParam(2), vm.GetIntParam(1) != 0);
        else
            vm.Reg().IntVal() = -1;
    }
    else
        vm.Reg().IntVal() = -1;
}

void WrapStopSounds(TomVM& vm) {
    if (CheckSoundEngine())
        SndStopSounds();
}

void WrapPlayMusic(TomVM& vm) {
    if (CheckSoundEngine()) {
        std::string filename = files->FilenameForRead(vm.GetStringParam(1).c_str(), false);
        SndPlayMusic((char *) filename.c_str(), 1, false);
    }
}

void WrapPlayMusic2(TomVM& vm) {
    if (CheckSoundEngine()) {
        std::string filename = files->FilenameForRead(vm.GetStringParam(3).c_str(), false);
        SndPlayMusic((char *) filename.c_str(), vm.GetRealParam(2), vm.GetIntParam(1) != 0);
    }
}

void WrapStopMusic(TomVM& vm) {
    if (CheckSoundEngine())
        SndStopMusic();
}

void WrapMusicPlaying(TomVM& vm) {
    vm.Reg().IntVal() = CheckSoundEngine() && SndMusicPlaying() ? -1 : 0;
}

void WrapSetMusicVolume(TomVM& vm) {
    if (CheckSoundEngine())
        SndSetMusicVolume(vm.GetRealParam(1));
}

void WrapSoundError(TomVM& vm) {
    if (CheckSoundEngine()) {
        char buffer[1024];
        SndGetError(buffer, 1024);
        vm.RegString() = buffer;
    }
    else
        vm.RegString() = "Sound playback requires Audiere.dll and B4GLSound.dll to be placed in the same folder";
}

void WrapStopSoundVoice(TomVM& vm) {
    if (CheckSoundEngine())
        SndStopSoundVoice(vm.GetIntParam(1));
}

void InitTomSoundBasicLib (TomBasicCompiler& comp, FileOpener *_files) {

	// Save file opener pointer
	assert (_files != NULL);
	files = _files;

    // Register sound resources
    comp.VM().AddResources(sounds);

    // Register initialisation function
    comp.VM().AddInitFunc (Init);

    comp.AddFunction("loadsound",       WrapLoadSound,      compParamTypeList() << VTP_STRING,                          true,   true,   VTP_INT, true);
    comp.AddFunction("deletesound",     WrapDeleteSound,    compParamTypeList() << VTP_INT,                             true,   false,  VTP_INT);
    comp.AddFunction("playsound",       WrapPlaySound,      compParamTypeList() << VTP_INT,                             true,   true,   VTP_INT);
    comp.AddFunction("playsound",       WrapPlaySound2,     compParamTypeList() << VTP_INT << VTP_REAL << VTP_INT,      true,   true,   VTP_INT);
    comp.AddFunction("stopsoundvoice",  WrapStopSoundVoice, compParamTypeList() << VTP_INT,                             true,   false,  VTP_INT);
    comp.AddFunction("stopsounds",      WrapStopSounds,     compParamTypeList(),                                        true,   false,  VTP_INT);
    comp.AddFunction("playmusic",       WrapPlayMusic,      compParamTypeList() << VTP_STRING,                          true,   false,  VTP_INT, true);
    comp.AddFunction("playmusic",       WrapPlayMusic2,     compParamTypeList() << VTP_STRING << VTP_REAL << VTP_INT,   true,   false,  VTP_INT, true);
    comp.AddFunction("stopmusic",       WrapStopMusic,      compParamTypeList(),                                        true,   false,  VTP_INT);
    comp.AddFunction("musicplaying",    WrapMusicPlaying,   compParamTypeList(),                                        true,   true,   VTP_INT);
    comp.AddFunction("setmusicvolume",  WrapSetMusicVolume, compParamTypeList() << VTP_REAL,                            true,   false,  VTP_INT);
    comp.AddFunction("sounderror",      WrapSoundError,     compParamTypeList(),                                        true,   true,   VTP_STRING);
}

void CleanupTomSoundBasicLib() {
   if (dll != NULL) {
        SndShutdown();
        FreeLibrary(dll);
   }
}

void StopTomSoundBasicLib() {
    if (dll != NULL)
        SndReset();
}
