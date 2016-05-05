#pragma once

#include "FFTWater.h"
#include "RenderSystem.h"
#include "Material.h"
#include "GeometryChunk.h"
#include <tbb\parallel_for.h>
#include "Plane.h"
#include "Camera3D.h"
#include "Buffer.h"

FFTWater::FFTWater(RenderSystem* ptr, const int N, const float A, const Vector2 w, const float length, const bool geometry, const bool infinite) :
		m_pRenderSystem(ptr), 
		m_pMesh(NULL), 
		g(9.81f), 
		geometry(geometry), 
		N(N),  
		A(A), 
		w(w), 
		length(length),
		vertices(0), 
		indices(0), 
		h_tilde0(0), 
		h_tilde0mk_conj(0), 
		h_tilde(0), 
		h_tilde_slopex(0), 
		h_tilde_slopez(0), 
		h_tilde_dx(0), 
		h_tilde_dy(0), 
		fft(0),
		infinite(infinite),
		positionsBase(NULL)
{
	h_tilde0       = new Complex[N*N];
	h_tilde0mk_conj= new Complex[N*N];
	h_tilde        = new Complex[N*N];
	h_tilde_slopex = new Complex[N*N];
	h_tilde_slopez = new Complex[N*N];
	h_tilde_dx     = new Complex[N*N];
	h_tilde_dy     = new Complex[N*N];
	fft            = new FFT(N);

	lodLevels = 10;

	displacement	= new Vector3*[lodLevels];
	for (int i=0; i<lodLevels; i++)
	{
		int lodN = N>>i;
		displacement[i] = new Vector3[lodN*lodN];
	}

	normals			= new Vector3*[lodLevels];
	for (int i=0; i<lodLevels; i++)
	{
		int lodN = N>>i;
		normals[i] = new Vector3[lodN*lodN];
	}

	CreateSpectrum( A, w );

	if (!infinite)
	{
		Ndisplay		= N+1;
		vertices		= new WaterVertex[Ndisplay*Ndisplay];
		indices			= new UINT[Ndisplay*Ndisplay*6];
		CreateGrid(N, N);
	}
	else
	{
		Ndisplay		= N;
		vertices		= new WaterVertex[Ndisplay*Ndisplay];
		indices			= new UINT[Ndisplay*Ndisplay*6];
		positionsBase	= new Vector2[Ndisplay*Ndisplay];
		CreateRadialGrid(Ndisplay, Ndisplay);
	}

	// Create the mesh, set to dynamic so we can update data
	BufferLayout vertexLayout;
	vertexLayout.AddElement( "POSITION", 0, R32G32B32_FLOAT );
	vertexLayout.AddElement( "NORMAL", 0, R32G32B32_FLOAT );
	vertexLayout.AddElement( "TEXCOORD", 0, R32G32_FLOAT );

	m_pMesh = m_pRenderSystem->CreateMesh( (float*)&vertices[0], Ndisplay*Ndisplay, &indices[0], indices_count, vertexLayout, true );
	Material_Water* mat = Material_Water::Create( m_pRenderSystem );
	mat->SetDiffusemap( m_pRenderSystem->LoadTexture2D(L"Media/perlin.bmp") );
	mat->SetNormalmap( m_pRenderSystem->LoadTexture2D(L"Media/water_normal1.png") );
	mat->SetFoamTexture( m_pRenderSystem->LoadTexture2D(L"Media/SimpleFoam.png") );
	mat->SetIBL( ptr->LoadTexture2D( L"Media/cubemap4.dds" ) );
	m_pMesh->SetMaterial( mat );
}

FFTWater::~FFTWater() 
{
	if (h_tilde0)		delete [] h_tilde0;
	if (h_tilde0mk_conj)delete [] h_tilde0mk_conj;
	if (h_tilde)		delete [] h_tilde;
	if (h_tilde_slopex)	delete [] h_tilde_slopex;
	if (h_tilde_slopez)	delete [] h_tilde_slopez;
	if (h_tilde_dx)		delete [] h_tilde_dx;
	if (h_tilde_dy)		delete [] h_tilde_dy;
	if (fft)			delete fft;
	if (vertices)		delete [] vertices;
	if (indices)		delete [] indices;

	Release();
}

void FFTWater::Release() 
{
	//m_pMesh->release();
}

float FFTWater::Dispersion(int n_prime, int m_prime) 
{
	float w_0 = 2.0f * PI / 200.0f;
	float kx = PI * (2 * n_prime - N) / length;
	float kz = PI * (2 * m_prime - N) / length;
	return floor(sqrt(g * sqrt(kx * kx + kz * kz)) / w_0) * w_0;
}

