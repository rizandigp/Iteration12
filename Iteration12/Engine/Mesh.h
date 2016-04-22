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

	void SetGeometryChunk(GeometryChunk* ptr)	{ m_pGeometryChunk = ptr; };
	void SetMaterial(Material* ptr)				{ m_pMaterial = ptr; };
	void SetName(std::string name)				{ m_Name = name; };

	GeometryChunk* GetGeometryChunk()	{ return m_pGeometryChunk; };
	Material* GetMaterial()				{ return m_pMaterial; };
	XNA::AxisAlignedBox* GetAABB()		{ return m_pGeometryChunk->GetAABB(); };
	std::string GetName()				{ return m_Name; };

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

	void SetName( std::string name )		{ m_Name = name; };
	void AddSubmesh( Submesh* submesh )		{ m_pSubmeshes.push_back(submesh); };

	void SetMaterial(Material* ptr);
	
	void Render();

	UINT GetNumberOfSubmeshes() const		{ return m_pSubmeshes.size(); };
	std::string GetName() const				{ return m_Name; };
	Submesh* GetSubmesh(UINT index);
	Submesh* GetSubmesh(std::string name);

private:
	std::vector<Submesh*>	m_pSubmeshes;
	std::string m_Name;
};