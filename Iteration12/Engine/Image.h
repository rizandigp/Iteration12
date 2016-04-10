#pragma once

#include "Prerequisites.h"
#include <vector>
#include "Color.h"

/*
	2D bitmap class. 8 bit RGBA only at the moment
*/
class Image
{
public:
	Image(size_t width, size_t height, Color initValue = 0):
      m_Width(width), m_Height(height), m_Data(width*height, initValue)
    {}

	// Returns reference to pixel at location (x,y)
    inline Color& operator()(size_t x, size_t y) 
	{
        return m_Data.at(x + y * m_Width);
    }

	// Returns pixel at location (x,y)
	inline Color getPixel(size_t x, size_t y) const
	{
		return m_Data.at(x + y * m_Width);
	}

	// Copy pixel data from &other, does not change dimensions
	void copyFrom( Image& other )
	{
		std::copy( other.data(), other.data()+other.dataSize(), m_Data.begin() );
	}

	// Returns pointer to raw pixel data. Read-only
	inline const Color* data() const	{ return &m_Data[0]; };
	// Returns pointer to raw pixel data. Can modify content.
	inline Color* dataRaw()				{ return &m_Data[0]; }; 

	// Returns stride. Distance between each element of the array in bytes
	inline size_t stride()	const	{ return sizeof(Color); };
	// Returns pitch. The length of an image row in memory (width in pixels * bytes per pixel + padding ) in bytes
	inline size_t pitch()	const	{ return sizeof(Color)*m_Width; };
	inline size_t width()	const	{ return m_Width; };
	inline size_t height()	const	{ return m_Height; };
	inline size_t dataSize() const	{ return m_Width*m_Height*sizeof(Color); };

private:
	size_t m_Width, m_Height;
	std::vector<Color>	m_Data;
};