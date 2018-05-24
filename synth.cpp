/*This is the big dump file I'm keeping stuff in until I can work with it. Hopefully it'll one day contain main*/

#include "config.h"
#include <string>
#include <map>

export class Operator {
	public frequency: number = 0;
	public amplitude: number = 0;
	public envelope: number = 0;
	
	constructor(index: number) {
		this.reset(index);
	}
	
	public reset(index: number): void {
		this.frequency = 0;
		this.amplitude = (index <= 1) ? Config.operatorAmplitudeMax : 0;
		this.envelope = (index == 0) ? 0 : 1;
	}
	
	public copy(other: Operator): void {
		this.frequency = other.frequency;
		this.amplitude = other.amplitude;
		this.envelope = other.envelope;
	}
}

export class Instrument {
	public type: InstrumentType = 0;
	public wave: number = 1;
	public filter: number = 1;
	public transition: number = 1;
	public effect: number = 0;
	public chorus: number = 0;
	public volume: number = 0;
	public algorithm: number = 0;
	public feedbackType: number = 0;
	public feedbackAmplitude: number = 0;
	public feedbackEnvelope: number = 1;
	public readonly operators: Operator[] = [];
	
	constructor() {
		for (let i = 0; i < Config.operatorCount; i++) {
			this.operators.push(new Operator(i));
		}
	}
	
	public reset(): void {
		this.type = 0;
		this.wave = 1;
		this.filter = 1;
		this.transition = 1;
		this.effect = 0;
		this.chorus = 0;
		this.volume = 0;
		this.algorithm = 0;
		this.feedbackType = 0;
		this.feedbackAmplitude = 0;
		this.feedbackEnvelope = 1;
		for (let i: number = 0; i < this.operators.length; i++) {
			this.operators[i].reset(i);
		}
	}
	
	public setTypeAndReset(type: InstrumentType): void {
		this.type = type;
		switch (type) {
			case InstrumentType.chip:
				this.wave = 1;
				this.filter = 1;
				this.transition = 1;
				this.effect = 0;
				this.chorus = 0;
				this.volume = 0;
				break;
			case InstrumentType.fm:
				this.wave = 1;
				this.transition = 1;
				this.volume = 0;
				break;
			case InstrumentType.noise:
				this.transition = 1;
				this.effect = 0;
				this.algorithm = 0;
				this.feedbackType = 0;
				this.feedbackAmplitude = 0;
				this.feedbackEnvelope = 1;
				for (let i: number = 0; i < this.operators.length; i++) {
					this.operators[i].reset(i);
				}
				break;
		}
	}
	
	public copy(other: Instrument): void {
		this.type = other.type;
		this.wave = other.wave;
		this.filter = other.filter;
		this.transition = other.transition;
		this.effect = other.effect;
		this.chorus = other.chorus;
		this.volume = other.volume;
		this.algorithm = other.algorithm;
		this.feedbackType = other.feedbackType;
		this.feedbackAmplitude = other.feedbackAmplitude;
		this.feedbackEnvelope = other.feedbackEnvelope;
		for (let i: number = 0; i < this.operators.length; i++) {
			this.operators[i].copy(other.operators[i]);
		}
	}
}

export class Channel {
	public octave: number = 0;
	public readonly instruments: Instrument[] = [];
	public readonly patterns: Pattern[] = [];
	public readonly bars: number[] = [];
}

class SynthChannel {
	 sample: number = 0.0;
	 readonly phases: number[] = [];
	 readonly phaseDeltas: number[] = [];
	 readonly volumeStarts: number[] = [];
	 readonly volumeDeltas: number[] = [];
	 phaseDeltaScale: number = 0.0;
	 filter: number = 0.0;
	 filterScale: number = 0.0;
	 vibratoScale: number = 0.0;
	 harmonyMult: number = 0.0;
	 harmonyVolumeMult: number = 1.0;
	 feedbackOutputs: number[] = [];
	 feedbackMult: number = 0.0;
	 feedbackDelta: number = 0.0;
	
	constructor() {
		this.reset();
	}
	
	 reset(): void {
		for (let i: number = 0; i < Config.operatorCount; i++) {
			this.phases[i] = 0.0;
			this.feedbackOutputs[i] = 0.0;
		}
		this.sample = 0.0;
	}
}


export class Synth {
	
	private static warmUpSynthesizer(song: Song | null): void {
		// Don't bother to generate the drum waves unless the song actually
		// uses them, since they may require a lot of computation.
		if (song != null) {
			for (let i: number = 0; i < song.instrumentsPerChannel; i++) {
				for (let j: number = song.pitchChannelCount; j < song.pitchChannelCount + song.drumChannelCount; j++) {
					Config.getDrumWave(song.channels[j].instruments[i].wave);
				}
			}
			for (let i: number = 0; i < song.barCount; i++) {
				Synth.getGeneratedSynthesizer(song, i);
			}
		}
	}
	
	private static operatorAmplitudeCurve(amplitude: number): number {
		return (Math.pow(16.0, amplitude / 15.0) - 1.0) / 15.0;
	}
	
	private static readonly negativePhaseGuard: number = 1000;
	
	public samplesPerSecond: number = 44100;
	private effectDuration: number = 0.14;
	private effectAngle: number = Math.PI * 2.0 / (this.effectDuration * this.samplesPerSecond);
	private effectYMult: number = 2.0 * Math.cos(this.effectAngle);
	private limitDecay: number = 1.0 / (2.0 * this.samplesPerSecond);
	
	public song: Song | null = null;
	public pianoPressed: boolean = false;
	public pianoPitch: number[] = [0];
	public pianoChannel: number = 0;
	public enableIntro: boolean = true;
	public enableOutro: boolean = false;
	public loopCount: number = -1;
	public volume: number = 1.0;
	
	private playheadInternal: number = 0.0;
	private bar: number = 0;
	private beat: number = 0;
	private part: number = 0;
	private arpeggio: number = 0;
	private arpeggioSampleCountdown: number = 0;
	private paused: boolean = true;
	
	private readonly channels: SynthChannel[] = [];
	private stillGoing: boolean = false;
	private effectPhase: number = 0.0;
	private limit: number = 0.0;
	
	private delayLine: Float32Array = new Float32Array(16384);
	private delayPos: number = 0;
	private delayFeedback0: number = 0.0;
	private delayFeedback1: number = 0.0;
	private delayFeedback2: number = 0.0;
	private delayFeedback3: number = 0.0;
	
