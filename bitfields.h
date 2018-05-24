/*Bitfield nonsense. This is not done yet. I think the dude went and made the system way more complicated than it needs to be. But it's still simpler to port this than to work with something we can't test*/
struct BitFieldReader {
	private _bits: number[] = [];
	private _readIndex: number = 0;
	
	constructor(base64CharCodeToInt: ReadonlyArray<number>, source: string, startIndex: number, stopIndex: number) {
		for (let i: number = startIndex; i < stopIndex; i++) {
			const value: number = base64CharCodeToInt[source.charCodeAt(i)];
			this._bits.push((value >> 5) & 0x1);
			this._bits.push((value >> 4) & 0x1);
			this._bits.push((value >> 3) & 0x1);
			this._bits.push((value >> 2) & 0x1);
			this._bits.push((value >> 1) & 0x1);
			this._bits.push( value       & 0x1);
		}
	}
	
	public read(bitCount: number): number {
		let result: number = 0;
		while (bitCount > 0) {
			result = result << 1;
			result += this._bits[this._readIndex++];
			bitCount--;
		}
		return result;
	}
	
	public readLongTail(minValue: number, minBits: number): number {
		let result: number = minValue;
		let numBits: number = minBits;
		while (this._bits[this._readIndex++]) {
			result += 1 << numBits;
			numBits++;
		}
		while (numBits > 0) {
			numBits--;
			if (this._bits[this._readIndex++]) {
				result += 1 << numBits;
			}
		}
		return result;
	}
	
	public readPartDuration(): number {
		return this.readLongTail(1, 2);
	}
	
	public readPinCount(): number {
		return this.readLongTail(1, 0);
	}
	
	public readPitchInterval(): number {
		if (this.read(1)) {
			return -this.readLongTail(1, 3);
		} else {
			return this.readLongTail(1, 3);
		}
	}
}

class BitFieldWriter {
	private _bits: number[] = [];
	
	public write(bitCount: number, value: number): void {
		bitCount--;
		while (bitCount >= 0) {
			this._bits.push((value >>> bitCount) & 1);
			bitCount--;
		}
	}
	
	public writeLongTail(minValue: number, minBits: number, value: number): void {
		if (value < minValue) throw new Error("value out of bounds");
		value -= minValue;
		let numBits: number = minBits;
		while (value >= (1 << numBits)) {
			this._bits.push(1);
			value -= 1 << numBits;
			numBits++;
		}
		this._bits.push(0);
		while (numBits > 0) {
			numBits--;
			this._bits.push((value >>> numBits) & 1);
		}
	}
	
	public writePartDuration(value: number): void {
		this.writeLongTail(1, 2, value);
	}
	
	public writePinCount(value: number): void {
		this.writeLongTail(1, 0, value);
	}
	
	public writePitchInterval(value: number): void {
		if (value < 0) {
			this.write(1, 1); // sign
			this.writeLongTail(1, 3, -value);
		} else {
			this.write(1, 0); // sign
			this.writeLongTail(1, 3, value);
		}
	}
	
	public concat(other: BitFieldWriter): void {
		this._bits = this._bits.concat(other._bits);
	}
	
	public encodeBase64(base64IntToCharCode: ReadonlyArray<number>, buffer: number[]): number[] {
		for (let i: number = 0; i < this._bits.length; i += 6) {
			const value: number = (this._bits[i] << 5) | (this._bits[i+1] << 4) | (this._bits[i+2] << 3) | (this._bits[i+3] << 2) | (this._bits[i+4] << 1) | this._bits[i+5];
			buffer.push(base64IntToCharCode[value]);
		}
		return buffer;
	}
	
	public lengthBase64(): number {
		return Math.ceil(this._bits.length / 6);
	}
}

