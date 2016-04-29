#pragma once

#include "Texture2D.h"

Texture2D::Texture2D() :	m_Height(0),
							m_Width(0),
							m_pRenderSystem(NULL),
							m_Format(TEXTURE_FORMAT_UNKNOWN)
{
}

Texture2D::~Texture2D()
{
}