	private audioCtx: any;
	private scriptNode: any;
	
	public get playing(): boolean {
		return !this.paused;
	}
	
	public get playhead(): number {
		return this.playheadInternal;
	}
	
	public set playhead(value: number) {
		if (this.song != null) {
			this.playheadInternal = Math.max(0, Math.min(this.song.barCount, value));
			let remainder: number = this.playheadInternal;
			this.bar = Math.floor(remainder);
			remainder = this.song.beatsPerBar * (remainder - this.bar);
			this.beat = Math.floor(remainder);
			remainder = this.song.partsPerBeat * (remainder - this.beat);
			this.part = Math.floor(remainder);
			remainder = 4 * (remainder - this.part);
			this.arpeggio = Math.floor(remainder);
			const samplesPerArpeggio: number = this.getSamplesPerArpeggio();
			remainder = samplesPerArpeggio * (remainder - this.arpeggio);
			this.arpeggioSampleCountdown = Math.floor(samplesPerArpeggio - remainder);
			if (this.bar < this.song.loopStart) {
				this.enableIntro = true;
			}
			if (this.bar > this.song.loopStart + this.song.loopLength) {
				this.enableOutro = true;
			}
		}
	}
	
	public get totalSamples(): number {
		if (this.song == null) return 0;
		const samplesPerBar: number = this.getSamplesPerArpeggio() * 4 * this.song.partsPerBeat * this.song.beatsPerBar;
		let loopMinCount: number = this.loopCount;
		if (loopMinCount < 0) loopMinCount = 1;
		let bars: number = this.song.loopLength * loopMinCount;
		if (this.enableIntro) bars += this.song.loopStart;
		if (this.enableOutro) bars += this.song.barCount - (this.song.loopStart + this.song.loopLength);
		return bars * samplesPerBar;
	}
	
	public get totalSeconds(): number {
		return this.totalSamples / this.samplesPerSecond;
	}
	
	public get totalBars(): number {
		if (this.song == null) return 0.0;
		return this.song.barCount;
	}
	
	constructor(song: any = null) {
		if (song != null) this.setSong(song);
	}
	
	public setSong(song: any): void {
		if (typeof(song) == "string") {
			this.song = new Song(song);
		} else if (song instanceof Song) {
			this.song = song;
		}
	}
	
	public play(): void {
		if (!this.paused) return;
		this.paused = false;
		
		Synth.warmUpSynthesizer(this.song);
		
		const contextClass = (window.AudioContext || window.webkitAudioContext || window.mozAudioContext || window.oAudioContext || window.msAudioContext);
		this.audioCtx = this.audioCtx || new contextClass();
		this.scriptNode = this.audioCtx.createScriptProcessor ? this.audioCtx.createScriptProcessor(2048, 0, 1) : this.audioCtx.createJavaScriptNode(2048, 0, 1); // 2048, 0 input channels, 1 output
		this.scriptNode.onaudioprocess = this.audioProcessCallback;
		this.scriptNode.channelCountMode = 'explicit';
		this.scriptNode.channelInterpretation = 'speakers';
		this.scriptNode.connect(this.audioCtx.destination);
		
		this.samplesPerSecond = this.audioCtx.sampleRate;
		this.effectAngle = Math.PI * 2.0 / (this.effectDuration * this.samplesPerSecond);
		this.effectYMult = 2.0 * Math.cos(this.effectAngle);
		this.limitDecay = 1.0 / (2.0 * this.samplesPerSecond);
	}
	
	public pause(): void {
		if (this.paused) return;
		this.paused = true;
		this.scriptNode.disconnect(this.audioCtx.destination);
		if (this.audioCtx.close) {
			this.audioCtx.close(); // firefox is missing this function?
			this.audioCtx = null;
		}
		this.scriptNode = null;
	}
	
	public snapToStart(): void {
		this.bar = 0;
		this.enableIntro = true;
		this.snapToBar();
	}
	
	public snapToBar(bar?: number): void {
		if (bar !== undefined) this.bar = bar;
		this.playheadInternal = this.bar;
		this.beat = 0;
		this.part = 0;
		this.arpeggio = 0;
		this.arpeggioSampleCountdown = 0;
		this.effectPhase = 0.0;
		
		for (const channel of this.channels) channel.reset();
		
		this.delayPos = 0;
		this.delayFeedback0 = 0.0;
		this.delayFeedback1 = 0.0;
		this.delayFeedback2 = 0.0;
		this.delayFeedback3 = 0.0;
		for (let i: number = 0; i < this.delayLine.length; i++) this.delayLine[i] = 0.0;
	}
	
	public nextBar(): void {
		if (!this.song) return;
		const oldBar: number = this.bar;
		this.bar++;
		if (this.enableOutro) {
			if (this.bar >= this.song.barCount) {
				this.bar = this.enableIntro ? 0 : this.song.loopStart;
			}
		} else {
			if (this.bar >= this.song.loopStart + this.song.loopLength || this.bar >= this.song.barCount) {
				this.bar = this.song.loopStart;
			}
		}
		this.playheadInternal += this.bar - oldBar;
	}
	
	public prevBar(): void {
		if (!this.song) return;
		const oldBar: number = this.bar;
		this.bar--;
		if (this.bar < 0) {
			this.bar = this.song.loopStart + this.song.loopLength - 1;
		}
		if (this.bar >= this.song.barCount) {
			this.bar = this.song.barCount - 1;
		}
		if (this.bar < this.song.loopStart) {
			this.enableIntro = true;
		}
		if (!this.enableOutro && this.bar >= this.song.loopStart + this.song.loopLength) {
			this.bar = this.song.loopStart + this.song.loopLength - 1;
		}
		this.playheadInternal += this.bar - oldBar;
	}
	
	private audioProcessCallback = (audioProcessingEvent: any): void => {
		const outputBuffer = audioProcessingEvent.outputBuffer;
		const outputData: Float32Array = outputBuffer.getChannelData(0);
		this.synthesize(outputData, outputBuffer.length);
	}
	
