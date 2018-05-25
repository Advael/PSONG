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
struct Note {
	Note() : start(0), end(0) {}
	Note(int p, int s, int e, int volume, bool fade = false){
		pitches.push_back(p);
		pins.push_back(NotePin(0, 0, volume));
		pins.push_back(NotePin(0, end - start, fadeout ? 0 : volume));
		start = s;
		end = e;
	}

	Note(const Note& o){
		for(auto& i:o.pitches) pitches.push_back(i);
		for(auto& i:o.pins) pins.push_back(i);
		start = o.start;
		end = o.end;
	}

	vector<int> pitches;
	vector<NotePin> pins;
	int start;
	int end;
};
