#pragma once

#include "Entity_Prop.h"

Entity_KursiRoda::Entity_KursiRoda(RenderSystem* ptr) : Entity_StaticProp( ptr, NULL )
{
	//DX11Material_DiffuseDetailbump* pMaterial = new DX11Material_DiffuseDetailbump(ptr);
	Material_Deferred* pMaterial = Material_Deferred::Create( ptr );
	pMaterial->SetDiffusemap( ptr->LoadTexture2D( L"Media/props34_san_kol.2048.crn.bmp" ) );
	pMaterial->SetNormalmap( ptr->LoadTexture2D( L"Media/props34_san_kol_normal.2048.crn.bmp" ) );
	//pMaterial->SetDetailNormalmap( ptr->LoadTexture2D( L"Media/Tex_0010_5.dds" ) );
	//pMaterial->SetDetailNormalStrength( 3.0f );
	//pMaterial->SetDetailTiling( 12.0f );
	pMaterial->SetSpecularIntensity( 0.22f );
	//pMaterial->SetSpecularPower( 12.0f );

	Mesh* pMesh = ptr->LoadMesh( "Media/metro_kursiroda.3DS" );
	pMesh->SetMaterial( pMaterial );

	SetRenderSystem( ptr );
	SetMesh( pMesh );
}

Entity_Kasur::Entity_Kasur(RenderSystem* ptr) : Entity_StaticProp( ptr, NULL )
{
	//DX11Material_DiffuseDetailbump* mat0 = new DX11Material_DiffuseDetailbump(ptr);
	Material_Deferred* mat0 = Material_Deferred::Create(ptr);
	mat0->SetDiffusemap( ptr->LoadTexture2D( L"Media/metal34_props_01.1024.dds" ) );
	mat0->SetNormalmap( ptr->LoadTexture2D( L"Media/metal34_props_01_normal.2048.bmp" ) );
	//mat0->SetDetailNormalmap( ptr->LoadTexture2D( L"Media/Tex_0010_5.dds" ) );
	//mat0->SetDetailNormalStrength( 5.0f );
	//mat0->SetDetailTiling( 12.0f );
	mat0->SetSpecularIntensity( 0.3f );
	//mat0->SetSpecularPower( 32.0f );

	//DX11Material_DiffuseDetailbump* mat1 = new DX11Material_DiffuseDetailbump(ptr);
	Material_Deferred* mat1 = Material_Deferred::Create(ptr);
	mat1->SetDiffusemap( ptr->LoadTexture2D( L"Media/props34_san_kol.2048.crn.bmp" ) );
	mat1->SetNormalmap( ptr->LoadTexture2D( L"Media/props34_san_kol_normal.2048.crn.bmp" ) );
	//mat1->SetDetailNormalmap( ptr->LoadTexture2D( L"Media/Tex_0010_5.dds" ) );
	//mat1->SetDetailNormalStrength( 5.0f );
	//mat1->SetDetailTiling( 12.0f );
	mat1->SetSpecularIntensity( 0.3f );
	//mat1->SetSpecularPower( 32.0f );

	//DX11Material_DiffuseDetailbump* mat2 = new DX11Material_DiffuseDetailbump(ptr);
	Material_Deferred* mat2 = Material_Deferred::Create(ptr);
	mat2->SetDiffusemap( ptr->LoadTexture2D( L"Media/props_mattress.512.dds" ) );
	mat2->SetNormalmap( ptr->LoadTexture2D( L"Media/props_mattress_normal.512.bmp" ) );
	//mat2->SetDetailNormalmap( ptr->LoadTexture2D( L"Media/Tex_0010_5.dds" ) );
	//mat2->SetDetailNormalStrength( 2.0f );
	//mat2->SetDetailTiling( 8.0f );
	mat2->SetSpecularIntensity( 0.15f );
	//mat2->SetSpecularPower( 16.0f );

	//DX11Material_DiffuseDetailbump* mat3 = new DX11Material_DiffuseDetailbump(ptr);
	Material_Deferred* mat3 = Material_Deferred::Create(ptr);
	mat3->SetDiffusemap( ptr->LoadTexture2D( L"Media/props_fabric.2048.dds" ) );
	mat3->SetNormalmap( ptr->LoadTexture2D( L"Media/props_fabric_normal.2048.bmp" ) );
	//mat3->SetDetailNormalmap( ptr->LoadTexture2D( L"Media/Tex_0010_5.dds" ) );
	//mat3->SetDetailNormalStrength( 1.0f );
	//mat3->SetDetailTiling( 12.0f );
	mat3->SetSpecularIntensity( 0.15f );
	//mat3->SetSpecularPower( 16.0f );

	Mesh* pMesh = ptr->LoadMesh( "Media/metro_kasur.obj" );
	pMesh->SetMaterial( mat2 );
	pMesh->GetSubmesh(2)->SetMaterial( mat3 );
	pMesh->GetSubmesh(3)->SetMaterial( mat0 );

	SetRenderSystem( ptr );
	SetMesh( pMesh );

}

