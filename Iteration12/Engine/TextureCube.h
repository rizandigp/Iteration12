#pragma once

#include "Prerequisites.h"
#include "Texture.h"
#include "Vector2.h"

class TextureCube : public Texture
{
public:
	TextureCube();
	virtual ~TextureCube();

	inline void SetRenderSystem( RenderSystem* ptr )		{ m_pRenderSystem = ptr; };
	inline void SetDimensions( UINT height, UINT width )	{ m_Height = height; m_Width = width; };
	inline void SetFormat( TEXTURE_FORMAT format )			{ m_Format = format; };
	inline void SetMipLevels( UINT levels )					{ m_MipLevels = levels; };

	inline RenderSystem* GetRenderSystem()					{ return m_pRenderSystem; };
	inline UINT GetHeight()									{ return m_Height; };
	inline UINT GetWidth()									{ return m_Width; };
	inline Vector2 GetDimensions()							{ return Vector2( (float)m_Width, (float)m_Height ); };
	inline TEXTURE_FORMAT GetFormat()						{ return m_Format; };
	inline UINT GetMipLevels()								{ return m_MipLevels; };

	virtual void ClearBuffer( Vector4 clearColorRGBA, float depth, UINT8 stencil )=0;
	virtual void ReleaseResources() = 0;

	// TODO : different brdfs
	virtual void Prefilter() = 0;

protected:
	UINT				m_Height, m_Width, m_MipLevels;
	RenderSystem*		m_pRenderSystem;		// Owner
	TEXTURE_FORMAT		m_Format;
};