float FFTWater::Phillips(int n_prime, int m_prime) 
{
	Vector2 k(PI * (2 * n_prime - N) / length,
				PI * (2 * m_prime - N) / length);
	float k_length  = k.length();
	if (k_length < 0.000001) return 0.0;

	float k_length2 = k_length  * k_length;
	float k_length4 = k_length2 * k_length2;

	float k_dot_w   = k.unit().dotProduct( w.unit() );
	float k_dot_w2  = k_dot_w * k_dot_w * k_dot_w * k_dot_w;

	float w_length  = w.length();
	float L         = w_length * w_length / g;
	float L2        = L * L;
	
	float damping   = 0.001f;
	float l2        = L2 * damping * damping;

	if (k_dot_w<0.0f)
		return A * exp(-1.0f / (k_length2 * L2)) / k_length4 * k_dot_w2 * exp(-k_length2 * l2);
	else
		return 0.5f * A * exp(-1.0f / (k_length2 * L2)) / k_length4 * k_dot_w2 * exp(-k_length2 * l2);
}

Complex FFTWater::hTilde_0(int n_prime, int m_prime) 
{
	Complex r = GaussianRandomVariable();
	return r * sqrt(Phillips(n_prime, m_prime) / 2.0f);
}

Complex FFTWater::hTilde(float t, int n_prime, int m_prime) 
{
	int index = m_prime * N + n_prime;

	float omegat = Dispersion(n_prime, m_prime) * t;

	float cos_ = cos(omegat);
	float sin_ = sin(omegat);

	Complex c0(cos_,  sin_);
	Complex c1(cos_, -sin_);

	Complex res = h_tilde0[index] * c0 + h_tilde0mk_conj[index] * c1;

	return res;
}

void FFTWater::EvaluateWavesFFT(float t, bool parallel) 
{
	// Generate H_tilde(K,t) at time t
	tbb::parallel_for(0, N, 1, [&] (int m_prime)
	{
		float kx, ky, len;
		int index;
		ky = PI * (2.0f * m_prime - N) / length;
		for (int n_prime = 0; n_prime < N; n_prime++) 
		{
			kx = PI*(2 * n_prime - N) / length;
			len = sqrt(kx * kx + ky * ky);
			index = m_prime * N + n_prime;

			h_tilde[index] = hTilde(t, n_prime, m_prime);
			h_tilde_slopex[index] = h_tilde[index] * Complex(0, kx);
			h_tilde_slopez[index] = h_tilde[index] * Complex(0, ky);
			if (len < 0.000001f) 
			{
				h_tilde_dx[index]     = Complex(0.0f, 0.0f);
				h_tilde_dy[index]     = Complex(0.0f, 0.0f);
			} 
			else 
			{
				h_tilde_dx[index]     = h_tilde[index] * Complex(0, -kx/len);
				h_tilde_dy[index]     = h_tilde[index] * Complex(0, -ky/len);
			}
		}
	});
	
	if (!parallel)
	{
		// Perform single-threaded fast fourier transform
		for (int m_prime = 0; m_prime < N; m_prime++) 
		{
			fft->IterativeFFT(h_tilde, h_tilde, 1, m_prime * N);
			fft->IterativeFFT(h_tilde_slopex, h_tilde_slopex, 1, m_prime * N);
			fft->IterativeFFT(h_tilde_slopez, h_tilde_slopez, 1, m_prime * N);
			fft->IterativeFFT(h_tilde_dx, h_tilde_dx, 1, m_prime * N);
			fft->IterativeFFT(h_tilde_dy, h_tilde_dy, 1, m_prime * N);
		}
		for (int n_prime = 0; n_prime < N; n_prime++) 
		{
			fft->IterativeFFT(h_tilde, h_tilde, N, n_prime);
			fft->IterativeFFT(h_tilde_slopex, h_tilde_slopex, N, n_prime);
			fft->IterativeFFT(h_tilde_slopez, h_tilde_slopez, N, n_prime);
			fft->IterativeFFT(h_tilde_dx, h_tilde_dx, N, n_prime);
			fft->IterativeFFT(h_tilde_dy, h_tilde_dy, N, n_prime);
		}
	}
	else
	{
		// Perform multithreaded fast fourier transform
		tbb::parallel_for( 0, N, 1, [&](int m_prime)
		{
			fft->ParallelFFT(m_prime, h_tilde, h_tilde, 1, m_prime * N);
			fft->ParallelFFT(m_prime, h_tilde_slopex, h_tilde_slopex, 1, m_prime * N);
			fft->ParallelFFT(m_prime, h_tilde_slopez, h_tilde_slopez, 1, m_prime * N);
			fft->ParallelFFT(m_prime, h_tilde_dx, h_tilde_dx, 1, m_prime * N);
			fft->ParallelFFT(m_prime, h_tilde_dy, h_tilde_dy, 1, m_prime * N);
		});
		tbb::parallel_for( 0, N, 1, [&](int n_prime)
		{
			fft->ParallelFFT(n_prime, h_tilde, h_tilde, N, n_prime);
			fft->ParallelFFT(n_prime, h_tilde_slopex, h_tilde_slopex, N, n_prime);
			fft->ParallelFFT(n_prime, h_tilde_slopez, h_tilde_slopez, N, n_prime);
			fft->ParallelFFT(n_prime, h_tilde_dx, h_tilde_dx, N, n_prime);
			fft->ParallelFFT(n_prime, h_tilde_dy, h_tilde_dy, N, n_prime);
		});
	}

	float signs[] = { 1.0f, -1.0f };	
	for (int m_prime=0; m_prime<N; m_prime++)
	{
		for (int n_prime=0; n_prime<N; n_prime++)
		{
			Vector3 n;
			UINT index  = m_prime * N + n_prime;		// index into h_tilde..

			float sign = signs[(n_prime + m_prime) & 1];

			// Height
			h_tilde[index]     = h_tilde[index] * sign;

			// Horizontal displacement
			h_tilde_dx[index] = h_tilde_dx[index] * sign;
			h_tilde_dy[index] = h_tilde_dy[index] * sign;

			// Normal
			h_tilde_slopex[index] = h_tilde_slopex[index] * sign;
			h_tilde_slopez[index] = h_tilde_slopez[index] * sign;
		}
	}

		// Lod 0
	tbb::parallel_for( 0, N, 1, [&](int m_prime)
	{
		for(int n_prime=0; n_prime<N; n_prime++)
		{
			UINT index  = m_prime * N + n_prime;		// index into h_tilde..

			// Height
			displacement[0][index].z = h_tilde[index].re;

			// Horizontal displacement
			displacement[0][index].y = h_tilde_dx[index].re;
			displacement[0][index].x = h_tilde_dy[index].re;
			
			// Normal
			Vector3 normal = Vector3(0.0f - h_tilde_slopex[index].re, 0.0f - h_tilde_slopez[index].re, 0.75f).unit();
			normals[0][index].x =  normal.x;
			normals[0][index].y =  normal.y;
			normals[0][index].z =  normal.z;
		}
	});
	
	// Mipmap-style create subsequent lods
	for (int lod=1; lod<lodLevels; lod++)
	{
		int lodN = N>>lod;
		int lowerLodN = N>>(lod-1);
		tbb::parallel_for( 0, lodN, 1, [&](int m_prime)
		{
			for(int n_prime=0; n_prime<lodN; n_prime++)
			{
				// index into this lod
				UINT index  = m_prime * lodN + n_prime;

				// index into lower lod
				UINT index00  = (m_prime*2) * lowerLodN + (n_prime*2);
				UINT index01  = (m_prime*2) * lowerLodN + (n_prime*2+1);
				UINT index10  = (m_prime*2+1) * lowerLodN + (n_prime*2);
				UINT index11  = (m_prime*2+1) * lowerLodN + (n_prime*2+1);

				// Height & horizontal displacement
				displacement[lod][index] = (displacement[lod-1][index00]
											+displacement[lod-1][index01]
											+displacement[lod-1][index10]
											+displacement[lod-1][index11])/4.0f;
			
				// Normal
				normals[lod][index] = (normals[lod-1][index00]
										+normals[lod-1][index01]
										+normals[lod-1][index10]
										+normals[lod-1][index11])/4.0f;
				normals[lod][index].normalise();
			}
		});
	}
}

