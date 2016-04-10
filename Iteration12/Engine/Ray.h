#pragma once

#include "Vector3.h"

class Matrix4x4;

class Ray
{
public:
	Ray()	{};
	Ray(const Vector3 &origin, const Vector3 &direction) : 
		Origin(origin),
		Direction(direction) {}

	// Returns point along ray at @distance from origin
	inline Vector3 getPoint(float distance) const
	{
		return Origin + Direction*distance;
	}

	// Returns the distance of the point along the ray that is nearest to a given test @point
	inline float getNearestToPoint(const Vector3& point) const
	{
		Vector3 temp = point - Origin;
		return Direction.dotProduct(temp);
	}

	// Returns this ray transformed by @mat
	Ray transformedCopy( const Matrix4x4& mat ) const;

	// Transform by @mat
	void transform( const Matrix4x4& mat );

public:
	Vector3 Origin;
	Vector3 Direction;
};