#pragma once
#include "Prerequisites.h"
#include "Debug.h"

#include <stdio.h>
#include <fstream>

// assimp include files. These three are usually needed.
#include "assimp/Importer.hpp"	//OO version Header!
#include "assimp/Scene.h"
#include <assimp/postprocess.h>

//#include "Mesh.h"
#include "RenderDispatcher.h"
#include "Buffer.h"
#include "Mesh.h"
//#include "IRenderAgent.h"

class MeshLoader
{
public:
//	void _register( GeometryChunkRenderAgent* pRenderAgent )	{ m_GeometryChunkRenderAgent = pRenderAgent; };
//	Mesh* fromFile( const char* pFile );

	static GeometryChunk* FromFile( DX11RenderDispatcher* pDispatcher , const char* pFile, UINT meshNumber );

	static Mesh* FromFile( DX11RenderDispatcher* pDispatcher, const char* filename );

};