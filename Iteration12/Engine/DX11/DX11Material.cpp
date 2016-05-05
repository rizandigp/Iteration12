#pragma once

#include "DX11Material.h"
#include "..\TextureCube.h"
#include "DX11RenderDispatcher.h"

//
//	DX11Material_DiffuseDetailbump
//
DX11Material_DiffuseDetailbump::DX11Material_DiffuseDetailbump( RenderSystem* pRenderSystem ) :	Material_DiffuseDetailbump( pRenderSystem )
{
	// Shaders:
	// 0: 2 omni + 2 spot lights
	// 1: 2 omni lights only
	// 2: 2 spot lights only
	m_pShader[0] = m_pRenderSystem->LoadShaderset( L"Shaders/DiffDetailbump.hlsl", "VS", "PS", SM_5_0 );
	m_pShader[1] = m_pRenderSystem->LoadShaderset( L"Shaders/DiffDetailbump_omni.hlsl", "VS", "PS", SM_5_0 );
	m_pShader[2] = m_pRenderSystem->LoadShaderset( L"Shaders/DiffDetailbump_spot.hlsl", "VS", "PS", SM_5_0 );
	m_pShaderNoLight = m_pRenderSystem->LoadShaderset( L"Shaders/OneColor.hlsl", "VS", "PS", SM_5_0 );
}


