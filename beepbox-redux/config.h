#include <string>
#include <vector>
#include <random>
#include <math.h>
#include "FFT.h"
using std::string;
using std::vector;
enum struct EnvelopeType {
	custom,
	steady,
	punch,
	flare,
	pluck,
	tremolo,
};

enum struct InstrumentType {
	chip = 0,
	fm = 1,
	noise = 2,
};


enum struct CharCode {
	SPACE = 32,
	HASH = 35,
	PERCENT = 37,
	AMPERSAND = 38,
	PLUS = 43,
	DASH = 45,
	DOT = 46,
	NUM_0 = 48,
	NUM_1 = 49,
	NUM_2 = 50,
	NUM_3 = 51,
	NUM_4 = 52,
	NUM_5 = 53,
	NUM_6 = 54,
	NUM_7 = 55,
	NUM_8 = 56,
	NUM_9 = 57,
	EQUALS = 61,
	A =  65,
	B =  66,
	C =  67,
	D =  68,
	E =  69,
	F =  70,
	G =  71,
	H =  72,
	I =  73,
	J =  74,
	K =  75,
	L =  76,
	M =  77,
	N =  78,
	O =  79,
	P =  80,
	Q =  81,
	R =  82,
	S =  83,
	T =  84,
	U =  85,
	V =  86,
	W =  87,
	X =  88,
	Y =  89,
	Z =  90,
	UNDERSCORE = 95,
	a =  97,
	b =  98,
	c =  99,
	d = 100,
	e = 101,
	f = 102,
	g = 103,
	h = 104,
	i = 105,
	j = 106,
	k = 107,
	l = 108,
	m = 109,
	n = 110,
	o = 111,
	p = 112,
	q = 113,
	r = 114,
	s = 115,
	t = 116,
	u = 117,
	v = 118,
	w = 119,
	x = 120,
	y = 121,
	z = 122,
	LEFT_CURLY_BRACE = 123,
	RIGHT_CURLY_BRACE = 125,
};

enum struct SongTagCode {
	beatCount = int(CharCode::a),
	bars = int(CharCode::b),
	effect = int(CharCode::c),
	transition = int(CharCode::d),
	loopEnd = int(CharCode::e),
	filter = int(CharCode::f),
	barCount = int(CharCode::g),
	chorus = int(CharCode::h),
	instrumentCount = int(CharCode::i),
	patternCount = int(CharCode::j),
	key = int(CharCode::k),
	loopStart = int(CharCode::l),
	reverb = int(CharCode::m),
	channelCount = int(CharCode::n),
	channelOctave = int(CharCode::o),
	patterns = int(CharCode::p),
	
	rhythm = int(CharCode::r),
	scale = int(CharCode::s),
	tempo = int(CharCode::t),
	
	volume = int(CharCode::v),
	wave = int(CharCode::w),
	
	algorithm = int(CharCode::A),
	feedbackAmplitude = int(CharCode::B),
	operatorEnvelopes = int(CharCode::E),
	feedbackType = int(CharCode::F),
	operatorAmplitudes = int(CharCode::P),
	operatorFrequencies = int(CharCode::Q),
	startInstrument = int(CharCode::T),
	feedbackEnvelope = int(CharCode::V),
};

