/*This will be where we most need to focus. We need to strip the song-parser out of this and make it a "music chunk" parser. Maybe the notes/notepins/channels/other constructs they use will be of use to us there. I'm making decent progress but could use some help for sure*/
#include <vector>
#include <string>
#include <math.h>
#include "config.h"
using std::string;
using std::vector;
struct Song {
	static const string _format = "BeepBox";
	static const int _oldestVersion = 2;
	static const int _latestVersion = 6;
	static const vector<int> _base64CharCodeToInt;
	static const vector<int> _base64IntToCharCode;
	int scale;
	int key;
	int tempo;
	int reverb;
	int beatsPerBar;
	int barCount;
	int patternsPerChannel;
	int partsPerBeat;
	int instrumentsPerChannel;
	int loopStart;
	int loopLength;
	unsigned int pitchChannelCount;
	unsigned int drumChannelCount;
	vector<Channel> channels;

	Song() {}

	Song(const Song &o){
		this.fromBase64String(o.toBase64String());
	}

	Song(string s) {
		if (!string.empty()) {
		} else {
			this.initToDefault(true);
		}
	}

	unsigned int getChannelCount() {
		return this.pitchChannelCount + this.drumChannelCount;
	}

	bool getChannelIsDrum(int channel) {
		return (channel >= this.pitchChannelCount);
	}

	string getChannelColorDim(int channel) {
		return channel < this.pitchChannelCount ? Config.pitchChannelColorsDim[channel] : Config.drumChannelColorsDim[channel - this.pitchChannelCount];
	}
	string getChannelColorBright(int channel): string {
		return channel < this.pitchChannelCount ? Config.pitchChannelColorsBright[channel] : Config.drumChannelColorsBright[channel - this.pitchChannelCount];
	}
	getNoteColorDim(channel: number): string {
		return channel < this.pitchChannelCount ? Config.pitchNoteColorsDim[channel] : Config.drumNoteColorsDim[channel - this.pitchChannelCount];
	}
	getNoteColorBright(channel: number): string {
		return channel < this.pitchChannelCount ? Config.pitchNoteColorsBright[channel] : Config.drumNoteColorsBright[channel - this.pitchChannelCount];
	}
	
	void initToDefault(andResetChannels: boolean = true): void {
		this.scale = 0;
		this.key = Config.keyNames.length - 1;
		this.loopStart = 0;
		this.loopLength = 4;
		this.tempo = 7;
		this.reverb = 0;
		this.beatsPerBar = 8;
		this.barCount = 16;
		this.patternsPerChannel = 8;
		this.partsPerBeat = 4;
		this.instrumentsPerChannel = 1;
		
		if (andResetChannels) {
			this.pitchChannelCount = 3;
			this.drumChannelCount = 1;
			for (let channelIndex = 0; channelIndex < this.getChannelCount(); channelIndex++) {
				if (this.channels.length <= channelIndex) {
					this.channels[channelIndex] = new Channel();
				}
				const channel: Channel = this.channels[channelIndex];
				channel.octave = 3 - channelIndex; // [3, 2, 1, 0]; Descending octaves with drums at zero in last channel.
			
				for (let pattern = 0; pattern < this.patternsPerChannel; pattern++) {
					if (channel.patterns.length <= pattern) {
						channel.patterns[pattern] = new Pattern();
					} else {
						channel.patterns[pattern].reset();
					}
				}
				channel.patterns.length = this.patternsPerChannel;
			
				for (let instrument = 0; instrument < this.instrumentsPerChannel; instrument++) {
					if (channel.instruments.length <= instrument) {
						channel.instruments[instrument] = new Instrument();
					} else {
						channel.instruments[instrument].reset();
					}
				}
				channel.instruments.length = this.instrumentsPerChannel;
			
				for (let bar = 0; bar < this.barCount; bar++) {
					channel.bars[bar] = 1;
				}
				channel.bars.length = this.barCount;
			}
			this.channels.length = this.getChannelCount();
		}
	}
	
