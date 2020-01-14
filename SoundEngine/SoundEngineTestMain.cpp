#include "SoundEngine.h"
#include <iostream>
#include <conio.h>

using namespace std;
using namespace SndEngine;

int main(void) {
	cout << "Starting sound engine" << endl;
	SoundEngine engine(8);
	float gain = 0.5f;

	// Load some sounds
	Sound *sounds[4];
	sounds[0] = new Sound("INSECTS.wav");
	sounds[1] = new Sound("BARBPOLE.wav");
	sounds[2] = new Sound("BKSPCGTR.wav");
	sounds[3] = new Sound("C3CANNON.wav");

	// Play some music
	MusicStream music;
	music.OpenFile("bitten.ogg", 0.5f, true);

	bool done = false;
	bool loop = false;
	while (!done) {
		if (kbhit()) {
			char key = getch();
			if (key >= '1' && key <= '4')
				engine.PlaySound(*sounds[key - '1'], gain, loop);
			else if (key == 'l' || key == 'L')
				loop = !loop;
			else if (key == 's' || key == 'S')
				engine.StopAll();
			else if (key == 27)
				done = true;
			else if (key == '+' || key == '=')
				gain += 0.1f;
			else if (key == '-' && gain > 0.1f)
				gain -= 0.1f;
		}
		
		for (int i = 0; i < 8; i++) {
			if (engine.VoiceIsPlaying(i))
				putch('1' + i);
			else
				putch(' ');
		}
		if (loop) 
			cprintf(" [loop]");
		else
			cprintf("       ");
		cprintf(" [gain = %f]    ", gain);
		putch('\r');
	}

	// Clean up
	engine.StopAll();
	for (int i = 0; i < 4; i++) 
		delete sounds[i];

	return 0;
}