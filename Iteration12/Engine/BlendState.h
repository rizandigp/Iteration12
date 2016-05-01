#pragma once

#include "Prerequisites.h"


// Blend options
enum BLEND
{	
	BLEND_ZERO	= 1,
	BLEND_ONE	= 2,
	BLEND_SRC_COLOR	= 3,
	BLEND_INV_SRC_COLOR	= 4,
	BLEND_SRC_ALPHA	= 5,
	BLEND_INV_SRC_ALPHA	= 6,
	BLEND_DEST_ALPHA	= 7,
	BLEND_INV_DEST_ALPHA	= 8,
	BLEND_DEST_COLOR	= 9,
	BLEND_INV_DEST_COLOR	= 10,
	BLEND_SRC_ALPHA_SAT	= 11,
	BLEND_BLEND_FACTOR	= 14,
	BLEND_INV_BLEND_FACTOR	= 15,
	BLEND_SRC1_COLOR	= 16,
	BLEND_INV_SRC1_COLOR	= 17,
	BLEND_SRC1_ALPHA	= 18,
	BLEND_INV_SRC1_ALPHA	= 19
};

// Blend operations
enum BLEND_OP
{	
	BLEND_OP_ADD	= 1,
	BLEND_OP_SUBTRACT	= 2,
	BLEND_OP_REV_SUBTRACT	= 3,
	BLEND_OP_MIN	= 4,
	BLEND_OP_MAX	= 5
};

// Describes the blend state for a render target
struct BlendState
{
	// Enables blending. Defaults to false
	bool BlendEnable;
	// This blend option specifies the operation to perform on the RGB value that the pixel shader outputs. Defaults to BLEND_ONE
    BLEND SrcBlend;
	// This blend option specifies the operation to perform on the current RGB value in the render target. Defaults to BLEND_ZERO
    BLEND DestBlend;
	// This blend operation defines how to combine the SrcBlend and DestBlend operations. Defaults to BLEND_OP_ADD
    BLEND_OP BlendOp;
	// This blend option specifies the operation to perform on the alpha value that the pixel shader outputs. Blend options that end in _COLOR are not allowed. Defaults to BLEND_ONE
    BLEND SrcBlendAlpha;
	// This blend option specifies the operation to perform on the current alpha value in the render target. Blend options that end in _COLOR are not allowed. Defaults to BLEND_ZERO
    BLEND DestBlendAlpha;
	//This blend operation defines how to combine the SrcBlendAlpha and DestBlendAlpha operations. Defaults to BLEND_OP_ADD
    BLEND_OP BlendOpAlpha;

	// Default values
	BlendState() :
		BlendEnable(false),
		SrcBlend(BLEND_ONE),
		DestBlend(BLEND_ZERO),
		BlendOp(BLEND_OP_ADD),
		SrcBlendAlpha(BLEND_ONE),
		DestBlendAlpha(BLEND_ZERO),
		BlendOpAlpha(BLEND_OP_ADD)	{};

	inline bool operator==(const BlendState& other) const
	{
		return (memcmp(this, &other, sizeof(BlendState))==0);
	}
};

struct BlendStateHasher
{
	std::size_t operator()(const BlendState& state) const
	{
		std::size_t seed = state.BlendEnable ? 1 : 0;

		boost::hash_combine(seed, boost::hash_value(state.SrcBlend));
		boost::hash_combine(seed, boost::hash_value(state.DestBlend));
		boost::hash_combine(seed, boost::hash_value(state.BlendOp));
		boost::hash_combine(seed, boost::hash_value(state.SrcBlendAlpha));
		boost::hash_combine(seed, boost::hash_value(state.DestBlendAlpha));
		boost::hash_combine(seed, boost::hash_value(state.BlendOpAlpha));

		return seed;
	}
};