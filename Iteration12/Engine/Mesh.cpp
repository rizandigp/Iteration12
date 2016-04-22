#pragma once

#include "Mesh.h"

Mesh::Mesh( Mesh &other )
{
	m_Name = other.GetName();
	for (int i=0; i < other.GetNumberOfSubmeshes(); i++)
	{
		AddSubmesh( new Submesh(*other.GetSubmesh(i)) );
	}
}

void Mesh::SetMaterial(Material* ptr)
{
	for (std::vector<Submesh*>::iterator it = m_pSubmeshes.begin(); it != m_pSubmeshes.end(); ++it)
	{
		(*it)->SetMaterial( ptr );
	}
}

Submesh* Mesh::GetSubmesh(UINT index)
{ 
	if (index<=m_pSubmeshes.size()) 
		return m_pSubmeshes[index]; 
	else 
		//return m_pSubmeshes[0]; 
		return NULL;
};

Submesh* Mesh::GetSubmesh(std::string name)
{
	for (std::vector<Submesh*>::iterator it = m_pSubmeshes.begin(); it != m_pSubmeshes.end(); ++it)
	{
		if((*it)->GetName()==name)
			return (*it);
	}
	return NULL;
}