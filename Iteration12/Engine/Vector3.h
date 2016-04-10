#pragma once

#include "Math.h"
#include "Vector2.h"
#include "Quaternion.h"


class  Vector3
{
public:
    float x, y, z;

public:
	inline Vector3()
	{
	}

	inline Vector3( const float fX, const float fY, const float fZ )
		: x( fX ), y( fY ), z( fZ )
	{
	}

	inline explicit Vector3( const float afCoordinate[3] )
		: x( afCoordinate[0] ),
		y( afCoordinate[1] ),
		z( afCoordinate[2] )
	{
	}

	inline explicit Vector3( const int afCoordinate[3] )
	{
		x = (float)afCoordinate[0];
		y = (float)afCoordinate[1];
		z = (float)afCoordinate[2];
	}

	inline explicit Vector3( float* const r )
		: x( r[0] ), y( r[1] ), z( r[2] )
	{
	}

	inline explicit Vector3( const float scaler )
		: x( scaler )
		, y( scaler )
		, z( scaler )
	{
	}
	
	inline explicit Vector3( const Vector2 &rhs, const float z )
		: x( rhs.x )
		, y( rhs.y )
		, z( z )
	{
	}

	inline void swap(Vector3& other)
	{
		std::swap(x, other.x);
		std::swap(y, other.y);
		std::swap(z, other.z);
	}

	inline float operator [] ( const size_t i ) const
	{
		//assert( i < 3 );
		return *(&x+i);
	}

	inline float& operator [] ( const size_t i )
	{
		//assert( i < 3 );
		return *(&x+i);
	}

	inline float* ptr()
	{
		return &x;
	}

	inline const float* ptr() const
	{
		return &x;
	}

	inline Vector3& operator = ( const Vector3& rkVector )
	{
		x = rkVector.x;
		y = rkVector.y;
		z = rkVector.z;

		return *this;
	}

	inline Vector3& operator = ( const float fScaler )
	{
		x = fScaler;
		y = fScaler;
		z = fScaler;
	
		return *this;
	}

	inline bool operator == ( const Vector3& rkVector ) const
	{
		return ( x == rkVector.x && y == rkVector.y && z == rkVector.z );
	}

	inline bool operator != ( const Vector3& rkVector ) const
	{
		return ( x != rkVector.x || y != rkVector.y || z != rkVector.z );
	}

	// arithmetic operations
	inline Vector3 operator + ( const Vector3& rkVector ) const
	{
		return Vector3(	
			x + rkVector.x,
			y + rkVector.y,
			z + rkVector.z);
	}

	inline Vector3 operator - ( const Vector3& rkVector ) const
	{
		return Vector3(
			x - rkVector.x,
			y - rkVector.y,
			z - rkVector.z);
	}

	inline Vector3 operator * ( const float fScalar ) const
	{
		return Vector3(
			x * fScalar,
			y * fScalar,
			z * fScalar);
	}

	 inline Vector3 operator * ( const Vector3& rhs) const
	{
		return Vector3(
			x * rhs.x,
			y * rhs.y,
			z * rhs.z);
	 }

	inline Vector3 operator / ( const float fScalar ) const
	{
		//assert( fScalar != 0.0 );

		float fInv = 1.0f / fScalar;

		return Vector3(
			x * fInv,
			y * fInv,
			z * fInv);
	}

	inline Vector3 operator / ( const Vector3& rhs) const
	{
		return Vector3(
			x / rhs.x,
			y / rhs.y,
			z / rhs.z);
	}

	inline const Vector3& operator + () const
	{
		return *this;
	}

	inline Vector3 operator - () const
	{
		return Vector3(-x, -y, -z);
	}

	// overloaded operators to help Vector3
	inline friend Vector3 operator * ( const float fScalar, const Vector3& rkVector )
	{
		return Vector3(
			fScalar * rkVector.x,
			fScalar * rkVector.y,
			fScalar * rkVector.z);
	}

	inline friend Vector3 operator / ( const float fScalar, const Vector3& rkVector )
	{
		return Vector3(
			fScalar / rkVector.x,
			fScalar / rkVector.y,
			fScalar / rkVector.z);
	}

	inline friend Vector3 operator + (const Vector3& lhs, const float rhs)
	{
		return Vector3(
			lhs.x + rhs,
			lhs.y + rhs,
			lhs.z + rhs);
	}

	inline friend Vector3 operator + (const float lhs, const Vector3& rhs)
	{
		return Vector3(
			lhs + rhs.x,
			lhs + rhs.y,
			lhs + rhs.z);
	}

	inline friend Vector3 operator - (const Vector3& lhs, const float rhs)
	{
		return Vector3(
			lhs.x - rhs,
			lhs.y - rhs,
			lhs.z - rhs);
	}

	inline friend Vector3 operator - (const float lhs, const Vector3& rhs)
	{
		return Vector3(
			lhs - rhs.x,
			lhs - rhs.y,
			lhs - rhs.z);
	}

