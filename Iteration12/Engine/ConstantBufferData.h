#pragma once

#include <vector>
#include "Prerequisites.h"

// Containter storing aligned data for a constant buffer
class ConstantBufferData
{
public:
	inline void init( UINT byteSize )		{ m_Data.resize(byteSize); };
	void setData( UINT byteOffset, void* pData, UINT byteSize );

	inline UINT getByteSize()				{ return m_Data.size(); };
	inline UINT getVectorCapacity()			{ return m_Data.capacity(); };
	inline PBYTE getData()					{ return (PBYTE)m_Data.data(); };

protected:
	std::vector<BYTE,tbb::scalable_allocator<BYTE>>	m_Data;
};