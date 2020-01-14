/*	B4GLSound.h

	Created 3-Jun-06: Thomas Mulgrew (tmulgrew@slingshot.co.nz)

	Basic4GL sound library DLL header.
*/

// DLL function exporting
#ifndef DLLExport
#define DLLExport __declspec(dllexport)
#endif

#ifndef DLLFUNC
#define DLLFUNC __cdecl
#endif

#include <string>

#ifndef B4GLSOUND_EXPORTS

typedef void (DLLFUNC *B4GLSound_Init)(int voices);
typedef void (DLLFUNC *B4GLSound_Shutdown)();
typedef void (DLLFUNC *B4GLSound_Reset)();
typedef void *(DLLFUNC *B4GLSound_LoadSound)(char *filename);
typedef void (DLLFUNC *B4GLSound_DeleteSound)(void *s);
typedef int (DLLFUNC *B4GLSound_DoPlaySound)(void *s, float gain, bool looped);
typedef void (DLLFUNC *B4GLSound_StopSounds)();
typedef void (DLLFUNC *B4GLSound_PlayMusic)(char *filename, float gain, bool looped);
typedef void (DLLFUNC *B4GLSound_StopMusic)();
typedef bool (DLLFUNC *B4GLSound_MusicPlaying)();
typedef void (DLLFUNC *B4GLSound_SetMusicVolume)(float gain);
typedef void (DLLFUNC *B4GLSound_StopSoundVoice)(int voice);
typedef void (DLLFUNC *B4GLSound_GetError)(char *buffer, int len);

#endif