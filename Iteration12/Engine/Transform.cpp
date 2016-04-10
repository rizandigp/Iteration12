#pragma once
#include "Transform.h"

Transform::Transform()
{
	m_Position = Vector3( 0.0f, 0.0f, 0.0f );
	m_Orientation = Quaternion( 1.0f, 0.0f, 0.0f, 0.0f );
	m_Scale = Vector3( 1.0f, 1.0f, 1.0f );
	m_Matrix = Matrix4x4::IDENTITY;
}

void Transform::update()
{
	/*
	XMFLOAT4X4 mat;
	XMStoreFloat4x4( &mat, XMMatrixAffineTransformation( XMLoadFloat3(&m_Scale.intoXMFLOAT3()), XMVectorZero(), XMLoadFloat4(&m_Orientation.intoXMFLOAT4()), XMLoadFloat3(&m_Position.intoXMFLOAT3()) ));
	m_Matrix = Matrix4x4(mat._11,mat._12,mat._13,mat._14,mat._21,mat._22,mat._23,mat._24,mat._31,mat._32,mat._33,mat._34,mat._41,mat._42,mat._43,mat._44);
	*/
	
	m_Matrix.makeTransform( m_Position, m_Scale, m_Orientation );
	m_Matrix = m_Matrix.transpose();
}

void Transform::setPosition( const Vector3& pos )
{
	m_Position = pos;
}

void Transform::setPosition( const XMFLOAT3& pos )
{
	m_Position = Vector3( pos.x, pos.y, pos.z );
}

void Transform::setPosition( float x, float y, float z )
{
	m_Position = Vector3( x, y, z );
}
void Transform::setOrientation( const Quaternion& q )
{
	m_Orientation = q;
}

void Transform::setOrientation( const XMFLOAT4& quaternion )
{
	m_Orientation.x = quaternion.y;
	m_Orientation.y = quaternion.z;
	m_Orientation.z = quaternion.w;
	m_Orientation.w = quaternion.x;
}

void Transform::setOrientation( float w, float x, float y, float z )
{
	m_Orientation = Quaternion( w, x, y, z );
}

void Transform::setScale( const XMFLOAT3& scale )
{
	m_Scale.x = scale.x;
	m_Scale.y = scale.y;
	m_Scale.z = scale.z;
}

void Transform::setScale( float x, float y, float z )
{
	m_Scale.x = x;
	m_Scale.y = y;
	m_Scale.z = z;
}

void Transform::rotate( const Quaternion& q )
{
	m_Orientation = m_Orientation*q;
}

void Transform::translate( const Vector3& xyz )
{
	m_Position += xyz;
}

void Transform::translate( const XMFLOAT3& xyz )
{
	m_Position.x += xyz.x;
	m_Position.y += xyz.y;
	m_Position.z += xyz.z;
}

void Transform::translate( float x, float y, float z )
{
	m_Position.x += x;
	m_Position.y += y;
	m_Position.z += z;
}

void Transform::scale( const Vector3& xyz )
{
	m_Scale *= xyz;
}

void Transform::scale( const XMFLOAT3& xyz )
{
	m_Scale.x *= xyz.x;
	m_Scale.y *= xyz.y;
	m_Scale.z *= xyz.z;
}

void Transform::scale( float x, float y, float z )
{
	m_Scale.x *= x;
	m_Scale.y *= y;
	m_Scale.z *= z;
}

void Transform::lookAt( const XMFLOAT3& target, float roll )
{
	Vector3 v(target.x,target.y,target.z);
	lookAt( v, roll );
}

void Transform::lookAt( const Vector3& target, float roll )
{
	//Vector3 v(target.x - m_Position.x, target.y - m_Position.y, target.z - m_Position.z);
	Vector3 v = target-m_Position;
	v.normalise();
	Quaternion q;
	q.lookAt( v, Vector3::UNIT_Z );
	//q = position.getRotationTo( target );
	m_Orientation = q;
}

void Transform::orientToVector( XMFLOAT3 directionVector, float roll )
{
	m_Orientation.lookAt( Vector3(directionVector.x,directionVector.y,directionVector.z), Vector3::UNIT_Z );
}

XMMATRIX Transform::getXMMatrix() const
{
	return XMLoadFloat4x4(reinterpret_cast<const XMFLOAT4X4*>(&m_Matrix));
}