	// arithmetic updates
	inline Vector3& operator += ( const Vector3& rkVector )
	{
		x += rkVector.x;
		y += rkVector.y;
		z += rkVector.z;

		return *this;
	}

	inline Vector3& operator += ( const float fScalar )
	{
		  x += fScalar;
		  y += fScalar;
		  z += fScalar;
            
		  return *this;
	}

	inline Vector3& operator -= ( const Vector3& rkVector )
	{
		x -= rkVector.x;
		y -= rkVector.y;
		z -= rkVector.z;
	
		return *this;
	}

	inline Vector3& operator -= ( const float fScalar )
	{
		x -= fScalar;
		y -= fScalar;
		z -= fScalar;
	
		return *this;
	}

	inline Vector3& operator *= ( const float fScalar )
	{
		x *= fScalar;
		y *= fScalar;
		z *= fScalar;
            
		  return *this;
	}

	inline Vector3& operator *= ( const Vector3& rkVector )
	{
		x *= rkVector.x;
		y *= rkVector.y;
		z *= rkVector.z;
	
		return *this;
	}

	inline Vector3& operator /= ( const float fScalar )
	{
		//assert( fScalar != 0.0 );

		float fInv = 1.0f / fScalar;

		x *= fInv;
		y *= fInv;
		z *= fInv;
	
		return *this;
	}

	inline Vector3& operator /= ( const Vector3& rkVector )
	{
		x /= rkVector.x;
		y /= rkVector.y;
		z /= rkVector.z;
	
		return *this;
	}

	inline float length () const
	{
		return sqrt( x * x + y * y + z * z );
	}

	inline float squaredLength () const
	{
		return x * x + y * y + z * z;
	}

	inline float distance(const Vector3& rhs) const
	{
		return (*this - rhs).length();
	}

	inline float squaredDistance(const Vector3& rhs) const
	{
		return (*this - rhs).squaredLength();
	}

	inline float dotProduct(const Vector3& vec) const
	{
		return x * vec.x + y * vec.y + z * vec.z;
	}

	inline float absDotProduct(const Vector3& vec) const
	{
		return abs(x * vec.x) + abs(y * vec.y) + abs(z * vec.z);
	}

	inline float normalise()
	{
		float fLength = sqrt( x * x + y * y + z * z );

		// Will also work for zero-sized vectors, but will change nothing
		// We're not using epsilons because we don't need to.
		// Read http://www.ogre3d.org/forums/viewtopic.php?f=4&t=61259
		if ( fLength > float(0.0f) )
		{
			float fInvLength = 1.0f / fLength;
			x *= fInvLength;
			y *= fInvLength;
			z *= fInvLength;
		}

		return fLength;
	}

	inline Vector3 crossProduct( const Vector3& rkVector ) const
	{
		return Vector3(
			y * rkVector.z - z * rkVector.y,
			z * rkVector.x - x * rkVector.z,
			x * rkVector.y - y * rkVector.x);
	}

	inline Vector3 midPoint( const Vector3& vec ) const
	{
		return Vector3(
			( x + vec.x ) * 0.5f,
			( y + vec.y ) * 0.5f,
			( z + vec.z ) * 0.5f );
	}

	inline bool operator < ( const Vector3& rhs ) const
	{
		if( x < rhs.x && y < rhs.y && z < rhs.z )
			return true;return false;
	}

	inline bool operator > ( const Vector3& rhs ) const
	{
		if( x > rhs.x && y > rhs.y && z > rhs.z )
			return true;
		return false;
	}

	inline void makeFloor( const Vector3& cmp )
	{
		  if( cmp.x < x ) x = cmp.x;
		  if( cmp.y < y ) y = cmp.y;
		  if( cmp.z < z ) z = cmp.z;
	}

	inline void makeCeil( const Vector3& cmp )
	{
		if( cmp.x > x ) x = cmp.x;
		if( cmp.y > y ) y = cmp.y;
		if( cmp.z > z ) z = cmp.z;
	}

	inline Vector3 perpendicular(void) const
	{
		static const float fSquareZero = (float)(1e-06 * 1e-06);
		Vector3 perp = this->crossProduct( Vector3::UNIT_X );
	
		// Check length
		if( perp.squaredLength() < fSquareZero )
		{
			/* This vector is the Y axis multiplied by a scalar, so we have
			to use another axis.
			*/
			perp = this->crossProduct( Vector3::UNIT_Y );
		}
		perp.normalise();
		return perp;
	}

	inline Vector3 randomDeviant(
		const float& radians,
		const Vector3& up = Vector3::ZERO ) const
	{
		Vector3 newUp;
	
		if (up == Vector3::ZERO)
		{
			// Generate an up vector
			newUp = this->perpendicular();
		}
		else
		{
			newUp = up;
		}
	
		// Rotate up vector by random amount around this
		Quaternion q;
		q.FromAngleAxis( (UnitRandom() * TWO_PI), *this );
		newUp = q * newUp;
	
		// Finally rotate this by given angle around randomised up
		q.FromAngleAxis( radians, newUp );
		return q * (*this);
	}

