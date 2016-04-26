#pragma once

#include "Prerequisites.h"
#include "Texture2D.h"

class TextureCube : public DX11Texture2D
{
public:
	// TODO : different brdfs
	void Prefilter();
};