UINT DX11Material_DiffuseDetailbump::Bind(Renderer* pRenderer, RenderCommand* pRenderCommands[], SubmeshRenderData* pRenderData, Transform* pTransform )
{
	// Get lights that affect the rendered submesh
	std::vector<PointLight*>* pPointLights = &pRenderData->AffectingPointLights;
	std::vector<SpotLight*>* pSpotLights = &pRenderData->AffectingSpotLights;

	int numOmni = pPointLights->size();
	int numSpot = pSpotLights->size();

	int pass = 0, omni = 0, spot = 0;

	while(1)
	{
		pass++;
		DX11RenderCommand_Draw* command = dynamic_cast<DX11RenderCommand_Draw*>(pRenderCommands[pass-1]);

		// Set render state & textures
		command->SetBlendState(m_BlendState);
		command->SetRenderState(m_RenderState);

		command->SetTexture( "txDiffuse", m_pDiffuse );
		command->SetTexture( "txNormal", m_pNormal );
		command->SetTexture( "txDetailNormal", m_pDetailNormal );

		// No affecting lights, simply use the default ambient/black shader
		if ( pSpotLights->empty() && pPointLights->empty() )
		{
			ShaderParams params2;
			params2.initialize( m_pShaderNoLight );
			params2.setParam( "ViewProjection", 0, &m_pRenderSystem->GetCamera()->GetViewProjectionMatrix().transpose() );
			params2.setParam( "World", 0, &pTransform->GetMatrix().transpose() );
			params2.setParam( "vMeshColor", 0, &XMFLOAT4( 0.0f, 0.0f, 0.0f, 0.0f ) );
			command->SetShaderset( (DX11Shaderset*)m_pShaderNoLight );
			command->SetShaderParams( &params2 );
			return 1;
		}

		// Set common shader parameters
		Timer paraminit;
		ShaderParams params;
		params.initialize( m_pShader[0] );

		params.setParam( "ViewProjection", 0, &m_pRenderSystem->GetCamera()->GetViewProjectionMatrix().transpose() );
		params.setParam( "NormalMatrix", 0, &pTransform->GetMatrix().transpose() );
		params.setParam( "World", 0, &pTransform->GetMatrix().transpose() );

		params.setParam( "DetailNormalStrength", m_DetailNormalStrength );
		params.setParam( "DetailTiling", m_DetailTiling );
		params.setParam( "SpecularParams", 0, &XMFLOAT4( m_SpecIntensity, m_SpecPower, 0.0f ,0.0f) );
		params.setParam( "vEyePos", 0, &Vector4(m_pRenderSystem->GetCamera()->GetPosition(),0.0f) );
	
		// PointLight-specific shader params
		if (!pPointLights->empty()&&(omni<numOmni))
		{
			int i = 0;
			for (std::vector<PointLight*>::const_iterator it = pPointLights->begin() + omni; it != pPointLights->end(); ++it)
			{
				paraminit.Start();
				params.setParam( "vPointLightPos", i, &Vector4((*it)->Transformation()->GetPosition(),(*it)->GetRadius()));
				params.setParam( "vPointLightColor", i, &Vector4(pow((*it)->getColor().x,2.2f)*(*it)->GetIntensity(),pow((*it)->getColor().y,2.2f)*(*it)->GetIntensity(),pow((*it)->getColor().z,2.2f)*(*it)->GetIntensity(),(*it)->GetIntensity()));
				//params.setParam( "vPointLightColor", i, &XMFLOAT4((*it)->getColor().x*(*it)->getIntensity(),(*it)->getColor().y*(*it)->getIntensity(),(*it)->getColor().z*(*it)->getIntensity(),(*it)->getIntensity()));
				m_pRenderSystem->t_shaderparams += paraminit.GetMiliseconds();
				i++;
				omni++;
				if (i>=2)
					break;
			}
		}

		// SpotLight-specific shader params
		if (!pSpotLights->empty()&&(spot<numSpot))
		{
			int i = 0;
			for (std::vector<SpotLight*>::const_iterator it = pSpotLights->begin() + spot; it != pSpotLights->end(); ++it)
			{
				paraminit.Start();

				params.setParam( "vSpotLightPos", i, &Vector4((*it)->Transformation()->GetPosition(),(*it)->GetRadius()));
				params.setParam( "vSpotLightColor", i, &XMFLOAT4(pow((*it)->GetColor().x,2.2f)*(*it)->GetIntensity(),pow((*it)->GetColor().y,2.2f)*(*it)->GetIntensity(),pow((*it)->GetColor().z,2.2f)*(*it)->GetIntensity(),(*it)->GetIntensity()));
				//params.setParam( "vSpotLightColor", i, &XMFLOAT4((*it)->getColor().x*(*it)->getIntensity(),(*it)->getColor().y*(*it)->getIntensity(),(*it)->getColor().z*(*it)->getIntensity(),(*it)->getIntensity()));
				//params.setParam( "SpotLightViewProjection", i, &XMMatrixTranspose(XMLoadFloat4x4(&(*it)->getProjectionCamera()->getViewProjectionMatrix())) );
				params.setParam( "SpotLightViewProjection", i, &(*it)->GetProjectionCamera()->GetViewProjectionMatrix().transpose() );
				params.setParam( "iSpotLightShadowEnabled", i, (int)( (*it)->IsCastingShadow() ? 1 : 0 ) );
			
				m_pRenderSystem->t_shaderparams += paraminit.GetMiliseconds();

				if (i==0)
				{
					command->SetTexture( "txShadowmap", (*it)->GetShadowmap() );
					command->SetTexture( "txCookie", (*it)->GetCookie() );
				}
				else
				{
					command->SetTexture( "txShadowmap2", (*it)->GetShadowmap() );
					command->SetTexture( "txCookie2", (*it)->GetCookie() );
				}
				i++;
				spot++;
				if (i>=2)
				break;
			}
		}

		m_pRenderSystem->t_shaderparams += paraminit.GetMiliseconds();

		command->SetShaderParams( &params );
		command->SetShaderset( (DX11Shaderset*)m_pShader[0] );

		if(pass>1)
		{
			BlendState AdditiveBlendState;
			AdditiveBlendState.BlendEnable = true;
			AdditiveBlendState.SrcBlend = BLEND_ONE;
			AdditiveBlendState.DestBlend = BLEND_ONE;
			AdditiveBlendState.BlendOp = BLEND_OP_ADD;
			AdditiveBlendState.SrcBlendAlpha = BLEND_ONE;
			AdditiveBlendState.DestBlendAlpha = BLEND_ONE;
			AdditiveBlendState.BlendOpAlpha = BLEND_OP_ADD;
			command->SetBlendState(AdditiveBlendState);

			RenderState AdditiveRenderState;
			AdditiveRenderState.FillSolid = true;
			AdditiveRenderState.CullingMode = CULL_BACK;
			AdditiveRenderState.EnableDepthTest = true;
			AdditiveRenderState.DepthBias = -1;
			AdditiveRenderState.DepthComparison = COMPARISON_LESS;
			AdditiveRenderState.EnableStencil = false;

			command->SetRenderState(AdditiveRenderState);
		}

		if (omni>=numOmni && spot>=numSpot)
			break;
	}

	
	// Return number of passes
	return pass;
}