	public synthesize(data: Float32Array, bufferLength: number): void {
		if (this.song == null) {
			for (let i: number = 0; i < bufferLength; i++) {
				data[i] = 0.0;
			}
			return;
		}
		
		const channelCount: number = this.song.getChannelCount();
		for (let i: number = this.channels.length; i < channelCount; i++) {
			this.channels[i] = new SynthChannel();
		}
		this.channels.length = channelCount;
		
		const samplesPerArpeggio: number = this.getSamplesPerArpeggio();
		let bufferIndex: number = 0;
		let ended: boolean = false;
		
		// Check the bounds of the playhead:
		if (this.arpeggioSampleCountdown == 0 || this.arpeggioSampleCountdown > samplesPerArpeggio) {
			this.arpeggioSampleCountdown = samplesPerArpeggio;
		}
		if (this.part >= this.song.partsPerBeat) {
			this.beat++;
			this.part = 0;
			this.arpeggio = 0;
			this.arpeggioSampleCountdown = samplesPerArpeggio;
		}
		if (this.beat >= this.song.beatsPerBar) {
			this.bar++;
			this.beat = 0;
			this.part = 0;
			this.arpeggio = 0;
			this.arpeggioSampleCountdown = samplesPerArpeggio;
			
			if (this.loopCount == -1) {
				if (this.bar < this.song.loopStart && !this.enableIntro) this.bar = this.song.loopStart;
				if (this.bar >= this.song.loopStart + this.song.loopLength && !this.enableOutro) this.bar = this.song.loopStart;
			}
		}
		if (this.bar >= this.song.barCount) {
			if (this.enableOutro) {
				this.bar = 0;
				this.enableIntro = true;
				ended = true;
				this.pause();
			} else {
				this.bar = this.song.loopStart;
			}
		}
		if (this.bar >= this.song.loopStart) {
			this.enableIntro = false;
		}
		
		while (true) {
			if (ended) {
				while (bufferIndex < bufferLength) {
					data[bufferIndex] = 0.0;
					bufferIndex++;
				}
				break;
			}
			
			const generatedSynthesizer: Function = Synth.getGeneratedSynthesizer(this.song, this.bar);
			bufferIndex = generatedSynthesizer(this, this.song, data, bufferLength, bufferIndex, samplesPerArpeggio);
			
			const finishedBuffer: boolean = (bufferIndex == -1);
			if (finishedBuffer) {
				break;
			} else {
				// bar changed, reset for next bar:
				this.beat = 0;
				this.effectPhase = 0.0;
				this.bar++;
				if (this.bar < this.song.loopStart) {
					if (!this.enableIntro) this.bar = this.song.loopStart;
				} else {
					this.enableIntro = false;
				}
				if (this.bar >= this.song.loopStart + this.song.loopLength) {
					if (this.loopCount > 0) this.loopCount--;
					if (this.loopCount > 0 || !this.enableOutro) {
						this.bar = this.song.loopStart;
					}
				}
				if (this.bar >= this.song.barCount) {
					this.bar = 0;
					this.enableIntro = true;
					ended = true;
					this.pause();
				}
			}
		}
		
		this.playheadInternal = (((this.arpeggio + 1.0 - this.arpeggioSampleCountdown / samplesPerArpeggio) / 4.0 + this.part) / this.song.partsPerBeat + this.beat) / this.song.beatsPerBar + this.bar;
	}
	
	private static computeOperatorEnvelope(envelope: number, time: number, beats: number, customVolume: number): number {
		switch(Config.operatorEnvelopeType[envelope]) {
			case EnvelopeType.custom: return customVolume;
			case EnvelopeType.steady: return 1.0;
			case EnvelopeType.pluck:
				let curve: number = 1.0 / (1.0 + time * Config.operatorEnvelopeSpeed[envelope]);
				if (Config.operatorEnvelopeInverted[envelope]) {
					return 1.0 - curve;
				} else {
					return curve;
				}
			case EnvelopeType.tremolo: 
				return 0.5 - Math.cos(beats * 2.0 * Math.PI * Config.operatorEnvelopeSpeed[envelope]) * 0.5;
			case EnvelopeType.punch: 
				return Math.max(1.0, 2.0 - time * 10.0);
			case EnvelopeType.flare:
				const speed: number = Config.operatorEnvelopeSpeed[envelope];
				const attack: number = 0.25 / Math.sqrt(speed);
				return time < attack ? time / attack : 1.0 / (1.0 + (time - attack) * speed);
			default: throw new Error("Unrecognized operator envelope type.");
		}
	}
	
