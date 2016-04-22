#pragma once

#include "Prerequisites.h"
#include <Windows.h>
#include <d3dx11.h>

class RenderOutput
{
public:
	RenderOutput( HWND hWnd, UINT width, UINT height, Texture2D* pRenderTarget, IDXGISwapChain* pSwapChain )
		: m_hWnd(hWnd),
		m_Width(width),
		m_Height(height),
		m_pRenderTarget(pRenderTarget),
		m_pSwapChain(pSwapChain)	{};

	IDXGISwapChain* GetSwapChain()	{ return m_pSwapChain; };

	void Present();

protected:
	HWND m_hWnd;
	UINT m_Width, m_Height;
	Texture2D* m_pRenderTarget;
	IDXGISwapChain* m_pSwapChain;
}