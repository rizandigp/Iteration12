#pragma once

#include "Prerequisites.h"
#include "Buffer.h"
#include "xnaCollision.h"
#include "GPUResource.h"

enum PRIMITIVE_TOPOLOGY
{	
	PRIMITIVE_TOPOLOGY_UNDEFINED	= 0,
	PRIMITIVE_TOPOLOGY_POINTLIST	= 1,
	PRIMITIVE_TOPOLOGY_LINELIST	= 2,
	PRIMITIVE_TOPOLOGY_LINESTRIP	= 3,
	PRIMITIVE_TOPOLOGY_TRIANGLELIST	= 4,
	PRIMITIVE_TOPOLOGY_TRIANGLESTRIP	= 5,
	PRIMITIVE_TOPOLOGY_LINELIST_ADJ	= 10,
	PRIMITIVE_TOPOLOGY_LINESTRIP_ADJ	= 11,
	PRIMITIVE_TOPOLOGY_TRIANGLELIST_ADJ	= 12,
	PRIMITIVE_TOPOLOGY_TRIANGLESTRIP_ADJ	= 13,
	PRIMITIVE_TOPOLOGY_1_CONTROL_POINT_PATCHLIST	= 33,
	PRIMITIVE_TOPOLOGY_2_CONTROL_POINT_PATCHLIST	= 34,
	PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST	= 35,
	PRIMITIVE_TOPOLOGY_4_CONTROL_POINT_PATCHLIST	= 36,
	PRIMITIVE_TOPOLOGY_5_CONTROL_POINT_PATCHLIST	= 37,
	PRIMITIVE_TOPOLOGY_6_CONTROL_POINT_PATCHLIST	= 38,
	PRIMITIVE_TOPOLOGY_7_CONTROL_POINT_PATCHLIST	= 39,
	PRIMITIVE_TOPOLOGY_8_CONTROL_POINT_PATCHLIST	= 40,
	PRIMITIVE_TOPOLOGY_9_CONTROL_POINT_PATCHLIST	= 41,
	PRIMITIVE_TOPOLOGY_10_CONTROL_POINT_PATCHLIST	= 42,
	PRIMITIVE_TOPOLOGY_11_CONTROL_POINT_PATCHLIST	= 43,
	PRIMITIVE_TOPOLOGY_12_CONTROL_POINT_PATCHLIST	= 44,
	PRIMITIVE_TOPOLOGY_13_CONTROL_POINT_PATCHLIST	= 45,
	PRIMITIVE_TOPOLOGY_14_CONTROL_POINT_PATCHLIST	= 46,
	PRIMITIVE_TOPOLOGY_15_CONTROL_POINT_PATCHLIST	= 47,
	PRIMITIVE_TOPOLOGY_16_CONTROL_POINT_PATCHLIST	= 48,
	PRIMITIVE_TOPOLOGY_17_CONTROL_POINT_PATCHLIST	= 49,
	PRIMITIVE_TOPOLOGY_18_CONTROL_POINT_PATCHLIST	= 50,
	PRIMITIVE_TOPOLOGY_19_CONTROL_POINT_PATCHLIST	= 51,
	PRIMITIVE_TOPOLOGY_20_CONTROL_POINT_PATCHLIST	= 52,
	PRIMITIVE_TOPOLOGY_21_CONTROL_POINT_PATCHLIST	= 53,
	PRIMITIVE_TOPOLOGY_22_CONTROL_POINT_PATCHLIST	= 54,
	PRIMITIVE_TOPOLOGY_23_CONTROL_POINT_PATCHLIST	= 55,
	PRIMITIVE_TOPOLOGY_24_CONTROL_POINT_PATCHLIST	= 56,
	PRIMITIVE_TOPOLOGY_25_CONTROL_POINT_PATCHLIST	= 57,
	PRIMITIVE_TOPOLOGY_26_CONTROL_POINT_PATCHLIST	= 58,
	PRIMITIVE_TOPOLOGY_27_CONTROL_POINT_PATCHLIST	= 59,
	PRIMITIVE_TOPOLOGY_28_CONTROL_POINT_PATCHLIST	= 60,
	PRIMITIVE_TOPOLOGY_29_CONTROL_POINT_PATCHLIST	= 61,
	PRIMITIVE_TOPOLOGY_30_CONTROL_POINT_PATCHLIST	= 62,
	PRIMITIVE_TOPOLOGY_31_CONTROL_POINT_PATCHLIST	= 63,
	PRIMITIVE_TOPOLOGY_32_CONTROL_POINT_PATCHLIST	= 64,
};

class GeometryChunk : public GPUResource
{
public:
	virtual void Bind( RenderDispatcher* pDispatcher )=0;
	virtual void Unbind( RenderDispatcher* pDispatcher )=0;

	virtual void UpdateVertexBuffer( float* vertices, UINT vertexCount, UINT byteSize, UINT stride )=0;

	void SetRenderSystem( RenderSystem* ptr )			{ m_pRenderSystem = ptr; };		
	void SetAABB( XNA::AxisAlignedBox* pAABB )			{ m_AABB = *pAABB; };
	void SetPrimitiveTopology( PRIMITIVE_TOPOLOGY topology ) { m_PrimitiveTopology = topology; };
	inline XNA::AxisAlignedBox* GetAABB()				{ return &m_AABB; };
	inline void SetDynamic( bool dyn )					{ m_bDynamic = dyn; };
	inline bool IsDynamic()								{ return m_bDynamic; };
	inline PRIMITIVE_TOPOLOGY GetPrimitiveTopology()	{ return m_PrimitiveTopology; };

protected:
	RenderSystem* m_pRenderSystem;
	XNA::AxisAlignedBox m_AABB;
	bool m_bDynamic;
	PRIMITIVE_TOPOLOGY m_PrimitiveTopology;
};