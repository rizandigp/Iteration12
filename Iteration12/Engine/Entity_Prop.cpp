#pragma once

#include "Entity_Prop.h"

Entity_KursiRoda::Entity_KursiRoda(RenderSystem* ptr) : Entity_StaticProp( ptr, NULL )
{
	//DX11Material_DiffuseDetailbump* pMaterial = new DX11Material_DiffuseDetailbump(ptr);
	DX11Material_Deferred* pMaterial = new DX11Material_Deferred(ptr);
	pMaterial->setDiffusemap( ptr->loadTexture( L"Media/props34_san_kol.2048.crn.bmp" ) );
	pMaterial->setNormalmap( ptr->loadTexture( L"Media/props34_san_kol_normal.2048.crn.bmp" ) );
	//pMaterial->setDetailNormalmap( ptr->loadTexture( L"Media/Tex_0010_5.dds" ) );
	//pMaterial->setDetailNormalStrength( 3.0f );
	//pMaterial->setDetailTiling( 12.0f );
	pMaterial->setSpecularIntensity( 0.22f );
	//pMaterial->setSpecularPower( 12.0f );

	Mesh* pMesh = ptr->loadMesh( "Media/metro_kursiroda.3DS" );
	pMesh->setMaterial( pMaterial );

	setRenderSystem( ptr );
	setMesh( pMesh );
}

Entity_Kasur::Entity_Kasur(RenderSystem* ptr) : Entity_StaticProp( ptr, NULL )
{
	//DX11Material_DiffuseDetailbump* mat0 = new DX11Material_DiffuseDetailbump(ptr);
	DX11Material_Deferred* mat0 = new DX11Material_Deferred(ptr);
	mat0->setDiffusemap( ptr->loadTexture( L"Media/metal34_props_01.1024.dds" ) );
	mat0->setNormalmap( ptr->loadTexture( L"Media/metal34_props_01_normal.2048.bmp" ) );
	//mat0->setDetailNormalmap( ptr->loadTexture( L"Media/Tex_0010_5.dds" ) );
	//mat0->setDetailNormalStrength( 5.0f );
	//mat0->setDetailTiling( 12.0f );
	mat0->setSpecularIntensity( 0.3f );
	//mat0->setSpecularPower( 32.0f );

	//DX11Material_DiffuseDetailbump* mat1 = new DX11Material_DiffuseDetailbump(ptr);
	DX11Material_Deferred* mat1 = new DX11Material_Deferred(ptr);
	mat1->setDiffusemap( ptr->loadTexture( L"Media/props34_san_kol.2048.crn.bmp" ) );
	mat1->setNormalmap( ptr->loadTexture( L"Media/props34_san_kol_normal.2048.crn.bmp" ) );
	//mat1->setDetailNormalmap( ptr->loadTexture( L"Media/Tex_0010_5.dds" ) );
	//mat1->setDetailNormalStrength( 5.0f );
	//mat1->setDetailTiling( 12.0f );
	mat1->setSpecularIntensity( 0.3f );
	//mat1->setSpecularPower( 32.0f );

	//DX11Material_DiffuseDetailbump* mat2 = new DX11Material_DiffuseDetailbump(ptr);
	DX11Material_Deferred* mat2 = new DX11Material_Deferred(ptr);
	mat2->setDiffusemap( ptr->loadTexture( L"Media/props_mattress.512.dds" ) );
	mat2->setNormalmap( ptr->loadTexture( L"Media/props_mattress_normal.512.bmp" ) );
	//mat2->setDetailNormalmap( ptr->loadTexture( L"Media/Tex_0010_5.dds" ) );
	//mat2->setDetailNormalStrength( 2.0f );
	//mat2->setDetailTiling( 8.0f );
	mat2->setSpecularIntensity( 0.15f );
	//mat2->setSpecularPower( 16.0f );

	//DX11Material_DiffuseDetailbump* mat3 = new DX11Material_DiffuseDetailbump(ptr);
	DX11Material_Deferred* mat3 = new DX11Material_Deferred(ptr);
	mat3->setDiffusemap( ptr->loadTexture( L"Media/props_fabric.2048.dds" ) );
	mat3->setNormalmap( ptr->loadTexture( L"Media/props_fabric_normal.2048.bmp" ) );
	//mat3->setDetailNormalmap( ptr->loadTexture( L"Media/Tex_0010_5.dds" ) );
	//mat3->setDetailNormalStrength( 1.0f );
	//mat3->setDetailTiling( 12.0f );
	mat3->setSpecularIntensity( 0.15f );
	//mat3->setSpecularPower( 16.0f );

	Mesh* pMesh = ptr->loadMesh( "Media/metro_kasur.obj" );
	pMesh->setMaterial( mat2 );
	pMesh->getSubmesh(2)->setMaterial( mat3 );
	pMesh->getSubmesh(3)->setMaterial( mat0 );

	setRenderSystem( ptr );
	setMesh( pMesh );

}

