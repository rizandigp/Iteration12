#pragma once

#include <vector>

template<typename T>
class Array3D
{
public:
    Array3D(size_t x, size_t y, size_t z, T initValue = 0):
      m_Width(x), m_Height(y), m_Depth(z), m_Data(x*y*z, initValue)
    {}

    inline T& operator()(size_t x, size_t y, size_t z) {
        return m_Data.at(x + y * m_Width + z * m_Width * m_Height);
    }

	// Returns pointer to data. Read-only
	inline const T* Data()	const	{ return &m_Data[0]; };
	// Returns pointer to data. Can modify content
	inline T* DataRaw()				{ return &m_Data[0]; };

	inline size_t Width()	const	{ return m_Width; };
	inline size_t Height()	const	{ return m_Height; };
	inline size_t Depth()	const	{ return m_Depth; };
	inline size_t Stride()	const	{ return sizeof(T); };
	inline size_t Pitch()	const	{ return m_Width*sizeof(T); };
	inline size_t SlicePitch()const { return m_Width*m_Depth*sizeof(T); };
	inline size_t Size()	const	{ return m_Width*m_Height*m_Depth*sizeof(T); };

private:
	size_t m_Width, m_Height, m_Depth;
	std::vector<T> m_Data;
};

typedef Array3D<float> FloatArray3D;
typedef Array3D<int> IntArray3D;
typedef Array3D<unsigned int> UintArray3D;
