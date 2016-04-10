#pragma once

#include "Math.h"
#include "Complex.h"

class FFT 
{
public:
	FFT(unsigned int N);
	~FFT();
	unsigned int reverse(unsigned int i);
	Complex w(unsigned int x, unsigned int N);
	Complex twiddleFactor(UINT x, UINT N) ;
	void fft(Complex* input, Complex* output, int stride, int offset);
	void iterativeIFFT( Complex* primal, Complex* dual, UINT stride, UINT zeroOffset );
	void parallelfft( UINT index, Complex* primal, Complex* dual, UINT stride, UINT zeroOffset );

 private:
	unsigned int N, parity;
	unsigned int log_2_N;
	float pi2;
	unsigned int *reversed;
	Complex **W;
	Complex **c;
};
