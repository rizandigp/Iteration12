#pragma once

#include "Prerequisites.h"
#include "Texture2D.h"

class TextureCube : public virtual DX11Texture2D
{
public:
	TextureCube() : DX11Texture2D() {};
};

class D3D11TextureCube : public TextureCube
{
	public:
		D3D11TextureCube() : TextureCube()	{};
};