struct Config {
	static const vector<string> scaleNames; 
	static const vector<vector<bool>> scaleFlags;
	static const vector<bool> pianoScaleFlags;
	static const vector<int> blackKeyNameParents ;
	static const vector<string> pitchNames; 
	static const vector<string> keyNames ;
	// C1 has index 24 on the MIDI scale. C8 is 108, and C9 is 120. C10 is barely in the audible range.
	static const vector<int> keyTransposes ;
	static const int temposteps = 15;
	static const int reverbRange = 4;
	static const int beatsPerBarMin = 3;
	static const int beatsPerBarMax = 16;
	static const int barCountMin = 1;
	static const int barCountMax = 128;
	static const int patternsPerChannelMin = 1;
	static const int patternsPerChannelMax = 64;
	static const int instrumentsPerChannelMin = 1;
	static const int instrumentsPerChannelMax = 10;
	static const vector<string> partNames ;
	static const vector<int> partCounts ;
	static const vector<string> waveNames ;
	static const vector<float> waveVolumes ;
	// the "clang" and "buzz" drums are inspired by similar drums in the modded beepbox! :D
	static const vector<string> drumNames ;
	static const vector<float> drumVolumes ;
	static const vector<float> drumBasePitches ;
	static const vector<float> drumPitchFilterMult ;
	static const vector<bool> drumWaveIsSoft ;
	// Noise waves have too many samples to write by hand, they're generated on-demand by getDrumWave instead.
	static const vector<string> filterNames ;
	static const vector<float> filterBases ;
	static const vector<float> filterDecays ;
	static const vector<float> filterVolumes ;
	static const vector<string> transitionNames ;
	static const vector<string> effectNames ;
	static const vector<float> effectVibratos ;
	static const vector<float> effectTremolos ;
	static const vector<float> effectVibratoDelays ;
	static const vector<string> chorusNames ;
	static const vector<float> chorusIntervals ;
	static const vector<float> chorusOffsets ;
	static const vector<float> chorusVolumes ;
	static const vector<float> chorusSigns ;
	static const vector<bool> chorusHarmonizes ;
	static const vector<string> volumeNames ;
	static const vector<float> volumeValues ;
	static const int operatorCount = 4;
	static const vector<string> operatorAlgorithmNames;
	static const vector<string> midiAlgorithmNames ;
	static const vector<vector<vector<float>>> operatorModulatedBy;
	static const vector<vector<float>> operatorAssociatedCarrier;
	static const vector<float> operatorCarrierCounts ;
	static const vector<float> operatorCarrierChorus ;
	static const int operatorAmplitudeMax = 15;
	static const vector<string> operatorFrequencyNames ;
	static const vector<string> midiFrequencyNames ;
	static const vector<float> operatorFrequencies ;
	static const vector<float> operatorHzOffsets ;
	static const vector<float> operatorAmplitudeSigns ;
	static const vector<string> operatorEnvelopeNames ;
	static const vector<EnvelopeType> operatorEnvelopeType ;
	static const vector<float> operatorEnvelopeSpeed ;
	static const vector<bool> operatorEnvelopeInverted ;
	static const vector<string> operatorFeedbackNames;
	static const vector<string> midiFeedbackNames;
	static const vector<vector<vector<float>>> operatorFeedbackIndices ;
	static const vector<string> pitchChannelTypeNames ;
	static const vector<string> instrumentTypeNames ;
	static const vector<string> pitchChannelColorsDim    ;
	static const vector<string> pitchChannelColorsBright ;
	static const vector<string> pitchNoteColorsDim       ;
	static const vector<string> pitchNoteColorsBright    ;
	static const vector<string> drumChannelColorsDim;
	static const vector<string> drumChannelColorsBright;
	static const vector<string> drumNoteColorsDim;
	static const vector<string> drumNoteColorsBright;
	static const vector<string> midiPitchChannelNames;
	static const vector<string> midiDrumChannelNames;
	static const vector<int> midiSustainInstruments;
	static const vector<int> midiDecayInstruments;
	static const int drumInterval = 6;
	static const int drumCount = 12;
	static const int pitchCount = 37;
	static const int maxPitch = 84;
	static const int pitchChannelCountMin = 1;
	static const int pitchChannelCountMax = 6;
	static const int drumChannelCountMin = 0;
	static const int drumChannelCountMax = 2;
	static const vector<vector<double>> waves;
	static const unsigned int sineWaveLength = 1 << 8; // 256
	static const unsigned int sineWaveMask = sineWaveLength - 1;
	static const vector<double> sineWave;

