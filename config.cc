/*Turns out you can't initialize big static vectors in C++ classes. So I put the initializations here instead*/

#include "config.h"
using std::string;
const vector<string> Config :: scaleNames = {"easy :)", "easy :(", "island :)", "island :(", "blues :)", "blues :(", "normal :)", "normal :(", "dbl harmonic :)", "dbl harmonic :(", "enigma", "expert"};
const vector<vector<bool>> Config :: scaleFlags  = {
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
const vector<bool> Config :: pianoScaleFlags = { true, false,  true, false,  true,  true, false,  true, false,  true, false,  true};
const vector<int> Config :: blackKeyNameParents = {-1, 1, -1, 1, -1, 1, -1, -1, 1, -1, 1, -1};
const vector<string> Config :: pitchNames = {"C", "", "D", "", "E", "F", "", "G", "", "A", "", "B"};
const vector<string> Config :: keyNames = {"B", "A♯", "A", "G♯", "G", "F♯", "F", "E", "D♯", "D", "C♯", "C"};
// C1 has index 24 on the MIDI scale. C8 is 108, and C9 is 120. C10 is barely in the audible range.
const vector<int> Config :: keyTransposes = {23, 22, 21, 20, 19, 18, 17, 16, 15, 14, 13, 12};
const vector<string> Config :: partNames = {"÷3 (triplets)", "÷4 (standard)", "÷6", "÷8"};
const vector<int> Config :: partCounts = {3, 4, 6, 8};
const vector<string> Config :: waveNames = {"triangle", "square", "pulse wide", "pulse narrow", "sawtooth", "double saw", "double pulse", "spiky", "plateau"};
const vector<float> Config :: waveVolumes = {1.0,         0.5,       0.5,          0.5,          0.65,          0.5,          0.4,         0.4,      0.94};
// the "clang" and "buzz" drums are inspired by similar drums in the modded beepbox! :D
const vector<string> Config :: drumNames = {"retro", "white", "clang", "buzz", "hollow" /*"tom-tom", "cymbal", "bass"*/};
const vector<float> Config :: drumVolumes = {0.25, 1.0, 0.4, 0.3, 1.5 /*, 1.5, 1.5, 1.5*/};
const vector<float> Config :: drumBasePitches = {69, 69, 69, 69, 96 /*, 96, 90, 126*/};
const vector<float> Config :: drumPitchFilterMult = {100.0, 8.0, 100.0, 100.0, 1.0 /*, 1.0, 1.0, 1.0*/};
const vector<bool> Config :: drumWaveIsSoft = {false, true, false, false, true /*, true, true, true*/};
// Noise waves have too many samples to write by hand, they're generated on-demand by getDrumWave instead.
const vector<string> Config :: filterNames = {"none", "bright", "medium", "soft", "decay bright", "decay medium", "decay soft"};
const vector<float> Config :: filterBases = {0.0, 2.0, 3.5, 5.0, 1.0, 2.5, 4.0};
const vector<float> Config :: filterDecays = {0.0, 0.0, 0.0, 0.0, 10.0, 7.0, 4.0};
const vector<float> Config :: filterVolumes = {0.2, 0.4, 0.7, 1.0, 0.5, 0.75, 1.0};
const vector<string> Config :: transitionNames = {"seamless", "sudden", "smooth", "slide"};
const vector<string> Config :: effectNames = {"none", "vibrato light", "vibrato delayed", "vibrato heavy", "tremolo light", "tremolo heavy"};
const vector<float> Config :: effectVibratos = {0.0, 0.15, 0.3, 0.45, 0.0, 0.0};
const vector<float> Config :: effectTremolos = {0.0, 0.0, 0.0, 0.0, 0.25, 0.5};
const vector<float> Config :: effectVibratoDelays = {0, 0, 3, 0, 0, 0};
const vector<string> Config :: chorusNames = {"union", "shimmer", "hum", "honky tonk", "dissonant", "fifths", "octaves", "bowed", "custom harmony"};
const vector<float> Config :: chorusIntervals = {0.0, 0.02, 0.05, 0.1, 0.25, 3.5, 6, 0.02, 0.05};
const vector<float> Config :: chorusOffsets = {0.0, 0.0, 0.0, 0.0, 0.0, 3.5, 6, 0.0, 0.0};
const vector<float> Config :: chorusVolumes = {0.7, 0.8, 1.0, 1.0, 0.9, 0.9, 0.8, 1.0, 1.0};
const vector<float> Config :: chorusSigns = {1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, -1.0, 1.0};
const vector<bool> Config :: chorusHarmonizes = {false, false, false, false, false, false, false, false, true};
const vector<string> Config :: volumeNames = {"loudest", "loud", "medium", "quiet", "quietest", "mute"};
const vector<float> Config :: volumeValues = {0.0, 0.5, 1.0, 1.5, 2.0, -1.0};
const vector<string> Config :: operatorAlgorithmNames = {
	"1←(2 3 4)",
	"1←(2 3←4)",
	"1←2←(3 4)",
	"1←(2 3)←4",
	"1←2←3←4",
	"1←3 2←4",
	"1 2←(3 4)",
	"1 2←3←4",
	"(1 2)←3←4",
	"(1 2)←(3 4)",
	"1 2 3←4",
	"(1 2 3)←4",
	"1 2 3 4",
};
const vector<string> Config :: midiAlgorithmNames = {"1<(2 3 4)", "1<(2 3<4)", "1<2<(3 4)", "1<(2 3)<4", "1<2<3<4", "1<3 2<4", "1 2<(3 4)", "1 2<3<4", "(1 2)<3<4", "(1 2)<(3 4)", "1 2 3<4", "(1 2 3)<4", "1 2 3 4"};
const vector<vector<vector<float>>> Config :: operatorModulatedBy = {
	{{2, 3, 4}, {},     {},  {}},
	{{2, 3},    {},     {4}, {}},
	{{2},       {3, 4}, {},  {}},
	{{2, 3},    {4},    {4}, {}},
	{{2},       {3},    {4}, {}},
	{{3},       {4},    {},  {}},
	{{},        {3, 4}, {},  {}},
	{{},        {3},    {4}, {}},
	{{3},       {3},    {4}, {}},
	{{3, 4},    {3, 4}, {},  {}},
	{{},        {},     {4}, {}},
	{{4},       {4},    {4}, {}},
	{{},        {},     {},  {}},
};
const vector<vector<float>> Config :: operatorAssociatedCarrier = {
	{1, 1, 1, 1},
	{1, 1, 1, 1},
	{1, 1, 1, 1},
	{1, 1, 1, 1},
	{1, 1, 1, 1},
	{1, 2, 1, 2},
	{1, 2, 2, 2},
	{1, 2, 2, 2},
	{1, 2, 2, 2},
	{1, 2, 2, 2},
	{1, 2, 3, 3},
	{1, 2, 3, 3},
	{1, 2, 3, 4},
};
const vector<float> Config :: operatorCarrierCounts = {1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 3, 3, 4};
const vector<float> Config :: operatorCarrierChorus = {0.0, 0.04, -0.073, 0.091};
const vector<string> Config :: operatorFrequencyNames = {"1×", "~1×", "2×", "~2×", "3×", "4×", "5×", "6×", "7×", "8×", "9×", "11×", "13×", "16×", "20×"};
const vector<string> Config :: midiFrequencyNames = {"1x", "~1x", "2x", "~2x", "3x", "4x", "5x", "6x", "7x", "8x", "9x", "11x", "13x", "16x", "20x"};
const vector<float> Config :: operatorFrequencies =    { 1.0,   1.0,   2.0,   2.0,  3.0,  4.0,  5.0,  6.0,  7.0,  8.0,  9.0, 11.0, 13.0, 16.0, 20.0};
const vector<float> Config :: operatorHzOffsets =      { 0.0,   1.5,   0.0,  -1.3,  0.0,  0.0,  0.0,  0.0,  0.0,  0.0,  0.0,  0.0,  0.0,  0.0,  0.0};
const vector<float> Config :: operatorAmplitudeSigns = { 1.0,  -1.0,   1.0,  -1.0,  1.0,  1.0,  1.0,  1.0,  1.0,  1.0,  1.0,  1.0,  1.0,  1.0,  1.0};
const vector<string> Config :: operatorEnvelopeNames = {"custom", "steady", "punch", "flare 1", "flare 2", "flare 3", "pluck 1", "pluck 2", "pluck 3", "swell 1", "swell 2", "swell 3", "tremolo1", "tremolo2", "tremolo3"};
const vector<EnvelopeType> Config :: operatorEnvelopeType = {EnvelopeType::custom, EnvelopeType::steady, EnvelopeType::punch, EnvelopeType::flare, EnvelopeType::flare, EnvelopeType::flare, EnvelopeType::pluck, EnvelopeType::pluck, EnvelopeType::pluck, EnvelopeType::pluck, EnvelopeType::pluck, EnvelopeType::pluck, EnvelopeType::tremolo, EnvelopeType::tremolo, EnvelopeType::tremolo};
const vector<float> Config :: operatorEnvelopeSpeed = {0.0, 0.0, 0.0, 32.0, 8.0, 2.0, 32.0, 8.0, 2.0, 32.0, 8.0, 2.0, 4.0, 2.0, 1.0};
const vector<bool> Config :: operatorEnvelopeInverted = {false, false, false, false, false, false, false, false, false, true, true, true, false, false, false};
const vector<string> Config :: operatorFeedbackNames = {
	"1⟲",
	"2⟲",
	"3⟲",
	"4⟲",
	"1⟲ 2⟲",
	"3⟲ 4⟲",
	"1⟲ 2⟲ 3⟲",
	"2⟲ 3⟲ 4⟲",
	"1⟲ 2⟲ 3⟲ 4⟲",
	"1→2",
	"1→3",
	"1→4",
	"2→3",
	"2→4",
	"3→4",
	"1→3 2→4",
	"1→4 2→3",
	"1→2→3→4",
};
const vector<string> Config :: midiFeedbackNames = {
	"1",
	"2",
	"3",
	"4",
	"1 2",
	"3 4",
	"1 2 3",
	"2 3 4",
	"1 2 3 4",
	"1>2",
	"1>3",
	"1>4",
	"2>3",
	"2>4",
	"3>4",
	"1>3 2>4",
	"1>4 2>3",
	"1>2>3>4",
};
const vector<vector<vector<float>>> Config :: operatorFeedbackIndices = {
	{{1}, {}, {}, {}},
	{{}, {2}, {}, {}},
	{{}, {}, {3}, {}},
	{{}, {}, {}, {4}},
	{{1}, {2}, {}, {}},
	{{}, {}, {3}, {4}},
	{{1}, {2}, {3}, {}},
	{{}, {2}, {3}, {4}},
	{{1}, {2}, {3}, {4}},
	{{}, {1}, {}, {}},
	{{}, {}, {1}, {}},
	{{}, {}, {}, {1}},
	{{}, {}, {2}, {}},
	{{}, {}, {}, {2}},
	{{}, {}, {}, {3}},
	{{}, {}, {1}, {2}},
	{{}, {}, {2}, {1}},
	{{}, {1}, {2}, {3}},
};
const vector<string> Config :: pitchChannelTypeNames = {"chip", "FM (expert)"};
const vector<string> Config :: instrumentTypeNames = {"chip", "FM", "noise"};
const vector<string> Config :: pitchChannelColorsDim    = {"#0099a1", "#a1a100", "#c75000", "#00a100", "#d020d0", "#7777b0"};
const vector<string> Config :: pitchChannelColorsBright = {"#25f3ff", "#ffff25", "#ff9752", "#50ff50", "#ff90ff", "#a0a0ff"};
const vector<string> Config :: pitchNoteColorsDim       = {"#00bdc7", "#c7c700", "#ff771c", "#00c700", "#e040e0", "#8888d0"};
const vector<string> Config :: pitchNoteColorsBright    = {"#92f9ff", "#ffff92", "#ffcdab", "#a0ffa0", "#ffc0ff", "#d0d0ff"};
const vector<string> Config :: drumChannelColorsDim    = {"#6f6f6f", "#996633"};
const vector<string> Config :: drumChannelColorsBright = {"#aaaaaa", "#ddaa77"};
const vector<string> Config :: drumNoteColorsDim       = {"#aaaaaa", "#cc9966"};
const vector<string> Config :: drumNoteColorsBright    = {"#eeeeee", "#f0d0bb"};
const vector<string> Config :: midiPitchChannelNames = {"cyan channel", "yellow channel", "orange channel", "green channel", "purple channel", "blue channel"};
const vector<string> Config :: midiDrumChannelNames = {"gray channel", "brown channel"};
const vector<int> Config :: midiSustainInstruments = { 
	0x47, // triangle -> clarinet
	0x50, // square -> square wave
	0x46, // pulse wide -> bassoon
	0x44, // pulse narrow -> oboe
	0x51, // sawtooth -> sawtooth wave
	0x51, // double saw -> sawtooth wave
	0x51, // double pulse -> sawtooth wave
	0x51, // spiky -> sawtooth wave
	0x4A, // plateau -> recorder
};
const vector<int> Config :: midiDecayInstruments = {
	0x2E, // triangle -> harp
	0x2E, // square -> harp
	0x06, // pulse wide -> harpsichord
	0x18, // pulse narrow -> nylon guitar
	0x19, // sawtooth -> steel guitar
	0x19, // double saw -> steel guitar
	0x6A, // double pulse -> shamisen
	0x6A, // spiky -> shamisen
	0x21, // plateau -> fingered bass
};
const vector<vector<double>> Config :: waves = {
	_centerWave({1.0/15.0, 3.0/15.0, 5.0/15.0, 7.0/15.0, 9.0/15.0, 11.0/15.0, 13.0/15.0, 15.0/15.0, 15.0/15.0, 13.0/15.0, 11.0/15.0, 9.0/15.0, 7.0/15.0, 5.0/15.0, 3.0/15.0, 1.0/15.0, -1.0/15.0, -3.0/15.0, -5.0/15.0, -7.0/15.0, -9.0/15.0, -11.0/15.0, -13.0/15.0, -15.0/15.0, -15.0/15.0, -13.0/15.0, -11.0/15.0, -9.0/15.0, -7.0/15.0, -5.0/15.0, -3.0/15.0, -1.0/15.0}),
	_centerWave({1.0, -1.0}),
	_centerWave({1.0, -1.0, -1.0, -1.0}),
	_centerWave({1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0}),
	_centerWave({1.0/31.0, 3.0/31.0, 5.0/31.0, 7.0/31.0, 9.0/31.0, 11.0/31.0, 13.0/31.0, 15.0/31.0, 17.0/31.0, 19.0/31.0, 21.0/31.0, 23.0/31.0, 25.0/31.0, 27.0/31.0, 29.0/31.0, 31.0/31.0, -31.0/31.0, -29.0/31.0, -27.0/31.0, -25.0/31.0, -23.0/31.0, -21.0/31.0, -19.0/31.0, -17.0/31.0, -15.0/31.0, -13.0/31.0, -11.0/31.0, -9.0/31.0, -7.0/31.0, -5.0/31.0, -3.0/31.0, -1.0/31.0}),
	_centerWave({0.0, -0.2, -0.4, -0.6, -0.8, -1.0, 1.0, -0.8, -0.6, -0.4, -0.2, 1.0, 0.8, 0.6, 0.4, 0.2}),
	_centerWave({1.0, 1.0, 1.0, 1.0, 1.0, -1.0, -1.0, -1.0, 1.0, 1.0, 1.0, 1.0, -1.0, -1.0, -1.0, -1.0}),
	_centerWave({1.0, -1.0, 1.0, -1.0, 1.0, 0.0}),
	_centerWave({0.0, 0.2, 0.4, 0.5, 0.6, 0.7, 0.8, 0.85, 0.9, 0.95, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 0.95, 0.9, 0.85, 0.8, 0.7, 0.6, 0.5, 0.4, 0.2, 0.0, -0.2, -0.4, -0.5, -0.6, -0.7, -0.8, -0.85, -0.9, -0.95, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -0.95, -0.9, -0.85, -0.8, -0.7, -0.6, -0.5, -0.4, -0.2}),
};
const vector<double> Config :: sineWave = generateSineWave();