Entity_Tumba01::Entity_Tumba01(RenderSystem* ptr) : Entity_StaticProp( ptr, NULL )
{
	Material_DiffuseDetailbump* mat0 = Material_DiffuseDetailbump::Create(ptr);
	mat0->SetDiffusemap( ptr->LoadTexture2D( L"Media/props_shkaf_01.1024.bmp" ) );
	mat0->SetNormalmap( ptr->LoadTexture2D( L"Media/props_shkaf_01_normal.1024.bmp" ) );
	mat0->SetDetailNormalmap( ptr->LoadTexture2D( L"Media/Tex_0010_5.dds" ) );
	mat0->SetDetailNormalStrength( 2.0f );
	mat0->SetDetailTiling( 10.0f );
	mat0->SetSpecularIntensity( 0.34f );
	mat0->SetSpecularPower( 32.0f );

	Material_DiffuseDetailbump* mat1 = Material_DiffuseDetailbump::Create(ptr);
	mat1->SetDiffusemap( ptr->LoadTexture2D( L"Media/props_shkaf_01.1024.bmp" ) );
	mat1->SetNormalmap( ptr->LoadTexture2D( L"Media/props_shkaf_01_normal.1024.bmp" ) );
	mat1->SetDetailNormalmap( ptr->LoadTexture2D( L"Media/Tex_0010_5.dds" ) );
	mat1->SetDetailNormalStrength( 2.0f );
	mat1->SetDetailTiling( 10.0f );
	mat1->SetSpecularIntensity( 0.34f );
	mat1->SetSpecularPower( 32.0f );

	Mesh* pMesh = ptr->LoadMesh( "Media/metro_tumba01.obj" );
	pMesh->SetMaterial( mat0 );
	pMesh->GetSubmesh(1)->SetMaterial( mat1 );

	MyEntity::MyEntity();
	SetRenderSystem( ptr );
	SetMesh( pMesh );
}

Entity_Plane::Entity_Plane(RenderSystem* ptr, Material* mat, XMFLOAT2 dimensions, XMFLOAT2 uvscale ) : Entity_StaticProp( ptr, NULL )
{
	Mesh* pMesh = ptr->CreatePlaneMesh( dimensions, uvscale );
	pMesh->SetMaterial(mat);

	MyEntity::MyEntity();
	SetRenderSystem( ptr );
	SetMesh( pMesh );
}

Entity_StaticProp::Entity_StaticProp(RenderSystem* ptr, Mesh* pmesh)
{
	SetRenderSystem(ptr);
	SetMesh(pmesh);
}
/*
MyEntity::MyEntity( const MyEntity& other ) : Entity( other )
{
	SetRenderSystem( other.m_pRenderSystem );
	setMesh( other.m_pMesh );
}*/

