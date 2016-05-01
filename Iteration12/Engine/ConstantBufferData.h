#pragma once

#include "Prerequisites.h"


// Containter storing aligned data for a constant buffer
class ConstantBufferData
{
public:
	inline void Init( UINT byteSize )		{ m_Data.resize(byteSize); };
	void SetData( UINT byteOffset, void* pData, UINT byteSize );

	inline UINT GetByteSize()				{ return m_Data.size(); };
	inline UINT GetVectorCapacity()			{ return m_Data.capacity(); };
	inline PBYTE GetData()					{ return (PBYTE)m_Data.data(); };

protected:
	std::vector<BYTE,tbb::scalable_allocator<BYTE>>	m_Data;
};