//
//	DX11Material_BlinnPhong
//
DX11Material_BlinnPhong::DX11Material_BlinnPhong( RenderSystem* pRenderSystem )	:	Material_BlinnPhong( pRenderSystem )
{
	// TODO : different light configurations
	m_pShaderNoLight = m_pRenderSystem->LoadShaderset( L"Shaders/Black.hlsl", "VS", "PS", SM_5_0 );
	m_pShader[0] = m_pRenderSystem->LoadShaderset( L"Shaders/BlinnPhong.hlsl", "VS", "PS", SM_5_0 );
	m_pShader[1] = m_pRenderSystem->LoadShaderset( L"Shaders/BlinnPhong.hlsl", "VS", "PS", SM_5_0 );
	m_pShader[2] = m_pRenderSystem->LoadShaderset( L"Shaders/BlinnPhong.hlsl", "VS", "PS", SM_5_0 );
}

UINT DX11Material_BlinnPhong::Bind(Renderer* pRenderer, RenderCommand* pRenderCommands[], SubmeshRenderData* pRenderData, Transform* pTransform )
{
	// Get lights that affect the rendered submesh
	std::vector<PointLight*>* pPointLights = &pRenderData->AffectingPointLights;
	std::vector<SpotLight*>* pSpotLights = &pRenderData->AffectingSpotLights;

	int numOmni = pPointLights->size();
	int numSpot = pSpotLights->size();

	int pass = 0, omni = 0, spot = 0;

	while(1)
	{
		pass++;
		DX11RenderCommand_Draw* command = dynamic_cast<DX11RenderCommand_Draw*>(pRenderCommands[pass-1]);

		// Set render states & textures
		command->SetBlendState(m_BlendState);
		command->SetRenderState(m_RenderState);

		command->SetTexture( "txDiffuse", m_pDiffuse );
		command->SetTexture( "txNormal", m_pNormal );
		command->SetTexture( "txSpecular", m_pSpecular );

		// No affecting lights, simply use the default ambient/black shader
		if ( pSpotLights->empty() && pPointLights->empty() )
		{
			ShaderParams params2;
			params2.initialize( m_pShaderNoLight );
			params2.setParam( "ViewProjection", 0, &m_pRenderSystem->GetCamera()->GetViewProjectionMatrix().transpose() );
			params2.setParam( "World", 0, &pTransform->GetMatrix().transpose() );
			params2.setParam( "vMeshColor", 0, &XMFLOAT4( 0.0f, 0.0f, 0.0f, 0.0f ) );
			command->SetShaderset( (DX11Shaderset*)m_pShaderNoLight );
			command->SetShaderParams( &params2 );
			return 1;
		}

		Timer paraminit;
		// Set common shader parameters
		ShaderParams params;
		params.initialize( m_pShader[0] );
		//m_pRenderSystem->t_shaderparams += paraminit.GetMiliseconds();

		params.setParam( "ViewProjection", 0, &m_pRenderSystem->GetCamera()->GetViewProjectionMatrix().transpose() );
		params.setParam( "NormalMatrix", 0, &pTransform->GetMatrix().transpose() ) ;
		params.setParam( "World", 0, &pTransform->GetMatrix().transpose() );

		params.setParam( "SpecularParams", 0, &XMFLOAT4( m_SpecIntensity, m_SpecPower, 0.0f ,0.0f) );
		params.setParam( "vEyePos", 0, &Vector4(m_pRenderSystem->GetCamera()->GetPosition(),0.0f) );

		m_pRenderSystem->t_shaderparams += paraminit.GetMiliseconds();

		// PointLight-specific shader params
		if (!pPointLights->empty()&&(omni<numOmni))
		{
			int i = 0;
			for (std::vector<PointLight*>::const_iterator it = pPointLights->begin() + omni; it != pPointLights->end(); ++it)
			{
				paraminit.Start();

				params.setParam( "vPointLightPos", i, &Vector4((*it)->Transformation()->GetPosition(),(*it)->GetRadius()));
				params.setParam( "vPointLightColor", i, &Vector4(pow((*it)->getColor().x,2.2f)*(*it)->GetIntensity(),pow((*it)->getColor().y,2.2f)*(*it)->GetIntensity(),pow((*it)->getColor().z,2.2f)*(*it)->GetIntensity(),(*it)->GetIntensity()));
				//params.setParam( "vPointLightColor", i, &XMFLOAT4((*it)->getColor().x*(*it)->getIntensity(),(*it)->getColor().y*(*it)->getIntensity(),(*it)->getColor().z*(*it)->getIntensity(),(*it)->getIntensity()));
				
				m_pRenderSystem->t_shaderparams += paraminit.GetMiliseconds();

				i++;
				omni++;
				if (i>=2)
					break;
			}
		}

		// SpotLight-specific shader params
		if (!pSpotLights->empty()&&(spot<numSpot))
		{
			int i = 0;
			for (std::vector<SpotLight*>::const_iterator it = pSpotLights->begin() + spot; it != pSpotLights->end(); ++it)
			{
				paraminit.Start();

				params.setParam( "vSpotLightPos", i, &Vector4((*it)->Transformation()->GetPosition(),(*it)->GetRadius()));
				params.setParam( "vSpotLightColor", i, &Vector4(pow((*it)->GetColor().x,2.2f)*(*it)->GetIntensity(),pow((*it)->GetColor().y,2.2f)*(*it)->GetIntensity(),pow((*it)->GetColor().z,2.2f)*(*it)->GetIntensity(),(*it)->GetIntensity()));
				//params.setParam( "vSpotLightColor", i, &XMFLOAT4((*it)->getColor().x*(*it)->getIntensity(),(*it)->getColor().y*(*it)->getIntensity(),(*it)->getColor().z*(*it)->getIntensity(),(*it)->getIntensity()));
				//params.setParam( "SpotLightViewProjection", i, &XMMatrixTranspose(XMLoadFloat4x4(&(*it)->getProjectionCamera()->getViewProjectionMatrix())) );
				params.setParam( "SpotLightViewProjection", i, &(*it)->GetProjectionCamera()->GetViewProjectionMatrix().transpose() );
				params.setParam( "iSpotLightShadowEnabled", i, (int)( (*it)->IsCastingShadow() ? 1 : 0 ) );
	
				m_pRenderSystem->t_shaderparams += paraminit.GetMiliseconds();

				if (i==0)
				{
					command->SetTexture( "txShadowmap", (*it)->GetShadowmap() );
					command->SetTexture( "txCookie", (*it)->GetCookie() );
				}
				else
				{
					command->SetTexture( "txShadowmap2", (*it)->GetShadowmap() );
					command->SetTexture( "txCookie2", (*it)->GetCookie() );
				}
				i++;
				spot++;
				if (i>=2)
					break;
			}
		}

		//m_pRenderSystem->t_shaderparams += paraminit.GetMiliseconds();

		command->SetShaderset( (DX11Shaderset*)m_pShader[0] );
		command->SetShaderParams( &params );

		if(pass>1)
		{
			BlendState AdditiveBlendState;
			AdditiveBlendState.BlendEnable = true;
			AdditiveBlendState.SrcBlend = BLEND_ONE;
			AdditiveBlendState.DestBlend = BLEND_ONE;
			AdditiveBlendState.BlendOp = BLEND_OP_ADD;
			AdditiveBlendState.SrcBlendAlpha = BLEND_ONE;
			AdditiveBlendState.DestBlendAlpha = BLEND_ONE;
			AdditiveBlendState.BlendOpAlpha = BLEND_OP_ADD;
			command->SetBlendState(AdditiveBlendState);

			RenderState AdditiveRenderState;
			AdditiveRenderState.FillSolid = true;
			AdditiveRenderState.CullingMode = CULL_BACK;
			AdditiveRenderState.EnableDepthTest = true;
			AdditiveRenderState.DepthBias = -1;
			AdditiveRenderState.DepthComparison = COMPARISON_LESS;
			AdditiveRenderState.EnableStencil = false;
			command->SetRenderState(AdditiveRenderState);
		}

		if (omni>=numOmni && spot>=numSpot)
			break;
	}

	// Return number of passes
	return pass;
}