void FFTWater::GenerateWaterSurface()
{
	float lambda = -1.5f;

	if (!infinite)
	{
		// Update vertices
		//float signs[] = { 1.0f, -1.0f };	
		tbb::parallel_for( 0, N, 1, [&](int m_prime)
		{
			tbb::parallel_for( 0, N, 1, [&](int n_prime)
			{
				Vector3 n;
				UINT index  = m_prime * N + n_prime;		// index into h_tilde..
				UINT index1 = m_prime * Ndisplay + n_prime;	// index into vertices

				//float sign = signs[(n_prime + m_prime) & 1];

				//h_tilde[index]     = h_tilde[index] * sign;

				// Height
				vertices[index1].Position.z = h_tilde[index].re;

				// Horizontal displacement
				vertices[index1].Position.x = (n_prime - N / 2.0f) * length / N + h_tilde_dx[index].re * lambda;
				vertices[index1].Position.y = (m_prime - N / 2.0f) * length / N + h_tilde_dy[index].re * lambda;
			
				// Normal
				n = Vector3(0.0f - h_tilde_slopex[index].re, 0.0f - h_tilde_slopez[index].re, 0.75f).unit();
				vertices[index1].Normal.x =  n.x;
				vertices[index1].Normal.y =  n.y;
				vertices[index1].Normal.z =  n.z;
			
				// For tiling
				if (n_prime == 0 && m_prime == 0) 
				{
					vertices[index1 + N + Ndisplay * N].Position.x = length/2.0f + h_tilde_dx[index].re * lambda;
					vertices[index1 + N + Ndisplay * N].Position.y = length/2.0f + h_tilde_dy[index].re * lambda;
					vertices[index1 + N + Ndisplay * N].Position.z = h_tilde[index].re;
			
					vertices[index1 + N + Ndisplay * N].Normal.x =  n.x;
					vertices[index1 + N + Ndisplay * N].Normal.y =  n.y;
					vertices[index1 + N + Ndisplay * N].Normal.z =  n.z;
				}
				if (n_prime == 0) 
				{
					vertices[index1 + N].Position.x = length/2.0f						+ h_tilde_dx[index].re * lambda;
					vertices[index1 + N].Position.y = (m_prime - N / 2.0f) * length / N	+ h_tilde_dy[index].re * lambda;
					vertices[index1 + N].Position.z = h_tilde[index].re;
			
					vertices[index1 + N].Normal.x =  n.x;
					vertices[index1 + N].Normal.y =  n.y;
					vertices[index1 + N].Normal.z =  n.z;
				}
				if (m_prime == 0) 
				{
					vertices[index1 + Ndisplay * N].Position.x = (n_prime - N / 2.0f) * length / N	+ h_tilde_dx[index].re * lambda;
					vertices[index1 + Ndisplay * N].Position.y = length/2.0f							+ h_tilde_dy[index].re * lambda;
					vertices[index1 + Ndisplay * N].Position.z = h_tilde[index].re;
			
					vertices[index1 + Ndisplay * N].Normal.x =  n.x;
					vertices[index1 + Ndisplay * N].Normal.y =  n.y;
					vertices[index1 + Ndisplay * N].Normal.z =  n.z;
				}
			});
		});

		// Send this vertex data to the GPU
		// Index buffer remains the same
		BufferLayout layout;
		layout.AddElement( "POSITION", 0, R32G32B32_FLOAT );
		layout.AddElement( "NORMAL", 0, R32G32B32_FLOAT );
		layout.AddElement( "TEXCOORD", 0, R32G32_FLOAT );
		m_pMesh->GetSubmesh(0)->GetGeometryChunk()->UpdateVertexBuffer( (float*)&vertices[0], Ndisplay*Ndisplay, layout.GetByteSize()*Ndisplay*Ndisplay, layout.GetByteSize() ); 
	}
	else
	{
		// Fuck projected grid it's driving me insane, just use a radial grid
		tbb::parallel_for( 0, Ndisplay, 1, [&](int x)
		//for (int x = 0; x < Ndisplay; x++)
		{
			for (int y = 0; y < Ndisplay; y++)
			{
				float scale = N/length/2.0f;
				UINT index = x + y * Ndisplay;
				
				// Base position
				//vertices[index].Position.x = r * cos(TAU * (float)y / (float)(Ndisplay - 1));
				//vertices[index].Position.y = r * sin(TAU * (float)y / (float)(Ndisplay - 1));
				//vertices[index].Position.z = 0.0f;

				// Position
				Vector3 cameraPosition = m_pProjectorCamera->GetPosition();
				vertices[index].Position.x = positionsBase[index].x + cameraPosition.x;
				vertices[index].Position.y = positionsBase[index].y + cameraPosition.y;
				vertices[index].Position.z = SampleBilinear( h_tilde, Vector2(vertices[index].Position.x,vertices[index].Position.y)*scale ).re * scale;
				
				// Texture coordinates
				vertices[index].Tex.x = vertices[index].Position.x;
				vertices[index].Tex.y = vertices[index].Position.y;
				
				// Normal
				float slopeX = SampleBilinear( h_tilde_slopez, Vector2(vertices[index].Position.x,vertices[index].Position.y)*scale ).re;
				float slopeY = SampleBilinear( h_tilde_slopex, Vector2(vertices[index].Position.x,vertices[index].Position.y)*scale ).re;
				Vector3 normal = Vector3(0.0f - slopeX, 0.0f - slopeY, 0.75f).unit();

				vertices[index].Normal.x =  normal.x;
				vertices[index].Normal.y =  normal.y;
				vertices[index].Normal.z =  normal.z;
				
				// Horizontal displacement
				vertices[index].Position.y += SampleBilinear( h_tilde_dx, Vector2(vertices[index].Position.x,vertices[index].Position.y)*scale ).re * lambda * scale;
				vertices[index].Position.x += SampleBilinear( h_tilde_dy, Vector2(vertices[index].Position.x,vertices[index].Position.y)*scale ).re * lambda * scale;
				
			}
		});

		// Send this vertex data to the GPU
		// Index buffer remains the same
		BufferLayout layout;
		layout.AddElement( "POSITION", 0, R32G32B32_FLOAT );
		layout.AddElement( "NORMAL", 0, R32G32B32_FLOAT );
		layout.AddElement( "TEXCOORD", 0, R32G32_FLOAT );
		m_pMesh->GetSubmesh(0)->GetGeometryChunk()->UpdateVertexBuffer( (float*)&vertices[0], Ndisplay*Ndisplay, layout.GetByteSize()*Ndisplay*Ndisplay, layout.GetByteSize() ); 

	}
	/*
	{
		// Intersect 4 corners of the frustum against the water plane
		Plane waterPlane( Vector3(0.0f, 0.0f, 1.0f), 0.0f );
		
		Ray upperLeftRay = m_pProjectorCamera->getRayFromScreenPosition( Vector2(0.0f, 0.0f) );
		Ray lowerLeftRay = m_pProjectorCamera->getRayFromScreenPosition( Vector2(0.0f, 1.0f) );
		Ray upperRightRay = m_pProjectorCamera->getRayFromScreenPosition( Vector2(1.0f, 0.0f) );
		Ray lowerRightRay = m_pProjectorCamera->getRayFromScreenPosition( Vector2(1.0f, 1.0f) );

		float ZNear = m_pProjectorCamera->getNearPlane();
		float upperLeftIntersectDistance = waterPlane.rayIntersects(upperLeftRay).second-ZNear;
		float lowerLeftIntersectDistance = waterPlane.rayIntersects(lowerLeftRay).second-ZNear; 
		float upperRightIntersectDistance = waterPlane.rayIntersects(upperRightRay).second-ZNear;
		float lowerRightIntersectDistance = waterPlane.rayIntersects(lowerRightRay).second-ZNear; 

		Vector3 upperLeftIntersect = upperLeftRay.getPoint( upperLeftIntersectDistance );
		Vector3 lowerLeftIntersect = lowerLeftRay.getPoint( lowerLeftIntersectDistance );
		Vector3 upperRightIntersect = upperRightRay.getPoint( upperRightIntersectDistance );
		Vector3 lowerRightIntersect = lowerRightRay.getPoint( lowerRightIntersectDistance );

		// Transform to projector space to get bounds
		Matrix4x4 viewProjection = m_pProjectorCamera->getViewProjectionMatrix();
		
		Vector4 upperLeftProjected = Vector4(upperLeftIntersect)*viewProjection;
		Vector4 lowerLeftProjected = Vector4(lowerLeftIntersect)*viewProjection;
		Vector4 upperRightProjected = Vector4(upperRightIntersect)*viewProjection;
		Vector4 lowerRightProjected = Vector4(lowerRightIntersect)*viewProjection;

		upperLeftProjected /= upperLeftProjected.w;
		lowerLeftProjected /= lowerLeftProjected.w;
		upperRightProjected /= upperRightProjected.w;
		lowerRightProjected /= lowerRightProjected.w;
		
		Matrix4x4 invViewProjection = viewProjection.inverse();
		Matrix4x4 invView = m_pProjectorCamera->getViewMatrix().inverse();
		
		float signs[] = { 1.0f, -1.0f };	

		for(int m_prime=0; m_prime<N; m_prime++)
		{
			for(int n_prime=0; n_prime<N; n_prime++)
			{
				Vector3 n;
				UINT index  = m_prime * N + n_prime;		// index into h_tilde..
				UINT index1 = m_prime * Nplus1 + n_prime;	// index into vertices

				float sign = signs[(n_prime + m_prime) & 1];

				// Height
				h_tilde[index] = h_tilde[index] * sign;

				// Horizontal displacement
				h_tilde_dx[index] = h_tilde_dx[index] * sign;
				h_tilde_dy[index] = h_tilde_dy[index] * sign;
			
				// Normal
				h_tilde_slopex[index] = h_tilde_slopex[index] * sign;
				h_tilde_slopez[index] = h_tilde_slopez[index] * sign;
				n = Vector3(0.0f - h_tilde_slopex[index].re, 0.0f - h_tilde_slopez[index].re, 0.75f).unit();
			}
		}

		//tbb::parallel_for( 0, N, 1, [&](int m_prime)
		for(int m_prime=0; m_prime<N; m_prime++)
		{
			//tbb::parallel_for( 0, N, 1, [&](int n_prime)
			for(int n_prime=0; n_prime<N; n_prime++)
			{
				Vector3 n;
				UINT index  = m_prime * N + n_prime;		// index into h_tilde..
				UINT index1 = m_prime * Nplus1 + n_prime;	// index into vertices

				float sign = signs[(n_prime + m_prime) & 1];

				h_tilde[index]     = h_tilde[index] * sign;

				// Interpolate z in projector space
				// Linear interpolation :
				// f(x,y) = f(0,0)(1-x)(1-y) + f(1,0)x(1-y) + f(0,1)(1-x)y + f(1,1)xy;
				float x = m_prime/(float)N;
				float y = n_prime/(float)N;
				/*
				float z = upperLeftIntersectDistance*(1.0f-x)*(1.0f-y) + upperRightIntersectDistance*x*(1.0f-y) 
						+ lowerLeftIntersectDistance*(1.0f-x)*y + lowerRightIntersectDistance*x*y;
				
				float WIDTH_DIV_2 = 0.5f;
				float HEIGHT_DIV_2 = 0.5f;
				float dx=tanf(m_pProjectorCamera->getFov()*0.5f)*(x/WIDTH_DIV_2-1.0f)*m_pProjectorCamera->getAspect();
				float dy=tanf(m_pProjectorCamera->getFov()*0.5f)*(1.0f-y/HEIGHT_DIV_2);
				float m_ZNear = z;
				float m_ZFar = m_pProjectorCamera->getFarPlane();

				Vector4 start(dx*m_ZNear, dy*m_ZNear, -m_ZNear, 1.0f);
				Vector4 end(dx*m_ZFar, dy*m_ZFar, -m_ZFar, 1.0f);
				
				Ray ray = m_pProjectorCamera->getRayFromScreenPosition( Vector2(x,y) );
				std::pair<bool, float> hit = waterPlane.rayIntersects( ray );
				Vector3 positionWS(0.0f, 0.0f, -100.0f);

				if (hit.first)
					positionWS = ray.getPoint( hit.second );

				// Unproject to world space
				//Vector4 positionWS = start*invView;

				// 
				vertices[index1].Position.x = positionWS.x;
				vertices[index1].Position.y = positionWS.y;
				vertices[index1].Position.z = positionWS.z + sampleBilinear( h_tilde, Vector2(positionWS.x,positionWS.y)*8.0f ).re;

				float slopeX = sampleBilinear( h_tilde_slopex, Vector2(positionWS.x,positionWS.y)*8.0f ).re;
				float slopeY = sampleBilinear( h_tilde_slopez, Vector2(positionWS.x,positionWS.y)*8.0f ).re;
				Vector3 normal = Vector3(0.0f - slopeX, 0.0f - slopeY, 0.75f).unit();

				vertices[index1].Normal.x =  normal.x;
				vertices[index1].Normal.y =  normal.y;
				vertices[index1].Normal.z =  normal.z;
				/*
				// Height
				vertices[index1].Position.z = h_tilde[index].re;

				// Horizontal displacement
				h_tilde_dx[index] = h_tilde_dx[index] * sign;
				h_tilde_dy[index] = h_tilde_dy[index] * sign;
				vertices[index1].Position.x = (n_prime - N / 2.0f) * length / N + h_tilde_dx[index].re * lambda;
				vertices[index1].Position.y = (m_prime - N / 2.0f) * length / N + h_tilde_dy[index].re * lambda;
			
				// Normal
				h_tilde_slopex[index] = h_tilde_slopex[index] * sign;
				h_tilde_slopez[index] = h_tilde_slopez[index] * sign;
				n = Vector3(0.0f - h_tilde_slopex[index].re, 0.0f - h_tilde_slopez[index].re, 0.75f).unit();
				vertices[index1].Normal.x =  n.x;
				vertices[index1].Normal.y =  n.y;
				vertices[index1].Normal.z =  n.z;
				
				// For tiling
				if (n_prime == 0 && m_prime == 0) 
				{
					vertices[index1 + N + Nplus1 * N].Position.x = 0.0f;
					vertices[index1 + N + Nplus1 * N].Position.y = 0.0f;
					vertices[index1 + N + Nplus1 * N].Position.z = -100.0f;
			
					vertices[index1 + N + Nplus1 * N].Normal.x =  0.0f;
					vertices[index1 + N + Nplus1 * N].Normal.y =  0.0f;
					vertices[index1 + N + Nplus1 * N].Normal.z =  1.0f;
				}
				if (n_prime == 0) 
				{
					vertices[index1 + N].Position.x = 0.0f;
					vertices[index1 + N].Position.y = 0.0f;
					vertices[index1 + N].Position.z = -100.0f;
			
					vertices[index1 + N].Normal.x =  0.0f;
					vertices[index1 + N].Normal.y =  0.0f;
					vertices[index1 + N].Normal.z =  1.0f;
				}
				if (m_prime == 0) 
				{
					vertices[index1 + Nplus1 * N].Position.x = 0.0f;
					vertices[index1 + Nplus1 * N].Position.y = 0.0f;
					vertices[index1 + Nplus1 * N].Position.z = -100.0f;
			
					vertices[index1 + Nplus1 * N].Normal.x =  0.0f;
					vertices[index1 + Nplus1 * N].Normal.y =  0.0f;
					vertices[index1 + Nplus1 * N].Normal.z =  1.0f;
				}
			};
		};

		// Send this vertex data to the GPU
		// Index buffer remains the same
		BufferLayout layout;
		layout.addElement( "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT );
		layout.addElement( "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT );
		layout.addElement( "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT );
		m_pMesh->getSubmesh(0)->GetGeometryChunk()->updateVertexBuffer( (float*)&vertices[0], Nplus1*Nplus1, layout.getByteSize()*Nplus1*Nplus1, layout.getByteSize() ); 

	}*/
}

