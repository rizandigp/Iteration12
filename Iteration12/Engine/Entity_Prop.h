#pragma once

#include "Prerequisites.h"

#include "Entity.h"
#include "PhysicsSystem.h"
#include "Scene.h"



// Static "decorative" prop class with a rendered mesh but does nothing
class Entity_StaticProp : public Entity
{
public:
	Entity_StaticProp(RenderSystem* ptr, Mesh* pmesh);
	void setRenderSystem( RenderSystem* ptr );

	void setMesh( Mesh* ptr );
	Mesh* getMesh()				{ return m_pMesh; };

	void update( float deltaTime );
	void clearLights();
	void cull( XNA::Frustum* frustum );
	void cullLight( PointLight* light );
	void cullLight( SpotLight* light );
	void render();
	void renderShadowmap( Camera3D* pShadowCamera );
	void renderRSM( Camera3D* pShadowCamera, SpotLight* pLightSource );
	void renderBoundingBox();

protected:
	Mesh* m_pMesh;
	DX11Renderer m_Renderer;
};

class Entity_KursiRoda : public Entity_StaticProp
{
public:
	Entity_KursiRoda(RenderSystem* ptr);
};

class Entity_Kasur : public Entity_StaticProp
{
public:
	Entity_Kasur(RenderSystem* ptr);
};

class Entity_Tumba01 : public Entity_StaticProp
{
public:
	Entity_Tumba01(RenderSystem* ptr);
};

class Entity_Plane : public Entity_StaticProp
{
public:
	Entity_Plane(RenderSystem* ptr, Material* mat, XMFLOAT2 dimensions, XMFLOAT2 uvscale );
};

class Entity_PolyMesh : public Entity_StaticProp
{
public:
	Entity_PolyMesh(RenderSystem* ptr);
};

/*	Basic prop with a renderable mesh and physical rigid body.
	User provides a hkpRigidBody instance (can be either dynamic or static, can be NULL).
	Transform is tied to the rigid body, so calling transform() to change position/orientation/scale
	does not work, use m_pRigidBody->setXXX() instead.
	@param
		rigidBodyRelative - defines the relative transformation/offset of the rigid body to the mesh in local space.
		Defaults to identity.
*/
class Entity_Prop : public Entity_StaticProp
{
public:
	Entity_Prop(RenderSystem* pRender, PhysicsSystem* pPhysics, Mesh* pMesh, hkpRigidBody* pRigidBody, Transform rigidBodyRelative = Transform());
	Entity_Prop(Entity_Prop& other);
	virtual void setPhysicsSystem( PhysicsSystem* ptr )		{ m_pPhysicsSystem = ptr; };
	PhysicsSystem* getPhysicsSystem()						{ return m_pPhysicsSystem; };

	void onAddToScene( Scene* ptr );
	void update( float deltaTime );

public:
	hkpRigidBody* m_pRigidBody;

protected:
	PhysicsSystem* m_pPhysicsSystem;
	Transform m_MeshRelativeTransform;
};