void Entity_StaticProp::SetRenderSystem( RenderSystem* ptr )
{
	Entity::SetRenderSystem( ptr );
	m_Renderer = ptr->CreateRenderer();
	m_Renderer->SetRenderSystem( ptr );
}

void Entity_StaticProp::SetMesh( Mesh* ptr )
{
	m_pMesh = ptr;
	m_Renderer->SetMesh( ptr );
};

void Entity_StaticProp::Update(float deltaTime)
{
	m_Transform.Update();
}

void Entity_StaticProp::ClearLights()
{
	m_Renderer->ClearLights();
}

void Entity_StaticProp::Render()
{
	m_Renderer->Render( &m_Transform );
}

void Entity_StaticProp::RenderShadowmap( Camera3D* pShadowCamera )
{
	m_Renderer->RenderShadowmap( &m_Transform, pShadowCamera );
}

void Entity_StaticProp::RenderRSM( Camera3D* pShadowCamera, SpotLight* pLightSource )
{
	m_Renderer->RenderRSM( &m_Transform, pShadowCamera, pLightSource );
}

void Entity_StaticProp::RenderBoundingBox()
{
	m_Renderer->RenderOBB( &m_Transform );
}

void Entity_StaticProp::Cull( XNA::Frustum* frustum )
{
	m_Renderer->Cull( frustum, &m_Transform );
}

void Entity_StaticProp::CullLight( PointLight* light )
{
	m_Renderer->CullLight( light, &m_Transform );
}

void Entity_StaticProp::CullLight( SpotLight* light )
{
	m_Renderer->CullLight( light, &m_Transform );
}


Entity_Prop::Entity_Prop(RenderSystem* pRender, PhysicsSystem* pPhysics, Mesh* pMesh, hkpRigidBody* pRigidBody, Transform rigidBodyRelative) : Entity_StaticProp(pRender, pMesh)
{
	SetPhysicsSystem(pPhysics);
	m_pRigidBody = pRigidBody;
	rigidBodyRelative.SetPosition( -rigidBodyRelative.GetPosition() );
	rigidBodyRelative.SetOrientation( rigidBodyRelative.GetOrientation().Inverse() );
	m_MeshRelativeTransform = rigidBodyRelative;
}

Entity_Prop::Entity_Prop(Entity_Prop& other) : Entity_StaticProp( other.GetRenderSystem(), other.GetMesh() )	// "no default constructor" blaaaaaah
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

	SetPhysicsSystem(other.GetPhysicsSystem());
	m_pRigidBody = rigidBody;
	m_MeshRelativeTransform = other.m_MeshRelativeTransform;
}

void Entity_Prop::OnAddToScene( Scene* ptr )
{
	//ptr->GetPhysicsSystem()->getWorld()->AddEntity( m_pRigidBody );
	if (m_pRigidBody)
	{
		m_pPhysicsSystem->GetWorld()->lock();
		m_pPhysicsSystem->GetWorld()->addEntity( m_pRigidBody )->removeReference();
		m_pPhysicsSystem->GetWorld()->unlock();
	}
	Entity::OnAddToScene(ptr);
}

void Entity_Prop::Update( float deltaTime )
{
	if (m_pRigidBody)
	{
		// Update transform
		hkTransform hktransform = m_pRigidBody->getTransform();
		hkVector4 pos = hktransform.getTranslation();
		hkQuaternion rot = hkQuaternion(hktransform.getRotation());	// Get quaternion from rotation matrix
		Quaternion meshCenterRot(rot(3), rot(0), rot(1), rot(2));
		Quaternion meshRot = meshCenterRot*m_MeshRelativeTransform.GetOrientation();
		meshRot.normalise();
		Vector3 meshPos(pos.getComponent(0), pos.getComponent(1), pos.getComponent(2));
		meshPos += meshCenterRot*m_MeshRelativeTransform.GetPosition();
		Transformation()->SetPosition( meshPos );
		Transformation()->SetOrientation( meshRot );
	}

	Entity_StaticProp::Update( deltaTime );
};