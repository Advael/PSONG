/*
Copyright (C) 2012 John Nesky

Permission is hereby granted, free of charge, to any person obtaining a copy of 
this software and associated documentation files (the "Software"), to deal in 
the Software without restriction, including without limitation the rights to 
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies 
of the Software, and to permit persons to whom the Software is furnished to do 
so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all 
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR 
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE 
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, 
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE 
SOFTWARE.
*/
#include <string>
#include <vector>
struct Music {
//	static const int scaleNames{"easy :)", "easy :(", "island :)", "island :(", "blues :)", "blues :(", "normal :)", "normal :(", "dbl harmonic :)", "dbl harmonic :(", "enigma", "expert"};
	static const vector <vector<bool>> scaleFlags {
		{ true, false,  true, false,  true, false, false,  true, false,  true, false, false},
		{ true, false, false,  true, false,  true, false,  true, false, false,  true, false},
		{ true, false, false, false,  true,  true, false,  true, false, false, false,  true},
		{ true,  true, false,  true, false, false, false,  true,  true, false, false, false},
		{ true, false,  true,  true,  true, false, false,  true, false,  true, false, false},
		{ true, false, false,  true, false,  true,  true,  true, false, false,  true, false},
		{ true, false,  true, false,  true,  true, false,  true, false,  true, false,  true},
		{ true, false,  true,  true, false,  true, false,  true,  true, false,  true, false},
		{ true,  true, false, false,  true,  true, false,  true,  true, false, false,  true},
		{ true, false,  true,  true, false, false,  true,  true,  true, false, false,  true},
		{ true, false,  true, false,  true, false,  true, false,  true, false,  true, false},
		{ true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true}
	};
	static const vector <bool> pianoScaleFlags = { true, false,  true, false,  true,  true, false,  true, false,  true, false,  true};
	// C1 has index 24 on the MIDI scale. C8 is 108, and C9 is 120. C10 is barely in the audible range.
	static const vector <string> keyNames= {"B", "A#", "A", "G#", "G", "F#", "F", "E", "D#", "D", "C#", "C"};
	static const vector <int> keyTransposes= {23, 22, 21, 20, 19, 18, 17, 16, 15, 14, 13, 12};
	static const vector <string> tempoNames = {"molasses", "slow", "leisurely", "moderate", "steady", "brisk", "hasty", "fast", "strenuous", "grueling", "hyper", "ludicrous"};
	static const int reverbRange = 4;
	static const int beatsMin = 3;
	static const int beatsMax = 15;
	static const int barsMin = 1;
	static const int barsMax = 128;
	static const int patternsMin = 1;
	static const int patternsMax = 64;
	static const int instrumentsMin = 1;
	static const int instrumentsMax = 10;
	static const vector <string> partNames = {"triples", "standard"};
	static const vector <int> partCounts = {3, 4};
	static const vector <string> pitchNames = {"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};
	static const vector <string> waveNames = {"triangle", "square", "pulse wide", "pulse narrow", "sawtooth", "double saw", "double pulse", "spiky", "plateau"};
	static const vector <float> waveVolumes = {1.0, 0.5, 0.5, 0.5, 0.65, 0.5, 0.4, 0.4, 0.94};
	static const vector <string> drumNames = {"retro", "white"};
	static const vector drumVolumes = {0.25, 1.0};
	static const filterNames = {"sustain sharp", "sustain medium", "sustain soft", "decay sharp", "decay medium", "decay soft"};
	static const filterBases = {2.0, 3.5, 5.0, 1.0, 2.5, 4.0};
	static const filterDecays = {0.0, 0.0, 0.0, 10.0, 7.0, 4.0};
	static const filterVolumes = {0.4, 0.7, 1.0, 0.5, 0.75, 1.0};
	static const attackNames = {"binary", "sudden", "smooth", "slide"};
	static const effectNames = {"none", "vibrato light", "vibrato delayed", "vibrato heavy", "tremelo light", "tremelo heavy"};
	static const effectVibratos = {0.0, 0.15, 0.3, 0.45, 0.0, 0.0};
	static const effectTremelos = {0.0, 0.0, 0.0, 0.0, 0.25, 0.5};
	static const chorusNames = {"union", "shimmer", "hum", "honky tonk", "dissonant", "fifths", "octaves", "bowed"};
	static const chorusValues = {0.0, 0.02, 0.05, 0.1, 0.25, 3.5, 6, 0.02};
	static const chorusOffsets = {0.0, 0.0, 0.0, 0.0, 0.0, 3.5, 6, 0.0};
	static const chorusVolumes = {0.7, 0.8, 1.0, 1.0, 0.9, 0.9, 0.8, 1.0};
	static const volumeNames = {"loudest", "loud", "medium", "quiet", "quietest", "mute"};
	static const volumeValues = {0.0, 0.5, 1.0, 1.5, 2.0, -1.0};
	static const channelVolumes = {0.27, 0.27, 0.27, 0.19};
	static const drumInterval = 6;
	static const numChannels = 4;
	static const drumCount = 12;
	static const pitchCount = 37;
	static const maxPitch = 84;
}
