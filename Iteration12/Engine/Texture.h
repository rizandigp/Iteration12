#pragma once

#include "GPUResource.h"

class Texture : public GPUResource
{
public:
	// Bind as shader input to the pipeline
	virtual void Bind( const std::string& name, RenderDispatcher* pDispatcher )=0;
	// Unbind from pipeline
	virtual void Unbind( const std::string& name, RenderDispatcher* pDispatcher )=0;
};