/*	SoundEngine

*/

#include "SoundEngine.h"
#include <assert.h>

using namespace std;
namespace SndEngine {

///////////////////////////////////////////////////////////////////////////////
//	Constants

const int MUSICSTREAMBUFFERS	= 64;
const int STREAMBLOCKSIZE		= 4096;
	// Note: This is about enough space for 1 second of 
	// stereo 44KHz 16bit music.
	// Our update frequency is 10 times per second, which
	// should be plenty enough to ensure that the stream
	// won't be interrupted.

///////////////////////////////////////////////////////////////////////////////
//	Helper functions

string GetALErrorString(ALenum error) {
	switch (error) {
    case AL_INVALID_NAME:       return "AL_INVALID_NAME: Invalid name";
    case AL_INVALID_ENUM:       return "AL_INVALID_ENUM: Invalid enumeration";
	case AL_INVALID_VALUE:		return "AL_INVALID_VALUE: Invalid parameter value";
	case AL_INVALID_OPERATION:	return "AL_INVALID_OPERATION: Invalid operation";
	case AL_OUT_OF_MEMORY:		return "AL_OUT_OF_MEMORY: Out of memory";
	default:					return "OpenAL error";
	}
}

string GetVorbisFileErrorString(int error) {
    switch (error) {
    case OV_EREAD:              return "OV_EREAD: Error reading from file";
    case OV_EFAULT:             return "OV_EFAULT: Internal VorbisFile fault";
    case OV_EIMPL:              return "OV_EIMPL";
    case OV_EINVAL:             return "OV_EINVAL";
    case OV_ENOTVORBIS:         return "OV_ENOTVORBIS: This is not an Ogg Vorbis file";
    case OV_EBADHEADER:         return "OV_EBADHEADER: Error in Vorbis file header";
    case OV_EVERSION:           return "OV_EVERSION: Vorbis version mismatch";
    case OV_ENOTAUDIO:          return "OV_ENOTAUDIO: This is not an audio Ogg Vorbis file";
    case OV_EBADPACKET:         return "OV_EBADPACKET";
    case OV_EBADLINK:           return "OV_EBADLINK";
    case OV_ENOSEEK:            return "OV_ENOSEEK: File is not seekable";
    default:                    return "VorbisFile error";
    }
}

// Vorbisfile callbacks
size_t ovCB_read(void *buf,unsigned int a,unsigned int b,void * fp) {
    return fread(buf,a,b,(FILE *)fp);
}

int ovCB_close(void * fp) {
    return fclose((FILE *)fp);
}

int ovCB_seek(void *fp,__int64 a,int b) {
    return fseek((FILE *)fp,(long)a,b);
}

long ovCB_tell(void *fp) {
    return ftell((FILE *)fp);
}

///////////////////////////////////////////////////////////////////////////////
//	Sound
Sound::Sound(string filename) {

	// Open file
    alutGetError();
	buffer = alutCreateBufferFromFile(filename.c_str());

	// Check for errors
	if (buffer == AL_NONE)
		SetError(alutGetErrorString(alutGetError()));
	else
		ClearError();
}

///////////////////////////////////////////////////////////////////////////////
//	MusicStreamPolled
MusicStreamPolled::MusicStreamPolled() : file(NULL), buffers(NULL), usedBufCount(0), initialised(false), data(NULL) {
	
	// Allocate source
	alGetError();
	alGenSources(1, &source);
    ALenum error = alGetError();
    if (error != AL_NO_ERROR) {
        SetError(GetALErrorString(error));
        return;
    }

	// Allocate buffers
	buffers = new ALuint[MUSICSTREAMBUFFERS];
	alGenBuffers(MUSICSTREAMBUFFERS, buffers);
    error = alGetError();
    if (error != AL_NO_ERROR) {
        SetError(GetALErrorString(error));
        return;
    }

	// Allocate temp data block
	data = new char[STREAMBLOCKSIZE];

	initialised = true;
}

MusicStreamPolled::~MusicStreamPolled() {

	// Close any open file
	CloseFile();

	// Free buffers
	if (buffers != NULL) {
		alDeleteBuffers(MUSICSTREAMBUFFERS, buffers);
		delete[] buffers;
	}

	// Close source	
	alDeleteSources(1, &source);

	// Free data
	if (data != NULL)
		delete[] data;
}

void MusicStreamPolled::FillBuffer(ALuint buffer) {
	int size = ReadData();
	alBufferData(buffer, format, data, size, freq);
}

int MusicStreamPolled::ReadData() {
	int offset = 0;
	bool justLooped = false;

	// Request data from Vorbisfile.
	// Vorbisfile doesn't always return as many bytes as we ask for, so we simply keep asking
	// until our data block is full, or the end of the file is reached.
	while (offset < STREAMBLOCKSIZE && file != NULL) {
		int read = ov_read(&ogg, &data[offset], STREAMBLOCKSIZE - offset, 0, 2, 1, &bitstream);
		if (read <= 0) {
			// Error or end of file.

            // Set error status (if any)
            if (read < 0)
                SetError(GetVorbisFileErrorString(read));
            else
                ClearError();

			// If EOF and we are looping, then loop
            if (read == 0 && looping && !justLooped) {
                int error = ov_raw_seek(&ogg, 0);
                if (error != 0) {
                    SetError(GetVorbisFileErrorString(error));
                    DoClose();
                }
                else
                    justLooped = true;  // This detects 0 length files. Otherwise we would end up with an infinite loop here!
            }
            else
                DoClose();              // Error or EOF (and not looping)
		}
		else {
			offset += read;
			justLooped = false;
		}
	}

	// Return number of bytes read
	return offset;
}

void MusicStreamPolled::DoClose() {

    // Cleanup and close file
    // Note: Unlike the public CloseFile() method, we don't stop the music
    // playing. We simply close the file.
    if (file != NULL) {
        ov_clear(&ogg);
        file = NULL;
        usedBufCount = 0;
    }
}

void MusicStreamPolled::OpenFile(string filename, float gain, bool _looping) {
	if (!initialised)
		return;

	looping = _looping;

	// Close any existing open file
	CloseFile();

	// Open file
	file = fopen(filename.c_str(), "rb");
	if (file == NULL) {
		SetError("Could not open file: " + filename);
		return;
	}

	// Build callbacks structure
	callbacks.read_func		= ovCB_read;
	callbacks.close_func	= ovCB_close;
	callbacks.seek_func		= ovCB_seek;
	callbacks.tell_func		= ovCB_tell;

	// Open vorbis
    int error = ov_open_callbacks(file, &ogg, NULL, 0, callbacks);
    if (error != 0) {
        SetError(GetVorbisFileErrorString(error));
        fclose(file);
        return;
    }
	bitstream = 0;

	// Extract file parameters
	vorbis_info *info = ov_info(&ogg, -1);
    if (info == NULL) {
        SetError("Unable to extract audio info from file");
        DoClose();
        return;
    }
	freq = info->rate;
	if (info->channels == 1)
		format = AL_FORMAT_MONO16;
	else
		format = AL_FORMAT_STEREO16;

	// Fill sound buffers
	while (file != NULL && usedBufCount < MUSICSTREAMBUFFERS) {
		FillBuffer(buffers[usedBufCount]);
		usedBufCount++;
	}
	
	// Queue them into source
    alGetError();
	alSourceQueueBuffers(source, usedBufCount, buffers);

	// Set the gain
	alSourcef(source, AL_GAIN, gain);

	// Play the source
	alSourcePlay(source);

    // Check for OpenAL errors
    error = alGetError();
    if (error != AL_NO_ERROR) {
        SetError(GetALErrorString(error));
        DoClose();
        return;
    }

    ClearError();
}

void MusicStreamPolled::CloseFile() {
	if (!initialised)
		return;

	// Stop playing
	if (Playing())
		alSourceStop(source);

	// Close file
    DoClose();

    // Clear error status
    ClearError();
}

void MusicStreamPolled::SetGain(float gain) {
    if (!initialised)
        return;
        
	// Set the gain
	alSourcef(source, AL_GAIN, gain);
}

bool MusicStreamPolled::Playing() {

	// If file is still being played, return true
	if (file != NULL)
		return true;
	else {

		// Otherwise, it's possible the file has run out, but the 
		// last buffers are still being played, so we must check 
		// for this also
		ALint state;
		alGetSourcei(source, AL_SOURCE_STATE, &state);
		return state == AL_PLAYING;
	}
}

void MusicStreamPolled::Update() {
	if (!initialised)
		return;

    // Check state is streaming state
    ALint sourceType;
    alGetSourcei(source, AL_SOURCE_TYPE, &sourceType);
    if (sourceType != AL_STREAMING)
        return;

	// Look for processed buffers
    ALint processed;
    alGetSourcei(source, AL_BUFFERS_PROCESSED, &processed);

    // Remove them
    if (processed > 0) {
        assert(processed <= MUSICSTREAMBUFFERS);
        ALuint processedBuffers[MUSICSTREAMBUFFERS];
        alSourceUnqueueBuffers(source, processed, processedBuffers);

        // Refill
        int count = 0;
        while (count < processed && file != NULL) {
            FillBuffer(processedBuffers[count]);
            count++;
        }

        // Requeue
        if (count > 0) {
            alGetError();
            alSourceQueueBuffers(source, count, processedBuffers);

            // Make sure the source keeps playing.
            // (This prevents lag hicups from stopping the music.
            // Otherwise the source could stop if the buffers run out.)
            int state;
            alGetSourcei(source, AL_SOURCE_STATE, &state);
            if (state != AL_PLAYING)
                alSourcePlay(source);

            // Check for OpenAL error
            ALenum error = alGetError();
            if (error != AL_NO_ERROR) {
                SetError(GetALErrorString(error));
                return;
            }
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
//	MusicStream
MusicStream::MusicStream() : playing(false), stream(NULL), readyEvent(true) {

	// Start service thread
	thread.Start(this);
}

MusicStream::~MusicStream() {

	// Stop service thread
	SendCmd(MSC_Shutdown);
	thread.WaitFor();
}

void MusicStream::SendCmd(MusicStreamCmdCode code, std::string filename, float gain, bool looping) {

	// Build command
	MusicStreamCmd cmd;
	cmd.code		= code;
	cmd.filename	= filename;
	cmd.gain		= gain;
	cmd.looping		= looping;

	// Add to command queue
	commandQueueLock.Lock();
	commandQueue.push_back(cmd);

    // Clear the ready event. This indicates that the object has not finished
    // processing commands yet, and is not ready to be examined.
    readyEvent.Reset();
	commandQueueLock.Unlock();

	// Wake up service thread
	wakeEvent.Set();
}

void MusicStream::ThreadExecute () {

	// Create stream
	stream = new MusicStreamPolled();

	// Main service loop
	bool shuttingDown = false;

    // State has been setup
	while (!shuttingDown) {

		// Process any queued commands
		bool foundCmd;
		do {
			// Look for command
			MusicStreamCmd cmd;
			commandQueueLock.Lock();
			if (commandQueue.size() > 0) {

				// Extract command
				cmd = commandQueue.front();
				commandQueue.pop_front();
				foundCmd = true;
			}
			else {
				foundCmd = false;

                // All commmands have been executed.
                // Set the "ready" event.
                // (This means that the object has been updated, and is ready
                // to be examined. Note that we do this while in the command
                // queue lock.)
                readyEvent.Set();
            }
			commandQueueLock.Unlock();

			// Process command
			if (foundCmd) {
                stateLock.Lock();
				switch (cmd.code) {
				case MSC_Shutdown:
					shuttingDown = true;
					break;

				case MSC_OpenFile:
					stream->OpenFile(cmd.filename, cmd.gain, cmd.looping);
					break;

				case MSC_CloseFile:
					stream->CloseFile();
					break;

                case MSC_SetGain:
                    stream->SetGain(cmd.gain);
                    break;
				}
                stateLock.Unlock();
			}
		} while (foundCmd);

		// Update music stream, to ensure music keeps playing
        stateLock.Lock();
		stream->Update();
        stateLock.Unlock();

        // Sleep until worken.
        // If the stream is playing, we wake every 100ms regardless, so that we
        // can update the stream and keep the music playing.
        // Otherwise we wait indefinitely.
		if (!shuttingDown) {
			if (stream->Playing())
				wakeEvent.WaitFor(100);
			else
				wakeEvent.WaitFor();
		}
	}

	// Close stream
	delete stream;
	stream = NULL;
}

void MusicStream::OpenFile(std::string filename, float gain, bool _looping) {
	SendCmd(MSC_OpenFile, filename, gain, _looping);
}

void MusicStream::CloseFile() {
	SendCmd(MSC_CloseFile);
}

void MusicStream::SetGain(float gain) {
    SendCmd(MSC_SetGain, "", gain);
}

bool MusicStream::Playing() {
    readyEvent.WaitFor();
	stateLock.Lock();
	bool result = stream->Playing();
	stateLock.Unlock();
    return result;
}

void MusicStream::UpdateErrorState() {
    readyEvent.WaitFor();
	stateLock.Lock();
    if (stream->Error())
        SetError(stream->ErrorText());
    else
        ClearError();
	stateLock.Unlock();
}

///////////////////////////////////////////////////////////////////////////////
//	SoundEngine
SoundEngine::SoundEngine(int _voiceCount) : voiceCount(_voiceCount) {
	assert(voiceCount > 0);
	assert(voiceCount <= 1000);
	
	// Initialise OpenAL
    alutGetError();
	if (!alutInit(NULL, NULL)) {
		SetError(alutGetErrorString(alutGetError()));
		voices = NULL;
		initialised = false;
		return;
	}

	// Allocate voices
	voices = new ALuint[voiceCount];
    alGetError();
	alGenSources(voiceCount, voices);
    ALenum error = alGetError();
    if (error != AL_NO_ERROR) {
        SetError(GetALErrorString(error));
        delete[] voices;
		alutExit();
        return;
    }

	// Setup voice queue
	RebuildQueue();	

	initialised = true;
}

SoundEngine::~SoundEngine() {

	// Stop voices playing
	if (initialised) {
		StopAll();

		// Delete voices
		alDeleteSources(voiceCount, voices);
		delete[] voices;

		// Shut down OpenAL
        alutGetError();
		alutExit();
	}
}

void SoundEngine::RebuildQueue() {
	
	// Rebuild queue of voices
	queue.clear();
	for (int i = 0; i < voiceCount; i++)
		queue.push_back(i);
}

ALuint SoundEngine::FindFreeVoice() {
	
	// Find a free voice

	// First look for a free voice that isn't playing
	list<int>::iterator i;
	for (
		i = queue.begin();
		i != queue.end();
		i++) {

		// Find source
		int index = *i;
		ALuint source = voices[index];

		// Check if it's playing
		int state;
		alGetSourcei(source, AL_SOURCE_STATE, &state);
		if (state != AL_PLAYING) {

			// Move source to back of queue
			queue.erase(i);
			queue.push_back(index);

			// Use this source as our "voice"
			return index;
		}
	}

	// No non-playing source found.
	// Now we look for the oldest playing source that isn't looping
	for (
		i = queue.begin();
		i != queue.end();
		i++) {

		// Find source
		int index = *i;
		ALuint source = voices[index];

		// Check if it's looping
		int looping;
		alGetSourcei(source, AL_LOOPING, &looping);
		if (!looping) {

			// Stop old sound
			alSourceStop(source);

			// Move source to back of queue
			queue.erase(i);
			queue.push_back(index);

			// Use this source as our "voice"
			return index;
		}
	}

	// All sounds are playing and looping.
	// Just use oldest voice
	int index = queue.front();
	ALuint source = voices[index];

	// Stop old sound
	alSourceStop(source);

	// Move source to back of queue
	queue.erase(queue.begin());
	queue.push_back(index);

	return index;
}

int SoundEngine::PlaySound(const Sound& sound, float gain, bool looped) {
	if (!initialised)
		return -1;

	if (sound.Error()) {
		SetError("Error opening sound: " + sound.ErrorText());
		return -1;
	}

	// Find a suitable voice
	int index = FindFreeVoice();
	ALuint source = voices[index];

	// Set looping state
	alSourcei(source, AL_LOOPING, looped ? AL_TRUE : AL_FALSE);

	// Set gain
	alSourcef(source, AL_GAIN, gain);
	
	// Connect the sound buffer
	alSourcei(source, AL_BUFFER, sound.Buffer());

	// Play it
	alSourcePlay(source);

	ClearError();
	return index;
}

void SoundEngine::StopVoice(int index) {
	if (!initialised)
		return;

	if (index >= 0 && index < voiceCount)
		alSourceStop(voices[index]);
}

void SoundEngine::StopAll() {
	if (!initialised)
		return;

	for (int i = 0; i < voiceCount; i++)
		alSourceStop(voices[i]);
	RebuildQueue();
}

bool SoundEngine::VoiceIsPlaying(int index) {
	if (!initialised)
		return false;

	if (index >= 0 && index < voiceCount) {
		ALuint source = voices[index];
		int state;
		alGetSourcei(source, AL_SOURCE_STATE, &state);
		return state == AL_PLAYING;
	}
	else
		return false;
}

}