void FFTWater::GenerateWaterSurfaceLod( int lod )
{
	float lambda = -1.5f;

	if (!infinite)
	{
		int lodN = N>>lod;
		int lodNdisplay = lodN+1;
		// Update vertices
		//float signs[] = { 1.0f, -1.0f };	
		tbb::parallel_for( 0, lodN, 1, [&](int m_prime)
		{
			for( int n_prime=0; n_prime<lodN; n_prime++)
			{
				Vector3 n;
				UINT index  = m_prime * lodN + n_prime;		// index into h_tilde..
				UINT index1 = m_prime * lodNdisplay + n_prime;	// index into vertices

				//float sign = signs[(n_prime + m_prime) & 1];

				//h_tilde[index]     = h_tilde[index] * sign;

				// Height
				vertices[index1].Position.z = displacement[lod][index].z;

				// Horizontal displacement
				vertices[index1].Position.x = (n_prime - lodN / 2.0f) * length / lodN + displacement[lod][index].x * lambda;
				vertices[index1].Position.y = (m_prime - lodN / 2.0f) * length / lodN + displacement[lod][index].y * lambda;
			
				// Normal
				n = displacement[lod][index];
				vertices[index1].Normal.x =  n.x;
				vertices[index1].Normal.y =  n.y;
				vertices[index1].Normal.z =  n.z;
				/*
				// For tiling
				if (n_prime == 0 && m_prime == 0) 
				{
					vertices[index1 + N + Ndisplay * N].Position.x = length/2.0f + h_tilde_dx[index].re * lambda;
					vertices[index1 + N + Ndisplay * N].Position.y = length/2.0f + h_tilde_dy[index].re * lambda;
					vertices[index1 + N + Ndisplay * N].Position.z = h_tilde[index].re;
			
					vertices[index1 + N + Ndisplay * N].Normal.x =  n.x;
					vertices[index1 + N + Ndisplay * N].Normal.y =  n.y;
					vertices[index1 + N + Ndisplay * N].Normal.z =  n.z;
				}
				if (n_prime == 0) 
				{
					vertices[index1 + N].Position.x = length/2.0f						+ h_tilde_dx[index].re * lambda;
					vertices[index1 + N].Position.y = (m_prime - N / 2.0f) * length / N	+ h_tilde_dy[index].re * lambda;
					vertices[index1 + N].Position.z = h_tilde[index].re;
			
					vertices[index1 + N].Normal.x =  n.x;
					vertices[index1 + N].Normal.y =  n.y;
					vertices[index1 + N].Normal.z =  n.z;
				}
				if (m_prime == 0) 
				{
					vertices[index1 + Ndisplay * N].Position.x = (n_prime - N / 2.0f) * length / N	+ h_tilde_dx[index].re * lambda;
					vertices[index1 + Ndisplay * N].Position.y = length/2.0f							+ h_tilde_dy[index].re * lambda;
					vertices[index1 + Ndisplay * N].Position.z = h_tilde[index].re;
			
					vertices[index1 + Ndisplay * N].Normal.x =  n.x;
					vertices[index1 + Ndisplay * N].Normal.y =  n.y;
					vertices[index1 + Ndisplay * N].Normal.z =  n.z;
				}*/
			}
		});

		// Send this vertex data to the GPU
		// Index buffer remains the same
		BufferLayout layout;
		layout.AddElement( "POSITION", 0, R32G32B32_FLOAT );
		layout.AddElement( "NORMAL", 0, R32G32B32_FLOAT );
		layout.AddElement( "TEXCOORD", 0, R32G32_FLOAT );
		m_pMesh->GetSubmesh(0)->GetGeometryChunk()->UpdateVertexBuffer( (float*)&vertices[0], Ndisplay*Ndisplay, layout.GetByteSize()*Ndisplay*Ndisplay, layout.GetByteSize() ); 
	}
}

