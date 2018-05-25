/*Note classes and such*/
#include <vector>
struct NotePin {
	int interval;
	float time;
	float volume;
	interval: number;
	time: number;
	volume: number;
}

export function makeNotePin(interval: number, time: number, volume: number): NotePin {
	return {interval: interval, time: time, volume: volume};
}

export interface Note {
	pitches: number[];
	pins: NotePin[];
	start: number;
	end: number;
}

export function makeNote(pitch: number, start: number, end: number, volume: number, fadeout: boolean = false) {
	return {
		pitches: [pitch],
		pins: [makeNotePin(0, 0, volume), makeNotePin(0, end - start, fadeout ? 0 : volume)],
		start: start,
		end: end,
	};
}

export class Pattern {
	public notes: Note[] = [];
	public instrument: number = 0;
	
	public cloneNotes(): Note[] {
		const result: Note[] = [];
		for (const oldNote of this.notes) {
			const newNote: Note = makeNote(-1, oldNote.start, oldNote.end, 3);
			newNote.pitches = oldNote.pitches.concat();
			newNote.pins = [];
			for (const oldPin of oldNote.pins) {
				newNote.pins.push(makeNotePin(oldPin.interval, oldPin.time, oldPin.volume));
			}
			result.push(newNote);
		}
		return result;
	}
	
	public reset(): void {
		this.notes.length = 0;
		this.instrument = 0;
	}
}