	toBase64String(): string {
		let bits: BitFieldWriter;
		let buffer: number[] = [];
		
		const base64IntToCharCode: ReadonlyArray<number> = Song._base64IntToCharCode;
		
		buffer.push(base64IntToCharCode[Song._latestVersion]);
		buffer.push(SongTagCode.channelCount, base64IntToCharCode[this.pitchChannelCount], base64IntToCharCode[this.drumChannelCount]);
		buffer.push(SongTagCode.scale, base64IntToCharCode[this.scale]);
		buffer.push(SongTagCode.key, base64IntToCharCode[this.key]);
		buffer.push(SongTagCode.loopStart, base64IntToCharCode[this.loopStart >> 6], base64IntToCharCode[this.loopStart & 0x3f]);
		buffer.push(SongTagCode.loopEnd, base64IntToCharCode[(this.loopLength - 1) >> 6], base64IntToCharCode[(this.loopLength - 1) & 0x3f]);
		buffer.push(SongTagCode.tempo, base64IntToCharCode[this.tempo]);
		buffer.push(SongTagCode.reverb, base64IntToCharCode[this.reverb]);
		buffer.push(SongTagCode.beatCount, base64IntToCharCode[this.beatsPerBar - 1]);
		buffer.push(SongTagCode.barCount, base64IntToCharCode[(this.barCount - 1) >> 6], base64IntToCharCode[(this.barCount - 1) & 0x3f]);
		buffer.push(SongTagCode.patternCount, base64IntToCharCode[this.patternsPerChannel - 1]);
		buffer.push(SongTagCode.instrumentCount, base64IntToCharCode[this.instrumentsPerChannel - 1]);
		buffer.push(SongTagCode.rhythm, base64IntToCharCode[Config.partCounts.indexOf(this.partsPerBeat)]);
		
		buffer.push(SongTagCode.channelOctave);
		for (let channel: number = 0; channel < this.getChannelCount(); channel++) {
			buffer.push(base64IntToCharCode[this.channels[channel].octave]);
		}
		
		for (let channel: number = 0; channel < this.getChannelCount(); channel++) {
			for (let i: number = 0; i < this.instrumentsPerChannel; i++) {
				const instrument: Instrument = this.channels[channel].instruments[i];
				
				if (channel < this.pitchChannelCount) {
					buffer.push(SongTagCode.startInstrument, base64IntToCharCode[instrument.type]);
					if (instrument.type == InstrumentType.chip) {
						// chip
						buffer.push(SongTagCode.wave, base64IntToCharCode[instrument.wave]);
						buffer.push(SongTagCode.filter, base64IntToCharCode[instrument.filter]);
						buffer.push(SongTagCode.transition, base64IntToCharCode[instrument.transition]);
						buffer.push(SongTagCode.effect, base64IntToCharCode[instrument.effect]);
						buffer.push(SongTagCode.chorus,base64IntToCharCode[instrument.chorus]);
						buffer.push(SongTagCode.volume, base64IntToCharCode[instrument.volume]);
					} else if (instrument.type == InstrumentType.fm) {
						// FM
						buffer.push(SongTagCode.transition, base64IntToCharCode[instrument.transition]);
						buffer.push(SongTagCode.effect, base64IntToCharCode[instrument.effect]);
						
						buffer.push(SongTagCode.algorithm, base64IntToCharCode[instrument.algorithm]);
						buffer.push(SongTagCode.feedbackType, base64IntToCharCode[instrument.feedbackType]);
						buffer.push(SongTagCode.feedbackAmplitude, base64IntToCharCode[instrument.feedbackAmplitude]);
						buffer.push(SongTagCode.feedbackEnvelope, base64IntToCharCode[instrument.feedbackEnvelope]);
						
						buffer.push(SongTagCode.operatorFrequencies);
						for (let o: number = 0; o < Config.operatorCount; o++) {
							buffer.push(base64IntToCharCode[instrument.operators[o].frequency]);
						}
						buffer.push(SongTagCode.operatorAmplitudes);
						for (let o: number = 0; o < Config.operatorCount; o++) {
							buffer.push(base64IntToCharCode[instrument.operators[o].amplitude]);
						}
						buffer.push(SongTagCode.operatorEnvelopes);
						for (let o: number = 0; o < Config.operatorCount; o++) {
							buffer.push(base64IntToCharCode[instrument.operators[o].envelope]);
						}
					} else {
						throw new Error("Unknown instrument type.");
					}
				} else {
					// NOISE
					buffer.push(SongTagCode.startInstrument, base64IntToCharCode[InstrumentType.noise]);
					buffer.push(SongTagCode.wave, base64IntToCharCode[instrument.wave]);
					buffer.push(SongTagCode.transition, base64IntToCharCode[instrument.transition]);
					buffer.push(SongTagCode.volume, base64IntToCharCode[instrument.volume]);
				}
			}
		}
		
		buffer.push(SongTagCode.bars);
		bits = new BitFieldWriter();
		let neededBits: number = 0;
		while ((1 << neededBits) < this.patternsPerChannel + 1) neededBits++;
		for (let channel: number = 0; channel < this.getChannelCount(); channel++) for (let i: number = 0; i < this.barCount; i++) {
			bits.write(neededBits, this.channels[channel].bars[i]);
		}
		bits.encodeBase64(base64IntToCharCode, buffer);
		
		buffer.push(SongTagCode.patterns);
		bits = new BitFieldWriter();
		let neededInstrumentBits: number = 0;
		while ((1 << neededInstrumentBits) < this.instrumentsPerChannel) neededInstrumentBits++;
		for (let channel: number = 0; channel < this.getChannelCount(); channel++) {
			const isDrum: boolean = this.getChannelIsDrum(channel);
			const octaveOffset: number = isDrum ? 0 : this.channels[channel].octave * 12;
			let lastPitch: number = (isDrum ? 4 : 12) + octaveOffset;
			const recentPitches: number[] = isDrum ? [4,6,7,2,3,8,0,10] : [12, 19, 24, 31, 36, 7, 0];
			const recentShapes: string[] = [];
			for (let i: number = 0; i < recentPitches.length; i++) {
				recentPitches[i] += octaveOffset;
			}
			for (const p of this.channels[channel].patterns) {
				bits.write(neededInstrumentBits, p.instrument);
				
				if (p.notes.length > 0) {
					bits.write(1, 1);
					
					let curPart: number = 0;
					for (const t of p.notes) {
						if (t.start > curPart) {
							bits.write(2, 0); // rest
							bits.writePartDuration(t.start - curPart);
						}
						
						const shapeBits: BitFieldWriter = new BitFieldWriter();
						
						// 0: 1 pitch, 10: 2 pitches, 110: 3 pitches, 111: 4 pitches
						for (let i: number = 1; i < t.pitches.length; i++) shapeBits.write(1,1);
						if (t.pitches.length < 4) shapeBits.write(1,0);
						
						shapeBits.writePinCount(t.pins.length - 1);
						
						shapeBits.write(2, t.pins[0].volume); // volume
						
						let shapePart: number = 0;
						let startPitch: number = t.pitches[0];
						let currentPitch: number = startPitch;
						const pitchBends: number[] = [];
						for (let i: number = 1; i < t.pins.length; i++) {
							const pin: NotePin = t.pins[i];
							const nextPitch: number = startPitch + pin.interval;
							if (currentPitch != nextPitch) {
								shapeBits.write(1, 1);
								pitchBends.push(nextPitch);
								currentPitch = nextPitch;
							} else {
								shapeBits.write(1, 0);
							}
							shapeBits.writePartDuration(pin.time - shapePart);
							shapePart = pin.time;
							shapeBits.write(2, pin.volume);
						}
						
						const shapeString: string = String.fromCharCode.apply(null, shapeBits.encodeBase64(base64IntToCharCode, []));
						const shapeIndex: number = recentShapes.indexOf(shapeString);
						if (shapeIndex == -1) {
							bits.write(2, 1); // new shape
							bits.concat(shapeBits);
						} else {
							bits.write(1, 1); // old shape
							bits.writeLongTail(0, 0, shapeIndex);
							recentShapes.splice(shapeIndex, 1);
						}
						recentShapes.unshift(shapeString);
						if (recentShapes.length > 10) recentShapes.pop();
						
						const allPitches: number[] = t.pitches.concat(pitchBends);
						for (let i: number = 0; i < allPitches.length; i++) {
							const pitch: number = allPitches[i];
							const pitchIndex: number = recentPitches.indexOf(pitch);
							if (pitchIndex == -1) {
								let interval: number = 0;
								let pitchIter: number = lastPitch;
								if (pitchIter < pitch) {
									while (pitchIter != pitch) {
										pitchIter++;
										if (recentPitches.indexOf(pitchIter) == -1) interval++;
									}
								} else {
									while (pitchIter != pitch) {
										pitchIter--;
										if (recentPitches.indexOf(pitchIter) == -1) interval--;
									}
								}
								bits.write(1, 0);
								bits.writePitchInterval(interval);
							} else {
								bits.write(1, 1);
								bits.write(3, pitchIndex);
								recentPitches.splice(pitchIndex, 1);
							}
							recentPitches.unshift(pitch);
							if (recentPitches.length > 8) recentPitches.pop();
							
							if (i == t.pitches.length - 1) {
								lastPitch = t.pitches[0];
							} else {
								lastPitch = pitch;
							}
						}
						curPart = t.end;
					}
					
					if (curPart < this.beatsPerBar * this.partsPerBeat) {
						bits.write(2, 0); // rest
						bits.writePartDuration(this.beatsPerBar * this.partsPerBeat - curPart);
					}
				} else {
					bits.write(1, 0);
				}
			}
		}
		let stringLength: number = bits.lengthBase64();
		let digits: number[] = [];
		while (stringLength > 0) {
			digits.unshift(base64IntToCharCode[stringLength & 0x3f]);
			stringLength = stringLength >> 6;
		}
		buffer.push(base64IntToCharCode[digits.length]);
		Array.prototype.push.apply(buffer, digits); // append digits to buffer.
		bits.encodeBase64(base64IntToCharCode, buffer);
		
		// HACK: This breaks for strings longer than 65535. 
		if (buffer.length >= 65535) throw new Error("Song hash code too long.");
		return String.fromCharCode.apply(null, buffer);
	}
	