DX11Material_Deferred::DX11Material_Deferred( RenderSystem* pRenderSystem )	:	Material_Deferred( pRenderSystem )
{
	m_pShader[0] = m_pRenderSystem->LoadShaderset( L"Shaders/DeferredGBuffer.hlsl", "VS", "PS", SM_5_0 );
}

UINT DX11Material_Deferred::Bind(Renderer* pRenderer, RenderCommand* pRenderCommands[], SubmeshRenderData* pRenderData, Transform* pTransform )
{
	DX11RenderCommand_Draw* command = dynamic_cast<DX11RenderCommand_Draw*>(pRenderCommands[0]);

	// Set render states & textures
	BlendState DefaultBlendState;
	RenderState DefaultRenderState;
	command->SetBlendState(DefaultBlendState);
	command->SetRenderState(DefaultRenderState);

	command->SetTexture( "txDiffuse", m_pDiffuse );
	command->SetTexture( "txNormal", m_pNormal );
	command->SetTexture( "txSpecular", m_pSpecular );

	Timer paraminit;
	// Set common shader parameters
	ShaderParams params;
	params.initialize( m_pShader[0] );
	//m_pRenderSystem->t_shaderparams += paraminit.GetMiliseconds();
	params.setParam( "View", 0, &m_pRenderSystem->GetCamera()->GetViewMatrix().transpose() );
	params.setParam( "ViewProjection", 0, &m_pRenderSystem->GetCamera()->GetViewProjectionMatrix().transpose() );
	params.setParam( "NormalMatrix", 0, &pTransform->GetMatrix().transpose() ) ;
	params.setParam( "World", 0, &pTransform->GetMatrix().transpose() );
	params.setParam( "fFarPlane", 0, m_pRenderSystem->GetCamera()->GetFarPlane() );

	params.setParam( "SpecularParams", 0, &XMFLOAT4( m_SpecIntensity, m_Glossiness, 0.0f ,0.0f) );
	params.setParam( "vEyePos", 0, &Vector4(m_pRenderSystem->GetCamera()->GetPosition(),0.0f) );

	m_pRenderSystem->t_shaderparams += paraminit.GetMiliseconds();

	command->SetShaderset( (DX11Shaderset*)m_pShader[0] );
	command->SetShaderParams( &params );

	return 1;
}

