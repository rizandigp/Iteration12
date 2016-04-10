#pragma once

#include "DX11Material.h"
#include "TextureCube.h"

//
//	DX11Material_DiffuseDetailbump
//
DX11Material_DiffuseDetailbump::DX11Material_DiffuseDetailbump( RenderSystem* pRenderSystem )	:	Material( pRenderSystem )
{
	m_DetailNormalStrength = 1.0f;
	m_DetailTiling = 10.0f;

	// Shaders:
	// 0: 2 omni + 2 spot lights
	// 1: 2 omni lights only
	// 2: 2 spot lights only
	m_pShader[0] = m_pRenderSystem->loadShaderset( L"Shaders/DiffDetailbump.hlsl", "VS", "PS", SM_5_0 );
	m_pShader[1] = m_pRenderSystem->loadShaderset( L"Shaders/DiffDetailbump_omni.hlsl", "VS", "PS", SM_5_0 );
	m_pShader[2] = m_pRenderSystem->loadShaderset( L"Shaders/DiffDetailbump_spot.hlsl", "VS", "PS", SM_5_0 );
	m_pShaderNoLight = m_pRenderSystem->loadShaderset( L"Shaders/OneColor.hlsl", "VS", "PS", SM_5_0 );
}


UINT DX11Material_DiffuseDetailbump::bind(Renderer* pRenderer, RenderCommand* pRenderCommands[], SubmeshRenderData* pRenderData, Transform* pTransform )
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
		command->setBlendState(m_pDefaultBlendState);
		command->setDepthStencilState(m_pDefaultDepthStencilState);
		command->setRasterizerState(m_pDefaultRasterizerState);

		command->setTexture( "txDiffuse", (DX11Texture2D*)m_pDiffuse );
		command->setTexture( "txNormal", (DX11Texture2D*)m_pNormal );
		command->setTexture( "txDetailNormal", (DX11Texture2D*)m_pDetailNormal );

		// No affecting lights, simply use the default ambient/black shader
		if ( pSpotLights->empty() && pPointLights->empty() )
		{
			ShaderParams params2;
			params2.initialize( m_pShaderNoLight );
			params2.setParam( "ViewProjection", 0, &m_pRenderSystem->getCamera()->getViewProjectionMatrix().transpose() );
			params2.setParam( "World", 0, &pTransform->getMatrix().transpose() );
			params2.setParam( "vMeshColor", 0, &XMFLOAT4( 0.0f, 0.0f, 0.0f, 0.0f ) );
			command->setShaderset( (D3D11Shaderset*)m_pShaderNoLight );
			command->setShaderParams( &params2 );
			return 1;
		}

		// Set common shader parameters
		Timer paraminit;
		ShaderParams params;
		params.initialize( m_pShader[0] );

		params.setParam( "ViewProjection", 0, &m_pRenderSystem->getCamera()->getViewProjectionMatrix().transpose() );
		params.setParam( "NormalMatrix", 0, &pTransform->getMatrix().transpose() );
		params.setParam( "World", 0, &pTransform->getMatrix().transpose() );

		params.setParam( "DetailNormalStrength", m_DetailNormalStrength );
		params.setParam( "DetailTiling", m_DetailTiling );
		params.setParam( "SpecularParams", 0, &XMFLOAT4( m_SpecIntensity, m_SpecPower, 0.0f ,0.0f) );
		params.setParam( "vEyePos", 0, &XMFLOAT4(m_pRenderSystem->getCamera()->getPosition().x,m_pRenderSystem->getCamera()->getPosition().y,m_pRenderSystem->getCamera()->getPosition().z,0.0f) );
	
		// PointLight-specific shader params
		if (!pPointLights->empty()&&(omni<numOmni))
		{
			int i = 0;
			for (std::vector<PointLight*>::const_iterator it = pPointLights->begin() + omni; it != pPointLights->end(); ++it)
			{
				paraminit.start();
				params.setParam( "vPointLightPos", i, &XMFLOAT4((*it)->transform()->getPosition().x,(*it)->transform()->getPosition().y,(*it)->transform()->getPosition().z,(*it)->getRadius()));
				params.setParam( "vPointLightColor", i, &XMFLOAT4(pow((*it)->getColor().x,2.2f)*(*it)->getIntensity(),pow((*it)->getColor().y,2.2f)*(*it)->getIntensity(),pow((*it)->getColor().z,2.2f)*(*it)->getIntensity(),(*it)->getIntensity()));
				//params.setParam( "vPointLightColor", i, &XMFLOAT4((*it)->getColor().x*(*it)->getIntensity(),(*it)->getColor().y*(*it)->getIntensity(),(*it)->getColor().z*(*it)->getIntensity(),(*it)->getIntensity()));
				m_pRenderSystem->t_shaderparams += paraminit.getMiliseconds();
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
				paraminit.start();

				params.setParam( "vSpotLightPos", i, &XMFLOAT4((*it)->transform()->getPosition().x,(*it)->transform()->getPosition().y,(*it)->transform()->getPosition().z,(*it)->getRadius()));
				params.setParam( "vSpotLightColor", i, &XMFLOAT4(pow((*it)->getColor().x,2.2f)*(*it)->getIntensity(),pow((*it)->getColor().y,2.2f)*(*it)->getIntensity(),pow((*it)->getColor().z,2.2f)*(*it)->getIntensity(),(*it)->getIntensity()));
				//params.setParam( "vSpotLightColor", i, &XMFLOAT4((*it)->getColor().x*(*it)->getIntensity(),(*it)->getColor().y*(*it)->getIntensity(),(*it)->getColor().z*(*it)->getIntensity(),(*it)->getIntensity()));
				//params.setParam( "SpotLightViewProjection", i, &XMMatrixTranspose(XMLoadFloat4x4(&(*it)->getProjectionCamera()->getViewProjectionMatrix())) );
				params.setParam( "SpotLightViewProjection", i, &(*it)->getProjectionCamera()->getViewProjectionMatrix().transpose() );
				params.setParam( "iSpotLightShadowEnabled", i, (int)( (*it)->isCastingShadow() ? 1 : 0 ) );
			
				m_pRenderSystem->t_shaderparams += paraminit.getMiliseconds();

				if (i==0)
				{
					command->setTexture( "txShadowmap", (DX11Texture2D*)(*it)->getShadowmap() );
					command->setTexture( "txCookie", (DX11Texture2D*)(*it)->getCookie() );
				}
				else
				{
					command->setTexture( "txShadowmap2", (DX11Texture2D*)(*it)->getShadowmap() );
					command->setTexture( "txCookie2", (DX11Texture2D*)(*it)->getCookie() );
				}
				i++;
				spot++;
				if (i>=2)
				break;
			}
		}

		m_pRenderSystem->t_shaderparams += paraminit.getMiliseconds();

		command->setShaderParams( &params );
		command->setShaderset( (D3D11Shaderset*)m_pShader[0] );

		if(pass>1)
		{
			command->setBlendState(m_pAdditiveBlendState);
			command->setRasterizerState(m_pAdditiveRasterizerState);
			command->setDepthStencilState(m_pAdditiveDepthStencilState);
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
DX11Material_BlinnPhong::DX11Material_BlinnPhong( RenderSystem* pRenderSystem )	:	Material( pRenderSystem )/*, m_SpecIntensity(1.0), m_SpecPower(2048.0f)*/
{
	m_pDiffuse = NULL;
	m_pNormal = NULL;
	m_pSpecular = NULL;

	// TODO : different light configurations
	m_pShaderNoLight = m_pRenderSystem->loadShaderset( L"Shaders/Black.hlsl", "VS", "PS", SM_5_0 );
	m_pShader[0] = m_pRenderSystem->loadShaderset( L"Shaders/BlinnPhong.hlsl", "VS", "PS", SM_5_0 );
	m_pShader[1] = m_pRenderSystem->loadShaderset( L"Shaders/BlinnPhong.hlsl", "VS", "PS", SM_5_0 );
	m_pShader[2] = m_pRenderSystem->loadShaderset( L"Shaders/BlinnPhong.hlsl", "VS", "PS", SM_5_0 );
}

UINT DX11Material_BlinnPhong::bind(Renderer* pRenderer, RenderCommand* pRenderCommands[], SubmeshRenderData* pRenderData, Transform* pTransform )
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
		command->setBlendState(m_pDefaultBlendState);
		command->setDepthStencilState(m_pDefaultDepthStencilState);
		command->setRasterizerState(m_pDefaultRasterizerState);

		command->setTexture( "txDiffuse", (DX11Texture2D*)m_pDiffuse );
		command->setTexture( "txNormal", (DX11Texture2D*)m_pNormal );
		command->setTexture( "txSpecular", (DX11Texture2D*)m_pSpecular );

		// No affecting lights, simply use the default ambient/black shader
		if ( pSpotLights->empty() && pPointLights->empty() )
		{
			ShaderParams params2;
			params2.initialize( m_pShaderNoLight );
			params2.setParam( "ViewProjection", 0, &m_pRenderSystem->getCamera()->getViewProjectionMatrix().transpose() );
			params2.setParam( "World", 0, &pTransform->getMatrix().transpose() );
			params2.setParam( "vMeshColor", 0, &XMFLOAT4( 0.0f, 0.0f, 0.0f, 0.0f ) );
			command->setShaderset( (D3D11Shaderset*)m_pShaderNoLight );
			command->setShaderParams( &params2 );
			return 1;
		}

		Timer paraminit;
		// Set common shader parameters
		ShaderParams params;
		params.initialize( m_pShader[0] );
		//m_pRenderSystem->t_shaderparams += paraminit.getMiliseconds();

		params.setParam( "ViewProjection", 0, &m_pRenderSystem->getCamera()->getViewProjectionMatrix().transpose() );
		params.setParam( "NormalMatrix", 0, &pTransform->getMatrix().transpose() ) ;
		params.setParam( "World", 0, &pTransform->getMatrix().transpose() );

		params.setParam( "SpecularParams", 0, &XMFLOAT4( m_SpecIntensity, m_SpecPower, 0.0f ,0.0f) );
		params.setParam( "vEyePos", 0, &XMFLOAT4(m_pRenderSystem->getCamera()->getPosition().x,m_pRenderSystem->getCamera()->getPosition().y,m_pRenderSystem->getCamera()->getPosition().z,0.0f) );

		m_pRenderSystem->t_shaderparams += paraminit.getMiliseconds();

		// PointLight-specific shader params
		if (!pPointLights->empty()&&(omni<numOmni))
		{
			int i = 0;
			for (std::vector<PointLight*>::const_iterator it = pPointLights->begin() + omni; it != pPointLights->end(); ++it)
			{
				paraminit.start();

				params.setParam( "vPointLightPos", i, &XMFLOAT4((*it)->transform()->getPosition().x,(*it)->transform()->getPosition().y,(*it)->transform()->getPosition().z,(*it)->getRadius()));
				params.setParam( "vPointLightColor", i, &XMFLOAT4(pow((*it)->getColor().x,2.2f)*(*it)->getIntensity(),pow((*it)->getColor().y,2.2f)*(*it)->getIntensity(),pow((*it)->getColor().z,2.2f)*(*it)->getIntensity(),(*it)->getIntensity()));
				//params.setParam( "vPointLightColor", i, &XMFLOAT4((*it)->getColor().x*(*it)->getIntensity(),(*it)->getColor().y*(*it)->getIntensity(),(*it)->getColor().z*(*it)->getIntensity(),(*it)->getIntensity()));
				
				m_pRenderSystem->t_shaderparams += paraminit.getMiliseconds();

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
				paraminit.start();

				params.setParam( "vSpotLightPos", i, &XMFLOAT4((*it)->transform()->getPosition().x,(*it)->transform()->getPosition().y,(*it)->transform()->getPosition().z,(*it)->getRadius()));
				params.setParam( "vSpotLightColor", i, &XMFLOAT4(pow((*it)->getColor().x,2.2f)*(*it)->getIntensity(),pow((*it)->getColor().y,2.2f)*(*it)->getIntensity(),pow((*it)->getColor().z,2.2f)*(*it)->getIntensity(),(*it)->getIntensity()));
				//params.setParam( "vSpotLightColor", i, &XMFLOAT4((*it)->getColor().x*(*it)->getIntensity(),(*it)->getColor().y*(*it)->getIntensity(),(*it)->getColor().z*(*it)->getIntensity(),(*it)->getIntensity()));
				//params.setParam( "SpotLightViewProjection", i, &XMMatrixTranspose(XMLoadFloat4x4(&(*it)->getProjectionCamera()->getViewProjectionMatrix())) );
				params.setParam( "SpotLightViewProjection", i, &(*it)->getProjectionCamera()->getViewProjectionMatrix().transpose() );
				params.setParam( "iSpotLightShadowEnabled", i, (int)( (*it)->isCastingShadow() ? 1 : 0 ) );
	
				m_pRenderSystem->t_shaderparams += paraminit.getMiliseconds();

				if (i==0)
				{
					command->setTexture( "txShadowmap", (DX11Texture2D*)(*it)->getShadowmap() );
					command->setTexture( "txCookie", (DX11Texture2D*)(*it)->getCookie() );
				}
				else
				{
					command->setTexture( "txShadowmap2", (DX11Texture2D*)(*it)->getShadowmap() );
					command->setTexture( "txCookie2", (DX11Texture2D*)(*it)->getCookie() );
				}
				i++;
				spot++;
				if (i>=2)
					break;
			}
		}

		//m_pRenderSystem->t_shaderparams += paraminit.getMiliseconds();

		command->setShaderset( (D3D11Shaderset*)m_pShader[0] );
		command->setShaderParams( &params );

		if(pass>1)
		{
			command->setBlendState(m_pAdditiveBlendState);
			command->setRasterizerState(m_pAdditiveRasterizerState);
			command->setDepthStencilState(m_pAdditiveDepthStencilState);
		}

		if (omni>=numOmni && spot>=numSpot)
			break;
	}

	// Return number of passes
	return pass;
}

