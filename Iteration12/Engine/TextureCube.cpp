#pragma once

#include "TextureCube.h"
#include "FullscreenQuad.h"

void TextureCube::Prefilter()
{
	// Create temporary render target texture for each mip level
	// Filtering will be done through these textures, and then copied to the TextureCube's mipmaps
	Texture2D **mips = new Texture2D*[m_MipLevels];
	int width = m_Width;
	int height = m_Height;
	for (int i=1; i<m_MipLevels; i++)
	{
		width /= 2;
		height /= 2;
		mips[i] = m_pRenderSystem->CreateTexture( height, width, m_Format );
	}

	FullscreenQuad filtering(m_pRenderSystem);
	filtering.SetShaderset( m_pRenderSystem->LoadShaderset( L"Shaders/PrefilterCubemap.hlsl", "VS", "PS", SM_AUTO ) );
	filtering.SetTexture( "texSource", (Texture2D*)this );

	// Get current bound render target so we can revert back to it later
	Texture2D* previousRenderTarget = m_pRenderSystem->GetRenderTarget();

	for (int face=0; face<6; face++)
	{
		float N = 4.0f;	// Number of samples will be N*N
		float Roughness;
		for (int i=1; i<m_MipLevels; i++)
		{
			N *= 1.5f;
			Roughness = i/(float)m_MipLevels;

			Matrix3x3 rotation;
			switch (face)
			{
			case 0:
				rotation.FromEulerAnglesXYZ(PI, PI/2.0f, PI );
				break;
			case 1:
				rotation.FromEulerAnglesXYZ(PI, -PI/2.0f, PI );
				break;
			case 2:
				rotation.FromEulerAnglesXYZ(-PI/2.0f, 0.0f, 0.0f );
				break;
			case 3:
				rotation.FromEulerAnglesXYZ(PI/2.0f, 0.0f, 0.0f );
				break;
			case 4:
				rotation.FromEulerAnglesXYZ(0.0f, 0.0f, 0.0f );
				break;
			case 5:
				rotation.FromEulerAnglesXYZ(0.0f, PI, 0.0f );
				break;
			};

			Vector4 clearColorRGBA( 0.0f, 0.0f, 0.0f, 0.0f );
			m_pRenderSystem->SetRenderTarget( mips[i] );
			m_pRenderSystem->ClearTexture( mips[i], clearColorRGBA );
			ShaderParamBlock shaderParams;
			shaderParams.assign( "vDimensions", 0, &Vector4( mips[i]->GetWidth(),  mips[i]->GetHeight(), 0.0f, 0.0f ) );
			shaderParams.assign( "World", 0, &Matrix4x4(rotation) );
			shaderParams.assign( "fRoughness", 0, Roughness );
			shaderParams.assign( "N", 0, N );
			filtering.SetShaderParams( shaderParams );
			filtering.Render();

			// Copy to mipmap
			// TODO : this should be a RenderSystem function
			DX11RenderCommand_CopySubresourceRegion copyCommand;
			copyCommand.SetSource( ((DX11Texture2D*)mips[i])->GetResource(), D3D11CalcSubresource(0, 0, m_MipLevels) );
			copyCommand.SetDestination( this->GetResource(), D3D11CalcSubresource(i, face, m_MipLevels) );

			m_pRenderSystem->Submit( &copyCommand );
		}
	}

	m_pRenderSystem->SetRenderTarget( previousRenderTarget );

	for (int i=1; i<m_MipLevels; i++)
	{
		delete mips[i];
	}
	delete[] mips;
}