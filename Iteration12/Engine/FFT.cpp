#pragma once
#pragma warning( disable : 4018 ) // "'<' : signed/unsigned mismatch"

#include "fft.h"
#include <stdint.h>

FFT::FFT(unsigned int N) : N(N), reversed(0), W(0), pi2(2 * PI) 
{
	log_2_N = logf(N)/logf(2);
	parity = 0;

	// Prepare bit-reserved indices
	reversed = new unsigned int[N];
	for (int i = 0; i < N; i++) reversed[i] = reverse(i);

	// Prepare danielson-lanczos lemma twiddle factor
	int pow2 = 1;
	W = new Complex*[log_2_N];
	for (int i = 0; i < log_2_N; i++) 
	{
		W[i] = new Complex[pow2];
		for (int j = 0; j < pow2; j++) 
			W[i][j] = w(j, pow2 * 2);
		pow2 *= 2;
	}

	// Temporary buffer for FFT coefficients
	c = new Complex*[N];
	for(int i=0; i<N; i++)
		c[i] = new Complex[N];
}

FFT::~FFT() 
{
	if (c[0]) delete [] c[0];
	if (c[1]) delete [] c[1];
	if (W) 
	{
		for (int i = 0; i < log_2_N; i++) 
			if (W[i]) delete [] W[i];
		delete [] W;
	}
	if (reversed) delete [] reversed;
}

unsigned int FFT::reverse(unsigned int i) 
{
	unsigned int res = 0;
	for (int j = 0; j < log_2_N; j++)
	{
		res = (res << 1) + (i & 1);
		i >>= 1;
	}
	return res;
}

Complex FFT::w(unsigned int x, unsigned int N) 
{
	return Complex(cos(pi2 * x / N), sin(pi2 * x / N));
}

void FFT::fft(Complex* input, Complex* output, int stride, int offset) 
{
	for (int i = 0; i < N; i++) 
		c[parity][i] = input[reversed[i] * stride + offset];

	int loops       = N>>1;
	int size        = 1<<1;
	int size_over_2 = 1;
	int w_          = 0;
	for (int i = 1; i <= log_2_N; i++) 
	{
		parity ^= 1;
		for (int j = 0; j < loops; j++) 
		{
			for (int k = 0; k < size_over_2; k++) 
			{
				c[parity][size * j + k] =  c[parity^1][size * j + k] +
							  c[parity^1][size * j + size_over_2 + k] * W[w_][k];
			}

			for (int k = size_over_2; k < size; k++) 
			{
				c[parity][size * j + k] =  c[parity^1][size * j - size_over_2 + k] -
							  c[parity^1][size * j + k] * W[w_][k - size_over_2];
			}
		}
		loops       >>= 1;
		size        <<= 1;
		size_over_2 <<= 1;
		w_++;
	}

	for (int i = 0; i < N; i++) 
		output[i * stride + offset] = c[parity][i];
}

uint32_t reverseBits(uint32_t i) 
{
  register uint32_t mask = 0x55555555; // 0101...
  i = ((i & mask) << 1) | ((i >> 1) & mask);
  mask = 0x33333333; // 0011...
  i = ((i & mask) << 2) | ((i >> 2) & mask);
  mask = 0x0f0f0f0f; // 00001111...
  i = ((i & mask) << 4) | ((i >> 4) & mask);
  mask = 0x00ff00ff; // 0000000011111111...
  i = ((i & mask) << 8) | ((i >> 8) & mask);
  // 00000000000000001111111111111111 no need for mask
  i = (i << 16) | (i >> 16);
  return i;
}

int lg(uint32_t i) 
{
  int count = -1;
  while (i) {
    i = i >> 1;
    count++;
  }
  return count;
}

// Russian peasant algorithm
int pown(const int p) 
{
  uint32_t w = p;
  w |= w >> 1;
  w |= w >> 2;
  w |= w >> 4;
  w |= w >> 8;
  w |= w >> 16;
  uint32_t mask = w & ~(w >> 1);

  int a = 1;
  while (mask) {
    a = a * a;
    if (mask & p)
      a *= 2;
    mask >>= 1;
  }

  return a;
}

Complex FFT::twiddleFactor(UINT x, UINT N) 
{
    return Complex(cos(2.0f * PI * x / N), sin(2.0f * PI * x / N));
}

void FFT::iterativeIFFT( Complex* primal, Complex* dual, UINT stride, UINT zeroOffset )
{
	const int absP = logf(N)/logf(2.0f);

	// Bottom level of iteration tree
	for (int i = 0; i < N; i++)
		c[0][i] = primal[(reverseBits(i) >> (32 - absP))*stride + zeroOffset];

	// There are absP levels above the bottom
	for (int p = 1; p <= absP; p++) 
	{
		// Complex root of unity
		const int unityStep = 0x1 << p;
		const double theta = 2.0f * PI / unityStep; // INVERSE
		const Complex unityRoot(cos(theta), sin(theta));

		// Each higher level doubles the step size
		for (int offset = 0; offset < N; offset += unityStep) 
		{
			Complex omega = Complex(1.0f,0.0f);

			// Combine within a step segment (note only iterate over half step)
			for (int k = 0; k < unityStep/2; k++) 
			{
				const Complex u = c[0][offset + k];

				const Complex t = omega * c[0][offset + k + unityStep/2];
				omega *= unityRoot;

				c[0][offset + k] = u + t;
				c[0][offset + k + unityStep/2] = u - t;
			}
		}
	}

	for (int j = 0; j < N; j++)
		dual[j*stride + zeroOffset] = c[0][j];
	/*
	// INVERSE
	for (int j = 0; j < N; j++)
		dual[j*stride + zeroOffset] /= N;*/
}

void FFT::parallelfft( UINT index, Complex* primal, Complex* dual, UINT stride, UINT zeroOffset )
{
	const int absP = logf(N)/logf(2.0f);

	// Bottom level of iteration tree
	for (int i = 0; i < N; i++)
		c[index][i] = primal[(reverseBits(i) >> (32 - absP))*stride + zeroOffset];

	// There are absP levels above the bottom
	for (int p = 1; p <= absP; p++) 
	{
		// Complex root of unity
		const int unityStep = 0x1 << p;
		const double theta = 2.0f * PI / unityStep; // INVERSE
		const Complex unityRoot(cos(theta), sin(theta));

		// Each higher level doubles the step size
		for (int offset = 0; offset < N; offset += unityStep) 
		{
			Complex omega = Complex(1.0f,0.0f);

			// Combine within a step segment (note only iterate over half step)
			for (int k = 0; k < unityStep/2; k++) 
			{
				const Complex u = c[index][offset + k];

				const Complex t = omega * c[index][offset + k + unityStep/2];
				omega *= unityRoot;

				c[index][offset + k] = u + t;
				c[index][offset + k + unityStep/2] = u - t;
			}
		}
	}

	for (int j = 0; j < N; j++)
		dual[j*stride + zeroOffset] = c[index][j];
}