#pragma once

#include "Mesh.h"

Mesh::Mesh( Mesh &other )
{
	m_Name = other.getName();
	for (int i=0; i < other.getNumberOfSubmeshes(); i++)
	{
		addSubmesh( new Submesh(*other.getSubmesh(i)) );
	}
}

void Mesh::setMaterial(Material* ptr)
{
	for (std::vector<Submesh*>::iterator it = m_pSubmeshes.begin(); it != m_pSubmeshes.end(); ++it)
	{
		(*it)->setMaterial( ptr );
	}
}

Submesh* Mesh::getSubmesh(UINT index)
{ 
	if (index<=m_pSubmeshes.size()) 
		return m_pSubmeshes[index]; 
	else 
		//return m_pSubmeshes[0]; 
		return NULL;
};

Submesh* Mesh::getSubmesh(std::string name)
{
	for (std::vector<Submesh*>::iterator it = m_pSubmeshes.begin(); it != m_pSubmeshes.end(); ++it)
	{
		if((*it)->getName()==name)
			return (*it);
	}
	return NULL;
}