/////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////
/*
UINT DX11Material_DiffuseDetailbump::bind(Renderer* pRenderer, RenderCommand* pRenderCommands[], SubmeshRenderData* pRenderData, Transform* pTransform )
{
	std::vector<PointLight*>* pPointLights = &pRenderData->AffectingPointLights;
	std::vector<SpotLight*>* pSpotLights = &pRenderData->AffectingSpotLights;

	int numOmni = pPointLights->size();
	int numSpot = pSpotLights->size();

	int pass = 0, omni = 0, spot = 0;

	while(1)
	{
	pass++;
	DX11RenderCommand_Draw* rc = dynamic_cast<DX11RenderCommand_Draw*>(pRenderCommands[pass-1]);

	rc->setBlendState(m_pDefaultBlendState);
	rc->setDepthStencilState(m_pDefaultDepthStencilState);
	rc->setRasterizerState(m_pDefaultRasterizerState);

	rc->setTexture( "txDiffuse", (DX11Texture2D*)m_pDiffuse );
	rc->setTexture( "txNormal", (DX11Texture2D*)m_pNormal );
	rc->setTexture( "txDetailNormal", (DX11Texture2D*)m_pDetailNormal );

	ShaderParamBlock paramBlock;

	paramBlock.clear();	// <------ takes too long WTF
	paramBlock.assign( "DetailNormalStrength", m_DetailNormalStrength );
	paramBlock.assign( "DetailTiling", m_DetailTiling );
	paramBlock.assign( "SpecularParams", 0, &XMFLOAT4( m_SpecIntensity, m_SpecPower, 0.0f ,0.0f) );

	paramBlock.assign( "ViewProjection", 0, &XMMatrixTranspose(m_pRenderSystem->getCamera()->getView()*m_pRenderSystem->getCamera()->getProjection()) );
	paramBlock.assign( "vEyePos", 0, &XMFLOAT4(m_pRenderSystem->getCamera()->getPosition().x,m_pRenderSystem->getCamera()->getPosition().y,m_pRenderSystem->getCamera()->getPosition().z,0.0f) );
	paramBlock.assign( "NormalMatrix", 0, &pTransform->getMatrix().transpose() ) ;
	paramBlock.assign( "World", 0, &pTransform->getMatrix().transpose() );
	
	if ( pSpotLights->empty() && pPointLights->empty() )
	{
		rc->setShaderset( (D3D11Shaderset*)m_pShaderNoLight );
		break;
	}
	
	if (!pPointLights->empty()&&(omni<numOmni))
	{
		int i = 0;
		for (std::vector<PointLight*>::const_iterator it = pPointLights->begin() + omni; it != pPointLights->end(); ++it)
		{
			paramBlock.assign( "vPointLightPos", i, &XMFLOAT4((*it)->transform()->getPosition().x,(*it)->transform()->getPosition().y,(*it)->transform()->getPosition().z,(*it)->getRadius()));
			paramBlock.assign( "vPointLightColor", i, &XMFLOAT4(pow((*it)->getColor().x,2.2f)*(*it)->getIntensity(),pow((*it)->getColor().y,2.2f)*(*it)->getIntensity(),pow((*it)->getColor().z,2.2f)*(*it)->getIntensity(),(*it)->getIntensity()));
			//rc->shaderParams()->assign( "vPointLightColor", i, &XMFLOAT4((*it)->getColor().x*(*it)->getIntensity(),(*it)->getColor().y*(*it)->getIntensity(),(*it)->getColor().z*(*it)->getIntensity(),(*it)->getIntensity()));
			i++;
			omni++;
			if (i>=2)
				break;
		}
	}

	if (!pSpotLights->empty()&&(spot<numSpot))
	{
		int i = 0;
		for (std::vector<SpotLight*>::const_iterator it = pSpotLights->begin() + spot; it != pSpotLights->end(); ++it)
		{
			paramBlock.assign( "vSpotLightPos", i, &XMFLOAT4((*it)->transform()->getPosition().x,(*it)->transform()->getPosition().y,(*it)->transform()->getPosition().z,(*it)->getRadius()));
			paramBlock.assign( "vSpotLightColor", i, &XMFLOAT4(pow((*it)->getColor().x,2.2f)*(*it)->getIntensity(),pow((*it)->getColor().y,2.2f)*(*it)->getIntensity(),pow((*it)->getColor().z,2.2f)*(*it)->getIntensity(),(*it)->getIntensity()));
			//paramBlock.assign( "vSpotLightColor", i, &XMFLOAT4((*it)->getColor().x*(*it)->getIntensity(),(*it)->getColor().y*(*it)->getIntensity(),(*it)->getColor().z*(*it)->getIntensity(),(*it)->getIntensity()));
			paramBlock.assign( "SpotLightViewProjection", i, &XMMatrixTranspose(XMLoadFloat4x4(&(*it)->getProjectionCamera()->getViewProjectionMatrix())) );
			paramBlock.assign( "iSpotLightShadowEnabled", i, (int)( (*it)->isCastingShadow() ? 1 : 0 ) );
			
			if (i==0)
			{
				rc->setTexture( "txShadowmap", (DX11Texture2D*)(*it)->getShadowmap() );
				rc->setTexture( "txCookie", (DX11Texture2D*)(*it)->getCookie() );
			}
			else
			{
				rc->setTexture( "txShadowmap2", (DX11Texture2D*)(*it)->getShadowmap() );
				rc->setTexture( "txCookie2", (DX11Texture2D*)(*it)->getCookie() );
			}
			i++;
			spot++;
			if (i>=2)
			break;
		}
	}

	Timer testTimer;
	ShaderParams params;
	params.initialize( m_pShader[0] );
	params.assign( &paramBlock );
	m_pRenderSystem->t_shaderparams += testTimer.getMiliseconds();
	
	rc->setShaderset( (D3D11Shaderset*)m_pShader[0] );
	rc->setShaderParams( &params );

	if(pass>1)
	{
		rc->setBlendState(m_pAdditiveBlendState);
		rc->setRasterizerState(m_pAdditiveRasterizerState);
		rc->setDepthStencilState(m_pAdditiveDepthStencilState);
	}

	if (omni>=numOmni && spot>=numSpot)
		break;
	}
	
	return pass;
}*/

