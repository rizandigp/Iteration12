#pragma once

#include "FPPlayerEntity.h"

FPPlayerEntity::FPPlayerEntity(RenderSystem* pRenderSystem)
{
	m_PointingEulerDir = Vector3( 0.0f, 0.0f, 0.0f );
	m_TimeSinceLastShot = 0.0f;
	m_ShotEntityId = 0;
	m_bMouseDragged = false;
	SetRenderSystem( pRenderSystem );

	// Set up flashlight
	m_pSpotlight = new SpotLight();
	m_pSpotlight->SetRadius( 100.0f );
	m_pSpotlight->SetIntensity( 20.0f );
	m_pSpotlight->SetColor( Vector3(1.0f,1.0f,0.8f) );
	m_pSpotlight->SetCone( Vector2( XM_PI/3.0f, XM_PI/3.0f ) );
	m_pSpotlight->SetCookie( m_pRenderSystem->LoadTexture2D( L"Media/spotlight.bmp" ) );
	m_pSpotlight->Transformation()->SetPosition( 0.0f, 0.0f, 0.0f );
	m_pSpotlight->PointTo( Vector3( 0.0f, 0.0f, 25.0f ), 0.0f );
	m_pSpotlight->SetRSMEnabled( false );
	//m_pSpotlight->disable();

	UINT width, height;
	RenderSystemConfig config = pRenderSystem->GetConfig();
	width = config.Width;
	height = config.Height;

	// Set up first person camera
	m_pCamera = new Camera3D();
	m_pCamera->SetPosition( Transformation()->GetPosition() );
	m_pCamera->SetLookAt( Vector3( 0.0f, 1.0f, 0.0f ) );
	m_pCamera->SetUpVector( Vector3( 0.0f, 0.0f, 1.0f ) );
	m_pCamera->SetProjection( XM_PI*0.35f, width / (float)height, 0.1f, 2000.0f );
	m_pCamera->Update( 0.0f );
}

void FPPlayerEntity::OnAddToScene( Scene* ptr )
{
	ptr->AddSpotLight( m_pSpotlight );
	ptr->SetCamera( m_pCamera );
	Entity::OnAddToScene(ptr);
}