Complex FFTWater::SampleBilinear( Complex* buffer, const Vector2 &ij )
{
	float i = ij.x;
	float j = ij.y;

	if (i<0.0f)
		i = N + fmod(i, N) - 1.0f;
	if (j<0.0f)
		j = N + fmod(j, N) - 1.0f;

	Vector2 wrappedij = Vector2(fmod(i,N), fmod(j,N)); // Wrap
	
	int index_00 = floor(wrappedij.x) * N + floor(wrappedij.y);
	int index_01 = floor(wrappedij.x) * N + ceil(wrappedij.y);
	int index_10 = ceil(wrappedij.x) * N + floor(wrappedij.y);
	int index_11 = ceil(wrappedij.x) * N + ceil(wrappedij.y);
	
	Complex value_00 = buffer[index_00];
	Complex value_01 = buffer[index_01];
	Complex value_10 = buffer[index_10];
	Complex value_11 = buffer[index_11];

	float x = fmod(wrappedij.x,1.0f);
	float y = fmod(wrappedij.y,1.0f);

	// Interpolation :
	// f(x,y) = f(0,0)(1-x)(1-y) + f(1,0)x(1-y) + f(0,1)(1-x)y + f(1,1)xy;
		
	return (value_00*(1.0f-x)*(1.0f-y) + value_10*x*(1.0f-y) + value_01*(1.0f-x)*y + value_11*x*y);
	
	//return buffer[int(floor(wrappedij.x) * N + floor(wrappedij.y))];
}

