#pragma once

#include "Ray.h"
#include "Vector4.h"
#include "Matrix4x4.h"

Ray Ray::transformedCopy( const Matrix4x4& mat ) const
{
	Vector4 transformedOrigin = Vector4(Origin)*mat;
	Vector4 transformedDirection = Vector4(Direction,0.0f)*mat;
	
	return Ray( Vector3(transformedOrigin.x, transformedOrigin.y, transformedOrigin.z), 
				Vector3(transformedDirection.x, transformedDirection.y, transformedDirection.z).normalisedCopy() );
}

void Ray::transform( const Matrix4x4& mat )
{
	(*this) = transformedCopy( mat );
}