	fromBase64String(compressed: string): void {
		if (compressed == null || compressed == "") {
			this.initToDefault(true);
			return;
		}
		let charIndex: number = 0;
		// skip whitespace.
		while (compressed.charCodeAt(charIndex) <= CharCode.SPACE) charIndex++;
		// skip hash mark.
		if (compressed.charCodeAt(charIndex) == CharCode.HASH) charIndex++;
		// if it starts with curly brace, treat it as JSON.
		if (compressed.charCodeAt(charIndex) == CharCode.LEFT_CURLY_BRACE) {
			this.fromJsonObject(JSON.parse(charIndex == 0 ? compressed : compressed.substring(charIndex)));
			return;
		}
		
		const version: number = Song._base64CharCodeToInt[compressed.charCodeAt(charIndex++)];
		if (version == -1 || version > Song._latestVersion || version < Song._oldestVersion) return;
		const beforeThree: boolean = version < 3;
		const beforeFour:  boolean = version < 4;
		const beforeFive:  boolean = version < 5;
		const beforeSix:   boolean = version < 6;
		const base64CharCodeToInt: ReadonlyArray<number> = Song._base64CharCodeToInt;
		this.initToDefault(beforeSix);
		
		if (beforeThree) {
			// Originally, the only instrument transition was "seamless" and the only drum wave was "retro".
			for (const channel of this.channels) channel.instruments[0].transition = 0;
			this.channels[3].instruments[0].wave = 0;
		}
		
		let instrumentChannelIterator: number = 0;
		let instrumentIndexIterator: number = -1;
		
		while (charIndex < compressed.length) {
			const command: number = compressed.charCodeAt(charIndex++);
			let channel: number;
			if (command == SongTagCode.channelCount) {
				this.pitchChannelCount = base64CharCodeToInt[compressed.charCodeAt(charIndex++)];
				this.drumChannelCount  = base64CharCodeToInt[compressed.charCodeAt(charIndex++)];
				this.pitchChannelCount = Song._clip(Config.pitchChannelCountMin, Config.pitchChannelCountMax + 1, this.pitchChannelCount);
				this.drumChannelCount = Song._clip(Config.drumChannelCountMin, Config.drumChannelCountMax + 1, this.drumChannelCount);
				for (let channelIndex = this.channels.length; channelIndex < this.getChannelCount(); channelIndex++) {
					this.channels[channelIndex] = new Channel();
				}
				this.channels.length = this.getChannelCount();
			} else if (command == SongTagCode.scale) {
				this.scale = base64CharCodeToInt[compressed.charCodeAt(charIndex++)];
				if (beforeThree && this.scale == 10) this.scale = 11;
			} else if (command == SongTagCode.key) {
				this.key = base64CharCodeToInt[compressed.charCodeAt(charIndex++)];
			} else if (command == SongTagCode.loopStart) {
				if (beforeFive) {
					this.loopStart = base64CharCodeToInt[compressed.charCodeAt(charIndex++)];
				} else {
					this.loopStart = (base64CharCodeToInt[compressed.charCodeAt(charIndex++)] << 6) + base64CharCodeToInt[compressed.charCodeAt(charIndex++)];
				}
			} else if (command == SongTagCode.loopEnd) {
				if (beforeFive) {
					this.loopLength = base64CharCodeToInt[compressed.charCodeAt(charIndex++)];
				} else {
					this.loopLength = (base64CharCodeToInt[compressed.charCodeAt(charIndex++)] << 6) + base64CharCodeToInt[compressed.charCodeAt(charIndex++)] + 1;
				}
			} else if (command == SongTagCode.tempo) {
				if (beforeFour) {
					this.tempo = [1, 4, 7, 10][base64CharCodeToInt[compressed.charCodeAt(charIndex++)]];
				} else {
					this.tempo = base64CharCodeToInt[compressed.charCodeAt(charIndex++)];
				}
				this.tempo = Song._clip(0, Config.tempoSteps, this.tempo);
			} else if (command == SongTagCode.reverb) {
				this.reverb = base64CharCodeToInt[compressed.charCodeAt(charIndex++)];
				this.reverb = Song._clip(0, Config.reverbRange, this.reverb);
			} else if (command == SongTagCode.beatCount) {
				if (beforeThree) {
					this.beatsPerBar = [6, 7, 8, 9, 10][base64CharCodeToInt[compressed.charCodeAt(charIndex++)]];
				} else {
					this.beatsPerBar = base64CharCodeToInt[compressed.charCodeAt(charIndex++)] + 1;
				}
				this.beatsPerBar = max(Config.beatsPerBarMin, min(Config.beatsPerBarMax, this.beatsPerBar));
			} else if (command == SongTagCode.barCount) {
				this.barCount = (base64CharCodeToInt[compressed.charCodeAt(charIndex++)] << 6) + base64CharCodeToInt[compressed.charCodeAt(charIndex++)] + 1;
				this.barCount = max(Config.barCountMin, min(Config.barCountMax, this.barCount));
				for (let channel = 0; channel < this.getChannelCount(); channel++) {
					for (let bar = this.channels[channel].bars.length; bar < this.barCount; bar++) {
						this.channels[channel].bars[bar] = 1;
					}
					this.channels[channel].bars.length = this.barCount;
				}
			} else if (command == SongTagCode.patternCount) {
				this.patternsPerChannel = base64CharCodeToInt[compressed.charCodeAt(charIndex++)] + 1;
				this.patternsPerChannel = max(Config.patternsPerChannelMin, min(Config.patternsPerChannelMax, this.patternsPerChannel));
				for (let channel = 0; channel < this.getChannelCount(); channel++) {
					for (let pattern = this.channels[channel].patterns.length; pattern < this.patternsPerChannel; pattern++) {
						this.channels[channel].patterns[pattern] = new Pattern();
					}
					this.channels[channel].patterns.length = this.patternsPerChannel;
				}
			} else if (command == SongTagCode.instrumentCount) {
				this.instrumentsPerChannel = base64CharCodeToInt[compressed.charCodeAt(charIndex++)] + 1;
				this.instrumentsPerChannel = max(Config.instrumentsPerChannelMin, min(Config.instrumentsPerChannelMax, this.instrumentsPerChannel));
				for (let channel = 0; channel < this.getChannelCount(); channel++) {
					for (let instrument = this.channels[channel].instruments.length; instrument < this.instrumentsPerChannel; instrument++) {
						this.channels[channel].instruments[instrument] = new Instrument();
					}
					this.channels[channel].instruments.length = this.instrumentsPerChannel;
				}
			} else if (command == SongTagCode.rhythm) {
				this.partsPerBeat = Config.partCounts[base64CharCodeToInt[compressed.charCodeAt(charIndex++)]];
			} else if (command == SongTagCode.channelOctave) {
				if (beforeThree) {
					channel = base64CharCodeToInt[compressed.charCodeAt(charIndex++)];
					this.channels[channel].octave = Song._clip(0, 5, base64CharCodeToInt[compressed.charCodeAt(charIndex++)]);
				} else {
					for (channel = 0; channel < this.getChannelCount(); channel++) {
						this.channels[channel].octave = Song._clip(0, 5, base64CharCodeToInt[compressed.charCodeAt(charIndex++)]);
					}
				}
			} else if (command == SongTagCode.startInstrument) {
				instrumentIndexIterator++;
				if (instrumentIndexIterator >= this.instrumentsPerChannel) {
					instrumentChannelIterator++;
					instrumentIndexIterator = 0;
				}
				const instrument: Instrument = this.channels[instrumentChannelIterator].instruments[instrumentIndexIterator];
				instrument.setTypeAndReset(Song._clip(0, 2, base64CharCodeToInt[compressed.charCodeAt(charIndex++)]));
			} else if (command == SongTagCode.wave) {
				if (beforeThree) {
					channel = base64CharCodeToInt[compressed.charCodeAt(charIndex++)];
					this.channels[channel].instruments[0].wave = Song._clip(0, Config.waveNames.length, base64CharCodeToInt[compressed.charCodeAt(charIndex++)]);
				} else if (beforeSix) {
					for (channel = 0; channel < this.getChannelCount(); channel++) {
						const isDrums = (channel >= this.pitchChannelCount);
						for (let i: number = 0; i < this.instrumentsPerChannel; i++) {
							this.channels[channel].instruments[i].wave = Song._clip(0, isDrums ? Config.drumNames.length : Config.waveNames.length, base64CharCodeToInt[compressed.charCodeAt(charIndex++)]);
						}
					}
				} else {
					const isDrums = (instrumentChannelIterator >= this.pitchChannelCount);
					this.channels[instrumentChannelIterator].instruments[instrumentIndexIterator].wave = Song._clip(0, isDrums ? Config.drumNames.length : Config.waveNames.length, base64CharCodeToInt[compressed.charCodeAt(charIndex++)]);
				}
			} else if (command == SongTagCode.filter) {
				if (beforeThree) {
					channel = base64CharCodeToInt[compressed.charCodeAt(charIndex++)];
					this.channels[channel].instruments[0].filter = [1, 3, 4, 5][Song._clip(0, Config.filterNames.length, base64CharCodeToInt[compressed.charCodeAt(charIndex++)])];
				} else if (beforeSix) {
					for (channel = 0; channel < this.getChannelCount(); channel++) {
						for (let i: number = 0; i < this.instrumentsPerChannel; i++) {
							this.channels[channel].instruments[i].filter = Song._clip(0, Config.filterNames.length, base64CharCodeToInt[compressed.charCodeAt(charIndex++)] + 1);
						}
					}
				} else {
					this.channels[instrumentChannelIterator].instruments[instrumentIndexIterator].filter = Song._clip(0, Config.filterNames.length, base64CharCodeToInt[compressed.charCodeAt(charIndex++)]);
				}
			} else if (command == SongTagCode.transition) {
				if (beforeThree) {
					channel = base64CharCodeToInt[compressed.charCodeAt(charIndex++)];
					this.channels[channel].instruments[0].transition = Song._clip(0, Config.transitionNames.length, base64CharCodeToInt[compressed.charCodeAt(charIndex++)]);
				} else if (beforeSix) {
					for (channel = 0; channel < this.getChannelCount(); channel++) {
						for (let i: number = 0; i < this.instrumentsPerChannel; i++) {
							this.channels[channel].instruments[i].transition = Song._clip(0, Config.transitionNames.length, base64CharCodeToInt[compressed.charCodeAt(charIndex++)]);
						}
					}
				} else {
					this.channels[instrumentChannelIterator].instruments[instrumentIndexIterator].transition = Song._clip(0, Config.transitionNames.length, base64CharCodeToInt[compressed.charCodeAt(charIndex++)]);
				}
			} else if (command == SongTagCode.effect) {
				if (beforeThree) {
					channel = base64CharCodeToInt[compressed.charCodeAt(charIndex++)];
					let effect: number = Song._clip(0, Config.effectNames.length, base64CharCodeToInt[compressed.charCodeAt(charIndex++)]);
					if (effect == 1) effect = 3;
					else if (effect == 3) effect = 5;
					this.channels[channel].instruments[0].effect = effect;
				} else if (beforeSix) {
					for (channel = 0; channel < this.getChannelCount(); channel++) {
						for (let i: number = 0; i < this.instrumentsPerChannel; i++) {
							this.channels[channel].instruments[i].effect = Song._clip(0, Config.effectNames.length, base64CharCodeToInt[compressed.charCodeAt(charIndex++)]);
						}
					}
				} else {
					this.channels[instrumentChannelIterator].instruments[instrumentIndexIterator].effect = Song._clip(0, Config.effectNames.length, base64CharCodeToInt[compressed.charCodeAt(charIndex++)]);
				}
			} else if (command == SongTagCode.chorus) {
				if (beforeThree) {
					channel = base64CharCodeToInt[compressed.charCodeAt(charIndex++)];
					this.channels[channel].instruments[0].chorus = Song._clip(0, Config.chorusNames.length, base64CharCodeToInt[compressed.charCodeAt(charIndex++)]);
				} else if (beforeSix) {
					for (channel = 0; channel < this.getChannelCount(); channel++) {
						for (let i: number = 0; i < this.instrumentsPerChannel; i++) {
							this.channels[channel].instruments[i].chorus = Song._clip(0, Config.chorusNames.length, base64CharCodeToInt[compressed.charCodeAt(charIndex++)]);
						}
					}
				} else {
					this.channels[instrumentChannelIterator].instruments[instrumentIndexIterator].chorus = Song._clip(0, Config.chorusNames.length, base64CharCodeToInt[compressed.charCodeAt(charIndex++)]);
				}
			} else if (command == SongTagCode.volume) {
				if (beforeThree) {
					channel = base64CharCodeToInt[compressed.charCodeAt(charIndex++)];
					this.channels[channel].instruments[0].volume = Song._clip(0, Config.volumeNames.length, base64CharCodeToInt[compressed.charCodeAt(charIndex++)]);
				} else if (beforeSix) {
					for (channel = 0; channel < this.getChannelCount(); channel++) {
						for (let i: number = 0; i < this.instrumentsPerChannel; i++) {
							this.channels[channel].instruments[i].volume = Song._clip(0, Config.volumeNames.length, base64CharCodeToInt[compressed.charCodeAt(charIndex++)]);
						}
					}
				} else {
					this.channels[instrumentChannelIterator].instruments[instrumentIndexIterator].volume = Song._clip(0, Config.volumeNames.length, base64CharCodeToInt[compressed.charCodeAt(charIndex++)]);
				}
			} else if (command == SongTagCode.algorithm) {
				this.channels[instrumentChannelIterator].instruments[instrumentIndexIterator].algorithm = Song._clip(0, Config.operatorAlgorithmNames.length, base64CharCodeToInt[compressed.charCodeAt(charIndex++)]);
			} else if (command == SongTagCode.feedbackType) {
				this.channels[instrumentChannelIterator].instruments[instrumentIndexIterator].feedbackType = Song._clip(0, Config.operatorFeedbackNames.length, base64CharCodeToInt[compressed.charCodeAt(charIndex++)]);
			} else if (command == SongTagCode.feedbackAmplitude) {
				this.channels[instrumentChannelIterator].instruments[instrumentIndexIterator].feedbackAmplitude = Song._clip(0, Config.operatorAmplitudeMax + 1, base64CharCodeToInt[compressed.charCodeAt(charIndex++)]);
			} else if (command == SongTagCode.feedbackEnvelope) {
				this.channels[instrumentChannelIterator].instruments[instrumentIndexIterator].feedbackEnvelope = Song._clip(0, Config.operatorEnvelopeNames.length, base64CharCodeToInt[compressed.charCodeAt(charIndex++)]);
			} else if (command == SongTagCode.operatorFrequencies) {
				for (let o: number = 0; o < Config.operatorCount; o++) {
					this.channels[instrumentChannelIterator].instruments[instrumentIndexIterator].operators[o].frequency = Song._clip(0, Config.operatorFrequencyNames.length, base64CharCodeToInt[compressed.charCodeAt(charIndex++)]);
				}
			} else if (command == SongTagCode.operatorAmplitudes) {
				for (let o: number = 0; o < Config.operatorCount; o++) {
					this.channels[instrumentChannelIterator].instruments[instrumentIndexIterator].operators[o].amplitude = Song._clip(0, Config.operatorAmplitudeMax + 1, base64CharCodeToInt[compressed.charCodeAt(charIndex++)]);
				}
			} else if (command == SongTagCode.operatorEnvelopes) {
				for (let o: number = 0; o < Config.operatorCount; o++) {
					this.channels[instrumentChannelIterator].instruments[instrumentIndexIterator].operators[o].envelope = Song._clip(0, Config.operatorEnvelopeNames.length, base64CharCodeToInt[compressed.charCodeAt(charIndex++)]);
				}
			} else if (command == SongTagCode.bars) {
				let subStringLength: number;
				if (beforeThree) {
					channel = base64CharCodeToInt[compressed.charCodeAt(charIndex++)];
					const barCount: number = base64CharCodeToInt[compressed.charCodeAt(charIndex++)];
					subStringLength = ceil(barCount * 0.5);
					const bits: BitFieldReader = new BitFieldReader(base64CharCodeToInt, compressed, charIndex, charIndex + subStringLength);
					for (let i: number = 0; i < barCount; i++) {
						this.channels[channel].bars[i] = bits.read(3) + 1;
					}
				} else if (beforeFive) {
					let neededBits: number = 0;
					while ((1 << neededBits) < this.patternsPerChannel) neededBits++;
					subStringLength = ceil(this.getChannelCount() * this.barCount * neededBits / 6);
					const bits: BitFieldReader = new BitFieldReader(base64CharCodeToInt, compressed, charIndex, charIndex + subStringLength);
					for (channel = 0; channel < this.getChannelCount(); channel++) {
						for (let i: number = 0; i < this.barCount; i++) {
							this.channels[channel].bars[i] = bits.read(neededBits) + 1;
						}
					}
				} else {
					let neededBits: number = 0;
					while ((1 << neededBits) < this.patternsPerChannel + 1) neededBits++;
					subStringLength = ceil(this.getChannelCount() * this.barCount * neededBits / 6);
					const bits: BitFieldReader = new BitFieldReader(base64CharCodeToInt, compressed, charIndex, charIndex + subStringLength);
					for (channel = 0; channel < this.getChannelCount(); channel++) {
						for (let i: number = 0; i < this.barCount; i++) {
							this.channels[channel].bars[i] = bits.read(neededBits);
						}
					}
				}
				charIndex += subStringLength;
			} else if (command == SongTagCode.patterns) {
				let bitStringLength: number = 0;
				if (beforeThree) {
					channel = base64CharCodeToInt[compressed.charCodeAt(charIndex++)];
					
					// The old format used the next character to represent the number of patterns in the channel, which is usually eight, the default. 
					charIndex++; //let patternCount: number = base64CharCodeToInt[compressed.charCodeAt(charIndex++)];
					
					bitStringLength = base64CharCodeToInt[compressed.charCodeAt(charIndex++)];
					bitStringLength = bitStringLength << 6;
					bitStringLength += base64CharCodeToInt[compressed.charCodeAt(charIndex++)];
				} else {
					channel = 0;
					let bitStringLengthLength: number = base64CharCodeToInt[compressed.charCodeAt(charIndex++)];
					while (bitStringLengthLength > 0) {
						bitStringLength = bitStringLength << 6;
						bitStringLength += base64CharCodeToInt[compressed.charCodeAt(charIndex++)];
						bitStringLengthLength--;
					}
				}
				
				const bits: BitFieldReader = new BitFieldReader(base64CharCodeToInt, compressed, charIndex, charIndex + bitStringLength);
				charIndex += bitStringLength;
				
				let neededInstrumentBits: number = 0;
				while ((1 << neededInstrumentBits) < this.instrumentsPerChannel) neededInstrumentBits++;
				while (true) {
					const isDrum: boolean = this.getChannelIsDrum(channel);
					
					const octaveOffset: number = isDrum ? 0 : this.channels[channel].octave * 12;
					let note: Note | null = null;
					let pin: NotePin | null = null;
					let lastPitch: number = (isDrum ? 4 : 12) + octaveOffset;
					const recentPitches: number[] = isDrum ? [4,6,7,2,3,8,0,10] : [12, 19, 24, 31, 36, 7, 0];
					const recentShapes: any[] = [];
					for (let i: number = 0; i < recentPitches.length; i++) {
						recentPitches[i] += octaveOffset;
					}
					for (let i: number = 0; i < this.patternsPerChannel; i++) {
						const newPattern: Pattern = this.channels[channel].patterns[i];
						newPattern.reset();
						newPattern.instrument = bits.read(neededInstrumentBits);
						
						if (!beforeThree && bits.read(1) == 0) continue;
						
						let curPart: number = 0;
						const newNotes: Note[] = newPattern.notes;
						while (curPart < this.beatsPerBar * this.partsPerBeat) {
							
							const useOldShape: boolean = bits.read(1) == 1;
							let newNote: boolean = false;
							let shapeIndex: number = 0;
							if (useOldShape) {
								shapeIndex = bits.readLongTail(0, 0);
							} else {
								newNote = bits.read(1) == 1;
							}
							
							if (!useOldShape && !newNote) {
								const restLength: number = bits.readPartDuration();
								curPart += restLength;
							} else {
								let shape: any;
								let pinObj: any;
								let pitch: number;
								if (useOldShape) {
									shape = recentShapes[shapeIndex];
									recentShapes.splice(shapeIndex, 1);
								} else {
									shape = {};
									
									shape.pitchCount = 1;
									while (shape.pitchCount < 4 && bits.read(1) == 1) shape.pitchCount++;
									
									shape.pinCount = bits.readPinCount();
									shape.initialVolume = bits.read(2);
									
									shape.pins = [];
									shape.length = 0;
									shape.bendCount = 0;
									for (let j: number = 0; j < shape.pinCount; j++) {
										pinObj = {};
										pinObj.pitchBend = bits.read(1) == 1;
										if (pinObj.pitchBend) shape.bendCount++;
										shape.length += bits.readPartDuration();
										pinObj.time = shape.length;
										pinObj.volume = bits.read(2);
										shape.pins.push(pinObj);
									}
								}
								recentShapes.unshift(shape);
								if (recentShapes.length > 10) recentShapes.pop();
								
								note = makeNote(0,curPart,curPart + shape.length, shape.initialVolume);
								note.pitches = [];
								note.pins.length = 1;
								const pitchBends: number[] = [];
								for (let j: number = 0; j < shape.pitchCount + shape.bendCount; j++) {
									const useOldPitch: boolean = bits.read(1) == 1;
									if (!useOldPitch) {
										const interval: number = bits.readPitchInterval();
										pitch = lastPitch;
										let intervalIter: number = interval;
										while (intervalIter > 0) {
											pitch++;
											while (recentPitches.indexOf(pitch) != -1) pitch++;
											intervalIter--;
										}
										while (intervalIter < 0) {
											pitch--;
											while (recentPitches.indexOf(pitch) != -1) pitch--;
											intervalIter++;
										}
									} else {
										const pitchIndex: number = bits.read(3);
										pitch = recentPitches[pitchIndex];
										recentPitches.splice(pitchIndex, 1);
									}
									
									recentPitches.unshift(pitch);
									if (recentPitches.length > 8) recentPitches.pop();
									
									if (j < shape.pitchCount) {
										note.pitches.push(pitch);
									} else {
										pitchBends.push(pitch);
									}
									
									if (j == shape.pitchCount - 1) {
										lastPitch = note.pitches[0];
									} else {
										lastPitch = pitch;
									}
								}
								
								pitchBends.unshift(note.pitches[0]);
								
								for (const pinObj of shape.pins) {
									if (pinObj.pitchBend) pitchBends.shift();
									pin = makeNotePin(pitchBends[0] - note.pitches[0], pinObj.time, pinObj.volume);
									note.pins.push(pin);
								}
								curPart = note.end;
								newNotes.push(note);
							}
						}
					}
					
					if (beforeThree) {
						break;
					} else {
						channel++;
						if (channel >= this.getChannelCount()) break;
					}
				} // while (true)
			}
		}
	}
	