DX11Material_Deferred::DX11Material_Deferred( RenderSystem* pRenderSystem )	:	Material( pRenderSystem )/*, m_SpecIntensity(1.0), m_SpecPower(2048.0f)*/
{
	m_pDiffuse = NULL;
	m_pNormal = NULL;
	m_pSpecular = NULL;
	m_SpecIntensity = 1.0f;
	m_Glossiness = 2048.0f;

	m_pShader[0] = m_pRenderSystem->loadShaderset( L"Shaders/DeferredGBuffer.hlsl", "VS", "PS", SM_5_0 );
}

UINT DX11Material_Deferred::bind(Renderer* pRenderer, RenderCommand* pRenderCommands[], SubmeshRenderData* pRenderData, Transform* pTransform )
{
	DX11RenderCommand_Draw* command = dynamic_cast<DX11RenderCommand_Draw*>(pRenderCommands[0]);

	// Set render states & textures
	command->setBlendState(m_pDefaultBlendState);
	command->setDepthStencilState(m_pDefaultDepthStencilState);
	command->setRasterizerState(m_pDefaultRasterizerState);

	command->setTexture( "txDiffuse", (DX11Texture2D*)m_pDiffuse );
	command->setTexture( "txNormal", (DX11Texture2D*)m_pNormal );
	command->setTexture( "txSpecular", (DX11Texture2D*)m_pSpecular );

	Timer paraminit;
	// Set common shader parameters
	ShaderParams params;
	params.initialize( m_pShader[0] );
	//m_pRenderSystem->t_shaderparams += paraminit.getMiliseconds();
	params.setParam( "View", 0, &m_pRenderSystem->getCamera()->getViewMatrix().transpose() );
	params.setParam( "ViewProjection", 0, &m_pRenderSystem->getCamera()->getViewProjectionMatrix().transpose() );
	params.setParam( "NormalMatrix", 0, &pTransform->getMatrix().transpose() ) ;
	params.setParam( "World", 0, &pTransform->getMatrix().transpose() );
	params.setParam( "fFarPlane", 0, m_pRenderSystem->getCamera()->getFarPlane() );

	params.setParam( "SpecularParams", 0, &XMFLOAT4( m_SpecIntensity, m_Glossiness, 0.0f ,0.0f) );
	params.setParam( "vEyePos", 0, &XMFLOAT4(m_pRenderSystem->getCamera()->getPosition().x,m_pRenderSystem->getCamera()->getPosition().y,m_pRenderSystem->getCamera()->getPosition().z,0.0f) );

	m_pRenderSystem->t_shaderparams += paraminit.getMiliseconds();

	command->setShaderset( (D3D11Shaderset*)m_pShader[0] );
	command->setShaderParams( &params );

	return 1;
}