	inline float angleBetween(const Vector3& dest) const
	{
		float lenProduct = length() * dest.length();
	
		// Divide by zero check
		if(lenProduct < 1e-6f)
			lenProduct = 1e-6f;
	
		float f = dotProduct(dest) / lenProduct;
		f = clamp(f, (float)-1.0, (float)1.0);
		return acos(f);
	}

	Quaternion getRotationTo(const Vector3& dest,
		const Vector3& fallbackAxis = Vector3::ZERO) const
	{
		// Based on Stan Melax's article in Game Programming Gems
		Quaternion q;
		// Copy, since cannot modify local
		Vector3 v0 = *this;
		Vector3 v1 = dest;
		v0.normalise();
		v1.normalise();
	
		float d = v0.dotProduct(v1);
		// If dot == 1, vectors are the same
		if (d >= 1.0f)
		{
			return Quaternion::IDENTITY;
		}
	
		if (d < (1e-6f - 1.0f))
		{
			if (fallbackAxis != Vector3::ZERO)
			{
				// rotate 180 degrees about the fallback axis
				q.FromAngleAxis(PI, fallbackAxis);
			}
			else
			{
				// Generate an axis
				Vector3 axis = Vector3::UNIT_X.crossProduct(*this);
				if (axis.isZeroLength()) // pick another if colinear
					axis = Vector3::UNIT_Y.crossProduct(*this);
				axis.normalise();
				q.FromAngleAxis(PI, axis);
			}
		}
		else
		{
			float s = sqrt( (1+d)*2 );
			float invs = 1 / s;
		
			Vector3 c = v0.crossProduct(v1);
		
			q.x = c.x * invs;
			q.y = c.y * invs;
			q.z = c.z * invs;
			q.w = s * 0.5f;
			q.normalise();
		}
	
		return q;
	}

	inline bool isZeroLength(void) const
	{
		float sqlen = (x * x) + (y * y) + (z * z);
		return (sqlen < (1e-06 * 1e-06));
	}

	inline Vector3 normalisedCopy(void) const
	{
		Vector3 ret = *this;
		ret.normalise();
		return ret;
	}

	inline Vector3 reflect(const Vector3& normal) const
	{
		return Vector3( *this - ( 2 * this->dotProduct(normal) * normal ) );
	}

	inline bool positionEquals(const Vector3& rhs, float tolerance = 1e-03) const
	{
		return floatEqual(x, rhs.x, tolerance) &&
			floatEqual(y, rhs.y, tolerance) &&
			floatEqual(z, rhs.z, tolerance);
	}

	inline bool positionCloses(const Vector3& rhs, float tolerance = 1e-03f) const
	{
		return squaredDistance(rhs) <=
			(squaredLength() + rhs.squaredLength()) * tolerance;
	}

	inline bool directionEquals(const Vector3& rhs,
		const float& toleranceInRadians) const
	{
		  float dot = dotProduct(rhs);
		  float fAngleRads = acos(dot);
	  
		  return abs(fAngleRads) <= toleranceInRadians;
	}
	/*
	inline bool isNaN() const
	{
		return Math::isNaN(x) || Math::isNaN(y) || Math::isNaN(z);
	}*/

	inline Vector3 primaryAxis() const
	{
		float absx = abs(x);
		float absy = abs(y);
		float absz = abs(z);
		if (absx > absy)
			if (absx > absz)
				return x > 0 ? Vector3::UNIT_X : Vector3::NEGATIVE_UNIT_X;
			else
				return z > 0 ? Vector3::UNIT_Z : Vector3::NEGATIVE_UNIT_Z;
		else // absx <= absy
			if (absy > absz)
				return y > 0 ? Vector3::UNIT_Y : Vector3::NEGATIVE_UNIT_Y;
			else
				return z > 0 ? Vector3::UNIT_Z : Vector3::NEGATIVE_UNIT_Z;
	}

	// Returns a unit vector of this
	inline Vector3 Vector3::unit() const
	{
		float l = this->length();
		return Vector3(this->x/l, this->y/l, this->z/l);
	}

	// special points
	static const Vector3 ZERO;
	static const Vector3 UNIT_X;
	static const Vector3 UNIT_Y;
	static const Vector3 UNIT_Z;
	static const Vector3 NEGATIVE_UNIT_X;
	static const Vector3 NEGATIVE_UNIT_Y;
	static const Vector3 NEGATIVE_UNIT_Z;
	static const Vector3 UNIT_SCALE;

	inline friend std::ostream& operator <<
		( std::ostream& o, const Vector3& v )
	{
		o << "Vector3(" << v.x << ", " << v.y << ", " << v.z << ")";
		return o;
	}

	inline XMFLOAT3 intoXMFLOAT3() const
	{
		return XMFLOAT3(x,y,z);
	}
};