DX11Material_DeferredIBL::DX11Material_DeferredIBL( RenderSystem* pRenderSystem )	:	Material_DeferredIBL( pRenderSystem )
{
	m_pShader[0] = m_pRenderSystem->LoadShaderset( L"Shaders/DeferredGBufferIBL.hlsl", "VS", "PS", SM_5_0, false );
}

UINT DX11Material_DeferredIBL::Bind(Renderer* pRenderer, RenderCommand* pRenderCommands[], SubmeshRenderData* pRenderData, Transform* pTransform )
{
	DX11RenderCommand_Draw* command = dynamic_cast<DX11RenderCommand_Draw*>(pRenderCommands[0]);

	// Set render states & textures
	command->SetBlendState(m_BlendState);
	command->SetRenderState(m_RenderState);

	command->SetTexture( "txDiffuse", m_pDiffuse );
	command->SetTexture( "txNormal", m_pNormal );
	command->SetTexture( "txSpecular", m_pSpecular );
	command->SetTexture( "txAO", m_pAO );
	command->SetTexture( "txIBL", m_pIBL );

	Timer paraminit;
	// Set common shader parameters
	ShaderParams params;
	params.initialize( m_pShader[0] );
	//m_pRenderSystem->t_shaderparams += paraminit.GetMiliseconds();
	params.setParam( "View", 0, &m_pRenderSystem->GetCamera()->GetViewMatrix().transpose() );
	params.setParam( "ViewProjection", 0, &m_pRenderSystem->GetCamera()->GetViewProjectionMatrix().transpose() );
	params.setParam( "NormalMatrix", 0, &pTransform->GetMatrix().transpose() ) ;
	params.setParam( "World", 0, &pTransform->GetMatrix().transpose() );
	params.setParam( "fFarPlane", 0, m_pRenderSystem->GetCamera()->GetFarPlane() );

	params.setParam( "SpecularParams", 0, &XMFLOAT4( m_SpecIntensity, m_Glossiness, 0.0f ,0.0f) );
	params.setParam( "vEyePos", 0, &Vector4(m_pRenderSystem->GetCamera()->GetPosition(),0.0f) );

	m_pRenderSystem->t_shaderparams += paraminit.GetMiliseconds();

	command->SetShaderset( (DX11Shaderset*)m_pShader[0] );
	command->SetShaderParams( &params );

	return 1;
}