DX11Material_DeferredIBL::DX11Material_DeferredIBL( RenderSystem* pRenderSystem )	:	DX11Material_Deferred( pRenderSystem )/*, m_SpecIntensity(1.0), m_SpecPower(2048.0f)*/
{
	m_pShader[0] = m_pRenderSystem->loadShaderset( L"Shaders/DeferredGBufferIBL.hlsl", "VS", "PS", SM_5_0, false );
}

UINT DX11Material_DeferredIBL::bind(Renderer* pRenderer, RenderCommand* pRenderCommands[], SubmeshRenderData* pRenderData, Transform* pTransform )
{
	DX11RenderCommand_Draw* command = dynamic_cast<DX11RenderCommand_Draw*>(pRenderCommands[0]);

	// Set render states & textures
	command->setBlendState(m_pDefaultBlendState);
	command->setDepthStencilState(m_pDefaultDepthStencilState);
	command->setRasterizerState(m_pDefaultRasterizerState);

	command->setTexture( "txDiffuse", (DX11Texture2D*)m_pDiffuse );
	command->setTexture( "txNormal", (DX11Texture2D*)m_pNormal );
	command->setTexture( "txSpecular", (DX11Texture2D*)m_pSpecular );
	command->setTexture( "txAO", (DX11Texture2D*)m_pAO );
	command->setTexture( "txIBL", dynamic_cast<DX11Texture2D*>(m_pIBL) );

	Timer paraminit;
	// Set common shader parameters
	ShaderParams params;
	params.initialize( m_pShader[0] );
	//m_pRenderSystem->t_shaderparams += paraminit.getMiliseconds();
	params.setParam( "View", 0, &m_pRenderSystem->getCamera()->getViewMatrix().transpose() );
	params.setParam( "ViewProjection", 0, &m_pRenderSystem->getCamera()->getViewProjectionMatrix().transpose() );
	params.setParam( "NormalMatrix", 0, &pTransform->getMatrix().transpose() ) ;
	params.setParam( "World", 0, &pTransform->getMatrix().transpose() );
	params.setParam( "fFarPlane", 0, m_pRenderSystem->getCamera()->getFarPlane() );

	params.setParam( "SpecularParams", 0, &XMFLOAT4( m_SpecIntensity, m_Glossiness, 0.0f ,0.0f) );
	params.setParam( "vEyePos", 0, &XMFLOAT4(m_pRenderSystem->getCamera()->getPosition().x,m_pRenderSystem->getCamera()->getPosition().y,m_pRenderSystem->getCamera()->getPosition().z,0.0f) );

	m_pRenderSystem->t_shaderparams += paraminit.getMiliseconds();

	command->setShaderset( (D3D11Shaderset*)m_pShader[0] );
	command->setShaderParams( &params );

	return 1;
}