	private static computeChannelInstrument(synth: Synth, song: Song, channel: number, time: number, sampleTime: number, samplesPerArpeggio: number, samples: number): void {
		const isDrum: boolean = song.getChannelIsDrum(channel);
		const synthChannel: SynthChannel = synth.channels[channel];
		const pattern: Pattern | null = song.getPattern(channel, synth.bar);
		const instrument: Instrument = song.channels[channel].instruments[pattern == null ? 0 : pattern.instrument];
		const pianoMode = (synth.pianoPressed && channel == synth.pianoChannel);
		const basePitch: number = isDrum ? Config.drumBasePitches[instrument.wave] : Config.keyTransposes[song.key];
		const intervalScale: number = isDrum ? Config.drumInterval : 1;
		const pitchDamping: number = isDrum ? (Config.drumWaveIsSoft[instrument.wave] ? 24.0 : 60.0) : 48.0;
		const secondsPerPart: number = 4.0 * samplesPerArpeggio / synth.samplesPerSecond;
		const beatsPerPart: number = 1.0 / song.partsPerBeat;
		
		synthChannel.phaseDeltaScale = 0.0;
		synthChannel.filter = 1.0;
		synthChannel.filterScale = 1.0;
		synthChannel.vibratoScale = 0.0;
		synthChannel.harmonyMult = 1.0;
		synthChannel.harmonyVolumeMult = 1.0;
		
		let partsSinceStart: number = 0.0;
		let arpeggio: number = synth.arpeggio;
		let arpeggioSampleCountdown: number = synth.arpeggioSampleCountdown;
		
		let pitches: number[] | null = null;
		let resetPhases: boolean = true;
		
		let intervalStart: number = 0.0;
		let intervalEnd: number = 0.0;
		let transitionVolumeStart: number = 1.0;
		let transitionVolumeEnd: number = 1.0;
		let envelopeVolumeStart: number = 0.0;
		let envelopeVolumeEnd: number = 0.0;
		// TODO: probably part time can be calculated independently of any notes?
		let partTimeStart: number = 0.0;
		let partTimeEnd:   number = 0.0;
		let decayTimeStart: number = 0.0;
		let decayTimeEnd:   number = 0.0;
		
		for (let i: number = 0; i < Config.operatorCount; i++) {
			synthChannel.phaseDeltas[i] = 0.0;
			synthChannel.volumeStarts[i] = 0.0;
			synthChannel.volumeDeltas[i] = 0.0;
		}
		
		if (pianoMode) {
			pitches = synth.pianoPitch;
			transitionVolumeStart = transitionVolumeEnd = 1;
			envelopeVolumeStart = envelopeVolumeEnd = 1;
			resetPhases = false;
			// TODO: track time since live piano note started for transition, envelope, decays, delayed vibrato, etc.
		} else if (pattern != null) {
			let note: Note | null = null;
			let prevNote: Note | null = null;
			let nextNote: Note | null = null;
			for (let i: number = 0; i < pattern.notes.length; i++) {
				if (pattern.notes[i].end <= time) {
					prevNote = pattern.notes[i];
				} else if (pattern.notes[i].start <= time && pattern.notes[i].end > time) {
					note = pattern.notes[i];
				} else if (pattern.notes[i].start > time) {
					nextNote = pattern.notes[i];
					break;
				}
			}
			if (note != null && prevNote != null && prevNote.end != note.start) prevNote = null;
			if (note != null && nextNote != null && nextNote.start != note.end) nextNote = null;
			
			if (note != null) {
				pitches = note.pitches;
				partsSinceStart = time - note.start;
				
				let endPinIndex: number;
				for (endPinIndex = 1; endPinIndex < note.pins.length - 1; endPinIndex++) {
					if (note.pins[endPinIndex].time + note.start > time) break;
				}
				const startPin: NotePin = note.pins[endPinIndex-1];
				const endPin: NotePin = note.pins[endPinIndex];
				const noteStart: number = note.start * 4;
				const noteEnd:   number = note.end   * 4;
				const pinStart: number  = (note.start + startPin.time) * 4;
				const pinEnd:   number  = (note.start +   endPin.time) * 4;
				
				const tickTimeStart: number = time * 4 + arpeggio;
				const tickTimeEnd:   number = time * 4 + arpeggio + 1;
				const pinRatioStart: number = (tickTimeStart - pinStart) / (pinEnd - pinStart);
				const pinRatioEnd:   number = (tickTimeEnd   - pinStart) / (pinEnd - pinStart);
				let envelopeVolumeTickStart: number = startPin.volume + (endPin.volume - startPin.volume) * pinRatioStart;
				let envelopeVolumeTickEnd:   number = startPin.volume + (endPin.volume - startPin.volume) * pinRatioEnd;
				let transitionVolumeTickStart: number = 1.0;
				let transitionVolumeTickEnd:   number = 1.0;
				let intervalTickStart: number = startPin.interval + (endPin.interval - startPin.interval) * pinRatioStart;
				let intervalTickEnd:   number = startPin.interval + (endPin.interval - startPin.interval) * pinRatioEnd;
				let partTimeTickStart: number = startPin.time + (endPin.time - startPin.time) * pinRatioStart;
				let partTimeTickEnd:   number = startPin.time + (endPin.time - startPin.time) * pinRatioEnd;
				let decayTimeTickStart: number = partTimeTickStart;
				let decayTimeTickEnd:   number = partTimeTickEnd;
				
				const startRatio: number = 1.0 - (arpeggioSampleCountdown + samples) / samplesPerArpeggio;
				const endRatio:   number = 1.0 - (arpeggioSampleCountdown)           / samplesPerArpeggio;
				resetPhases = (tickTimeStart + startRatio - noteStart == 0.0);
				
				const transition: number = instrument.transition;
				if (tickTimeStart == noteStart) {
					if (transition == 0) {
						// seamless start
						resetPhases = false;
					} else if (transition == 2) {
						// smooth start
						transitionVolumeTickStart = 0.0;
					} else if (transition == 3) {
						// slide start
						if (prevNote == null) {
							transitionVolumeTickStart = 0.0;
						} else if (prevNote.pins[prevNote.pins.length-1].volume == 0 || note.pins[0].volume == 0) {
							transitionVolumeTickStart = 0.0;
						} else {
							intervalTickStart = (prevNote.pitches[0] + prevNote.pins[prevNote.pins.length-1].interval - note.pitches[0]) * 0.5;
							decayTimeTickStart = prevNote.pins[prevNote.pins.length-1].time * 0.5;
							resetPhases = false;
						}
					}
				}
				if (tickTimeEnd == noteEnd) {
					if (transition == 0) {
						// seamless ending: fade out, unless adjacent to another note or at end of bar.
						if (nextNote == null && note.start + endPin.time != song.partsPerBeat * song.beatsPerBar) {
							transitionVolumeTickEnd = 0.0;
						}
					} else if (transition == 1 || transition == 2) {
						// sudden/smooth ending
						transitionVolumeTickEnd = 0.0;
					} else if (transition == 3) {
						// slide ending
						if (nextNote == null) {
							transitionVolumeTickEnd = 0.0;
						} else if (note.pins[note.pins.length-1].volume == 0 || nextNote.pins[0].volume == 0) {
							transitionVolumeTickEnd = 0.0;
						} else {
							intervalTickEnd = (nextNote.pitches[0] - note.pitches[0] + note.pins[note.pins.length-1].interval) * 0.5;
							decayTimeTickEnd *= 0.5;
						}
					}
				}
				
				intervalStart = intervalTickStart + (intervalTickEnd - intervalTickStart) * startRatio;
				intervalEnd   = intervalTickStart + (intervalTickEnd - intervalTickStart) * endRatio;
				envelopeVolumeStart = synth.volumeConversion(envelopeVolumeTickStart + (envelopeVolumeTickEnd - envelopeVolumeTickStart) * startRatio);
				envelopeVolumeEnd   = synth.volumeConversion(envelopeVolumeTickStart + (envelopeVolumeTickEnd - envelopeVolumeTickStart) * endRatio);
				transitionVolumeStart = transitionVolumeTickStart + (transitionVolumeTickEnd - transitionVolumeTickStart) * startRatio;
				transitionVolumeEnd   = transitionVolumeTickStart + (transitionVolumeTickEnd - transitionVolumeTickStart) * endRatio;
				partTimeStart = note.start + partTimeTickStart + (partTimeTickEnd - partTimeTickStart) * startRatio;
				partTimeEnd   = note.start + partTimeTickStart + (partTimeTickEnd - partTimeTickStart) * endRatio;
				decayTimeStart = decayTimeTickStart + (decayTimeTickEnd - decayTimeTickStart) * startRatio;
				decayTimeEnd   = decayTimeTickStart + (decayTimeTickEnd - decayTimeTickStart) * endRatio;
			}
		}
		
		if (pitches != null) {
			if (!isDrum && instrument.type == InstrumentType.fm) {
				// phase modulation!
				
				let sineVolumeBoost: number = 1.0;
				let totalCarrierVolume: number = 0.0;
				
				const carrierCount: number = Config.operatorCarrierCounts[instrument.algorithm];
				for (let i: number = 0; i < Config.operatorCount; i++) {
					const associatedCarrierIndex: number = Config.operatorAssociatedCarrier[instrument.algorithm][i] - 1;
					const pitch: number = pitches[(i < pitches.length) ? i : ((associatedCarrierIndex < pitches.length) ? associatedCarrierIndex : 0)];
					const freqMult = Config.operatorFrequencies[instrument.operators[i].frequency];
					const chorusInterval = Config.operatorCarrierChorus[associatedCarrierIndex];
					const startPitch: number = (pitch + intervalStart) * intervalScale + chorusInterval;
					const startFreq: number = freqMult * (synth.frequencyFromPitch(basePitch + startPitch)) + Config.operatorHzOffsets[instrument.operators[i].frequency];
					
					synthChannel.phaseDeltas[i] = startFreq * sampleTime * Config.sineWaveLength;
					if (resetPhases) synthChannel.reset();
					
					const amplitudeCurve: number = Synth.operatorAmplitudeCurve(instrument.operators[i].amplitude);
					const amplitudeMult: number = amplitudeCurve * Config.operatorAmplitudeSigns[instrument.operators[i].frequency];
					let volumeStart: number = amplitudeMult;
					let volumeEnd: number = amplitudeMult;
					if (i < carrierCount) {
						// carrier
						const volumeMult: number = 0.03;
						const endPitch: number = (pitch + intervalEnd) * intervalScale;
						const pitchVolumeStart: number = Math.pow(2.0, -startPitch / pitchDamping);
						const pitchVolumeEnd: number   = Math.pow(2.0,   -endPitch / pitchDamping);
						volumeStart *= pitchVolumeStart * volumeMult * transitionVolumeStart;
						volumeEnd *= pitchVolumeEnd * volumeMult * transitionVolumeEnd;
						
						totalCarrierVolume += amplitudeCurve;
					} else {
						// modulator
						volumeStart *= Config.sineWaveLength * 1.5;
						volumeEnd *= Config.sineWaveLength * 1.5;
						
						sineVolumeBoost *= 1.0 - Math.min(1.0, instrument.operators[i].amplitude / 15);
					}
					const envelope: number = instrument.operators[i].envelope;
					
					volumeStart *= Synth.computeOperatorEnvelope(envelope, secondsPerPart * decayTimeStart, beatsPerPart * partTimeStart, envelopeVolumeStart);
					volumeEnd *= Synth.computeOperatorEnvelope(envelope, secondsPerPart * decayTimeEnd, beatsPerPart * partTimeEnd, envelopeVolumeEnd);
					
					synthChannel.volumeStarts[i] = volumeStart;
					synthChannel.volumeDeltas[i] = (volumeEnd - volumeStart) / samples;
				}
				
				const feedbackAmplitude: number = Config.sineWaveLength * 0.3 * instrument.feedbackAmplitude / 15.0;
				let feedbackStart: number = feedbackAmplitude * Synth.computeOperatorEnvelope(instrument.feedbackEnvelope, secondsPerPart * decayTimeStart, beatsPerPart * partTimeStart, envelopeVolumeStart);
				let feedbackEnd: number = feedbackAmplitude * Synth.computeOperatorEnvelope(instrument.feedbackEnvelope, secondsPerPart * decayTimeEnd, beatsPerPart * partTimeEnd, envelopeVolumeEnd);
				synthChannel.feedbackMult = feedbackStart;
				synthChannel.feedbackDelta = (feedbackEnd - synthChannel.feedbackMult) / samples;
				
				sineVolumeBoost *= 1.0 - instrument.feedbackAmplitude / 15.0;
				
				sineVolumeBoost *= 1.0 - Math.min(1.0, Math.max(0.0, totalCarrierVolume - 1) / 2.0);
				for (let i: number = 0; i < carrierCount; i++) {
					synthChannel.volumeStarts[i] *= 1.0 + sineVolumeBoost * 3.0;
					synthChannel.volumeDeltas[i] *= 1.0 + sineVolumeBoost * 3.0;
				}
			} else {
				let pitch: number = pitches[0];
				if (Config.chorusHarmonizes[instrument.chorus]) {
					let harmonyOffset: number = 0.0;
					if (pitches.length == 2) {
						harmonyOffset = pitches[1] - pitches[0];
					} else if (pitches.length == 3) {
						harmonyOffset = pitches[(arpeggio >> 1) + 1] - pitches[0];
					} else if (pitches.length == 4) {
						harmonyOffset = pitches[(arpeggio == 3 ? 1 : arpeggio) + 1] - pitches[0];
					}
					synthChannel.harmonyMult = Math.pow(2.0, harmonyOffset / 12.0);
					synthChannel.harmonyVolumeMult = Math.pow(2.0, -harmonyOffset / pitchDamping)
				} else {
					if (pitches.length == 2) {
						pitch = pitches[arpeggio >> 1];
					} else if (pitches.length == 3) {
						pitch = pitches[arpeggio == 3 ? 1 : arpeggio];
					} else if (pitches.length == 4) {
						pitch = pitches[arpeggio];
					}
				}
				
				const startPitch: number = (pitch + intervalStart) * intervalScale;
				const endPitch: number = (pitch + intervalEnd) * intervalScale;
				const startFreq: number = synth.frequencyFromPitch(basePitch + startPitch);
				const pitchVolumeStart: number = Math.pow(2.0, -startPitch / pitchDamping);
				const pitchVolumeEnd: number   = Math.pow(2.0,   -endPitch / pitchDamping);
				if (isDrum && Config.drumWaveIsSoft[instrument.wave]) {
					synthChannel.filter = Math.min(1.0, startFreq * sampleTime * Config.drumPitchFilterMult[instrument.wave]);
				}
				let settingsVolumeMult: number;
				if (!isDrum) {
					const filterScaleRate: number = Config.filterDecays[instrument.filter];
					synthChannel.filter = Math.pow(2, -filterScaleRate * secondsPerPart * decayTimeStart);
					const endFilter: number = Math.pow(2, -filterScaleRate * secondsPerPart * decayTimeEnd);
					synthChannel.filterScale = Math.pow(endFilter / synthChannel.filter, 1.0 / samples);
					settingsVolumeMult = 0.27 * 0.5 * Config.waveVolumes[instrument.wave] * Config.filterVolumes[instrument.filter] * Config.chorusVolumes[instrument.chorus];
				} else {
					settingsVolumeMult = 0.19 * Config.drumVolumes[instrument.wave];
				}
				if (resetPhases && !isDrum) {
					synthChannel.reset();
				}
				
				synthChannel.phaseDeltas[0] = startFreq * sampleTime;
				
				const instrumentVolumeMult: number = (instrument.volume == 5) ? 0.0 : Math.pow(2, -Config.volumeValues[instrument.volume]);
				synthChannel.volumeStarts[0] = transitionVolumeStart * envelopeVolumeStart * pitchVolumeStart * settingsVolumeMult * instrumentVolumeMult;
				const volumeEnd: number = transitionVolumeEnd * envelopeVolumeEnd * pitchVolumeEnd * settingsVolumeMult * instrumentVolumeMult;
				synthChannel.volumeDeltas[0] = (volumeEnd - synthChannel.volumeStarts[0]) / samples;
			}
			
			synthChannel.phaseDeltaScale = Math.pow(2.0, ((intervalEnd - intervalStart) * intervalScale / 12.0) / samples);
			synthChannel.vibratoScale = (partsSinceStart < Config.effectVibratoDelays[instrument.effect]) ? 0.0 : Math.pow(2.0, Config.effectVibratos[instrument.effect] / 12.0) - 1.0;
		} else {
			if (!isDrum) {
				synthChannel.reset();
			}
			for (let i: number = 0; i < Config.operatorCount; i++) {
				synthChannel.phaseDeltas[0] = 0.0;
				synthChannel.volumeStarts[0] = 0.0;
				synthChannel.volumeDeltas[0] = 0.0;
			}
		}
	}
	