DX11Material_Spotlight::DX11Material_Spotlight( RenderSystem* pRenderSystem ) :	Material_Spotlight( pRenderSystem )
{
	m_pShader = pRenderSystem->LoadShaderset( L"Shaders/DeferredSpotlight.hlsl", "VS", "PS", SM_5_0 );
}

UINT DX11Material_Spotlight::Bind(Renderer* pRenderer, RenderCommand* pRenderCommands[], SubmeshRenderData* pRenderData, Transform* pTransform )
{
	DX11RenderCommand_Draw* command = dynamic_cast<DX11RenderCommand_Draw*>(pRenderCommands[0]);

	// Set render states & textures
	BlendState AdditiveBlendState;
	AdditiveBlendState.BlendEnable = true;
	AdditiveBlendState.SrcBlend = BLEND_ONE;
	AdditiveBlendState.DestBlend = BLEND_ONE;
	AdditiveBlendState.BlendOp = BLEND_OP_ADD;
	AdditiveBlendState.SrcBlendAlpha = BLEND_ONE;
	AdditiveBlendState.DestBlendAlpha = BLEND_ONE;
	AdditiveBlendState.BlendOpAlpha = BLEND_OP_ADD;
	command->SetBlendState(AdditiveBlendState);

	RenderState BackfaceRenderState;
	BackfaceRenderState.FillSolid = true;
	BackfaceRenderState.CullingMode = CULL_FRONT;
	BackfaceRenderState.EnableDepthTest = true;
	BackfaceRenderState.EnableDepthWrite = false;
	BackfaceRenderState.DepthBias = 0;
	BackfaceRenderState.DepthComparison = COMPARISON_GREATER;
	BackfaceRenderState.EnableStencil = false;
	command->SetRenderState(BackfaceRenderState);

	/*
	command->SetBlendState(m_pDefaultBlendState);
	command->SetDepthStencilState(m_pDefaultDepthStencilState);
	command->SetRasterizerState(m_pDefaultRasterizerState);*/

	command->SetTexture( "texGBuffer0", m_pGbuffer[0] );
	command->SetTexture( "texGBuffer1", m_pGbuffer[1] );
	command->SetTexture( "texGBuffer2", m_pGbuffer[2] );
	command->SetTexture( "texShadowmap", m_pShadowmap );
	command->SetTexture( "texCookie", m_pCookie );

	Timer paraminit;
	// Set common shader parameters
	ShaderParams params;
	params.initialize( m_pShader );
	//m_pRenderSystem->t_shaderparams += paraminit.GetMiliseconds();
	RenderSystemConfig config = m_pRenderSystem->GetConfig();
	params.setParam( "View", 0, &m_pRenderSystem->GetCamera()->GetViewMatrix().transpose() );
	params.setParam( "ViewProjection", 0, &m_pRenderSystem->GetCamera()->GetViewProjectionMatrix().transpose() );
	params.setParam( "InvView", 0, &m_pRenderSystem->GetCamera()->GetViewMatrix().inverse().transpose() );
	params.setParam( "InvProjection", 0, &m_pRenderSystem->GetCamera()->GetProjectionMatrix().inverse().transpose() );
	params.setParam( "InvViewProjection", 0, &m_pRenderSystem->GetCamera()->GetViewProjectionMatrix().inverse().transpose() );
	params.setParam( "World", 0, &pTransform->GetMatrix().transpose() );
	params.setParam( "InvSpotlightProjection", 0, &m_InvProjection.transpose() );
	params.setParam( "SpotlightViewProjection", 0, &m_ViewProjection.transpose() );
	params.setParam( "SpotlightView", 0, &m_View.transpose() );
	params.setParam( "vSpotLightColor", &Vector4(pow(m_Color.x,2.2f)*m_Intensity,pow(m_Color.y,2.2f)*m_Intensity,pow(m_Color.z,2.2f)*m_Intensity,m_Intensity) );
	params.setParam( "vEyePos", &Vector4(m_pRenderSystem->GetCamera()->GetPosition(),1.0f) );
	params.setParam( "ScreenDimensions", &Vector4(config.Width,config.Height,0.0f,0.0f) );
	params.setParam( "SpotLightRadius", m_Radius );
	params.setParam( "fFarPlane", m_pRenderSystem->GetCamera()->GetFarPlane() );

	m_pRenderSystem->t_shaderparams += paraminit.GetMiliseconds();

	command->SetShaderset( (DX11Shaderset*)m_pShader );
	command->SetShaderParams( &params );

	return 1;
}