DX11Material_Spotlight::DX11Material_Spotlight( RenderSystem* pRenderSystem ) :	Material( pRenderSystem )
{
	m_pShadowmap = NULL;
	m_pGbuffer[0] = NULL;
	m_pGbuffer[1] = NULL;
	m_pGbuffer[2] = NULL;

	m_pShader = pRenderSystem->loadShaderset( L"Shaders/DeferredSpotlight.hlsl", "VS", "PS", SM_5_0 );

	// Render states
	D3D11_RASTERIZER_DESC desc2;
	desc2.FillMode = D3D11_FILL_SOLID;
	desc2.CullMode = D3D11_CULL_FRONT;
	desc2.FrontCounterClockwise = true;
	desc2.DepthBias = 0;
	desc2.SlopeScaledDepthBias = 0.0f;
	desc2.DepthBiasClamp = 0.0f;
	desc2.DepthClipEnable = true;
	desc2.ScissorEnable = false;
	desc2.MultisampleEnable = true;
	desc2.AntialiasedLineEnable = false;
	((DX11RenderDispatcher*)m_pRenderSystem->getRenderDispatcher())->getDevice()->CreateRasterizerState( &desc2, &m_pBackfaceRasterizerState );

	D3D11_DEPTH_STENCIL_DESC desc3;
	desc3.DepthEnable = true;
	desc3.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	desc3.DepthFunc = D3D11_COMPARISON_GREATER;
	desc3.StencilEnable = false;
	((DX11RenderDispatcher*)m_pRenderSystem->getRenderDispatcher())->getDevice()->CreateDepthStencilState( &desc3, &m_pBackfaceDepthStencilState );
}