	toJsonObject(enableIntro: boolean = true, loopCount: number = 1, enableOutro: boolean = true): Object {
		const channelArray: Object[] = [];
		for (let channel: number = 0; channel < this.getChannelCount(); channel++) {
			const instrumentArray: Object[] = [];
			const isDrum: boolean = this.getChannelIsDrum(channel);
			for (let i: number = 0; i < this.instrumentsPerChannel; i++) {
				const instrument: Instrument = this.channels[channel].instruments[i];
				if (isDrum) {
					instrumentArray.push({
						type: Config.instrumentTypeNames[InstrumentType.noise],
						volume: (5 - instrument.volume) * 20,
						wave: Config.drumNames[instrument.wave],
						transition: Config.transitionNames[instrument.transition],
					});
				} else {
					if (instrument.type == InstrumentType.chip) {
						instrumentArray.push({
							type: Config.instrumentTypeNames[instrument.type],
							volume: (5 - instrument.volume) * 20,
							wave: Config.waveNames[instrument.wave],
							transition: Config.transitionNames[instrument.transition],
							filter: Config.filterNames[instrument.filter],
							chorus: Config.chorusNames[instrument.chorus],
							effect: Config.effectNames[instrument.effect],
						});
					} else if (instrument.type == InstrumentType.fm) {
						const operatorArray: Object[] = [];
						for (const operator of instrument.operators) {
							operatorArray.push({
								frequency: Config.operatorFrequencyNames[operator.frequency],
								amplitude: operator.amplitude,
								envelope: Config.operatorEnvelopeNames[operator.envelope],
							});
						}
						instrumentArray.push({
							type: Config.instrumentTypeNames[instrument.type],
							transition: Config.transitionNames[instrument.transition],
							effect: Config.effectNames[instrument.effect],
							algorithm: Config.operatorAlgorithmNames[instrument.algorithm],
							feedbackType: Config.operatorFeedbackNames[instrument.feedbackType],
							feedbackAmplitude: instrument.feedbackAmplitude,
							feedbackEnvelope: Config.operatorEnvelopeNames[instrument.feedbackEnvelope],
							operators: operatorArray,
						});
					} else {
						throw new Error("Unrecognized instrument type");
					}
				}
			}
			
			const patternArray: Object[] = [];
			for (const pattern of this.channels[channel].patterns) {
				const noteArray: Object[] = [];
				for (const note of pattern.notes) {
					const pointArray: Object[] = [];
					for (const pin of note.pins) {
						pointArray.push({
							tick: pin.time + note.start,
							pitchBend: pin.interval,
							volume: round(pin.volume * 100 / 3),
						});
					}
					
					noteArray.push({
						pitches: note.pitches,
						points: pointArray,
					});
				}
				
				patternArray.push({
					instrument: pattern.instrument + 1,
					notes: noteArray, 
				});
			}
			
			const sequenceArray: number[] = [];
			if (enableIntro) for (let i: number = 0; i < this.loopStart; i++) {
				sequenceArray.push(this.channels[channel].bars[i]);
			}
			for (let l: number = 0; l < loopCount; l++) for (let i: number = this.loopStart; i < this.loopStart + this.loopLength; i++) {
				sequenceArray.push(this.channels[channel].bars[i]);
			}
			if (enableOutro) for (let i: number = this.loopStart + this.loopLength; i < this.barCount; i++) {
				sequenceArray.push(this.channels[channel].bars[i]);
			}
			
			channelArray.push({
				type: isDrum ? "drum" : "pitch",
				octaveScrollBar: this.channels[channel].octave,
				instruments: instrumentArray,
				patterns: patternArray,
				sequence: sequenceArray,
			});
		}
		
		return {
			format: Song._format,
			version: Song._latestVersion,
			scale: Config.scaleNames[this.scale],
			key: Config.keyNames[this.key],
			introBars: this.loopStart,
			loopBars: this.loopLength,
			beatsPerBar: this.beatsPerBar,
			ticksPerBeat: this.partsPerBeat,
			beatsPerMinute: this.getBeatsPerMinute(), // represents tempo
			reverb: this.reverb,
			//outroBars: this.barCount - this.loopStart - this.loopLength; // derive this from bar arrays?
			//patternCount: this.patternsPerChannel, // derive this from pattern arrays?
			//instrumentsPerChannel: this.instrumentsPerChannel, //derive this from instrument arrays?
			channels: channelArray,
		};
	}
	
