#pragma once

#include "TextureCube.h"


TextureCube::TextureCube() :	m_Height(0), 
								m_Width(0), 
								m_pRenderSystem(NULL), 
								m_Format(TEXTURE_FORMAT_UNKNOWN)
{
}

TextureCube::~TextureCube()
{
}