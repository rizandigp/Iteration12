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
	void Update();

	void SetPosition( const Vector3& pos );
	void SetPosition( const XMFLOAT3& pos );
	void SetPosition( float x, float y, float z );
	void SetOrientation( const Quaternion& q );
	void SetOrientation( const XMFLOAT4& quaternion );
	void SetOrientation( float x, float y, float z, float w );
	void SetScale( const Vector3& scale );
	void SetScale( const XMFLOAT3& scale );
	void SetScale( float x, float y, float z );
	void Translate( const Vector3& xyz );
	void Translate( const XMFLOAT3& xyz );
	void Translate( float x, float y, float z );
	void Scale( const Vector3& xyz );
	void Scale( const XMFLOAT3& xyz );
	void Scale( float x, float y, float z );
	void Rotate( const Quaternion& q );
	
	void LookAt( const XMFLOAT3& target, float roll );
	void LookAt( const Vector3& target, float roll );
	void OrientToVector( XMFLOAT3 directionVector, float roll );

	inline const Vector3& GetPosition() const			{ return m_Position; };
	inline const Quaternion& GetOrientation() const		{ return m_Orientation; };
	inline const Vector3& GetScale() const				{ return m_Scale; };

	inline const Matrix4x4& GetMatrix() const			{ return m_Matrix; };
	XMMATRIX GetXMMatrix() const;
	
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