UINT DX11Material_Spotlight::bind(Renderer* pRenderer, RenderCommand* pRenderCommands[], SubmeshRenderData* pRenderData, Transform* pTransform )
{
	DX11RenderCommand_Draw* command = dynamic_cast<DX11RenderCommand_Draw*>(pRenderCommands[0]);

	// Set render states & textures
	command->setBlendState(m_pAdditiveBlendState);
	command->setDepthStencilState(m_pBackfaceDepthStencilState);
	command->setRasterizerState(m_pBackfaceRasterizerState);
	/*
	command->setBlendState(m_pDefaultBlendState);
	command->setDepthStencilState(m_pDefaultDepthStencilState);
	command->setRasterizerState(m_pDefaultRasterizerState);*/

	command->setTexture( "txGBuffer0", (DX11Texture2D*)m_pGbuffer[0] );
	command->setTexture( "txGBuffer1", (DX11Texture2D*)m_pGbuffer[1] );
	command->setTexture( "txGBuffer2", (DX11Texture2D*)m_pGbuffer[2] );
	command->setTexture( "txShadowmap", (DX11Texture2D*)m_pShadowmap );
	command->setTexture( "txCookie", (DX11Texture2D*)m_pCookie );

	Timer paraminit;
	// Set common shader parameters
	ShaderParams params;
	params.initialize( m_pShader );
	//m_pRenderSystem->t_shaderparams += paraminit.getMiliseconds();
	RenderSystemConfig config = m_pRenderSystem->getConfig();
	params.setParam( "View", 0, &m_pRenderSystem->getCamera()->getViewMatrix().transpose() );
	params.setParam( "ViewProjection", 0, &m_pRenderSystem->getCamera()->getViewProjectionMatrix().transpose() );
	params.setParam( "InvView", 0, &m_pRenderSystem->getCamera()->getViewMatrix().inverse().transpose() );
	params.setParam( "InvProjection", 0, &m_pRenderSystem->getCamera()->getProjectionMatrix().inverse().transpose() );
	params.setParam( "InvViewProjection", 0, &m_pRenderSystem->getCamera()->getViewProjectionMatrix().inverse().transpose() );
	params.setParam( "World", 0, &pTransform->getMatrix().transpose() );
	params.setParam( "InvSpotlightProjection", 0, &m_InvProjection.transpose() );
	params.setParam( "SpotlightViewProjection", 0, &m_ViewProjection.transpose() );
	params.setParam( "SpotlightView", 0, &m_View.transpose() );
	params.setParam( "vSpotLightColor", &XMFLOAT4(pow(m_Color.x,2.2f)*m_Intensity,pow(m_Color.y,2.2f)*m_Intensity,pow(m_Color.z,2.2f)*m_Intensity,m_Intensity) );
	params.setParam( "vEyePos", &XMFLOAT4(m_pRenderSystem->getCamera()->getPosition().x,m_pRenderSystem->getCamera()->getPosition().y,m_pRenderSystem->getCamera()->getPosition().z,1.0f) );
	params.setParam( "ScreenDimensions", &XMFLOAT4(config.Width,config.Height,0.0f,0.0f) );
	params.setParam( "SpotLightRadius", m_Radius );
	params.setParam( "fFarPlane", m_pRenderSystem->getCamera()->getFarPlane() );

	m_pRenderSystem->t_shaderparams += paraminit.getMiliseconds();

	command->setShaderset( (D3D11Shaderset*)m_pShader );
	command->setShaderParams( &params );

	return 1;
}


