#pragma once

#include <vector>

template<typename T>
class Array2D
{
public:
    Array2D(size_t x, size_t y, T initValue = 0):
      m_Width(x), m_Height(y), m_Data(x*y, initValue)
    {}

    inline T& operator()(size_t x, size_t y) {
        return m_Data.at(x + y * m_Width);
    }

	// Return pointer to data. Read-only
	inline const T* Data()	const	{ return &m_Data[0]; };
	// Return pointer to data. Can modify content
	inline T* DataRaw()				{ return &m_Data[0]; };

	inline size_t Width()	const	{ return m_Width; };
	inline size_t Height()	const	{ return m_Height; };
	inline size_t Stride()	const	{ return sizeof(T); };
	inline size_t Pitch()	const	{ return m_Width*sizeof(T); };
	inline size_t Size()	const	{ return m_Width*m_Height*sizeof(T); };

private:
	size_t m_Width, m_Height;
	std::vector<T> m_Data;
};

typedef Array2D<float> FloatArray2D;
typedef Array2D<int> IntArray2D;
typedef Array2D<unsigned int> UintArray2D;
