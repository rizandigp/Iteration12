#pragma once

#include "Entity.h"
#include "Scene.h"

MyEntity::MyEntity() : 	Entity()
{
}

MyEntity::MyEntity( const MyEntity& other ) : Entity( other )
{
	SetRenderSystem( other.m_pRenderSystem );
	SetMesh( other.m_pMesh );
}

void MyEntity::SetRenderSystem( RenderSystem* ptr )
{
	Entity::SetRenderSystem( ptr );
	m_Renderer.SetRenderSystem( ptr );
}

void MyEntity::SetMesh( Mesh* ptr )
{
	m_pMesh = ptr;
	m_Renderer.SetMesh( ptr );
};

void MyEntity::Update(float deltaTime)
{
	m_Transform.Update();
}

void MyEntity::ClearLights()
{
	m_Renderer.ClearLights();
}

void MyEntity::Render()
{
	m_Renderer.Render( &m_Transform );
}

void MyEntity::RenderShadowmap( Camera3D* pShadowCamera )
{
	m_Renderer.RenderShadowmap( &m_Transform, pShadowCamera );
}

void MyEntity::RenderBoundingBox()
{
	m_Renderer.RenderOBB( &m_Transform );
}

void MyEntity::Cull( XNA::Frustum* frustum )
{
	m_Renderer.Cull( frustum, &m_Transform );
}

void MyEntity::CullLight( PointLight* light )
{
	m_Renderer.CullLight( light, &m_Transform );
}

void MyEntity::CullLight( SpotLight* light )
{
	m_Renderer.CullLight( light, &m_Transform );
}

void MyEntity::SetCulled( bool Culled )
{
	m_Renderer.SetCulled( Culled );
}

PlayerEntity::PlayerEntity(RenderSystem* ptr)
{
	SetRenderSystem( ptr );

	m_DirEuler = XMFLOAT3( 0.0f , 0.0f , -XM_PIDIV2 );
	m_bMouseDragged = false;

	m_pSpotlight = new SpotLight();
	m_pSpotlight->SetRadius( 40.0f );
	m_pSpotlight->SetIntensity( 10.0f );
	m_pSpotlight->SetColor( Vector3(1.0f,1.0f,0.8f) );
	m_pSpotlight->SetCone( Vector2( XM_PI/4.0f, XM_PI/4.0f ) );
	m_pSpotlight->SetCookie( m_pRenderSystem->LoadTexture( L"Media/spotlight.bmp" ) );
	m_pSpotlight->Transformation()->SetPosition( 0.0f, 0.0f, 0.0f );
	m_pSpotlight->PointTo( Vector3( 0.0f, 0.0f, 25.0f ), 0.0f );
	m_pSpotlight->Disable();

	m_pPointLight = new PointLight();
	m_pPointLight->SetRadius( 10000.0f );
	m_pPointLight->SetIntensity( 0.03f );
	m_pPointLight->SetColor( Vector3(1.0f,1.0f,1.0f) );
	m_pPointLight->Transformation()->SetPosition( 0.0f, 0.0f, 0.0f );
	//m_pPointLight->disable();

	UINT width, height;
	RenderSystemConfig config = ptr->GetConfig();
	width = config.Width;
	height = config.Height;

	m_pCamera = new Camera3D();
	m_pCamera->SetPosition( Transformation()->GetPosition() );
	m_pCamera->SetLookAt( Vector3( 0.0f, 1.0f, 0.0f ) );
	m_pCamera->SetUpVector( Vector3( 0.0f, 0.0f, 1.0f ) );
	m_pCamera->SetProjection( XM_PI*0.35f, width / (float)height, 0.1f, 5000.0f );
	m_pCamera->Update( 0.0f );
}

void PlayerEntity::OnAddToScene( Scene* ptr )
{
	ptr->AddSpotLight( m_pSpotlight );
	//ptr->AddPointLight( m_pPointLight );
	ptr->SetCamera( m_pCamera );
	Entity::OnAddToScene(ptr);
}

void PlayerEntity::Update(float deltaTime)
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

	Transformation()->Translate( relPos );
	pos = Transformation()->GetPosition();
	//pos.z = 1.6f;
	Transformation()->SetPosition( pos );

	lookat.x = pos.x + cos(m_DirEuler.z)*cos(m_DirEuler.y);
	lookat.y = pos.y + sin(m_DirEuler.z)*cos(m_DirEuler.y);
	lookat.z = pos.z + sin(m_DirEuler.y);

	m_pCamera->SetPosition( Transformation()->GetPosition() );
	m_pCamera->SetLookAt( lookat );
	m_pCamera->Update( deltaTime );

	Vector3 flashlightpos, flashlightlookat;
	flashlightpos.x = pos.x + 0.35f*cosf(m_DirEuler.z-XM_PIDIV2);
	flashlightpos.y = pos.y + 0.35f*sinf(m_DirEuler.z-XM_PIDIV2);
	//flashlightpos.z = 1.45f;
	flashlightpos.z = pos.z;
	m_pSpotlight->Transformation()->SetPosition(flashlightpos);
	flashlightlookat.x = pos.x + 25.0f*cosf(m_DirEuler.z)*cosf(m_DirEuler.y);
	flashlightlookat.y = pos.y + 25.0f*sinf(m_DirEuler.z)*cosf(m_DirEuler.y);
	flashlightlookat.z = pos.z + 25.0f*sinf(m_DirEuler.y);
	m_pSpotlight->PointTo( flashlightlookat, 0.0f );
	m_pSpotlight->Update();

	m_pPointLight->Transformation()->SetPosition( pos );

	if ( KEYDOWN( 'F' ) )
		m_pSpotlight->SetEnabled(!m_pSpotlight->IsEnabled());
}