	private static readonly generatedSynthesizers: Dictionary<Function> = {};
	
	private static getGeneratedSynthesizer(song: Song, bar: number): Function {
		const fingerprint: string = song.getChannelFingerprint(bar);
		if (Synth.generatedSynthesizers[fingerprint] == undefined) {
			const synthSource: string[] = [];
			const instruments: Instrument[] = [];
			for (let channel = 0; channel < song.pitchChannelCount; channel++) {
				instruments[channel] = song.channels[channel].instruments[song.getPatternInstrument(channel, bar)];
			}
			
			for (const line of Synth.synthSourceTemplate) {
				if (line.indexOf("#") != -1) {
					if (line.indexOf("// PITCH") != -1) {
						for (let channel = 0; channel < song.pitchChannelCount; channel++) {
							synthSource.push(line.replace(/#/g, channel + ""));
						}
					} else if (line.indexOf("// CHIP") != -1) {
						for (let channel = 0; channel < song.pitchChannelCount; channel++) {
							if (instruments[channel].type == InstrumentType.chip) {
								synthSource.push(line.replace(/#/g, channel + ""));
							}
						}
					} else if (line.indexOf("// FM") != -1) {
						for (let channel = 0; channel < song.pitchChannelCount; channel++) {
							if (instruments[channel].type == InstrumentType.fm) {
								if (line.indexOf("$") != -1) {
									for (let j = 0; j < Config.operatorCount; j++) {
										synthSource.push(line.replace(/#/g, channel + "").replace(/\$/g, j + ""));
									}
								} else {
									synthSource.push(line.replace(/#/g, channel + ""));
								}
							}
						}
					} else if (line.indexOf("// CARRIER OUTPUTS") != -1) {
						for (let channel = 0; channel < song.pitchChannelCount; channel++) {
							if (instruments[channel].type == InstrumentType.fm) {
								const outputs: string[] = [];
								for (let j = 0; j < Config.operatorCarrierCounts[instruments[channel].algorithm]; j++) {
									outputs.push("channel" + channel + "Operator" + j + "Scaled");
								}
								synthSource.push(line.replace(/#/g, channel + "").replace("/*channel" + channel + "Operator$Scaled*/", outputs.join(" + ")));
							}
						}
					} else if (line.indexOf("// NOISE") != -1) {
						for (let channel = song.pitchChannelCount; channel < song.pitchChannelCount + song.drumChannelCount; channel++) {
							synthSource.push(line.replace(/#/g, channel + ""));
						}
					} else if (line.indexOf("// ALL") != -1) {
						for (let channel = 0; channel < song.pitchChannelCount + song.drumChannelCount; channel++) {
							synthSource.push(line.replace(/#/g, channel + ""));
						}
					} else {
						throw new Error("Missing channel type annotation for line: " + line);
					}
				} else if (line.indexOf("// INSERT OPERATOR COMPUTATION HERE") != -1) {
					for (let j = Config.operatorCount - 1; j >= 0; j--) {
						for (const operatorLine of Synth.operatorSourceTemplate) {
							for (let channel = 0; channel < song.pitchChannelCount; channel++) {
								if (instruments[channel].type == InstrumentType.fm) {
									
									if (operatorLine.indexOf("/* + channel#Operator@Scaled*/") != -1) {
										let modulators = "";
										for (const modulatorNumber of Config.operatorModulatedBy[instruments[channel].algorithm][j]) {
											modulators += " + channel" + channel + "Operator" + (modulatorNumber - 1) + "Scaled";
										}
										
										const feedbackIndices: ReadonlyArray<number> = Config.operatorFeedbackIndices[instruments[channel].feedbackType][j];
										if (feedbackIndices.length > 0) {
											modulators += " + channel" + channel + "FeedbackMult * (";
											const feedbacks: string[] = [];
											for (const modulatorNumber of feedbackIndices) {
												feedbacks.push("channel" + channel + "Operator" + (modulatorNumber - 1) + "Output");
											}
											modulators += feedbacks.join(" + ") + ")";
										}
										synthSource.push(operatorLine.replace(/#/g, channel + "").replace(/\$/g, j + "").replace("/* + channel" + channel + "Operator@Scaled*/", modulators));
									} else {
										synthSource.push(operatorLine.replace(/#/g, channel + "").replace(/\$/g, j + ""));
									}
								}
							}
						}
					}
				} else {
					synthSource.push(line);
				}
			}
			
			//console.log(synthSource.join("\n"));
			
			Synth.generatedSynthesizers[fingerprint] = new Function("synth", "song", "data", "bufferLength", "bufferIndex", "samplesPerArpeggio", synthSource.join("\n"));
		}
		return Synth.generatedSynthesizers[fingerprint];
	}
	
	private static synthSourceTemplate: string[] = (`
		var sampleTime = 1.0 / synth.samplesPerSecond;
		var effectYMult = +synth.effectYMult;
		var limitDecay = +synth.limitDecay;
		var volume = +synth.volume;
		var delayLine = synth.delayLine;
		var reverb = Math.pow(song.reverb / beepbox.Config.reverbRange, 0.667) * 0.425;
		var sineWave = beepbox.Config.sineWave;
		
		// Initialize instruments based on current pattern.
		var instrumentChannel# = song.getPatternInstrument(#, synth.bar); // ALL
		var instrument# = song.channels[#].instruments[instrumentChannel#]; // ALL
		var channel#Wave = beepbox.Config.waves[instrument#.wave]; // CHIP
		var channel#Wave = beepbox.Config.getDrumWave(instrument#.wave); // NOISE
		var channel#WaveLength = channel#Wave.length; // CHIP
		var channel#FilterBase = Math.pow(2, -beepbox.Config.filterBases[instrument#.filter]); // CHIP
		var channel#TremoloScale = beepbox.Config.effectTremolos[instrument#.effect]; // PITCH
		
		while (bufferIndex < bufferLength) {
			
			var samples;
			var samplesLeftInBuffer = bufferLength - bufferIndex;
			if (synth.arpeggioSampleCountdown <= samplesLeftInBuffer) {
				samples = synth.arpeggioSampleCountdown;
			} else {
				samples = samplesLeftInBuffer;
			}
			synth.arpeggioSampleCountdown -= samples;
			
			var time = synth.part + synth.beat * song.partsPerBeat;
			
			beepbox.Synth.computeChannelInstrument(synth, song, #, time, sampleTime, samplesPerArpeggio, samples); // ALL
			var synthChannel# = synth.channels[#]; // ALL
			
			var channel#ChorusA = Math.pow(2.0, (beepbox.Config.chorusOffsets[instrument#.chorus] + beepbox.Config.chorusIntervals[instrument#.chorus]) / 12.0); // CHIP
			var channel#ChorusB = Math.pow(2.0, (beepbox.Config.chorusOffsets[instrument#.chorus] - beepbox.Config.chorusIntervals[instrument#.chorus]) / 12.0); // CHIP
			var channel#ChorusSign = synthChannel#.harmonyVolumeMult * beepbox.Config.chorusSigns[instrument#.chorus]; // CHIP
			if (instrument#.chorus == 0) synthChannel#.phases[1] = synthChannel#.phases[0]; // CHIP
			channel#ChorusB *= synthChannel#.harmonyMult; // CHIP
			var channel#ChorusDeltaRatio = channel#ChorusB / channel#ChorusA; // CHIP
			
			var channel#PhaseDelta = synthChannel#.phaseDeltas[0] * channel#ChorusA; // CHIP
			var channel#PhaseDelta = synthChannel#.phaseDeltas[0] / 32768.0; // NOISE
			var channel#PhaseDeltaScale = synthChannel#.phaseDeltaScale; // ALL
			var channel#Volume = synthChannel#.volumeStarts[0]; // CHIP
			var channel#Volume = synthChannel#.volumeStarts[0]; // NOISE
			var channel#VolumeDelta = synthChannel#.volumeDeltas[0]; // CHIP
			var channel#VolumeDelta = synthChannel#.volumeDeltas[0]; // NOISE
			var channel#Filter = synthChannel#.filter * channel#FilterBase; // CHIP
			var channel#Filter = synthChannel#.filter; // NOISE
			var channel#FilterScale = synthChannel#.filterScale; // CHIP
			var channel#VibratoScale = synthChannel#.vibratoScale; // PITCH
			
			var effectY     = Math.sin(synth.effectPhase);
			var prevEffectY = Math.sin(synth.effectPhase - synth.effectAngle);
			
			var channel#PhaseA = synth.channels[#].phases[0] % 1; // CHIP
			var channel#PhaseB = synth.channels[#].phases[1] % 1; // CHIP
			var channel#Phase  = synth.channels[#].phases[0] % 1; // NOISE
			
			var channel#Operator$Phase       = ((synth.channels[#].phases[$] % 1) + ` + Synth.negativePhaseGuard + `) * ` + Config.sineWaveLength + `; // FM
			var channel#Operator$PhaseDelta  = synthChannel#.phaseDeltas[$]; // FM
			var channel#Operator$OutputMult  = synthChannel#.volumeStarts[$]; // FM
			var channel#Operator$OutputDelta = synthChannel#.volumeDeltas[$]; // FM
			var channel#Operator$Output      = synthChannel#.feedbackOutputs[$]; // FM
			var channel#FeedbackMult         = synthChannel#.feedbackMult; // FM
			var channel#FeedbackDelta        = synthChannel#.feedbackDelta; // FM
			
			var channel#Sample = +synth.channels[#].sample; // ALL
			
			var delayPos = 0|synth.delayPos;
			var delayFeedback0 = +synth.delayFeedback0;
			var delayFeedback1 = +synth.delayFeedback1;
			var delayFeedback2 = +synth.delayFeedback2;
			var delayFeedback3 = +synth.delayFeedback3;
			var limit = +synth.limit;
			
			while (samples) {
				var channel#Vibrato = 1.0 + channel#VibratoScale * effectY; // PITCH
				var channel#Tremolo = 1.0 + channel#TremoloScale * (effectY - 1.0); // PITCH
				var temp = effectY;
				effectY = effectYMult * effectY - prevEffectY;
				prevEffectY = temp;
				
				channel#Sample += ((channel#Wave[0|(channel#PhaseA * channel#WaveLength)] + channel#Wave[0|(channel#PhaseB * channel#WaveLength)] * channel#ChorusSign) * channel#Volume * channel#Tremolo - channel#Sample) * channel#Filter; // CHIP
				channel#Sample += (channel#Wave[0|(channel#Phase * 32768.0)] * channel#Volume - channel#Sample) * channel#Filter; // NOISE
				channel#Volume += channel#VolumeDelta; // CHIP
				channel#Volume += channel#VolumeDelta; // NOISE
				channel#PhaseA += channel#PhaseDelta * channel#Vibrato; // CHIP
				channel#PhaseB += channel#PhaseDelta * channel#Vibrato * channel#ChorusDeltaRatio; // CHIP
				channel#Phase += channel#PhaseDelta; // NOISE
				channel#Filter *= channel#FilterScale; // CHIP
				channel#PhaseA -= 0|channel#PhaseA; // CHIP
				channel#PhaseB -= 0|channel#PhaseB; // CHIP
				channel#Phase -= 0|channel#Phase; // NOISE
				channel#PhaseDelta *= channel#PhaseDeltaScale; // CHIP
				channel#PhaseDelta *= channel#PhaseDeltaScale; // NOISE
				
				// INSERT OPERATOR COMPUTATION HERE
				channel#Sample = channel#Tremolo * (/*channel#Operator$Scaled*/); // CARRIER OUTPUTS
				channel#FeedbackMult += channel#FeedbackDelta; // FM
				channel#Operator$OutputMult += channel#Operator$OutputDelta; // FM
				channel#Operator$Phase += channel#Operator$PhaseDelta * channel#Vibrato; // FM
				channel#Operator$PhaseDelta *= channel#PhaseDeltaScale; // FM
				
				// Reverb, implemented using a feedback delay network with a Hadamard matrix and lowpass filters.
				// good ratios:    0.555235 + 0.618033 + 0.818 +   1.0 = 2.991268
				// Delay lengths:  3041     + 3385     + 4481  +  5477 = 16384 = 2^14
				// Buffer offsets: 3041    -> 6426   -> 10907 -> 16384
				var delayPos1 = (delayPos +  3041) & 0x3FFF;
				var delayPos2 = (delayPos +  6426) & 0x3FFF;
				var delayPos3 = (delayPos + 10907) & 0x3FFF;
				var delaySample0 = (delayLine[delayPos]
					+ channel#Sample // PITCH
				);
				var delaySample1 = delayLine[delayPos1];
				var delaySample2 = delayLine[delayPos2];
				var delaySample3 = delayLine[delayPos3];
				var delayTemp0 = -delaySample0 + delaySample1;
				var delayTemp1 = -delaySample0 - delaySample1;
				var delayTemp2 = -delaySample2 + delaySample3;
				var delayTemp3 = -delaySample2 - delaySample3;
				delayFeedback0 += ((delayTemp0 + delayTemp2) * reverb - delayFeedback0) * 0.5;
				delayFeedback1 += ((delayTemp1 + delayTemp3) * reverb - delayFeedback1) * 0.5;
				delayFeedback2 += ((delayTemp0 - delayTemp2) * reverb - delayFeedback2) * 0.5;
				delayFeedback3 += ((delayTemp1 - delayTemp3) * reverb - delayFeedback3) * 0.5;
				delayLine[delayPos1] = delayFeedback0;
				delayLine[delayPos2] = delayFeedback1;
				delayLine[delayPos3] = delayFeedback2;
				delayLine[delayPos ] = delayFeedback3;
				delayPos = (delayPos + 1) & 0x3FFF;
				
				var sample = delaySample0 + delaySample1 + delaySample2 + delaySample3
					+ channel#Sample // NOISE
				;
				
				var abs = sample < 0.0 ? -sample : sample;
				limit -= limitDecay;
				if (limit < abs) limit = abs;
				sample /= limit * 0.75 + 0.25;
				sample *= volume;
				data[bufferIndex] = sample;
				bufferIndex++;
				samples--;
			}
			
			synthChannel#.phases[0] = channel#PhaseA; // CHIP
			synthChannel#.phases[1] = channel#PhaseB; // CHIP
			synthChannel#.phases[0] = channel#Phase; // NOISE
			synthChannel#.phases[$] = channel#Operator$Phase / ` + Config.sineWaveLength + `; // FM
			synthChannel#.feedbackOutputs[$] = channel#Operator$Output; // FM
			synthChannel#.sample = channel#Sample; // ALL
			
			synth.delayPos = delayPos;
			synth.delayFeedback0 = delayFeedback0;
			synth.delayFeedback1 = delayFeedback1;
			synth.delayFeedback2 = delayFeedback2;
			synth.delayFeedback3 = delayFeedback3;
			synth.limit = limit;
			
			if (effectYMult * effectY - prevEffectY > prevEffectY) {
				synth.effectPhase = Math.asin(effectY);
			} else {
				synth.effectPhase = Math.PI - Math.asin(effectY);
			}
			
			if (synth.arpeggioSampleCountdown == 0) {
				synth.arpeggio++;
				synth.arpeggioSampleCountdown = samplesPerArpeggio;
				if (synth.arpeggio == 4) {
					synth.arpeggio = 0;
					synth.part++;
					if (synth.part == song.partsPerBeat) {
						synth.part = 0;
						synth.beat++;
						if (synth.beat == song.beatsPerBar) {
							// The bar ended, may need to regenerate synthesizer.
							return bufferIndex;
						}
					}
				}
			}
		}
		
		// Indicate that the buffer is finished generating.
		return -1;
	`).split("\n");
	
	private static operatorSourceTemplate: string[] = (`
					var channel#Operator$PhaseMix = channel#Operator$Phase/* + channel#Operator@Scaled*/;
					var channel#Operator$PhaseInt = channel#Operator$PhaseMix|0;
					var channel#Operator$Index    = channel#Operator$PhaseInt & ` + Config.sineWaveMask + `;
					var channel#Operator$Sample   = sineWave[channel#Operator$Index];
					channel#Operator$Output       = channel#Operator$Sample + (sineWave[channel#Operator$Index + 1] - channel#Operator$Sample) * (channel#Operator$PhaseMix - channel#Operator$PhaseInt);
					var channel#Operator$Scaled   = channel#Operator$OutputMult * channel#Operator$Output;
	`).split("\n");
	
	private frequencyFromPitch(pitch: number): number {
		return 440.0 * Math.pow(2.0, (pitch - 69.0) / 12.0);
	}
	
	private volumeConversion(noteVolume: number): number {
		return Math.pow(noteVolume / 3.0, 1.5);
	}
	
	private getSamplesPerArpeggio(): number {
		if (this.song == null) return 0;
		const beatsPerMinute: number = this.song.getBeatsPerMinute();
		const beatsPerSecond: number = beatsPerMinute / 60.0;
		const partsPerSecond: number = beatsPerSecond * this.song.partsPerBeat;
		const arpeggioPerSecond: number = partsPerSecond * 4.0;
		return Math.floor(this.samplesPerSecond / arpeggioPerSecond);
	}
}