void FFTWater::CreateGrid( int Xsegments, int Ysegments )
{
	// Build vertex buffer at t=0, this is simply a tesselated flat plane
	Complex htilde0, htilde0mk_conj;
	UINT index;
	for (int m_prime = 0; m_prime < Xsegments; m_prime++) 
	{
		for (int n_prime = 0; n_prime < Ysegments; n_prime++) 
		{
			index = m_prime * Ndisplay + n_prime;

			vertices[index].Position.x =  (n_prime - Xsegments / 2.0f) * length / Xsegments;
			vertices[index].Position.y =  (m_prime - Ysegments / 2.0f) * length / Ysegments;
			vertices[index].Position.z =  0.0f;

			vertices[index].Normal.x = 0.0f;
			vertices[index].Normal.y = 0.0f;
			vertices[index].Normal.z = 1.0f;

			vertices[index].Tex.x = (n_prime - Xsegments / 2.0f) * length / Xsegments;
			vertices[index].Tex.y = (m_prime - Ysegments / 2.0f) * length / Ysegments;
		}
	}

	// Build index buffer
	indices_count = 0;
	for (int m_prime = 0; m_prime < Xsegments; m_prime++) 
	{
		for (int n_prime = 0; n_prime < Ysegments; n_prime++) 
		{
			index = m_prime * Ndisplay + n_prime;
			/*
			//if (geometry) 
			{
				indices[indices_count++] = index;				// lines
				indices[indices_count++] = index + 1;
				indices[indices_count++] = index;
				indices[indices_count++] = index + Nplus1;
				indices[indices_count++] = index;
				indices[indices_count++] = index + Nplus1 + 1;
				if (n_prime == N - 1) 
				{
					indices[indices_count++] = index + 1;
					indices[indices_count++] = index + Nplus1 + 1;
				}
				if (m_prime == N - 1) 
				{
					indices[indices_count++] = index + Nplus1;
					indices[indices_count++] = index + Nplus1 + 1;
				}
			} /*
			else */
			{
				// two triangles
				indices[indices_count++] = index + 1;
				indices[indices_count++] = index + Ndisplay + 1;
				indices[indices_count++] = index + Ndisplay;

				indices[indices_count++] = index + 1;
				indices[indices_count++] = index + Ndisplay;
				indices[indices_count++] = index;
			}
		}
	}
}

