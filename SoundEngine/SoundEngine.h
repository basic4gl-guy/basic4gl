/*	SoundEngine

	Created 22-May-06: Thomas Mulgrew (tmulgrew@slingshot.co.nz)

	Simple sound and music engine built on top of:
		OpenAL (with alut)
		Ogg Vorbis (with VorbisFile)
*/

#ifndef _SoundEngine_h
#define _SoundEngine_h

// OpenAL
#include <AL/al.h>
#include <AL/alc.h>
#include <AL/alut.h>

// Ogg vorbis
#include <vorbis/codec.h>
#include <vorbis/vorbisfile.h>

// STL
#include <string>
#include <set>
#include <list>
#include <fstream>

// File I/O
#include <stdio.h>

#include "ThreadUtils.h"
#include "ErrorHandling.h"

namespace SndEngine {

///////////////////////////////////////////////////////////////////////////////
//	Sound
//
///	Represents a sound effect. Wrapper for OpenAL buffer.

class Sound : public HasErrorState {
private:
	ALuint buffer;			// The OpenAL buffer
public:
	Sound(std::string filename);
	~Sound() {
		if (buffer != AL_NONE)
			alDeleteBuffers(1, &buffer);
	}

	// Member access
	ALuint Buffer() const	{ return buffer; }

	// Sound properties
	int Freq() const {
		int freq;
		alGetBufferi(buffer, AL_FREQUENCY, &freq);
		return freq;
	}
	int Bits() const {
		int bits;
		alGetBufferi(buffer, AL_BITS, &bits);
		return bits;
	}
};

///////////////////////////////////////////////////////////////////////////////
//	MusicStreamPolled
//
///	A polled implementation of a streaming music file.
/// Plays an Ogg Vorbis file using OpenAL.
/// Calling code must continually call ::Update() to ensure the 
/// music keeps playing.
///
/// Note: For a version that doesn't require continuous polling,
/// use class "MusicStream" instead. ("MusicStream" uses 
/// "MusicStreamPolled internally, but automates polling itself.)
class MusicStreamPolled : public HasErrorState {
	
	// File access
	FILE			*file;
	OggVorbis_File	ogg;
	ov_callbacks	callbacks;			// Used by VorbisFile to access the file data.

	// File info
	ALsizei	freq;
	ALenum	format;

	// OpenAL objects
	ALuint			source;				// OpenAL source. Music is streamed through this source
	ALuint			*buffers;			// OpenAL buffers. Filled with music data and streamed into the source
	int				usedBufCount;		// # used buffers from buffers array

	// State
	bool initialised;
	bool looping;

	// Temporary buffer
	char			*data;

	// Not sure what this is, but LibVorbis wants it
	int				bitstream;

	void FillBuffer(ALuint buffer);
	int ReadData();
    void DoClose();

public:
	MusicStreamPolled();
	virtual ~MusicStreamPolled();

	// Control interface
	void OpenFile(std::string filename, float gain = 1, bool _looping = false);	// Open file and start playing
	void CloseFile();
    void SetGain(float gain);
	bool Playing();

	// Must be called periodically to keep stream playing
	void Update();
};	

enum MusicStreamCmdCode {
	MSC_Shutdown,
	MSC_OpenFile,
	MSC_CloseFile,
    MSC_SetGain};

struct MusicStreamCmd {
	MusicStreamCmdCode code;
	std::string filename;
	float gain;
	bool looping;
};

///////////////////////////////////////////////////////////////////////////////
//	MusicStream
//
///	A polled implementation of a streaming music file.
/// Plays an Ogg Vorbis file using OpenAL.
/// Automatically creates its own service thread to ensure the 
/// music keeps playing.
class MusicStream : public Threaded, public HasErrorState {
private:

	// Internal polled music stream
	MusicStreamPolled			*stream;

	// Service thread
	Thread						thread;

	// Command queue
	std::list<MusicStreamCmd>	commandQueue;

	// State
	bool						playing;

	// Thread synchronisation
	ThreadLock					commandQueueLock, stateLock;
	ThreadEvent					wakeEvent, readyEvent;

	void SendCmd(MusicStreamCmdCode code, std::string filename = "", float gain = 1, bool looping = false);

protected:

	// Main thread procedure
	virtual void ThreadExecute ();

public:
	MusicStream();
	virtual ~MusicStream();

	// Control interface
	void OpenFile(std::string filename, float gain = 1, bool _looping = false);	// Open file and start playing
	void CloseFile();
    void SetGain(float gain);
	bool Playing();

    // Error status
    void UpdateErrorState();
};

///////////////////////////////////////////////////////////////////////////////
//	SoundEngine
//
///	Main interface to the sound engine.

class SoundEngine : public HasErrorState {
private:	
	bool			initialised;		// True if OpenAL successfully initialised. (If false, we won't try to shut it down when we are destroyed!)

	int				voiceCount;
	ALuint			*voices;			// OpenAL voices.
	std::list<int>	queue;				// Queued voices. Each entry indexes into voices array.
	
	void RebuildQueue();
	ALuint FindFreeVoice();
public:
	SoundEngine(int _voiceCount);
	~SoundEngine();

	///	Play a sound. Returns index of voice chosen
	int PlaySound(const Sound& sound, float gain = 1, bool looped = false);	

	/// Return true if a voice is playing
	bool VoiceIsPlaying(int index);

	/// Stop a voice.
	void StopVoice(int index);

	/// Stop all voices
	void StopAll();
};

}
#endif