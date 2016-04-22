#pragma once

#include "Prerequisites.h"

#include "Vector2.h"
#include "Vector3.h"
#include "Complex.h"
#include "FFT.h"

struct WaterVertex 
{
	Vector3	Position;
	Vector3 Normal;
	Vector2 Tex;
};

class FFTWater
{
public:
	FFTWater(RenderSystem* ptr, const int N, const float A, const Vector2 w, const float length, bool geometry, const bool infinite = false);
	~FFTWater();
	void Release();

	void CreateSpectrum( const float A, const Vector2 wind );
	// Deep water dispersion relation, Omega(n,m)
	float Dispersion(int n_prime, int m_prime);
	// Phillips wave spectrum, P(n,m)
	float Phillips(int n_prime, int m_prime);
	// Perform FFT and generate the wave displacement
	void EvaluateWavesFFT(float t, bool parallel = true);
	// Generate and update the water mesh
	void GenerateWaterSurface();
	void GenerateWaterSurfaceLod( int lod );
	// Interpolate a sample between indices
	Complex SampleBilinear( Complex* buffer, const Vector2 &ij );
	
	void CreateGrid( int Xsegments, int Ysegments );
	void CreateRadialGrid( int Xsegments, int Ysegments );

	void SetGridProjectorCamera( Camera3D* ptr )	{ m_pProjectorCamera = ptr; };
	Camera3D* GetGridProjectorCamera()				{ return m_pProjectorCamera; };
	Mesh* GetMesh()									{ return m_pMesh; };

private:
	Complex hTilde_0(int n_prime, int m_prime);
	Complex hTilde(float t, int n_prime, int m_prime);

	bool	infinite;			// Renders the ocean with a (practically) infinitie radial grid instead of a patch of it
	bool	geometry;			// Flag to render wireframe or surface
	float	g;					// Gravity constant
	int		N;					// Dimension -- N should be a power of 2
	int		Ndisplay;
	float	A;					// Phillips spectrum parameter -- affects heights of waves
	Vector2	w;					// Wind parameter
	float	length;				// Length parameter

	// Coefficients for fast fourier transform
	Complex *h_tilde, *h_tilde0, *h_tilde0mk_conj,
			*h_tilde_slopex, *h_tilde_slopez,
			*h_tilde_dx, *h_tilde_dy;

	FFT *fft;					// Fast fourier transform

	Vector2 *positionsBase;			// Precompute and store radial grid vertices
	WaterVertex *vertices;			// Vertices for vertex buffer
	UINT *indices;					// Indices for index buffer
	UINT indices_count;				// Number of indices
	UINT lodLevels;					// Number of LOD levels
	Vector3 **displacement, **normals;

	RenderSystem*	m_pRenderSystem;
	Mesh*	m_pMesh;
	Camera3D* m_pProjectorCamera;
};

// Uniform random variable [0...1]
float UniformRandomVariable();
// Generates a complex number where the real and imaginary parts are
// independet random draws
Complex GaussianRandomVariable();