#pragma once

#include "Prerequisites.h"


// Determines which-facing triangles are culled
enum CULLING_MODE
{
	CULL_NONE	= 1,
	CULL_FRONT	= 2,
	CULL_BACK	= 3
};

// Depth & stencil comparison functions
enum COMPARISON_FUNCTION
{
	COMPARISON_NEVER	= 1,
	COMPARISON_LESS	= 2,
	COMPARISON_EQUAL	= 3,
	COMPARISON_LESS_EQUAL	= 4,
	COMPARISON_GREATER	= 5,
	COMPARISON_NOT_EQUAL	= 6,
	COMPARISON_GREATER_EQUAL	= 7,
	COMPARISON_ALWAYS	= 8
};

// Stencil operations
enum STENCIL_OP
{	
	STENCIL_OP_KEEP	= 1,
	STENCIL_OP_ZERO	= 2,
	STENCIL_OP_REPLACE	= 3,
	STENCIL_OP_INCR_SAT	= 4,
	STENCIL_OP_DECR_SAT	= 5,
	STENCIL_OP_INVERT	= 6,
	STENCIL_OP_INCR	= 7,
	STENCIL_OP_DECR	= 8
};

// Stencil operations that can be performed based on the results of stencil test.
struct DepthStencilOpDesciption
{
	//The stencil operation to perform when stencil testing fails. Defaults to STENCIL_OP_KEEP
    STENCIL_OP StencilFailOp;
    // The stencil operation to perform when stencil testing passes and depth testing fails. Defaults to STENCIL_OP_KEEP
	STENCIL_OP StencilDepthFailOp;
    // The stencil operation to perform when stencil testing and depth testing both pass. Defaults to STENCIL_OP_KEEP
	STENCIL_OP StencilPassOp;
	// A function that compares stencil data against existing stencil data. Defaults to COMPARISON_ALWAYS
    COMPARISON_FUNCTION StencilFunc;

	// Default values
	DepthStencilOpDesciption() :
		StencilFailOp(STENCIL_OP_KEEP),
		StencilDepthFailOp(STENCIL_OP_KEEP),
		StencilPassOp(STENCIL_OP_KEEP),
		StencilFunc(COMPARISON_ALWAYS) {};
};

struct RenderState
{
	// When true, render solid. When false, render wireframe. Defaults to true
	bool FillSolid;
	// Indicates triangles facing the specified direction are not drawn. Defaults to CULL_BACK
	CULLING_MODE CullingMode;
	// Depth value added to a given pixel. Defaults to 0
	int DepthBias;
	// Enables depth testing. Defaults to true
	bool EnableDepthTest;
	// A function that compares depth data against existing depth data. Defaults to COMPARISON_LESS
	COMPARISON_FUNCTION DepthComparison;
	// Enables depth writing. Defaults to true
	bool EnableDepthWrite;
	// Whether to enable stencil or not. Defaults to false
	bool EnableStencil;
	// Bit value that will mask which bits can be read. Defaults to 0xff
	UINT8 StencilReadMask;
	// Bit value that will mask which bits can be written. Defaults to 0xff
    UINT8 StencilWriteMask;
	// Stencil operations to do on front faces
    DepthStencilOpDesciption FrontFace;
	// Stencil operations to do on back faces
    DepthStencilOpDesciption BackFace;

	// Default values
	RenderState() :
		FillSolid(true),
		CullingMode(CULL_BACK),
		DepthBias(0),
		EnableDepthTest(true),
		DepthComparison(COMPARISON_LESS),
		EnableDepthWrite(true),
		EnableStencil(false),
		StencilReadMask(0xff),
		StencilWriteMask(0xff) {}

	inline bool operator==(const RenderState& other) const
	{
		if (FillSolid==other.FillSolid)
			if (CullingMode==other.CullingMode)
				if (DepthBias==other.DepthBias)
					if (EnableDepthTest==other.EnableDepthTest)
						if (DepthComparison==other.DepthComparison)
							if (EnableDepthWrite==other.EnableDepthWrite)
								if (EnableStencil==other.EnableStencil)
									if (StencilReadMask==other.StencilReadMask)
										if (StencilWriteMask==other.StencilWriteMask)
											if (FrontFace.StencilFailOp==other.FrontFace.StencilFailOp)
												if (FrontFace.StencilDepthFailOp==other.FrontFace.StencilDepthFailOp)
													if (FrontFace.StencilPassOp==other.FrontFace.StencilPassOp)
														if (FrontFace.StencilFunc==other.FrontFace.StencilFunc)
															if (BackFace.StencilFailOp==other.BackFace.StencilFailOp)
																if (BackFace.StencilDepthFailOp==other.BackFace.StencilDepthFailOp)
																	if (BackFace.StencilPassOp==other.BackFace.StencilPassOp)
																		if (BackFace.StencilFunc==other.BackFace.StencilFunc)
																			return true;

		return false;

		//return (memcmp(this, &other, sizeof(RenderState))==0);
	}
};

struct RenderStateHasher
{
	std::size_t operator()(const RenderState& state) const
	{
		std::size_t seed = state.FillSolid ? 1 : 0;

		boost::hash_combine(seed, boost::hash_value(state.CullingMode));
		boost::hash_combine(seed, boost::hash_value(state.DepthBias));
		boost::hash_combine(seed, boost::hash_value(state.EnableDepthTest));
		boost::hash_combine(seed, boost::hash_value(state.DepthComparison));
		boost::hash_combine(seed, boost::hash_value(state.EnableDepthWrite));
		boost::hash_combine(seed, boost::hash_value(state.EnableStencil));
		boost::hash_combine(seed, boost::hash_value(state.StencilReadMask));
		boost::hash_combine(seed, boost::hash_value(state.StencilWriteMask));

		return seed;
	}
};