Entity_Tumba01::Entity_Tumba01(RenderSystem* ptr) : Entity_StaticProp( ptr, NULL )
{
	DX11Material_DiffuseDetailbump* mat0 = new DX11Material_DiffuseDetailbump(ptr);
	mat0->setDiffusemap( ptr->loadTexture( L"Media/props_shkaf_01.1024.bmp" ) );
	mat0->setNormalmap( ptr->loadTexture( L"Media/props_shkaf_01_normal.1024.bmp" ) );
	mat0->setDetailNormalmap( ptr->loadTexture( L"Media/Tex_0010_5.dds" ) );
	mat0->setDetailNormalStrength( 2.0f );
	mat0->setDetailTiling( 10.0f );
	mat0->setSpecularIntensity( 0.34f );
	mat0->setSpecularPower( 32.0f );

	DX11Material_DiffuseDetailbump* mat1 = new DX11Material_DiffuseDetailbump(ptr);
	mat1->setDiffusemap( ptr->loadTexture( L"Media/props_shkaf_01.1024.bmp" ) );
	mat1->setNormalmap( ptr->loadTexture( L"Media/props_shkaf_01_normal.1024.bmp" ) );
	mat1->setDetailNormalmap( ptr->loadTexture( L"Media/Tex_0010_5.dds" ) );
	mat1->setDetailNormalStrength( 2.0f );
	mat1->setDetailTiling( 10.0f );
	mat1->setSpecularIntensity( 0.34f );
	mat1->setSpecularPower( 32.0f );

	Mesh* pMesh = ptr->loadMesh( "Media/metro_tumba01.obj" );
	pMesh->setMaterial( mat0 );
	pMesh->getSubmesh(1)->setMaterial( mat1 );

	MyEntity::MyEntity();
	setRenderSystem( ptr );
	setMesh( pMesh );
}

Entity_Plane::Entity_Plane(RenderSystem* ptr, Material* mat, XMFLOAT2 dimensions, XMFLOAT2 uvscale ) : Entity_StaticProp( ptr, NULL )
{
	Mesh* pMesh = ptr->createPlaneMesh( dimensions, uvscale );
	pMesh->setMaterial(mat);

	MyEntity::MyEntity();
	setRenderSystem( ptr );
	setMesh( pMesh );
}

Entity_StaticProp::Entity_StaticProp(RenderSystem* ptr, Mesh* pmesh)
{
	setRenderSystem(ptr);
	setMesh(pmesh);
}
/*
MyEntity::MyEntity( const MyEntity& other ) : Entity( other )
{
	setRenderSystem( other.m_pRenderSystem );
	setMesh( other.m_pMesh );
}*/

void Entity_StaticProp::setRenderSystem( RenderSystem* ptr )
{
	Entity::setRenderSystem( ptr );
	m_Renderer.setRenderSystem( ptr );
}

void Entity_StaticProp::setMesh( Mesh* ptr )
{
	m_pMesh = ptr;
	m_Renderer.setMesh( ptr );
};

