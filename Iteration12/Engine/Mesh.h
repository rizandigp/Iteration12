#pragma once

#include "Prerequisites.h"

#include <vector>
#include <string>
#include "Material.h"
#include "GeometryChunk.h"

class Submesh
{
public:
	Submesh() : m_pGeometryChunk(NULL), m_pMaterial(NULL)	{};
	void render(D3D11RenderCommand_Draw* pRenderCommand);

	void setGeometryChunk(GeometryChunk* ptr)	{ m_pGeometryChunk = ptr; };
	void setMaterial(Material* ptr)				{ m_pMaterial = ptr; };
	void setName(std::string name)				{ m_Name = name; };

	GeometryChunk* getGeometryChunk()	{ return m_pGeometryChunk; };
	Material* getMaterial()				{ return m_pMaterial; };
	XNA::AxisAlignedBox* getAABB()		{ return m_pGeometryChunk->getAABB(); };
	std::string getName()				{ return m_Name; };

private:
	GeometryChunk* m_pGeometryChunk;
	Material* m_pMaterial;
	std::string m_Name;
};

/*
	A mesh consist of one or more submeshes. Each with a geometry and material
	for rendering.
*/
class Mesh
{
public:
	Mesh()	{};
	Mesh( Mesh &other );

	void setName( std::string name )		{ m_Name = name; };
	void addSubmesh( Submesh* submesh )		{ m_pSubmeshes.push_back(submesh); };

	void setMaterial(Material* ptr);
	
	void render();

	UINT getNumberOfSubmeshes() const		{ return m_pSubmeshes.size(); };
	std::string getName() const				{ return m_Name; };
	Submesh* getSubmesh(UINT index);
	Submesh* getSubmesh(std::string name);

private:
	std::vector<Submesh*>	m_pSubmeshes;
	std::string m_Name;
};