	fromJsonObject(jsonObject: any): void {
		this.initToDefault(true);
		if (!jsonObject) return;
		const version: any = jsonObject.version;
		if (version > Song._format) return;
		
		this.scale = 11; // default to expert.
		if (jsonObject.scale != undefined) {
			const oldScaleNames: Dictionary<number> = {"romani :)": 8, "romani :(": 9};
			const scale: number = oldScaleNames[jsonObject.scale] != undefined ? oldScaleNames[jsonObject.scale] : Config.scaleNames.indexOf(jsonObject.scale);
			if (scale != -1) this.scale = scale;
		}
		
		if (jsonObject.key != undefined) {
			if (typeof(jsonObject.key) == "number") {
				this.key = Config.keyNames.length - 1 - (((jsonObject.key + 1200) >>> 0) % Config.keyNames.length);
			} else if (typeof(jsonObject.key) == "string") {
				const key: string = jsonObject.key;
				const letter: string = key.charAt(0).toUpperCase();
				const symbol: string = key.charAt(1).toLowerCase();
				const letterMap: Readonly<Dictionary<number>> = {"C": 11, "D": 9, "E": 7, "F": 6, "G": 4, "A": 2, "B": 0};
				const accidentalMap: Readonly<Dictionary<number>> = {"#": -1, "♯": -1, "b": 1, "♭": 1};
				let index: number | undefined = letterMap[letter];
				const offset: number | undefined = accidentalMap[symbol];
				if (index != undefined) {
					if (offset != undefined) index += offset;
					if (index < 0) index += 12;
					index = index % 12;
					this.key = index;
				}
			}
		}
		
		if (jsonObject.beatsPerMinute != undefined) {
			const bpm: number = jsonObject.beatsPerMinute | 0;
			this.tempo = round(4.0 + 9.0 * log(bpm / 120) / LN2);
			this.tempo = Song._clip(0, Config.tempoSteps, this.tempo);
		}
		
		if (jsonObject.reverb != undefined) {
			this.reverb = Song._clip(0, Config.reverbRange, jsonObject.reverb | 0);
		}
		
		if (jsonObject.beatsPerBar != undefined) {
			this.beatsPerBar = max(Config.beatsPerBarMin, min(Config.beatsPerBarMax, jsonObject.beatsPerBar | 0));
		}
		
		if (jsonObject.ticksPerBeat != undefined) {
			this.partsPerBeat = jsonObject.ticksPerBeat | 0;
			if (Config.partCounts.indexOf(this.partsPerBeat) == -1) {
				this.partsPerBeat = Config.partCounts[Config.partCounts.length - 1];
			}
		}
		
		let maxInstruments: number = 1;
		let maxPatterns: number = 1;
		let maxBars: number = 1;
		if (jsonObject.channels) {
			for (const channelObject of jsonObject.channels) {
				if (channelObject.instruments) maxInstruments = max(maxInstruments, channelObject.instruments.length | 0);
				if (channelObject.patterns) maxPatterns = max(maxPatterns, channelObject.patterns.length | 0);
				if (channelObject.sequence) maxBars = max(maxBars, channelObject.sequence.length | 0);
			}
		}
		
		this.instrumentsPerChannel = maxInstruments;
		this.patternsPerChannel = maxPatterns;
		this.barCount = maxBars;
		
		if (jsonObject.introBars != undefined) {
			this.loopStart = Song._clip(0, this.barCount, jsonObject.introBars | 0);
		}
		if (jsonObject.loopBars != undefined) {
			this.loopLength = Song._clip(1, this.barCount - this.loopStart + 1, jsonObject.loopBars | 0);
		}
		
		let pitchChannelCount = 0;
		let drumChannelCount = 0;
		if (jsonObject.channels) {
			for (let channel: number = 0; channel < jsonObject.channels.length; channel++) {
				let channelObject: any = jsonObject.channels[channel];
				
				if (this.channels.length <= channel) this.channels[channel] = new Channel();
				
				if (channelObject.octaveScrollBar != undefined) {
					this.channels[channel].octave = Song._clip(0, 5, channelObject.octaveScrollBar | 0);
				}
				
				for (let i: number = this.channels[channel].instruments.length; i < this.instrumentsPerChannel; i++) {
					this.channels[channel].instruments[i] = new Instrument();
				}
				this.channels[channel].instruments.length = this.instrumentsPerChannel;
				
				for (let i: number = this.channels[channel].patterns.length; i < this.patternsPerChannel; i++) {
					this.channels[channel].patterns[i] = new Pattern();
				}
				this.channels[channel].patterns.length = this.patternsPerChannel;
				
				for (let i: number = 0; i < this.barCount; i++) {
					this.channels[channel].bars[i] = 1;
				}
				this.channels[channel].bars.length = this.barCount;
				
				let isDrum: boolean = false;
				if (channelObject.type) {
					isDrum = (channelObject.type == "drum");
				} else {
					// for older files, assume drums are channel 3.
					isDrum = (channel >= 3);
				}
				if (isDrum) drumChannelCount++; else pitchChannelCount++;
				
				for (let i: number = 0; i < this.instrumentsPerChannel; i++) {
					const instrument: Instrument = this.channels[channel].instruments[i];
					let instrumentObject: any = undefined;
					if (channelObject.instruments) instrumentObject = channelObject.instruments[i];
					if (instrumentObject == undefined) instrumentObject = {};
					
					const oldTransitionNames: Dictionary<number> = {"binary": 0};
					const transitionObject = instrumentObject.transition || instrumentObject.envelope; // the transition property used to be called envelope, so try that too.
					instrument.transition = oldTransitionNames[transitionObject] != undefined ? oldTransitionNames[transitionObject] : Config.transitionNames.indexOf(transitionObject);
					if (instrument.transition == -1) instrument.transition = 1;
					
					if (isDrum) {
						if (instrumentObject.volume != undefined) {
							instrument.volume = Song._clip(0, Config.volumeNames.length, round(5 - (instrumentObject.volume | 0) / 20));
						} else {
							instrument.volume = 0;
						}
						instrument.wave = Config.drumNames.indexOf(instrumentObject.wave);
						if (instrument.wave == -1) instrument.wave = 1;
					} else {
						instrument.type = Config.instrumentTypeNames.indexOf(instrumentObject.type);
						if (instrument.type == -1) instrument.type = InstrumentType.chip;
						
						if (instrument.type == InstrumentType.chip) {
							if (instrumentObject.volume != undefined) {
								instrument.volume = Song._clip(0, Config.volumeNames.length, round(5 - (instrumentObject.volume | 0) / 20));
							} else {
								instrument.volume = 0;
							}
							instrument.wave = Config.waveNames.indexOf(instrumentObject.wave);
							if (instrument.wave == -1) instrument.wave = 1;
							
							const oldFilterNames: Dictionary<number> = {"sustain sharp": 1, "sustain medium": 2, "sustain soft": 3, "decay sharp": 4};
							instrument.filter = oldFilterNames[instrumentObject.filter] != undefined ? oldFilterNames[instrumentObject.filter] : Config.filterNames.indexOf(instrumentObject.filter);
							if (instrument.filter == -1) instrument.filter = 0;
							
							instrument.chorus = Config.chorusNames.indexOf(instrumentObject.chorus);
							if (instrument.chorus == -1) instrument.chorus = 0;
							instrument.effect = Config.effectNames.indexOf(instrumentObject.effect);
							if (instrument.effect == -1) instrument.effect = 0;
						} else if (instrument.type == InstrumentType.fm) {
							instrument.effect = Config.effectNames.indexOf(instrumentObject.effect);
							if (instrument.effect == -1) instrument.effect = 0;
							
							instrument.algorithm = Config.operatorAlgorithmNames.indexOf(instrumentObject.algorithm);
							if (instrument.algorithm == -1) instrument.algorithm = 0;
							instrument.feedbackType = Config.operatorFeedbackNames.indexOf(instrumentObject.feedbackType);
							if (instrument.feedbackType == -1) instrument.feedbackType = 0;
							if (instrumentObject.feedbackAmplitude != undefined) {
								instrument.feedbackAmplitude = Song._clip(0, Config.operatorAmplitudeMax + 1, instrumentObject.feedbackAmplitude | 0);
							} else {
								instrument.feedbackAmplitude = 0;
							}
							instrument.feedbackEnvelope = Config.operatorEnvelopeNames.indexOf(instrumentObject.feedbackEnvelope);
							if (instrument.feedbackEnvelope == -1) instrument.feedbackEnvelope = 0;
							
							for (let j: number = 0; j < Config.operatorCount; j++) {
								const operator: Operator = instrument.operators[j];
								let operatorObject: any = undefined;
								if (instrumentObject.operators) operatorObject = instrumentObject.operators[j];
								if (operatorObject == undefined) operatorObject = {};
								
								operator.frequency = Config.operatorFrequencyNames.indexOf(operatorObject.frequency);
								if (operator.frequency == -1) operator.frequency = 0;
								if (operatorObject.amplitude != undefined) {
									operator.amplitude = Song._clip(0, Config.operatorAmplitudeMax + 1, operatorObject.amplitude | 0);
								} else {
									operator.amplitude = 0;
								}
								operator.envelope = Config.operatorEnvelopeNames.indexOf(operatorObject.envelope);
								if (operator.envelope == -1) operator.envelope = 0;
							}
						} else {
							throw new Error("Unrecognized instrument type.");
						}
					}
				}
			
				for (let i: number = 0; i < this.patternsPerChannel; i++) {
					const pattern: Pattern = this.channels[channel].patterns[i];
				
					let patternObject: any = undefined;
					if (channelObject.patterns) patternObject = channelObject.patterns[i];
					if (patternObject == undefined) continue;
				
					pattern.instrument = Song._clip(0, this.instrumentsPerChannel, (patternObject.instrument | 0) - 1);
				
					if (patternObject.notes && patternObject.notes.length > 0) {
						const maxNoteCount: number = min(this.beatsPerBar * this.partsPerBeat, patternObject.notes.length >>> 0);
					
						///@TODO: Consider supporting notes specified in any timing order, sorting them and truncating as necessary. 
						let tickClock: number = 0;
						for (let j: number = 0; j < patternObject.notes.length; j++) {
							if (j >= maxNoteCount) break;
						
							const noteObject = patternObject.notes[j];
							if (!noteObject || !noteObject.pitches || !(noteObject.pitches.length >= 1) || !noteObject.points || !(noteObject.points.length >= 2)) {
								continue;
							}
						
							const note: Note = makeNote(0, 0, 0, 0);
							note.pitches = [];
							note.pins = [];
						
							for (let k: number = 0; k < noteObject.pitches.length; k++) {
								const pitch: number = noteObject.pitches[k] | 0;
								if (note.pitches.indexOf(pitch) != -1) continue;
								note.pitches.push(pitch);
								if (note.pitches.length >= 4) break;
							}
							if (note.pitches.length < 1) continue;
						
							let noteClock: number = tickClock;
							let startInterval: number = 0;
							for (let k: number = 0; k < noteObject.points.length; k++) {
								const pointObject: any = noteObject.points[k];
								if (pointObject == undefined || pointObject.tick == undefined) continue;
								const interval: number = (pointObject.pitchBend == undefined) ? 0 : (pointObject.pitchBend | 0);
								const time: number = pointObject.tick | 0;
								const volume: number = (pointObject.volume == undefined) ? 3 : max(0, min(3, round((pointObject.volume | 0) * 3 / 100)));
							
								if (time > this.beatsPerBar * this.partsPerBeat) continue;
								if (note.pins.length == 0) {
									if (time < noteClock) continue;
									note.start = time;
									startInterval = interval;
								} else {
									if (time <= noteClock) continue;
								}
								noteClock = time;
							
								note.pins.push(makeNotePin(interval - startInterval, time - note.start, volume));
							}
							if (note.pins.length < 2) continue;
						
							note.end = note.pins[note.pins.length - 1].time + note.start;
						
							const maxPitch: number = isDrum ? Config.drumCount - 1 : Config.maxPitch;
							let lowestPitch: number = maxPitch;
							let highestPitch: number = 0;
							for (let k: number = 0; k < note.pitches.length; k++) {
								note.pitches[k] += startInterval;
								if (note.pitches[k] < 0 || note.pitches[k] > maxPitch) {
									note.pitches.splice(k, 1);
									k--;
								}
								if (note.pitches[k] < lowestPitch) lowestPitch = note.pitches[k];
								if (note.pitches[k] > highestPitch) highestPitch = note.pitches[k];
							}
							if (note.pitches.length < 1) continue;
						
							for (let k: number = 0; k < note.pins.length; k++) {
								const pin: NotePin = note.pins[k];
								if (pin.interval + lowestPitch < 0) pin.interval = -lowestPitch;
								if (pin.interval + highestPitch > maxPitch) pin.interval = maxPitch - highestPitch;
								if (k >= 2) {
									if (pin.interval == note.pins[k-1].interval && 
										pin.interval == note.pins[k-2].interval && 
										pin.volume == note.pins[k-1].volume && 
										pin.volume == note.pins[k-2].volume)
									{
										note.pins.splice(k-1, 1);
										k--;
									}    
								}
							}
						
							pattern.notes.push(note);
							tickClock = note.end;
						}
					}
				}
			
				for (let i: number = 0; i < this.barCount; i++) {
					this.channels[channel].bars[i] = channelObject.sequence ? min(this.patternsPerChannel, channelObject.sequence[i] >>> 0) : 0;
				}
			}
		}
		
		this.pitchChannelCount = pitchChannelCount;
		this.drumChannelCount = drumChannelCount;
		this.channels.length = this.getChannelCount();
	}
	
