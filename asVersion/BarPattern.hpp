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
#include <vector>
struct BarPattern {
	vector<Note> notes;
	int instrument;
	BarPattern() : instrument(0){
		notes = [];
		//notes = [new Note(12, 0, 8)];
		//notes[0].pins = [new NotePin(0, 0), new NotePin(0, 3), new NotePin(2, 4), new NotePin(2, 8)];
	}
	vector<Note> cloneNotes(){
		vector<Note> result;
		for(auto &oldNote:notes){
			Note newNote = Note(-1, oldNote.start, oldNote.end, 3);
			newNote.pitches.insert(newNote.end(), oldNote.pitches.begin(), oldNote.pitches.end())
			newNote.pins;
			for(auto &NotePin:oldNote.pins) {
				newNote.pins.push_back(NotePin(oldPin));
			}
			result.push_back(newNote);
		}
		return result;
	}
}