	static vector<double> getDrumWave(int index) {
		vector<double> wave;
		if (wave.empty()) {
			wave = vector<double>(32768);
			if (index == 0) {
				// The "retro" drum uses a "Linear Feedback Shift Register" similar to the NES noise channel.
				unsigned int drumBuffer = 1;
				for (unsigned int i = 0; i < 32768; i++) {
					wave[i] = (drumBuffer & 1) * 2.0 - 1.0;
					unsigned int newBuffer = drumBuffer >> 1;
					if (((drumBuffer + newBuffer) & 1) == 1) {
						newBuffer += 1 << 14;
					}
					drumBuffer = newBuffer;
				}
			} else if (index == 1) {
				// White noise is just random values for each sample.
				for (unsigned int i = 0; i < 32768; i++) {
					std::random_device rd;
					std::mt19937 mt(rd());
					std::uniform_real_distribution<double> dist(0, 1.0);
					if(dist(mt)){
						wave[i] = dist(mt) * 2.0 - 1.0;
					}
				}
			} else if (index == 2) {
				// The "clang" drums are inspired by similar drums in the modded beepbox! :D
				unsigned int drumBuffer = 1;
				for (unsigned int i = 0; i < 32768; i++) {
					wave[i] = (drumBuffer & 1) * 2.0 - 1.0;
					unsigned int newBuffer = drumBuffer >> 1;
					if (((drumBuffer + newBuffer) & 1) == 1) {
						newBuffer += 2 << 14;
					}
					drumBuffer = newBuffer;
				}
			} else if (index == 3) {
				// The "buzz" drums are inspired by similar drums in the modded beepbox! :D
				unsigned int drumBuffer = 1;
				for (unsigned int i = 0; i < 32768; i++) {
					wave[i] = (drumBuffer & 1) * 2.0 - 1.0;
					unsigned int newBuffer = drumBuffer >> 1;
					if (((drumBuffer + newBuffer) & 1) == 1) {
						newBuffer += 10 << 2;
					}
					drumBuffer = newBuffer;
				}
			} else if (index == 4) {
				// "hollow" drums, designed in frequency space and then converted via FFT:
				drawNoiseSpectrum(wave, 10, 11, 1, 1, 0);
				drawNoiseSpectrum(wave, 11, 14, -2, -2, 0);
				inverseRealFourierTransform(wave);
				scaleElementsByFactor(wave, 1.0 / sqrt(wave.size()));
			}
		}
		return wave;
	}
	private: 
		static vector<double> _centerWave(vector <double> wave) {
			float sum = 0.0;
			for (auto &i:wave) sum += i;
			double average = sum / wave.size();
			for (auto &i:wave) i -= average;
			return wave;
		}
		static void drawNoiseSpectrum(vector <double>& wave, unsigned int lowOctave, unsigned int highOctave, unsigned int lowPower, unsigned int highPower, float overalSlope) {
			const unsigned int referenceOctave = 11;
			const unsigned int referenceIndex = 1 << referenceOctave;
			const unsigned int lowIndex = pow(2, lowOctave);
			const unsigned int highIndex = pow(2, highOctave);
			const float log2 = log(2);
			for (unsigned int i = lowIndex; i < highIndex; i++) {
				float amplitude = pow(2, lowPower + (highPower - lowPower) * (log(i) / log2 - lowOctave) / (highOctave - lowOctave));
				amplitude *= pow(i / referenceIndex, overalSlope);
				float rn = 0.0;
				std::random_device rd;
				std::mt19937 mt(rd());
				std::uniform_real_distribution<float> dist(0, 1.0);
				if(dist(mt)){
					rn = dist(mt);
				}
				const float radians = M_PI * 2.0;
				wave[i] = cos(radians) * amplitude;
				wave[32768 - i] = sin(radians) * amplitude;
			}
		}
		
		static vector<double> generateSineWave() {
			vector<double> wave(sineWaveLength + 1);
			for (unsigned int i = 0; i < sineWaveLength + 1; i++) {
				wave[i] = sin(i * M_PI * 2.0 / sineWaveLength);
			}
			return wave;
		}
	
};