	static _clip(min: number, max: number, val: number): number {
		max = max - 1;
		if (val <= max) {
			if (val >= min) return val;
			else return min;
		} else {
			return max;
		}
	}
	
	getPattern(channel: number, bar: number): Pattern | null {
		const patternIndex: number = this.channels[channel].bars[bar];
		if (patternIndex == 0) return null;
		return this.channels[channel].patterns[patternIndex - 1];
	}
	
	getPatternInstrument(channel: number, bar: number): number {
		const pattern: Pattern | null = this.getPattern(channel, bar);
		return pattern == null ? 0 : pattern.instrument;
	}
	
	getBeatsPerMinute(): number {
		return round(120.0 * pow(2.0, (-4.0 + this.tempo) / 9.0));
	}
	
	const _fingerprint: Array<string | number> = [];
	string getChannelFingerprint(int bar) {
		const unsigned int channelCount = getChannelCount();
		unsigned int charCount = 0;
		for (unsigned int channel = 0; channel < channelCount; channel++) {
			if (channel < this.pitchChannelCount) {
				const instrumentIndex: number = this.getPatternInstrument(channel, bar);
				const instrument: Instrument = this.channels[channel].instruments[instrumentIndex];
				if (instrument.type == InstrumentType.chip) {
					this._fingerprint[charCount++] = "c";
				} else if (instrument.type == InstrumentType.fm) {
					this._fingerprint[charCount++] = "f"
					this._fingerprint[charCount++] = instrument.algorithm;
					this._fingerprint[charCount++] = instrument.feedbackType;
				} else {
					throw new Error("Unknown instrument type.");
				}
			} else {
				this._fingerprint[charCount++] = "d";
			}
		}
		this._fingerprint.length = charCount;
		return this._fingerprint.join("");
	}
}

