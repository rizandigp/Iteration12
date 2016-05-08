#pragma once
#pragma warning( disable : 4244 )

#include "Math.h"


// 8-bit RGBA color class
class Color
{
public:
	uint8_t r, g, b, a;
	
public:
	Color()	: r(0), g(0), b(0), a(255)	{};
	Color(uint8_t r, uint8_t g, uint8_t b, uint8_t a) : r(r), g(g), b(b), a(a) {};
	Color(uint8_t val) : r(val), g(val), b(val), a(255)	{};

	inline Color ToLinear()
	{
		return Color( pow(r,2.2f),
						pow(g,2.2f),
						pow(b,2.2f),
						pow(a,2.2f) );
	}

	inline Color ToGamma()
	{
		return Color( pow(r,1.0/2.2f),
						pow(g,1.0/2.2f),
						pow(b,1.0/2.2f),
						pow(a,1.0/2.2f) );
	}

	inline Color& operator = ( const Color& rkVector )
    {
        r = rkVector.r;
        g = rkVector.g;
        b = rkVector.b;
        a = rkVector.a;

        return *this;
    }

	inline Color& operator = ( const uint8_t fScalar)
	{
		r = fScalar;
		g = fScalar;
		b = fScalar;
		a = fScalar;
		return *this;
	}

    inline bool operator == ( const Color& rkVector ) const
    {
        return ( r == rkVector.r &&
            g == rkVector.g &&
            b == rkVector.b &&
            a == rkVector.a );
    }

    inline bool operator != ( const Color& rkVector ) const
    {
        return ( r != rkVector.r ||
            g != rkVector.g ||
            b != rkVector.b ||
            a != rkVector.a );
    }
	/*
    inline Color& operator = (const Vector3& rhs)
    {
        r = rhs.x;
        g = rhs.y;
        b = rhs.z;
        a = 1.0f;
        return *this;
    }*/

    // arithmetic operations
    inline Color operator + ( const Color& rkVector ) const
    {
        return Color(
            r + rkVector.r,
            g + rkVector.g,
            b + rkVector.b,
            a + rkVector.a);
    }

    inline Color operator - ( const Color& rkVector ) const
    {
        return Color(
            r - rkVector.r,
            g - rkVector.g,
            b - rkVector.b,
            a - rkVector.a);
    }

    inline Color operator * ( const uint8_t fScalar ) const
    {
        return Color(
            r * fScalar,
            g * fScalar,
            b * fScalar,
            a * fScalar);
    }

	 inline Color operator * ( const float fScalar ) const
    {
        return Color(
            r * fScalar,
            g * fScalar,
            b * fScalar,
            a * fScalar);
    }

    inline Color operator * ( const Color& rhs) const
    {
        return Color(
            rhs.r * r,
            rhs.g * g,
            rhs.b * b,
            rhs.a * a);
    }

    inline Color operator / ( const uint8_t fScalar ) const
    {
        assert( fScalar != 0.0 );

        uint8_t fInv = 1.0f / fScalar;

        return Color(
            r * fInv,
            g * fInv,
            b * fInv,
            a * fInv);
    }

    inline Color operator / ( const Color& rhs) const
    {
        return Color(
            r / rhs.r,
            g / rhs.g,
            b / rhs.b,
            a / rhs.a);
    }

    inline const Color& operator + () const
    {
        return *this;
    }

    inline Color operator - () const
    {
        return Color(-r, -g, -b, -a);
    }

    inline friend Color operator * ( const uint8_t fScalar, const Color& rkVector )
    {
        return Color(
            fScalar * rkVector.r,
            fScalar * rkVector.g,
            fScalar * rkVector.b,
            fScalar * rkVector.a);
    }

	inline friend Color operator * ( const float fScalar, const Color& rkVector )
    {
        return Color(
            fScalar * rkVector.r,
            fScalar * rkVector.g,
            fScalar * rkVector.b,
            fScalar * rkVector.a);
    }

    inline friend Color operator / ( const uint8_t fScalar, const Color& rkVector )
    {
        return Color(
            fScalar / rkVector.r,
            fScalar / rkVector.g,
            fScalar / rkVector.b,
            fScalar / rkVector.a);
    }

    inline friend Color operator + (const Color& lhs, const uint8_t rhs)
    {
        return Color(
            lhs.r + rhs,
            lhs.g + rhs,
            lhs.b + rhs,
            lhs.a + rhs);
    }

    inline friend Color operator + (const uint8_t lhs, const Color& rhs)
    {
        return Color(
            lhs + rhs.r,
            lhs + rhs.g,
            lhs + rhs.b,
            lhs + rhs.a);
    }

    inline friend Color operator - (const Color& lhs, uint8_t rhs)
    {
        return Color(
            lhs.r - rhs,
            lhs.g - rhs,
            lhs.b - rhs,
            lhs.a - rhs);
    }

    inline friend Color operator - (const uint8_t lhs, const Color& rhs)
    {
        return Color(
            lhs - rhs.r,
            lhs - rhs.g,
            lhs - rhs.b,
            lhs - rhs.a);
    }

    // arithmetic updates
    inline Color& operator += ( const Color& rkVector )
    {
        r += rkVector.r;
        g += rkVector.g;
        b += rkVector.b;
        a += rkVector.a;

        return *this;
    }

    inline Color& operator -= ( const Color& rkVector )
    {
        r -= rkVector.r;
        g -= rkVector.g;
        b -= rkVector.b;
        a -= rkVector.a;

        return *this;
    }

    inline Color& operator *= ( const uint8_t fScalar )
    {
        r *= fScalar;
        g *= fScalar;
        b *= fScalar;
        a *= fScalar;
        return *this;
    }

    inline Color& operator += ( const uint8_t fScalar )
    {
        r += fScalar;
        g += fScalar;
        b += fScalar;
        a += fScalar;
        return *this;
    }

    inline Color& operator -= ( const uint8_t fScalar )
    {
        r -= fScalar;
        g -= fScalar;
        b -= fScalar;
        a -= fScalar;
        return *this;
    }

    inline Color& operator *= ( const Color& rkVector )
    {
        r *= rkVector.r;
        g *= rkVector.g;
        b *= rkVector.b;
        a *= rkVector.a;

        return *this;
    }

    inline Color& operator /= ( const uint8_t fScalar )
    {
        assert( fScalar != 0.0 );

        //uint8_t fInv = 1.0f / fScalar;

        r /= fScalar;
        g /= fScalar;
        b /= fScalar;
        a /= fScalar;

        return *this;
    }

    inline Color& operator /= ( const Color& rkVector )
    {
        r /= rkVector.r;
        g /= rkVector.g;
        b /= rkVector.b;
        a /= rkVector.a;

        return *this;
    }
};