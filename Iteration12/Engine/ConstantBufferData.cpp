#pragma once

#include "ConstantBufferData.h"


void ConstantBufferData::SetData( UINT byteOffset, void* pData, UINT byteSize )
{
	//auto it = std::next(v.begin(), index);
	//std::vector<BYTE,tbb::scalable_allocator<BYTE>>::iterator it = ( m_Data.begin() + byteOffset ); 
	//std::copy()? memcpy()? which is fastest?
	//std::copy( (PBYTE)pData, (PBYTE)pData+byteSize, it );
	memcpy( &m_Data[byteOffset], pData, byteSize );
}