#pragma once

#include "Prerequisites.h"

#include <xnamath.h>
#include "Math.h"
#include "Vector3.h"
#include "Quaternion.h"
#include "Matrix4x4.h"

// 3D transformation. Position, orientation, and scale.
class Transform
{
public:
	Transform();

	// Updates the transformation matrix
	void update();

	void setPosition( const Vector3& pos );
	void setPosition( const XMFLOAT3& pos );
	void setPosition( float x, float y, float z );
	void setOrientation( const Quaternion& q );
	void setOrientation( const XMFLOAT4& quaternion );
	void setOrientation( float x, float y, float z, float w );
	void setScale( const Vector3& scale );
	void setScale( const XMFLOAT3& scale );
	void setScale( float x, float y, float z );
	void translate( const Vector3& xyz );
	void translate( const XMFLOAT3& xyz );
	void translate( float x, float y, float z );
	void scale( const Vector3& xyz );
	void scale( const XMFLOAT3& xyz );
	void scale( float x, float y, float z );
	void rotate( const Quaternion& q );
	
	void lookAt( const XMFLOAT3& target, float roll );
	void lookAt( const Vector3& target, float roll );
	void orientToVector( XMFLOAT3 directionVector, float roll );

	inline const Vector3& getPosition() const			{ return m_Position; };
	inline const Quaternion& getOrientation() const		{ return m_Orientation; };
	inline const Vector3& getScale() const				{ return m_Scale; };

	inline const Matrix4x4& getMatrix() const			{ return m_Matrix; };
	XMMATRIX getXMMatrix() const;
	
	Vector3			m_Position;
	Quaternion		m_Orientation;
	Vector3			m_Scale;
	Matrix4x4		m_Matrix;
};

//////////////////////
// Lesson learned:
// beware of alignment issues!!!
// _DECLSPEC_ALIGN_16_ <---- evil
//////////////////////