DX11Material_Pointlight::DX11Material_Pointlight( RenderSystem* pRenderSystem ) :	Material( pRenderSystem )
{
	m_pShadowmap = NULL;
	m_pGbuffer[0] = NULL;
	m_pGbuffer[1] = NULL;
	m_pGbuffer[2] = NULL;

	m_pShader = pRenderSystem->loadShaderset( L"Shaders/DeferredPointlight.hlsl", "VS", "PS", SM_5_0 );

	// Render states
	D3D11_RASTERIZER_DESC desc2;
	desc2.FillMode = D3D11_FILL_SOLID;
	desc2.CullMode = D3D11_CULL_FRONT;
	desc2.FrontCounterClockwise = true;
	desc2.DepthBias = 0;
	desc2.SlopeScaledDepthBias = 0.0f;
	desc2.DepthBiasClamp = 0.0f;
	desc2.DepthClipEnable = true;
	desc2.ScissorEnable = false;
	desc2.MultisampleEnable = true;
	desc2.AntialiasedLineEnable = false;
	((DX11RenderDispatcher*)m_pRenderSystem->getRenderDispatcher())->getDevice()->CreateRasterizerState( &desc2, &m_pBackfaceRasterizerState );

	D3D11_DEPTH_STENCIL_DESC desc3;
	desc3.DepthEnable = true;
	desc3.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	desc3.DepthFunc = D3D11_COMPARISON_GREATER;
	desc3.StencilEnable = false;
	((DX11RenderDispatcher*)m_pRenderSystem->getRenderDispatcher())->getDevice()->CreateDepthStencilState( &desc3, &m_pBackfaceDepthStencilState );
}

UINT DX11Material_Pointlight::bind(Renderer* pRenderer, RenderCommand* pRenderCommands[], SubmeshRenderData* pRenderData, Transform* pTransform )
{
	DX11RenderCommand_Draw* command = dynamic_cast<DX11RenderCommand_Draw*>(pRenderCommands[0]);

	// Set render states & textures
	command->setBlendState(m_pAdditiveBlendState);
	command->setDepthStencilState(m_pBackfaceDepthStencilState);
	command->setRasterizerState(m_pBackfaceRasterizerState);

	command->setTexture( "txGBuffer0", (DX11Texture2D*)m_pGbuffer[0] );
	command->setTexture( "txGBuffer1", (DX11Texture2D*)m_pGbuffer[1] );
	command->setTexture( "txGBuffer2", (DX11Texture2D*)m_pGbuffer[2] );
	command->setTexture( "txShadowmap", (DX11Texture2D*)m_pShadowmap );
	command->setTexture( "txCookie", (DX11Texture2D*)m_pCookie );

	Timer paraminit;
	// Set common shader parameters
	ShaderParams params;
	params.initialize( m_pShader );
	//m_pRenderSystem->t_shaderparams += paraminit.getMiliseconds();
	RenderSystemConfig config = m_pRenderSystem->getConfig();
	params.setParam( "ViewProjection", 0, &m_pRenderSystem->getCamera()->getViewProjectionMatrix().transpose() );
	params.setParam( "InvProjection", 0, &m_pRenderSystem->getCamera()->getProjectionMatrix().inverse().transpose() );
	params.setParam( "InvViewProjection", 0, &m_pRenderSystem->getCamera()->getViewProjectionMatrix().inverse().transpose() );
	params.setParam( "World", 0, &pTransform->getMatrix().transpose() );
	params.setParam( "vPointLightColor", &XMFLOAT4(pow(m_Color.x,2.2f)*m_Intensity,pow(m_Color.y,2.2f)*m_Intensity,pow(m_Color.z,2.2f)*m_Intensity,m_Intensity) );
	params.setParam( "vEyePos", &XMFLOAT4(m_pRenderSystem->getCamera()->getPosition().x,m_pRenderSystem->getCamera()->getPosition().y,m_pRenderSystem->getCamera()->getPosition().z,1.0f) );
	params.setParam( "ScreenDimensions", &XMFLOAT4(config.Width,config.Height,0.0f,0.0f) );
	params.setParam( "PointLightRadius", m_Radius );

	m_pRenderSystem->t_shaderparams += paraminit.getMiliseconds();

	command->setShaderset( (D3D11Shaderset*)m_pShader );
	command->setShaderParams( &params );

	return 1;
}

