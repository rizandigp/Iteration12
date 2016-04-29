#pragma once

#include "Texture3D.h"

Texture3D::Texture3D() :	m_Height(0),
							m_Width(0),
							m_Depth(0),
							m_pRenderSystem(NULL)
{
}

Texture3D::~Texture3D()
{
}