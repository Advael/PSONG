/*Fourier transform biz. lol at some of these helper functions*/

// A basic FFT operation scales the overall magnitude of elements by the
// square root of the length of the array, √N. Performing a forward FFT and
// then an inverse FFT results in the original array, but multiplied by N.
// This helper  can be used to compensate for that. 
#include <math.h>
#include <vector>
using std::vector;
void scaleElementsByFactor(vector<double>& array, double factor) {
	for(auto &i:array) i *= factor;
}

bool isPowerOf2(int n) {
	return n != 0 && !(n & (n - 1));
}

int countBits(int n) {
//	if (!isPowerOf2(n)) throw new Error("FFT array length must be a power of 2.");
	return round(log(n) / log(2));
}

// Rearranges the elements of the array, swapping the element at an index
// with an element at an index that is the bitwise reverse of the first
// index in base 2. Useful for computing the FFT.
void reverseIndexBits(vector <double>& array) {
	const unsigned int fullArrayLength = array.size();
	const unsigned int bitCount = countBits(fullArrayLength);
//	if (bitCount > 16) throw new Error("FFT array length must not be greater than 2^16.");
	const unsigned int finalShift = 16 - bitCount;
	for (unsigned int i = 0; i < fullArrayLength; i++) {
		// Dear Javascript: Please support bit order reversal intrinsics. Thanks! :D
		unsigned int j = 0;
		j = ((i & 0xaaaa) >> 1) | ((i & 0x5555) << 1);
		j = ((j & 0xcccc) >> 2) | ((j & 0x3333) << 2);
		j = ((j & 0xf0f0) >> 4) | ((j & 0x0f0f) << 4);
		j = ((j           >> 8) | ((j &   0xff) << 8)) >> finalShift;
		if (j > i) {
			double temp = array[i];
			array[i] = array[j];
			array[j] = temp;
		}
	}
}

// Computes the inverse Fourier transform from a specially formatted array of
// scalar values. Elements 0 through N/2 are expected to be the real values of
// the corresponding complex elements, representing cosine wave amplitudes in
// ascending frequency, and elements N/2+1 through N-1 correspond to the
// imaginary values, representing sine wave amplitudes in descending frequency.
// Generates real-valued time-domain samples. Overwrites the input array.
void inverseRealFourierTransform(vector<double> & array) {
	const unsigned int fullArrayLength = array.size();
	const unsigned int totalPasses = countBits(fullArrayLength);
	if (fullArrayLength < 4){ 
		fprintf(stderr, "FFT array length must be at least 4.");
		return;
	}

	// Perform all but the last few passes in reverse.
	for (unsigned int pass = totalPasses - 1; pass >= 2; pass--) {
		const unsigned int subStride = 1 << pass;
		const unsigned int midSubStride = subStride >> 1;
		const unsigned int stride = subStride << 1;
		const double radiansIncrement = M_PI * 2.0 / double(stride);
		const double cosIncrement = cos(radiansIncrement);
		const double sinIncrement = sin(radiansIncrement);
		const double oscillatorMultiplier = 2.0 * cosIncrement;
		
		for (unsigned int startIndex = 0; startIndex < fullArrayLength; startIndex += stride) {
			const unsigned int startIndexA = startIndex;
			const unsigned int midIndexA = startIndexA + midSubStride;
			const unsigned int startIndexB = startIndexA + subStride;
			const unsigned int midIndexB = startIndexB + midSubStride;
			const unsigned int stopIndex = startIndexB + subStride;
			const double realStartA = array[startIndexA];
			const double imagStartB = array[startIndexB];
			array[startIndexA] = realStartA + imagStartB;
			array[midIndexA] *= 2;
			array[startIndexB] = realStartA - imagStartB;
			array[midIndexB] *= 2;
			double c = cosIncrement;
			double s = -sinIncrement;
			double cPrev = 1.0;
			double sPrev = 0.0;
			for (unsigned int index = 1; index < midSubStride; index++) {
				const unsigned int indexA0 = startIndexA + index;
				const unsigned int indexA1 = startIndexB - index;
				const unsigned int indexB0 = startIndexB + index;
				const unsigned int indexB1 = stopIndex   - index;
				const double real0 = array[indexA0];
				const double real1 = array[indexA1];
				const double imag0 = array[indexB0];
				const double imag1 = array[indexB1];
				const double tempA = real0 - real1;
				const double tempB = imag0 + imag1;
				array[indexA0] = real0 + real1;
				array[indexA1] = imag1 - imag0;
				array[indexB0] = tempA * c - tempB * s;
				array[indexB1] = tempB * c + tempA * s;
				const double cTemp = oscillatorMultiplier * c - cPrev;
				const double sTemp = oscillatorMultiplier * s - sPrev;
				cPrev = c;
				sPrev = s;
				c = cTemp;
				s = sTemp;
			}
		}
	}
	for (unsigned int index = 0; index < fullArrayLength; index += 4) {
		const unsigned int index1 = index + 1;
		const unsigned int index2 = index + 2;
		const unsigned int index3 = index + 3;
		const double real0 = array[index ];
		const double real1 = array[index1] * 2;
		const double imag2 = array[index2];
		const double imag3 = array[index3] * 2;
		const double tempA = real0 + imag2;
		const double tempB = real0 - imag2;
		array[index ] = tempA + real1;
		array[index1] = tempA - real1;
		array[index2] = tempB + imag3;
		array[index3] = tempB - imag3;
	}
	
	reverseIndexBits(array);
}

