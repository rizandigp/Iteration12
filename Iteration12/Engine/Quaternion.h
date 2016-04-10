// This file is based on material originally from:
// Geometric Tools, LLC
// Copyright (c) 1998-2010
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt

#pragma once

#include <cmath>
#include "Prerequisites.h"
#include "Matrix3x3.h"


class Quaternion
{
public:
	inline Quaternion () : w(1), x(0), y(0), z(0) {}
	inline Quaternion (
                float fW,
                float fX, float fY, float fZ)
                : w(fW), x(fX), y(fY), z(fZ)
	{
	}

	inline Quaternion(const Matrix3x3& rot)
	{
		this->FromRotationMatrix(rot);
	}
	inline Quaternion(const float& rfAngle, const Vector3& rkAxis)
	{
		this->FromAngleAxis(rfAngle, rkAxis);
	}
	inline Quaternion(const Vector3& xaxis, const Vector3& yaxis, const Vector3& zaxis)
	{
		this->FromAxes(xaxis, yaxis, zaxis);
	}
	inline Quaternion(const Vector3* akAxis)
	{
		this->FromAxes(akAxis);
	}
	inline Quaternion(float* valptr)
	{
		memcpy(&w, valptr, sizeof(float)*4);
	}
    
	inline void swap(Quaternion& other)
	{
		std::swap(w, other.w);
		std::swap(x, other.x);
		std::swap(y, other.y);
		std::swap(z, other.z);
	}
   
	inline float operator [] ( const size_t i ) const
	{
		//assert( i < 4 );
   
		return *(&w+i);
	}
   
	inline float& operator [] ( const size_t i )
	{
		//assert( i < 4 );
   
		return *(&w+i);
	}
   
	inline float* ptr()
	{
		return &w;
	}
   
	inline const float* ptr() const
	{
		return &w;
	}
   
	void FromRotationMatrix (const Matrix3x3& kRot);
	void ToRotationMatrix (Matrix3x3& kRot) const;
	void FromAngleAxis (const float& rfAngle, const Vector3& rkAxis);
	void ToAngleAxis (float& rfRadians, Vector3& rkAxis) const;
	/*inline void ToAngleAxis (float& dDegrees, Vector3& rkAxis) const {
		float rAngle;
		ToAngleAxis ( rAngle, rkAxis );
		dAngle = rAngle;
	}*/
	void FromAxes (const Vector3* akAxis);
	void FromAxes (const Vector3& xAxis, const Vector3& yAxis, const Vector3& zAxis);
	void ToAxes (Vector3* akAxis) const;
	void ToAxes (Vector3& xAxis, Vector3& yAxis, Vector3& zAxis) const;
	void lookAt (const Vector3& forward, const Vector3& up);
   
	Vector3 xAxis(void) const;
   
	Vector3 yAxis(void) const;
   
	Vector3 zAxis(void) const;
   
	inline Quaternion& operator= (const Quaternion& rkQ)
	{
		w = rkQ.w;
		x = rkQ.x;
		y = rkQ.y;
		z = rkQ.z;
		return *this;
	}
	Quaternion operator+ (const Quaternion& rkQ) const;
	Quaternion operator- (const Quaternion& rkQ) const;
	Quaternion operator* (const Quaternion& rkQ) const;
	Quaternion operator* (float fScalar) const;
	friend Quaternion operator* (float fScalar,
		const Quaternion& rkQ);
	Quaternion operator- () const;
	inline bool operator== (const Quaternion& rhs) const
	{
		return (rhs.x == x) && (rhs.y == y) &&
			(rhs.z == z) && (rhs.w == w);
	}
	inline bool operator!= (const Quaternion& rhs) const
	{
		return !operator==(rhs);
	}
	// functions of a quaternion
	float Dot (const Quaternion& rkQ) const;
	/* Returns the normal length of this quaternion.
		@note This does <b>not</b> alter any values.
	*/
	float Norm () const;
	float normalise(void); 
	Quaternion Inverse () const;  
	Quaternion UnitInverse () const;  
	Quaternion Exp () const;
	Quaternion Log () const;
   
	Vector3 operator* (const Vector3& rkVector) const;
   
	float getRoll(bool reprojectAxis = true) const;
	float getPitch(bool reprojectAxis = true) const;
	float getYaw(bool reprojectAxis = true) const;     
           
	bool equals(const Quaternion& rhs, const float& toleranceInRadians) const;
           
	inline bool orientationEquals( const Quaternion& other, float tolerance = 1e-3 ) const
	{
		float d = this->Dot(other);
		return 1 - d*d < tolerance;
	}
           
	static Quaternion Slerp (float fT, const Quaternion& rkP,
		const Quaternion& rkQ, bool shortestPath = false);
   
	static Quaternion SlerpExtraSpins (float fT,
		const Quaternion& rkP, const Quaternion& rkQ,
		int iExtraSpins);
   
	static void Intermediate (const Quaternion& rkQ0,
		const Quaternion& rkQ1, const Quaternion& rkQ2,
		Quaternion& rka, Quaternion& rkB);
   
	static Quaternion Squad (float fT, const Quaternion& rkP,
		const Quaternion& rkA, const Quaternion& rkB,
		const Quaternion& rkQ, bool shortestPath = false);
   
	static Quaternion nlerp(float fT, const Quaternion& rkP, 
		const Quaternion& rkQ, bool shortestPath = false);
   
	static const float msEpsilon;
   
	// special values
	static const Quaternion ZERO;
	static const Quaternion IDENTITY;
   
	float w, x, y, z;
	/*
	inline bool isNaN() const
	{
		return Math::isNaN(x) || Math::isNaN(y) || Math::isNaN(z) || Math::isNaN(w);
	}*/
	
	inline friend std::ostream& operator <<
		( std::ostream& o, const Quaternion& q )
	{
		o << "Quaternion(" << q.w << ", " << q.x << ", " << q.y << ", " << q.z << ")";
		return o;
	}

	inline XMFLOAT4 intoXMFLOAT4() const
	{
		return XMFLOAT4(x,y,z,w);
	}
};