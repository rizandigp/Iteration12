/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2013 Torus Knot Software Ltd

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
-----------------------------------------------------------------------------
*/

// This file is based on material originally from:
// Geometric Tools, LLC
// Copyright (c) 1998-2010
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt

#pragma once

#include "Prerequisites.h"

#include "Vector3.h"
#include "Ray.h"


/** Defines a plane in 3D space.
    @remarks
        A plane is defined in 3D space by the equation
        Ax + By + Cz + D = 0
    @par
        This equates to a vector (the normal of the plane, whose x, y
        and z components equate to the coefficients A, B and C
        respectively), and a constant (D) which represents the offset of
        the plane from the origin along this normal.
    */
class Plane
{
public:
    /** Default constructor - sets everything to 0.
    */
    Plane ();
    Plane (const Plane& rhs);
    Plane (const Vector3& rkNormal, float fConstant);
    Plane (const Vector3& rkNormal, const Vector3& rkPoint);
    Plane (const Vector3& rkPoint0, const Vector3& rkPoint1,
        const Vector3& rkPoint2);

    /** The "positive side" of the plane is the half space to which the
        plane normal points. The "negative side" is the other half
        space. The flag "no side" indicates the plane itself.
    */
    enum Side
    {
        NO_SIDE,
        POSITIVE_SIDE,
        NEGATIVE_SIDE,
		BOTH_SIDE
    };

    Side getSide (const Vector3& rkPoint) const;
	//Side getSide (const AxisAlignedBox& rkBox) const;
	Side getSide (const Vector3& centre, const Vector3& halfSize) const;

    /** This is a pseudodistance. The sign of the return value is
        positive if the point is on the positive side of the plane,
        negative if the point is on the negative side, and zero if the
        point is on the plane.
        @par
        The absolute value of the return value is the true distance only
        when the plane normal is a unit length vector.
    */
    float getDistance (const Vector3& rkPoint) const;

	void redefine(const Vector3& rkPoint0, const Vector3& rkPoint1,
		const Vector3& rkPoint2);
 
	void redefine(const Vector3& rkNormal, const Vector3& rkPoint);

	Vector3 projectVector(const Vector3& v) const;
	float normalise(void);
 
	std::pair<bool, float> intersects(const Ray& ray, const Plane& plane)
    {
        float denom = plane.normal.dotProduct(ray.Direction);
        if (abs(denom) < std::numeric_limits<float>::epsilon())
        {
            // Parallel
            return std::pair<bool, float>(false, 0.0f);
        }
        else
        {
            float nom = plane.normal.dotProduct(ray.Origin) + plane.d;
            float t = -(nom/denom);
            return std::pair<bool, float>(t >= 0.0f, t);
        }
    }

	std::pair<bool, float> rayIntersects(const Ray& ray )
    {
        return intersects(ray, *this);
    }

	Vector3 normal;
    float d;

	bool operator==(const Plane& rhs) const
	{
		return (rhs.d == d && rhs.normal == normal);
	}
	bool operator!=(const Plane& rhs) const
	{
		return (rhs.d != d || rhs.normal != normal);
	}

    friend std::ostream& operator<< (std::ostream& o, Plane& p);
};
