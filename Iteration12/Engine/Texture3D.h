#pragma once

#include "Prerequisites.h"

#include <d3d11.h>
#include "GPUResource.h"
#include "Texture2D.h"

// TODO : 3D Texture as render target
class Texture3D : public Texture
{
public:
	Texture3D();
	virtual ~Texture3D();

	inline void SetRenderSystem( RenderSystem* ptr )					{ m_pRenderSystem = ptr; };
	inline void SetDimensions( UINT height, UINT width, UINT depth )	{ m_Height = height; m_Width = width; m_Depth = depth; };

	inline RenderSystem* GetRenderSystem()							{ return m_pRenderSystem; };
	inline UINT GetHeight()											{ return m_Height; };
	inline UINT GetWidth()											{ return m_Width; };
	inline UINT GetDepth()											{ return m_Depth; };

	virtual void Bind( const std::string& name, RenderDispatcher* pDispatcher ) = 0;
	virtual void Unbind( const std::string& name, RenderDispatcher* pDispatcher ) = 0;
	//virtual void BindRenderTarget( RenderDispatcher* pDispatcher)=0;
	//virtual void UnbindRenderTarget( RenderDispatcher* pDispatcher)=0;

	virtual void UpdateData(const void* data)=0;
	//virtual void ClearBuffer( float* clearColorRGBA, float depth, UINT8 stencil )=0;

protected:
	UINT						m_Height, m_Width, m_Depth, m_Miplevels;
	RenderSystem*				m_pRenderSystem;		// Owner
	TEXTURE_FORMAT				m_Format;
};