DX11Material_Pointlight::DX11Material_Pointlight( RenderSystem* pRenderSystem ) :	Material_Pointlight( pRenderSystem )
{
	m_pShader = pRenderSystem->LoadShaderset( L"Shaders/DeferredPointlight.hlsl", "VS", "PS", SM_5_0 );
}

UINT DX11Material_Pointlight::Bind(Renderer* pRenderer, RenderCommand* pRenderCommands[], SubmeshRenderData* pRenderData, Transform* pTransform )
{
	DX11RenderCommand_Draw* command = dynamic_cast<DX11RenderCommand_Draw*>(pRenderCommands[0]);

	// Set render states & textures
	BlendState AdditiveBlendState;
	AdditiveBlendState.BlendEnable = true;
	AdditiveBlendState.SrcBlend = BLEND_ONE;
	AdditiveBlendState.DestBlend = BLEND_ONE;
	AdditiveBlendState.BlendOp = BLEND_OP_ADD;
	AdditiveBlendState.SrcBlendAlpha = BLEND_ONE;
	AdditiveBlendState.DestBlendAlpha = BLEND_ONE;
	AdditiveBlendState.BlendOpAlpha = BLEND_OP_ADD;
	command->SetBlendState(AdditiveBlendState);

	RenderState BackfaceRenderState;
	BackfaceRenderState.FillSolid = true;
	BackfaceRenderState.CullingMode = CULL_FRONT;
	BackfaceRenderState.EnableDepthTest = true;
	BackfaceRenderState.EnableDepthWrite = false;
	BackfaceRenderState.DepthBias = 0;
	BackfaceRenderState.DepthComparison = COMPARISON_GREATER;
	BackfaceRenderState.EnableStencil = false;
	command->SetRenderState(BackfaceRenderState);

	command->SetTexture( "txGBuffer0", m_pGbuffer[0] );
	command->SetTexture( "txGBuffer1", m_pGbuffer[1] );
	command->SetTexture( "txGBuffer2", m_pGbuffer[2] );
	command->SetTexture( "txShadowmap", m_pShadowmap );
	command->SetTexture( "txCookie", m_pCookie );

	Timer paraminit;
	// Set common shader parameters
	ShaderParams params;
	params.initialize( m_pShader );
	//m_pRenderSystem->t_shaderparams += paraminit.GetMiliseconds();
	RenderSystemConfig config = m_pRenderSystem->GetConfig();
	params.setParam( "ViewProjection", 0, &m_pRenderSystem->GetCamera()->GetViewProjectionMatrix().transpose() );
	params.setParam( "InvProjection", 0, &m_pRenderSystem->GetCamera()->GetProjectionMatrix().inverse().transpose() );
	params.setParam( "InvViewProjection", 0, &m_pRenderSystem->GetCamera()->GetViewProjectionMatrix().inverse().transpose() );
	params.setParam( "World", 0, &pTransform->GetMatrix().transpose() );
	params.setParam( "vPointLightColor", &Vector4(pow(m_Color.x,2.2f)*m_Intensity,pow(m_Color.y,2.2f)*m_Intensity,pow(m_Color.z,2.2f)*m_Intensity,m_Intensity) );
	params.setParam( "vEyePos", &Vector4(m_pRenderSystem->GetCamera()->GetPosition(),1.0f) );
	params.setParam( "ScreenDimensions", &Vector4(config.Width,config.Height,0.0f,0.0f) );
	params.setParam( "PointLightRadius", m_Radius );

	m_pRenderSystem->t_shaderparams += paraminit.GetMiliseconds();

	command->SetShaderset( (DX11Shaderset*)m_pShader );
	command->SetShaderParams( &params );

	return 1;
}

