#pragma once

#include "Entity.h"
#include "Scene.h"

MyEntity::MyEntity() : 	Entity()
{
}

MyEntity::MyEntity( const MyEntity& other ) : Entity( other )
{
	setRenderSystem( other.m_pRenderSystem );
	setMesh( other.m_pMesh );
}

void MyEntity::setRenderSystem( RenderSystem* ptr )
{
	Entity::setRenderSystem( ptr );
	m_Renderer.setRenderSystem( ptr );
}

void MyEntity::setMesh( Mesh* ptr )
{
	m_pMesh = ptr;
	m_Renderer.setMesh( ptr );
};

void MyEntity::update(float deltaTime)
{
	m_Transform.update();
}

void MyEntity::clearLights()
{
	m_Renderer.clearLights();
}

void MyEntity::render()
{
	m_Renderer.render( &m_Transform );
}

void MyEntity::renderShadowmap( Camera3D* pShadowCamera )
{
	m_Renderer.renderShadowmap( &m_Transform, pShadowCamera );
}

void MyEntity::renderBoundingBox()
{
	m_Renderer.renderOBB( &m_Transform );
}

void MyEntity::cull( XNA::Frustum* frustum )
{
	m_Renderer.cull( frustum, &m_Transform );
}

void MyEntity::cullLight( PointLight* light )
{
	m_Renderer.cullLight( light, &m_Transform );
}

void MyEntity::cullLight( SpotLight* light )
{
	m_Renderer.cullLight( light, &m_Transform );
}

void MyEntity::setCulled( bool culled )
{
	m_Renderer.setCulled( culled );
}

PlayerEntity::PlayerEntity(RenderSystem* ptr)
{
	setRenderSystem( ptr );

	m_DirEuler = XMFLOAT3( 0.0f , 0.0f , -XM_PIDIV2 );
	m_bMouseDragged = false;

	m_pSpotlight = new SpotLight();
	m_pSpotlight->setRadius( 40.0f );
	m_pSpotlight->setIntensity( 10.0f );
	m_pSpotlight->setColor( Vector3(1.0f,1.0f,0.8f) );
	m_pSpotlight->setCone( Vector2( XM_PI/4.0f, XM_PI/4.0f ) );
	m_pSpotlight->setCookie( m_pRenderSystem->loadTexture( L"Media/spotlight.bmp" ) );
	m_pSpotlight->transform()->setPosition( 0.0f, 0.0f, 0.0f );
	m_pSpotlight->pointTo( Vector3( 0.0f, 0.0f, 25.0f ), 0.0f );
	m_pSpotlight->disable();

	m_pPointLight = new PointLight();
	m_pPointLight->setRadius( 10000.0f );
	m_pPointLight->setIntensity( 0.03f );
	m_pPointLight->setColor( Vector3(1.0f,1.0f,1.0f) );
	m_pPointLight->transform()->setPosition( 0.0f, 0.0f, 0.0f );
	//m_pPointLight->disable();

	UINT width, height;
	RenderSystemConfig config = ptr->getConfig();
	width = config.Width;
	height = config.Height;

	m_pCamera = new Camera3D();
	m_pCamera->setPosition( transform()->getPosition() );
	m_pCamera->setLookAt( Vector3( 0.0f, 1.0f, 0.0f ) );
	m_pCamera->setUpVector( Vector3( 0.0f, 0.0f, 1.0f ) );
	m_pCamera->setProjection( XM_PI*0.35f, width / (float)height, 0.1f, 5000.0f );
	m_pCamera->update( 0.0f );
}

void PlayerEntity::onAddToScene( Scene* ptr )
{
	ptr->addSpotLight( m_pSpotlight );
	//ptr->addPointLight( m_pPointLight );
	ptr->setCamera( m_pCamera );
	Entity::onAddToScene(ptr);
}

void PlayerEntity::update(float deltaTime)
{
	POINT ptMousePos;
	XMFLOAT2 ptMousePosDelta;

	GetCursorPos( &ptMousePos );

	if ( KEYDOWN( VK_LBUTTON ) && ( m_bMouseDragged == false ))
		{
		m_bMouseDragged = true;
		m_ptLastMousePos = ptMousePos; 
		}

	if ( KEYDOWN( VK_LBUTTON ) )
		{
		m_bMouseDragged = true;

		ptMousePosDelta.x = float(m_ptLastMousePos.x - ptMousePos.x);		// Yaw
		ptMousePosDelta.y = float(m_ptLastMousePos.y - ptMousePos.y);		// Pitch
		ptMousePosDelta.x /= 200.0f;
		ptMousePosDelta.y /= 200.0f;

		m_DirEuler.z += ptMousePosDelta.x;
		m_DirEuler.y += ptMousePosDelta.y;

		SetCursorPos( m_ptLastMousePos.x , m_ptLastMousePos.y );
		}
	else
		m_bMouseDragged = false;


	XMFLOAT3 vCamMovement( 0.0f , 0.0f , 0.0f );

	if ( KEYDOWN( 'A' ) )
		vCamMovement.x += 3.0f*deltaTime;
	if ( KEYDOWN( 'D' ) )
		vCamMovement.x -= 3.0f*deltaTime;
	if ( KEYDOWN( 'W' ) )
		vCamMovement.y += 3.0f*deltaTime;
	if ( KEYDOWN( 'S' ) )
		vCamMovement.y -= 3.0f*deltaTime;

	Vector3 pos, relPos, lookat;
	relPos.x = vCamMovement.y*cos(m_DirEuler.z)*cos(m_DirEuler.y);			//
	relPos.y = vCamMovement.y*sin(m_DirEuler.z)*cos(m_DirEuler.y);			// Maju-mundur
	relPos.z = vCamMovement.y*sin(m_DirEuler.y);								//

	relPos.x += vCamMovement.x*sin(-m_DirEuler.z);						// Kiri-Kanan
	relPos.y += vCamMovement.x*cos(-m_DirEuler.z);

	transform()->translate( relPos );
	pos = transform()->getPosition();
	//pos.z = 1.6f;
	transform()->setPosition( pos );

	lookat.x = pos.x + cos(m_DirEuler.z)*cos(m_DirEuler.y);
	lookat.y = pos.y + sin(m_DirEuler.z)*cos(m_DirEuler.y);
	lookat.z = pos.z + sin(m_DirEuler.y);

	m_pCamera->setPosition( transform()->getPosition() );
	m_pCamera->setLookAt( lookat );
	m_pCamera->update( deltaTime );

	Vector3 flashlightpos, flashlightlookat;
	flashlightpos.x = pos.x + 0.35f*cosf(m_DirEuler.z-XM_PIDIV2);
	flashlightpos.y = pos.y + 0.35f*sinf(m_DirEuler.z-XM_PIDIV2);
	//flashlightpos.z = 1.45f;
	flashlightpos.z = pos.z;
	m_pSpotlight->transform()->setPosition(flashlightpos);
	flashlightlookat.x = pos.x + 25.0f*cosf(m_DirEuler.z)*cosf(m_DirEuler.y);
	flashlightlookat.y = pos.y + 25.0f*sinf(m_DirEuler.z)*cosf(m_DirEuler.y);
	flashlightlookat.z = pos.z + 25.0f*sinf(m_DirEuler.y);
	m_pSpotlight->pointTo( flashlightlookat, 0.0f );
	m_pSpotlight->update();

	m_pPointLight->transform()->setPosition( pos );

	if ( KEYDOWN( 'F' ) )
		m_pSpotlight->setEnabled(!m_pSpotlight->isEnabled());
}