//
// DX11Material_Water
//
DX11Material_Water::DX11Material_Water( RenderSystem* pRenderSystem )	:	Material( pRenderSystem ), m_SpecIntensity(1.0), m_SpecPower(2048.0f)
{
	// Load the shader
	m_pShader = m_pRenderSystem->loadShaderset( L"Shaders/Water.hlsl", "VS", "PS", SM_5_0 );
}


UINT DX11Material_Water::bind(Renderer* pRenderer, RenderCommand* pRenderCommands[], SubmeshRenderData* pRenderData, Transform* pTransform )
{
	DX11RenderCommand_Draw* command = dynamic_cast<DX11RenderCommand_Draw*>(pRenderCommands[0]);

	// Set render states & textures
	command->setBlendState(m_pDefaultBlendState);
	command->setDepthStencilState(m_pDefaultDepthStencilState);
	command->setRasterizerState(m_pDefaultRasterizerState);

	command->setTexture( "txDiffuse", (DX11Texture2D*)m_pDiffuse );
	command->setTexture( "txNormal", (DX11Texture2D*)m_pNormal );
	command->setTexture( "txFoam", (DX11Texture2D*)m_pFoam );
	command->setTexture( "txCubemap", (DX11Texture2D*)m_pIBL );
	//command->setTexture( "txSpecular", (DX11Texture2D*)m_pSpecular );

	Timer paraminit;
	// Set common shader parameters
	ShaderParams params;
	params.initialize( m_pShader );
	//m_pRenderSystem->t_shaderparams += paraminit.getMiliseconds();

	params.setParam( "View", 0, &m_pRenderSystem->getCamera()->getViewMatrix().transpose() );
	params.setParam( "ViewProjection", 0, &m_pRenderSystem->getCamera()->getViewProjectionMatrix().transpose() );
	params.setParam( "NormalMatrix", 0, &pTransform->getMatrix().transpose() ) ;
	params.setParam( "World", 0, &pTransform->getMatrix().transpose() );

	//params.setParam( "SpecularParams", 0, &XMFLOAT4( m_SpecIntensity, m_Glossiness, 0.0f ,0.0f) );
	params.setParam( "vEyePos", 0, &XMFLOAT4(m_pRenderSystem->getCamera()->getPosition().x,m_pRenderSystem->getCamera()->getPosition().y,m_pRenderSystem->getCamera()->getPosition().z,0.0f) );
	params.setParam( "time", 0, pRenderer->getTime() );
	params.setParam( "fFarPlane", 0, m_pRenderSystem->getCamera()->getFarPlane() );

	m_pRenderSystem->t_shaderparams += paraminit.getMiliseconds();

	command->setShaderset( (D3D11Shaderset*)m_pShader );
	command->setShaderParams( &params );

	return 1;
}


DX11Material_Skybox::DX11Material_Skybox( RenderSystem* pRenderSystem )	:	Material( pRenderSystem )
{
	m_pCubemap = NULL;
	m_pShader = m_pRenderSystem->loadShaderset( L"Shaders/Skybox.hlsl", "VS", "PS", SM_5_0 );
}

UINT DX11Material_Skybox::bind(Renderer* pRenderer, RenderCommand* pRenderCommands[], SubmeshRenderData* pRenderData, Transform* pTransform )
{
	DX11RenderCommand_Draw* command = dynamic_cast<DX11RenderCommand_Draw*>(pRenderCommands[0]);

	// Set render states & textures
	command->setBlendState(m_pDefaultBlendState);
	command->setDepthStencilState(m_pDefaultDepthStencilState);
	command->setRasterizerState(m_pDefaultRasterizerState);

	command->setTexture( "txCubemap", (DX11Texture2D*)m_pCubemap );

	Timer paraminit;
	// Set common shader parameters
	ShaderParams params;
	params.initialize( m_pShader );
	//m_pRenderSystem->t_shaderparams += paraminit.getMiliseconds();

	params.setParam( "ViewProjection", 0, &m_pRenderSystem->getCamera()->getViewProjectionMatrix().transpose() );
	params.setParam( "World", 0, &pTransform->getMatrix().transpose() );

	m_pRenderSystem->t_shaderparams += paraminit.getMiliseconds();

	command->setShaderset( (D3D11Shaderset*)m_pShader );
	command->setShaderParams( &params );

	return 1;
}