void FPPlayerEntity::Update(float deltaTime)
{
	POINT ptMousePos;
	Vector2 ptMousePosDelta;
	// Screen space xy threshold
	Vector2 directionThreshold(PI/10.0f,PI/12.0f);
	
	GetCursorPos( &ptMousePos );

	if ( KEYDOWN( VK_RBUTTON ) && ( m_bMouseDragged == false ))
	{
		m_bMouseDragged = true;
		m_ptLastMousePos = ptMousePos; 
		// Hide cursor
		while(ShowCursor(false)>=0);
	}

	if ( KEYDOWN( VK_RBUTTON ) )
	{
		m_bMouseDragged = true; 

		ptMousePosDelta.x = float(m_ptLastMousePos.x - ptMousePos.x);		// Yaw
		ptMousePosDelta.y = float(m_ptLastMousePos.y - ptMousePos.y);		// Pitch
		ptMousePosDelta.x /= 600.0f;
		ptMousePosDelta.y /= 600.0f;

		m_PointingEulerDir.z += ptMousePosDelta.x;
		m_PointingEulerDir.y += ptMousePosDelta.y;
		
		SetCursorPos( m_ptLastMousePos.x , m_ptLastMousePos.y );
		}
	else
	{
		m_bMouseDragged = false;
		// Show cursor
		ShowCursor(true);
	}

	// Limits
	if (m_PointingEulerDir.y>PI/2.0f)
		m_PointingEulerDir.y = PI/2.0f;
	if (m_PointingEulerDir.y<-PI/2.0f)
		m_PointingEulerDir.y = -PI/2.0f;

	float dist_z = m_PointingEulerDir.z-m_DirEuler.z;
	float dist_y = m_PointingEulerDir.y-m_DirEuler.y;
	//float dist_sqr = dist_z*dist_z + dist_y*dist_y;
	
	//m_DirEuler.z += dist_z*dist_z*dist_z/10.0f;
	//m_DirEuler.y += dist_y*dist_y*dist_y/10.0f;

	m_DirEuler.z += fabs(dist_z)*dist_z/20.0f;
	m_DirEuler.y += fabs(dist_y)*dist_y/20.0f;

	
	if ((m_PointingEulerDir.z-m_DirEuler.z)>directionThreshold.x)
		m_DirEuler.z = m_PointingEulerDir.z-directionThreshold.x;
	else if ((m_PointingEulerDir.z-m_DirEuler.z)<-directionThreshold.x)
		m_DirEuler.z = m_PointingEulerDir.z+directionThreshold.x;

	if ((m_PointingEulerDir.y-m_DirEuler.y)>directionThreshold.y)
		m_DirEuler.y = m_PointingEulerDir.y-directionThreshold.y;
	else if ((m_PointingEulerDir.y-m_DirEuler.y)<-directionThreshold.y)
		m_DirEuler.y = m_PointingEulerDir.y+directionThreshold.y;
	

	XMFLOAT3 vCamMovement( 0.0f , 0.0f , 0.0f );
	float movementSpeed = 1.0f;

	if ( KEYDOWN( VK_SHIFT ) )
		movementSpeed = 8.0f;
	
	// Basic movement inputs
	if ( KEYDOWN( 'W' ) )
		vCamMovement.y += movementSpeed*2.5f*deltaTime;
	if ( KEYDOWN( 'S' ) )
		vCamMovement.y -= movementSpeed*2.5f*deltaTime;
	if ( KEYDOWN( 'A' ) )
		vCamMovement.x += movementSpeed*2.0f*deltaTime;
	if ( KEYDOWN( 'D' ) )
		vCamMovement.x -= movementSpeed*2.0f*deltaTime;

	if ( KEYDOWN( VK_UP ) )
		vCamMovement.z += 1.0f*deltaTime;
	if ( KEYDOWN( VK_DOWN ) )
		vCamMovement.z -= 1.0f*deltaTime;
	
	// Shoot a cardboard box when space bar is hit
	if ( KEYDOWN( VK_SPACE ) && m_pScene->GetPhysicsSystem())
	{
		if ( m_TimeSinceLastShot > 0.2f )
		{
			// Rotate between props to shoot
			std::wstring prop;
			switch (m_ShotEntityId)
			{
			case 0:	prop = L"prop_kardus_a"; break; 
			case 1:	prop = L"prop_kardus_b"; break;
			case 2:	prop = L"prop_kardus_c"; break;
			case 3:	prop = L"prop_kardus_d"; break;
			case 4:	prop = L"prop_kardus_e"; break;
			case 5:	prop = L"prop_kardus_f"; break;
			case 6:	prop = L"prop_kardus_g"; break;
			case 7:	prop = L"prop_kardus_h"; break;
			case 8:	prop = L"prop_kardus_i"; break;
			case 9:	prop = L"prop_kardus_j"; break;
			}

			m_ShotEntityId += 1;
			if (m_ShotEntityId>9)
				m_ShotEntityId = 0;

			m_TimeSinceLastShot = 0.0f;
			Vector3 position = Transformation()->GetPosition();
			float shootingSpeed = 10.0f;
			hkVector4 shootingDirection( cos(m_DirEuler.z)*cos(m_DirEuler.y)*shootingSpeed, sin(m_DirEuler.z)*cos(m_DirEuler.y)*shootingSpeed, sin(m_DirEuler.y)*shootingSpeed );
			Entity_Prop* entity = new Entity_Prop(*m_pScene->m_pPrefabs[prop]);
			entity->m_pRigidBody->setPosition( hkVector4(position.x, position.y, position.z ) );
			entity->m_pRigidBody->setLinearVelocity( shootingDirection );
			m_pScene->AddEntity(entity);
		}
	}
	
	m_TimeSinceLastShot += deltaTime;

	Vector3 pos, relPos, lookat;

	// Walking forwards & backwards
	relPos.x = vCamMovement.y*cos(m_DirEuler.z)*cos(m_DirEuler.y);
	relPos.y = vCamMovement.y*sin(m_DirEuler.z)*cos(m_DirEuler.y);			
	relPos.z = vCamMovement.y*sin(m_DirEuler.y) + vCamMovement.z;		

	// Strafing
	relPos.x += vCamMovement.x*sin(-m_DirEuler.z);						
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

	Vector3 flashlightpos, flashlighttarget;
	flashlightpos.x = pos.x + 0.35f*cosf(m_DirEuler.z-XM_PIDIV2);
	flashlightpos.y = pos.y + 0.35f*sinf(m_DirEuler.z-XM_PIDIV2);
	//flashlightpos.z = 1.45f;
	flashlightpos.z = pos.z;
	m_pSpotlight->Transformation()->SetPosition(flashlightpos);
	flashlighttarget.x = pos.x + 25.0f*cosf(m_PointingEulerDir.z)*cosf(m_PointingEulerDir.y);
	flashlighttarget.y = pos.y + 25.0f*sinf(m_PointingEulerDir.z)*cosf(m_PointingEulerDir.y);
	flashlighttarget.z = pos.z + 25.0f*sinf(m_PointingEulerDir.y);
	m_pSpotlight->PointTo( flashlighttarget, 0.0f );
	m_pSpotlight->Update();

	//m_pPointLight->transform()->SetPosition( pos );

	if ( KEYDOWN( 'F' ) )
	{
		if (m_FlashlightButtonCooldown < 0.0f)
		{
			m_pSpotlight->SetEnabled(!m_pSpotlight->IsEnabled());
			m_FlashlightButtonCooldown = 0.2f;
		}
	}

	m_FlashlightButtonCooldown -= deltaTime;
}