void Entity_StaticProp::update(float deltaTime)
{
	m_Transform.update();
}

void Entity_StaticProp::clearLights()
{
	m_Renderer.clearLights();
}

void Entity_StaticProp::render()
{
	m_Renderer.render( &m_Transform );
}

void Entity_StaticProp::renderShadowmap( Camera3D* pShadowCamera )
{
	m_Renderer.renderShadowmap( &m_Transform, pShadowCamera );
}

void Entity_StaticProp::renderRSM( Camera3D* pShadowCamera, SpotLight* pLightSource )
{
	m_Renderer.renderRSM( &m_Transform, pShadowCamera, pLightSource );
}

void Entity_StaticProp::renderBoundingBox()
{
	m_Renderer.renderOBB( &m_Transform );
}

void Entity_StaticProp::cull( XNA::Frustum* frustum )
{
	m_Renderer.cull( frustum, &m_Transform );
}

void Entity_StaticProp::cullLight( PointLight* light )
{
	m_Renderer.cullLight( light, &m_Transform );
}

void Entity_StaticProp::cullLight( SpotLight* light )
{
	m_Renderer.cullLight( light, &m_Transform );
}


Entity_Prop::Entity_Prop(RenderSystem* pRender, PhysicsSystem* pPhysics, Mesh* pMesh, hkpRigidBody* pRigidBody, Transform rigidBodyRelative) : Entity_StaticProp(pRender, pMesh)
{
	setPhysicsSystem(pPhysics);
	m_pRigidBody = pRigidBody;
	rigidBodyRelative.setPosition( -rigidBodyRelative.getPosition() );
	rigidBodyRelative.setOrientation( rigidBodyRelative.getOrientation().Inverse() );
	m_MeshRelativeTransform = rigidBodyRelative;
}

Entity_Prop::Entity_Prop(Entity_Prop& other) : Entity_StaticProp( other.getRenderSystem(), other.getMesh() )	// "no default constructor" blaaaaaah
{
	hkpRigidBody* rigidBody;
	if (other.m_pRigidBody)
	{
		rigidBody = other.m_pRigidBody->clone();
		/*hkpRigidBodyCinfo info;
		other.m_pRigidBody->getCinfo(info);
		hkpShape* cloneShape = new hkpShape(*info.m_shape);
		//info.m_shape = cloneShape;
		rigidBody = new hkpRigidBody(info);
		cloneShape->removeReference();*/
	}
	else
		rigidBody = NULL;

	setPhysicsSystem(other.getPhysicsSystem());
	m_pRigidBody = rigidBody;
	m_MeshRelativeTransform = other.m_MeshRelativeTransform;
}

void Entity_Prop::onAddToScene( Scene* ptr )
{
	//ptr->getPhysicsSystem()->getWorld()->addEntity( m_pRigidBody );
	if (m_pRigidBody)
	{
		m_pPhysicsSystem->getWorld()->lock();
		m_pPhysicsSystem->getWorld()->addEntity( m_pRigidBody )->removeReference();
		m_pPhysicsSystem->getWorld()->unlock();
	}
	Entity::onAddToScene(ptr);
}

void Entity_Prop::update( float deltaTime )
{
	if (m_pRigidBody)
	{
		// Update transform
		hkTransform hktransform = m_pRigidBody->getTransform();
		hkVector4 pos = hktransform.getTranslation();
		hkQuaternion rot = hkQuaternion(hktransform.getRotation());	// Get quaternion from rotation matrix
		Quaternion meshCenterRot(rot(3), rot(0), rot(1), rot(2));
		Quaternion meshRot = meshCenterRot*m_MeshRelativeTransform.getOrientation();
		meshRot.normalise();
		Vector3 meshPos(pos.getComponent(0), pos.getComponent(1), pos.getComponent(2));
		meshPos += meshCenterRot*m_MeshRelativeTransform.getPosition();
		transform()->setPosition( meshPos );
		transform()->setOrientation( meshRot );
	}

	Entity_StaticProp::update( deltaTime );
};