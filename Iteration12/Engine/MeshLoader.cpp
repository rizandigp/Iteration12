#pragma once

#include "MeshLoader.h"
#include "Vector3.h"
#include "Vector3.h"
#include "DX11/DX11RenderDispatcher.h"
#include "DX11/DX11GeometryChunk.h"

GeometryChunk* MeshLoader::FromFile( DX11RenderDispatcher* pDispatcher , const char* pFile, UINT meshNumber )
{
	// Create an instance of the Importer class   
	Assimp::Importer importer;   
	// And have it read the given file with some example postprocessing   
	const aiScene* scene = NULL;
	scene = importer.ReadFile( pFile,
		aiProcess_MakeLeftHanded		|
		aiProcess_FlipWindingOrder		|
		aiProcess_FlipUVs				|
		aiProcess_CalcTangentSpace		|
		//aiProcess_GenNormals			|
		//aiProcess_FixInfacingNormals	|
		//aiProcess_GenSmoothNormals		|
		aiProcess_JoinIdenticalVertices	|		// ALWAYS enable this! ALWAYS!!!!!
		aiProcess_OptimizeMeshes		|
		aiProcess_ImproveCacheLocality	|
		aiProcess_Triangulate		);   

	// If the import failed, report it
	if( !scene)
	{
		DEBUG_OUTPUT( __FUNCTION__ );
		DEBUG_OUTPUT( " : '" );
		DEBUG_OUTPUT( pFile );
		DEBUG_OUTPUT( "' : " );
		//DEBUG_OUTPUT( " : Failed loading file : " );
		DEBUG_OUTPUT( importer.GetErrorString() );
		DEBUG_OUTPUT( "\n" );
		return NULL;
	}

	std::vector<Vector3>	positions,normals,tangents,bitangents;
	std::vector<Vector2>	texcoords;
	BufferLayout layout;
	int vertexFloatSize = 0;	// size of a vertex in number of floats
	unsigned int i = 0, j = 0;

	// Get the first mesh
	const aiMesh* mesh = NULL;
	mesh = scene->mMeshes[meshNumber];
	if(!mesh)
	{		
		DEBUG_OUTPUT( __FUNCTION__ );
		DEBUG_OUTPUT( " : " );
		DEBUG_OUTPUT( pFile );
		DEBUG_OUTPUT( " : No mesh found in file : " );
		DEBUG_OUTPUT( importer.GetErrorString() );
		DEBUG_OUTPUT( "\n" );
		return NULL;
	}

	// Set up vertex buffer data layout
	if( mesh->HasPositions() )
	{
		layout.AddElement( "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT );
		vertexFloatSize += 3;
		for( i = 0; i<mesh->mNumVertices; i++ )
		{
			positions.push_back( *(Vector3*)&mesh->mVertices[i].x );
		}
	}
	
	if( mesh->HasTextureCoords(0) )
	{
		layout.AddElement( "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT );
		vertexFloatSize += 2;
		for( i = 0; i<mesh->mNumVertices; i++ )
		{
			texcoords.push_back( *(Vector2*)&mesh->mTextureCoords[0][i].x );

		}
	}
	if( mesh->HasNormals() )
	{
		layout.AddElement( "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT );
		vertexFloatSize += 3;
		for( i = 0; i<mesh->mNumVertices; i++ )
		{
			normals.push_back( *(Vector3*)&mesh->mNormals[i].x );
		}
	}
	if( mesh->HasTangentsAndBitangents() )
	{
		layout.AddElement( "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT );
		layout.AddElement( "BITANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT );
		vertexFloatSize += 6;
		for( i = 0; i<mesh->mNumVertices; i++ )
		{
			tangents.push_back( *(Vector3*)&mesh->mTangents[i].x );
			bitangents.push_back( *(Vector3*)&mesh->mBitangents[i].x );
		}
	}
	

	try
	{

	// Vertex buffer data
	int arraySize = mesh->mNumVertices*vertexFloatSize;
	float* pVertexData = new float [arraySize];
	ZeroMemory( pVertexData, arraySize*sizeof(float) );

	float* temp = pVertexData;
	for( i = 0; i<mesh->mNumVertices; i++ )
	{
		if( mesh->HasPositions() )
		{
			*temp = positions[i].x; temp++;
			*temp = positions[i].y; temp++;
			*temp = positions[i].z; temp++;
		}
		if( mesh->HasTextureCoords(0) )
		{
			*temp = texcoords[i].x; temp++;
			*temp = texcoords[i].y; temp++;
		}
		if( mesh->HasNormals() )
		{
			*temp = normals[i].x; temp++;
			*temp = normals[i].y; temp++;
			*temp = normals[i].z; temp++;
		}
		if( mesh->HasTangentsAndBitangents() )
		{
			*temp = tangents[i].x; temp++;
			*temp = tangents[i].y; temp++;
			*temp = tangents[i].z; temp++;

			*temp = bitangents[i].x; temp++;
			*temp = bitangents[i].y; temp++;
			*temp = bitangents[i].z; temp++;
		}
	}


	// Index buffer data
	int arraySize2 = mesh->mNumFaces*mesh->mFaces[0].mNumIndices;
	UINT* pIndexData = new UINT[arraySize2];
	ZeroMemory( pIndexData, arraySize2*sizeof(WORD) );

	UINT* temp2 = pIndexData;
	for( i = 0; i<mesh->mNumFaces; i++ )
	{
		for ( j = 0; j<mesh->mFaces[0].mNumIndices; j++ )
		{
			*temp2 = (UINT)mesh->mFaces[i].mIndices[j];	temp2++;
		}
	};
	
	GeometryChunk* geom = pDispatcher->CreateGeometryChunk( pVertexData, vertexFloatSize*sizeof(float), arraySize*sizeof(float), layout, pIndexData, arraySize2 );
	delete[] pVertexData;
	delete[] pIndexData;
	importer.FreeScene();
	return geom;
	}
	
	catch(std::bad_alloc& exc)
	{
	DEBUG_OUTPUT( __FUNCTION__ );
	DEBUG_OUTPUT(	" : new[] : " );
	DEBUG_OUTPUT( exc.what() );
	DEBUG_OUTPUT( "\n" );
	DEBUG_OUTPUT( __FUNCTION__ );
	DEBUG_OUTPUT( " : " );
	DEBUG_OUTPUT( pFile );
	DEBUG_OUTPUT( " : Memory allocation failed. RAM full?\n" );
	DX11GeometryChunk* ret = new DX11GeometryChunk();
	return ret;
	}
	
}


Mesh* MeshLoader::FromFile( DX11RenderDispatcher* pDispatcher, const char* filename )
{
	// Create an instance of the Importer class   
	Assimp::Importer importer;   
	// And have it read the given file with some example postprocessing   
	const aiScene* scene = NULL;
	scene = importer.ReadFile( filename,
		//aiProcess_MakeLeftHanded		|
		//aiProcess_FlipWindingOrder		|
		aiProcess_FlipUVs				|
		aiProcess_CalcTangentSpace		|
		//aiProcess_GenNormals			|
		//aiProcess_FixInfacingNormals	|
		aiProcess_GenSmoothNormals		|
		aiProcess_JoinIdenticalVertices	|		// ALWAYS enable this! ALWAYS!!!!!
		aiProcess_OptimizeMeshes		|
		aiProcess_ImproveCacheLocality	|
		aiProcess_Triangulate			
		//aiProcess_PreTransformVertices	|
		//aiProcess_Debone
		);   

	// If the import failed, report it
	if( !scene)
	{
		DEBUG_OUTPUT( __FUNCTION__ );
		DEBUG_OUTPUT( " : '" );
		DEBUG_OUTPUT( filename );
		DEBUG_OUTPUT( "' : " );
		DEBUG_OUTPUT( "Failed loading file : " );
		DEBUG_OUTPUT( importer.GetErrorString() );
		DEBUG_OUTPUT( "\n" );
		return NULL;
	}

	Mesh* ret = new Mesh();
	ret->SetName( filename );

	// Get the meshes
	const aiMesh* mesh = NULL;
	for (int ii=0; ii<scene->mNumMeshes; ii++)
	{
		std::vector<Vector3>	positions,normals,tangents,bitangents;
		std::vector<Vector2>	texcoords;
		BufferLayout layout;
		int vertexFloatSize = 0;	// size of a vertex in number of floats
		unsigned int i = 0, j = 0;

		mesh = scene->mMeshes[ii];
		/*
		if(!mesh)
		{		
			DEBUG_OUTPUT( __FUNCTION__ );
			DEBUG_OUTPUT( " : " );
			DEBUG_OUTPUT( pFile );
			DEBUG_OUTPUT( " : No mesh found in file : " );
			DEBUG_OUTPUT( importer.GetErrorString() );
			DEBUG_OUTPUT( "\n" );
			return NULL;
		}
		*/

		// Set up vertex buffer data layout
		if( mesh->HasPositions() )
		{
			layout.AddElement( "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT );
			vertexFloatSize += 3;
			for( i = 0; i<mesh->mNumVertices; i++ )
			{
				positions.push_back( *(Vector3*)&mesh->mVertices[i].x );
			}
		}
	
		if( mesh->HasTextureCoords(0) )
		{
			layout.AddElement( "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT );
			vertexFloatSize += 2;
			for( i = 0; i<mesh->mNumVertices; i++ )
			{
				texcoords.push_back( *(Vector2*)&mesh->mTextureCoords[0][i].x );
	
			}
		}
		if( mesh->HasNormals() )
		{
			layout.AddElement( "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT );
			vertexFloatSize += 3;
			for( i = 0; i<mesh->mNumVertices; i++ )
			{
				normals.push_back( *(Vector3*)&mesh->mNormals[i].x );
			}
		}
		if( mesh->HasTangentsAndBitangents() )
		{
			layout.AddElement( "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT );
			layout.AddElement( "BITANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT );
			vertexFloatSize += 6;
			for( i = 0; i<mesh->mNumVertices; i++ )
			{
				tangents.push_back( *(Vector3*)&mesh->mTangents[i].x );
				bitangents.push_back( *(Vector3*)&mesh->mBitangents[i].x );
			}
		
		}
		
	
		try
		{
	
			// Vertex buffer data
			int arraySize = mesh->mNumVertices*vertexFloatSize;
			float* pVertexData = new float [arraySize];
			ZeroMemory( pVertexData, arraySize*sizeof(float) );
	
			float* temp = pVertexData;
			for( i = 0; i<mesh->mNumVertices; i++ )
			{
				if( mesh->HasPositions() )
				{
					*temp = positions[i].x; temp++;
					*temp = positions[i].y; temp++;
					*temp = positions[i].z; temp++;
				}
				if( mesh->HasTextureCoords(0) )
				{
					*temp = texcoords[i].x; temp++;
					*temp = texcoords[i].y; temp++;
				}
				if( mesh->HasNormals() )
				{
					*temp = normals[i].x; temp++;
					*temp = normals[i].y; temp++;
					*temp = normals[i].z; temp++;
				}
				if( mesh->HasTangentsAndBitangents() )
				{
					*temp = tangents[i].x; temp++;
					*temp = tangents[i].y; temp++;
					*temp = tangents[i].z; temp++;
	
					*temp = bitangents[i].x; temp++;
					*temp = bitangents[i].y; temp++;
					*temp = bitangents[i].z; temp++;
				}
			}
	
	
			// Index buffer data
			int arraySize2 = mesh->mNumFaces*mesh->mFaces[0].mNumIndices;
			UINT* pIndexData = new UINT[arraySize2];
			ZeroMemory( pIndexData, arraySize2*sizeof(WORD) );
			
			UINT* temp2 = pIndexData;
			for( i = 0; i<mesh->mNumFaces; i++ )
			{
				for ( j = 0; j<mesh->mFaces[0].mNumIndices; j++ )
				{
					*temp2 = (UINT)mesh->mFaces[i].mIndices[j];	temp2++;
				}
			};
		
			GeometryChunk* geom = pDispatcher->CreateGeometryChunk( pVertexData, 
																	vertexFloatSize*sizeof(float), 
																	arraySize*sizeof(float), 
																	layout, pIndexData, 
																	arraySize2 );
			delete[] pVertexData;
			delete[] pIndexData;

			Submesh* submesh = new Submesh();
			submesh->SetGeometryChunk( geom );
			submesh->SetName(mesh->mName.C_Str());
	
			ret->AddSubmesh( submesh );
		}
		
		catch(std::bad_alloc& exc)
		{
			DEBUG_OUTPUT( __FUNCTION__ );
			DEBUG_OUTPUT(	" : new[] : " );
			DEBUG_OUTPUT( exc.what() );
			DEBUG_OUTPUT( "\n" );
			DEBUG_OUTPUT( __FUNCTION__ );
			DEBUG_OUTPUT( " : " );
			DEBUG_OUTPUT( filename );
			DEBUG_OUTPUT( " : Memory allocation failed. RAM full?\n" );
			//DX11GeometryChunk* ret = new DX11GeometryChunk();
			return NULL;
		}
	}
	importer.FreeScene();

	return ret;
}