// OpenAL32.lib alut.lib ogg_static_d.lib vorbis_static_d.lib vorbisfile_static_d.lib 

/*	B4GLSoundMain

	Basic4GL sound engine DLL main module.
*/

#include "B4GLSound.h"
#include "SoundEngine.h"
#include <string>

using namespace SndEngine;
using namespace std;

///////////////////////////////////////////////////////////////////////////////
//	Global variables

// Sound engine objects
SoundEngine *sound = NULL;
MusicStream *music = NULL;

// Error state
string error = "";

///////////////////////////////////////////////////////////////////////////////
//	Local functions
static bool CheckError(SndEngine::HasErrorState *obj) {
    if (obj->Error()) {
        error = obj->ErrorText();
        return false;
    }
    else {
        error = "";
        return true;
    }
}

///////////////////////////////////////////////////////////////////////////////
//	Exported functions

extern "C" {
	DLLExport void DLLFUNC Init(int voices) {
//		MessageBox(NULL, "Init", "B4GLSound", MB_OK);
		if (sound == NULL) {
			sound = new SoundEngine(voices);
			music = new MusicStream;
		}
	}
	DLLExport void DLLFUNC Shutdown() {
//		MessageBox(NULL, "Shutdown", "Shutdown", MB_OK);
		if (sound != NULL) {
//			MessageBox(NULL, "music", "Shutdown", MB_OK);
			delete music;
			music = NULL;
//			MessageBox(NULL, "sounds", "Shutdown", MB_OK);
			delete sound;
			sound = NULL;
		}
	}
	DLLExport void DLLFUNC Reset() {
//		MessageBox(NULL, "Reset", "Reset", MB_OK);
		sound->StopAll();
		music->CloseFile();
		error = "";
		alGetError();
		alutGetError();
	}
	DLLExport void *DLLFUNC LoadSound(char *filename) {
//		MessageBox(NULL, "LoadSound", "B4GLSound", MB_OK);
//		MessageBox(NULL, filename, "B4GLSound", MB_OK);
		Sound *s = new Sound(filename);
		if (CheckError(s))
			return s;
		else {
			delete s;
			return NULL;
		}
	}
	DLLExport void DLLFUNC DeleteSound(void *s) {
//		MessageBox(NULL, "DeleteSound", "B4GLSound", MB_OK);
		if (sound != NULL)
			delete (Sound *) s;
	}
	DLLExport int DLLFUNC DoPlaySound(void *s, float gain, bool looped) {
//		MessageBox(NULL, "DoPlaySound", "B4GLSound", MB_OK);
		if (s != NULL) {
			int voice = sound->PlaySound(*(Sound *) s, gain, looped);
			CheckError((Sound *) s);
			return voice;
		}
		else
			return -1;
	}
	DLLExport void DLLFUNC StopSounds() {
//		MessageBox(NULL, "StopSounds", "B4GLSound", MB_OK);
		sound->StopAll();
	}
	DLLExport void DLLFUNC PlayMusic(char *filename, float gain, bool looped) {
//		MessageBox(NULL, "PlayMusic", "B4GLSound", MB_OK);
//		MessageBox(NULL, filename, "B4GLSound", MB_OK);
		music->OpenFile(filename, gain, looped);
		music->UpdateErrorState();
		CheckError(music);
	}
	DLLExport void DLLFUNC StopMusic() {
//		MessageBox(NULL, "StopMusic", "B4GLSound", MB_OK);
		music->CloseFile();
	}
	DLLExport bool DLLFUNC MusicPlaying() {
		return music->Playing();
	}
	DLLExport void DLLFUNC SetMusicVolume(float gain) {
		music->SetGain(gain);
	}
	DLLExport void DLLFUNC StopSoundVoice(int voice) {
//		MessageBox(NULL, "StopSoundVoice", "B4GLSound", MB_OK);
		sound->StopVoice(voice);
	}
	DLLExport void DLLFUNC GetError(char *buffer, int len) {
		// This is basically a strcpy without a len.
//		MessageBox(NULL, "GetError", "B4GLSound", MB_OK);
		char *src = (char *) error.c_str();
		char *dst = buffer;
		while (*src != 0 && len > 1) {
			*dst = *src;
			src++;
			dst++;
			len--;
		}
		*dst = 0;
	}
}