void FFTWater::CreateRadialGrid(int Xsegments, int Ysegments)
{	
	float bias = 1.8f;
	float TAU = PI * 2.0f;
	float r;
	for (int x = 0; x < Xsegments; x++)
	{
		for (int y = 0; y < Ysegments; y++)
		{
			r = (float)x / (float)(Xsegments - 1);
			r = pow(r, bias) * 1000.0f;
					
			positionsBase[x + y * Xsegments].x = r * cos(TAU * (float)y / (float)(Ysegments - 1));
			positionsBase[x + y * Xsegments].y = r * sin(TAU * (float)y / (float)(Ysegments - 1));
		}
	}
						
	int num = 0;
	for (int x = 0; x < Xsegments - 1; x++)
	{
		for (int y = 0; y < Ysegments - 1; y++)
		{
			indices[num++] = x + y * Xsegments;
			indices[num++] = (x + 1) + y * Xsegments;
			indices[num++] = x + (y + 1) * Xsegments;
					
			indices[num++] = x + (y + 1) * Xsegments;
			indices[num++] = (x + 1) + y * Xsegments;
			indices[num++] = (x + 1) + (y + 1) * Xsegments;
					
		}
	}

	indices_count = num;
			
}

void FFTWater::CreateSpectrum( const float A, const Vector2 wind )
{
	// Build hTilde(K,t) at t=0
	srand(1);
	this->A = A;
	this->w = wind;
	for (int m_prime = 0; m_prime < N; m_prime++) 
	{
		for (int n_prime = 0; n_prime < N; n_prime++) 
		{
			h_tilde0[m_prime * N + n_prime] = hTilde_0( n_prime,  m_prime);
			h_tilde0mk_conj[m_prime * N + n_prime] = hTilde_0( -n_prime,  -m_prime).Conjugate();
		}
	}
}


float UniformRandomVariable() 
{
	return (float)rand()/RAND_MAX;
}

Complex GaussianRandomVariable() 
{
	float x1, x2, w;
	do 
	{
	    x1 = 2.f * UniformRandomVariable() - 1.f;
	    x2 = 2.f * UniformRandomVariable() - 1.f;
	    w = x1 * x1 + x2 * x2;
	} while ( w >= 1.f );
	w = sqrt((-2.f * log(w)) / w);
	return Complex(x1 * w, x2 * w);
}