//
// DX11Material_Water
//
DX11Material_Water::DX11Material_Water( RenderSystem* pRenderSystem )	:	Material_Water( pRenderSystem )
{
	// Load the shader
	m_pShader = m_pRenderSystem->LoadShaderset( L"Shaders/Water.hlsl", "VS", "PS", SM_5_0 );
}


UINT DX11Material_Water::Bind(Renderer* pRenderer, RenderCommand* pRenderCommands[], SubmeshRenderData* pRenderData, Transform* pTransform )
{
	DX11RenderCommand_Draw* command = dynamic_cast<DX11RenderCommand_Draw*>(pRenderCommands[0]);

	// Set render states & textures
	command->SetBlendState(m_BlendState);
	command->SetRenderState(m_RenderState);

	command->SetTexture( "txDiffuse", m_pDiffuse );
	command->SetTexture( "txNormal", m_pNormal );
	command->SetTexture( "txFoam", m_pFoam );
	command->SetTexture( "txCubemap", m_pIBL );
	//command->SetTexture( "txSpecular", (DX11Texture2D*)m_pSpecular );

	Timer paraminit;
	// Set common shader parameters
	ShaderParams params;
	params.initialize( m_pShader );
	//m_pRenderSystem->t_shaderparams += paraminit.GetMiliseconds();

	params.setParam( "View", 0, &m_pRenderSystem->GetCamera()->GetViewMatrix().transpose() );
	params.setParam( "ViewProjection", 0, &m_pRenderSystem->GetCamera()->GetViewProjectionMatrix().transpose() );
	params.setParam( "NormalMatrix", 0, &pTransform->GetMatrix().transpose() ) ;
	params.setParam( "World", 0, &pTransform->GetMatrix().transpose() );

	//params.setParam( "SpecularParams", 0, &XMFLOAT4( m_SpecIntensity, m_Glossiness, 0.0f ,0.0f) );
	params.setParam( "vEyePos", 0, &Vector4(m_pRenderSystem->GetCamera()->GetPosition(),0.0f) );
	params.setParam( "time", 0, pRenderer->GetTime() );
	params.setParam( "fFarPlane", 0, m_pRenderSystem->GetCamera()->GetFarPlane() );

	m_pRenderSystem->t_shaderparams += paraminit.GetMiliseconds();

	command->SetShaderset( (DX11Shaderset*)m_pShader );
	command->SetShaderParams( &params );

	return 1;
}


DX11Material_Skybox::DX11Material_Skybox( RenderSystem* pRenderSystem )	:	Material_Skybox( pRenderSystem )
{
	m_pCubemap = NULL;
	m_pShader = m_pRenderSystem->LoadShaderset( L"Shaders/Skybox.hlsl", "VS", "PS", SM_5_0 );
}

UINT DX11Material_Skybox::Bind(Renderer* pRenderer, RenderCommand* pRenderCommands[], SubmeshRenderData* pRenderData, Transform* pTransform )
{
	DX11RenderCommand_Draw* command = dynamic_cast<DX11RenderCommand_Draw*>(pRenderCommands[0]);

	// Set render states & textures
	command->SetBlendState(m_BlendState);
	command->SetRenderState(m_RenderState);

	command->SetTexture( "txCubemap", m_pCubemap );

	Timer paraminit;
	// Set common shader parameters
	ShaderParams params;
	params.initialize( m_pShader );
	//m_pRenderSystem->t_shaderparams += paraminit.GetMiliseconds();

	params.setParam( "ViewProjection", 0, &m_pRenderSystem->GetCamera()->GetViewProjectionMatrix().transpose() );
	params.setParam( "World", 0, &pTransform->GetMatrix().transpose() );

	m_pRenderSystem->t_shaderparams += paraminit.GetMiliseconds();

	command->SetShaderset( (DX11Shaderset*)m_pShader );
	command->SetShaderParams( &params );

	return 1;
}