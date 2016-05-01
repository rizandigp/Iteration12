#pragma once

#include "Material.h"
#include "DX11/DX11RenderDispatcher.h"

Material::Material( RenderSystem* pRenderSystem ) : m_RenderState(), m_BlendState()
{
	m_pRenderSystem = pRenderSystem;
	m_NumPasses = 0;
}
/*
Material_DiffuseBump::Material_DiffuseBump( RenderSystem* pRenderSystem )	:	Material( pRenderSystem ), m_SpecIntensity(0.12f), m_SpecPower(13.0f)
{
	m_pDiffuse = NULL;
	m_pNormal = NULL;


	m_pShaderNoLight = m_pRenderSystem->LoadShaderset( L"Shaders/OneColor.hlsl", "VS", "PS", SM_5_0, false );
	m_pShader[0] = m_pRenderSystem->LoadShaderset( L"Shaders/DiffBump.hlsl", "VS", "PS", SM_5_0, false );
	m_pShader[1] = m_pRenderSystem->LoadShaderset( L"Shaders/DiffBump_omni.hlsl", "VS", "PS", SM_5_0, false );
	m_pShader[2] = m_pRenderSystem->LoadShaderset( L"Shaders/DiffBump_spot.hlsl", "VS", "PS", SM_5_0, false );
}



Material_DiffuseDetailbump::Material_DiffuseDetailbump( RenderSystem* pRenderSystem )	:	Material_DiffuseBump( pRenderSystem )
{
	m_DetailNormalStrength = 1.0f;
	m_DetailTiling = 10.0f;
	m_pShader[0] = m_pRenderSystem->LoadShaderset( L"Shaders/DiffDetailbump.hlsl", "VS", "PS", SM_5_0, false );
	m_pShader[1] = m_pRenderSystem->LoadShaderset( L"Shaders/DiffDetailbump_omni.hlsl", "VS", "PS", SM_5_0, false );
	m_pShader[2] = m_pRenderSystem->LoadShaderset( L"Shaders/DiffDetailbump_spot.hlsl", "VS", "PS", SM_5_0, false );
}

Material_DiffuseBumpSpecular::Material_DiffuseBumpSpecular( RenderSystem* pRenderSystem )	:	Material_DiffuseBump( pRenderSystem )
{
	m_pShader[0] = m_pRenderSystem->LoadShaderset( L"Shaders/DiffBumpSpec.hlsl", "VS", "PS", SM_5_0, false );
	m_pShader[1] = m_pRenderSystem->LoadShaderset( L"Shaders/DiffBumpSpec_omni.hlsl", "VS", "PS", SM_5_0, false );
	m_pShader[2] = m_pRenderSystem->LoadShaderset( L"Shaders/DiffBumpSpec_spot.hlsl", "VS", "PS", SM_5_0, false );
}

Material_Diffuse::Material_Diffuse( RenderSystem* pRenderSystem )	:	Material( pRenderSystem ), m_SpecIntensity(0.15f), m_SpecPower(15.0f)
{
	m_pDiffuse = NULL;

	m_pBlendState = NULL;
	m_pDepthStencilState = NULL;
	m_pRasterizerState = NULL;

	m_pShader[0] = m_pRenderSystem->LoadShaderset( L"Shaders/Diff.hlsl", "VS", "PS", SM_5_0, false );
}

Material_BlinnPhong::Material_BlinnPhong( RenderSystem* pRenderSystem )	:	Material( pRenderSystem ), m_SpecIntensity(1.0), m_SpecPower(2048.0f)
{
	m_pDiffuse = NULL;
	m_pNormal = NULL;
	m_pSpecular = NULL;

	m_pShaderNoLight = m_pRenderSystem->LoadShaderset( L"Shaders/Black.hlsl", "VS", "PS", SM_5_0, false );
	m_pShader[0] = m_pRenderSystem->LoadShaderset( L"Shaders/BlinnPhong.hlsl", "VS", "PS", SM_5_0, false );
	m_pShader[1] = m_pRenderSystem->LoadShaderset( L"Shaders/BlinnPhong.hlsl", "VS", "PS", SM_5_0, false );
	m_pShader[2] = m_pRenderSystem->LoadShaderset( L"Shaders/BlinnPhong.hlsl", "VS", "PS", SM_5_0, false );
}


// TODO :	-optimize implementation
//			-decouple from submesh
//
// BOTTLENECK!!!
UINT Material_Diffuse::bind(Renderer* pRenderer, std::vector< D3D11RenderCommand_Draw* >* pRenderCommands, UINT submeshIndex, Transform* pTransform )
{
	std::vector<PointLight*>* pPointLights = pRenderer->GetAffectingPointLights(submeshIndex);
	D3D11RenderCommand_Draw* rc = pRenderCommands->at(0);
	int i = 0;
	
	rc->SetBlendState(m_pDefaultBlendState);
	rc->SetDepthStencilState(m_pDefaultDepthStencilState);
	rc->SetRasterizerState(m_pDefaultRasterizerState);

	rc->shaderParams()->clear();	// <------ takes too long WTF
	
	for (std::vector<PointLight*>::const_iterator it = pPointLights->begin(); it != pPointLights->end(); ++it)
	{
		rc->shaderParams()->assign( "vPointLightPos", i, &XMFLOAT4((*it)->transform()->GetPosition().x,(*it)->transform()->GetPosition().y,(*it)->transform()->GetPosition().z,0.0f));
		rc->shaderParams()->assign( "vPointLightColor", i, &XMFLOAT4((*it)->getColor().x*(*it)->getIntensity(),(*it)->getColor().y*(*it)->getIntensity(),(*it)->getColor().z*(*it)->getIntensity(),(*it)->getIntensity()));
		i++;
	}
	
	rc->shaderParams()->assign( "ViewProjection", 0, &XMMatrixTranspose(m_pRenderSystem->GetCamera()->getView()*m_pRenderSystem->GetCamera()->getProjection()) );
	rc->shaderParams()->assign( "vEyePos", 0, &XMFLOAT4(m_pRenderSystem->GetCamera()->GetPosition().x,m_pRenderSystem->GetCamera()->GetPosition().y,m_pRenderSystem->GetCamera()->GetPosition().z,0.0f) );
	rc->shaderParams()->assign( "NormalMatrix", 0, &pTransform->GetMatrix().transpose() ) ;
	rc->shaderParams()->assign( "World", 0, &XMMatrixTranspose(pTransform->getXMMatrix()) );
	
	return 1;
}

// TODO :	-optimize implementation
//			-decouple from submesh
//
// BOTTLENECK!!!
UINT Material_DiffuseBump::bind(Renderer* pRenderer, std::vector< D3D11RenderCommand_Draw* >* pRenderCommands, UINT submeshIndex, Transform* pTransform )
{
	std::vector<PointLight*>* pPointLights = pRenderer->GetAffectingPointLights(submeshIndex);
	std::vector<SpotLight*>* pSpotLights = pRenderer->GetAffectingSpotLights(submeshIndex);
	D3D11RenderCommand_Draw* rc = pRenderCommands->at(0);
	int i = 0;

	rc->SetBlendState(m_pDefaultBlendState);
	rc->SetDepthStencilState(m_pDefaultDepthStencilState);
	rc->SetRasterizerState(m_pDefaultRasterizerState);

	rc->shaderParams()->clear();	// <------ takes too long WTF
	
	if ( pSpotLights->empty() && pPointLights->empty() )
	{
		rc->shaderParams()->assign( "ViewProjection", 0, &XMMatrixTranspose(m_pRenderSystem->GetCamera()->getView()*m_pRenderSystem->GetCamera()->getProjection()) );
		rc->shaderParams()->assign( "World", 0, &XMMatrixTranspose(pTransform->getXMMatrix()) );
		rc->SetShaderset( (DX11Shaderset*)m_pShaderNoLight );
		return 1;
	}
	else
	{
		rc->shaderParams()->assign( "SpecularParams", 0, &XMFLOAT4( m_SpecIntensity, m_SpecPower, 0.0f ,0.0f) );

		rc->shaderParams()->assign( "ViewProjection", 0, &XMMatrixTranspose(m_pRenderSystem->GetCamera()->getView()*m_pRenderSystem->GetCamera()->getProjection()) );
		rc->shaderParams()->assign( "vEyePos", 0, &XMFLOAT4(m_pRenderSystem->GetCamera()->GetPosition().x,m_pRenderSystem->GetCamera()->GetPosition().y,m_pRenderSystem->GetCamera()->GetPosition().z,0.0f) );
		rc->shaderParams()->assign( "NormalMatrix", 0, &pTransform->GetMatrix().transpose() ) ;
		rc->shaderParams()->assign( "World", 0, &XMMatrixTranspose(pTransform->getXMMatrix()) );
	}

	if (!pPointLights->empty())
	{
		for (std::vector<PointLight*>::const_iterator it = pPointLights->begin(); it != pPointLights->end(); ++it)
		{
			rc->shaderParams()->assign( "vPointLightPos", i, &XMFLOAT4((*it)->transform()->GetPosition().x,(*it)->transform()->GetPosition().y,(*it)->transform()->GetPosition().z,(*it)->getRadius()));
			rc->shaderParams()->assign( "vPointLightColor", i, &XMFLOAT4((*it)->getColor().x*(*it)->getIntensity(),(*it)->getColor().y*(*it)->getIntensity(),(*it)->getColor().z*(*it)->getIntensity(),(*it)->getIntensity()));
			i++;
			if (i>=2)
				break;
		}
		if (pSpotLights->empty())
		{
			rc->SetShaderset( (DX11Shaderset*)m_pShader[1] );
			return 1;
		}
	}

	if (!pSpotLights->empty())
	{
		i = 0;
		for (std::vector<SpotLight*>::const_iterator it = pSpotLights->begin(); it != pSpotLights->end(); ++it)
		{
			rc->shaderParams()->assign( "vSpotLightPos", i, &XMFLOAT4((*it)->transform()->GetPosition().x,(*it)->transform()->GetPosition().y,(*it)->transform()->GetPosition().z,(*it)->getRadius()));
			rc->shaderParams()->assign( "vSpotLightColor", i, &XMFLOAT4((*it)->getColor().x*(*it)->getIntensity(),(*it)->getColor().y*(*it)->getIntensity(),(*it)->getColor().z*(*it)->getIntensity(),(*it)->getIntensity()));
			rc->shaderParams()->assign( "SpotLightViewProjection", i, &XMMatrixTranspose(XMLoadFloat4x4(&(*it)->getViewProjectionMatrix())) );
			rc->shaderParams()->assign( "iSpotLightShadowEnabled", i, (int)( (*it)->isCastingShadow() ? 1 : 0 ) );
			
			if (i==0)
			{
				rc->SetTexture( "txShadowmap", (DX11Texture2D*)(*it)->getShadowmap() );
				rc->SetTexture( "txCookie", (DX11Texture2D*)(*it)->getCookie() );
			}
			else
			{
				rc->SetTexture( "txShadowmap2", (DX11Texture2D*)(*it)->getShadowmap() );
				rc->SetTexture( "txCookie2", (DX11Texture2D*)(*it)->getCookie() );
			}
			i++;
			if (i>=2)
			break;
		}
		if (pPointLights->empty())
		{
			rc->SetShaderset( (DX11Shaderset*)m_pShader[2] );
			return 1;
		}
	}

	rc->SetShaderset( (DX11Shaderset*)m_pShader[0] );

	return 1;
}


// TODO :	-optimize implementation
//			-decouple from submesh
//
// BOTTLENECK!!!
//
UINT Material_DiffuseDetailbump::bind(Renderer* pRenderer, std::vector< D3D11RenderCommand_Draw* >* pRenderCommands, UINT submeshIndex, Transform* pTransform )
{
	std::vector<PointLight*>* pPointLights = pRenderer->GetAffectingPointLights(submeshIndex);
	std::vector<SpotLight*>* pSpotLights = pRenderer->GetAffectingSpotLights(submeshIndex);

	int numOmni = pPointLights->size();
	int numSpot = pSpotLights->size();

	int pass = 0, omni = 0, spot = 0;

	while(1)
	{
	pass++;
	D3D11RenderCommand_Draw* rc = pRenderCommands->at(pass-1);

	rc->SetBlendState(m_pDefaultBlendState);
	rc->SetDepthStencilState(m_pDefaultDepthStencilState);
	rc->SetRasterizerState(m_pDefaultRasterizerState);

	rc->SetTexture( "txDiffuse", (DX11Texture2D*)m_pDiffuse );
	rc->SetTexture( "txNormal", (DX11Texture2D*)m_pNormal );
	rc->SetTexture( "txDetailNormal", (DX11Texture2D*)m_pDetailNormal );

	rc->shaderParams()->clear();	// <------ takes too long WTF
	rc->shaderParams()->assign( "DetailNormalStrength", m_DetailNormalStrength );
	rc->shaderParams()->assign( "DetailTiling", m_DetailTiling );
	rc->shaderParams()->assign( "SpecularParams", 0, &XMFLOAT4( m_SpecIntensity, m_SpecPower, 0.0f ,0.0f) );

	rc->shaderParams()->assign( "ViewProjection", 0, &XMMatrixTranspose(m_pRenderSystem->GetCamera()->getView()*m_pRenderSystem->GetCamera()->getProjection()) );
	rc->shaderParams()->assign( "vEyePos", 0, &XMFLOAT4(m_pRenderSystem->GetCamera()->GetPosition().x,m_pRenderSystem->GetCamera()->GetPosition().y,m_pRenderSystem->GetCamera()->GetPosition().z,0.0f) );
	rc->shaderParams()->assign( "NormalMatrix", 0, &pTransform->GetMatrix().transpose() ) ;
	rc->shaderParams()->assign( "World", 0, &XMMatrixTranspose(pTransform->getXMMatrix()) );
	
	if ( pSpotLights->empty() && pPointLights->empty() )
	{
		rc->SetShaderset( (DX11Shaderset*)m_pShaderNoLight );
		break;
	}
	
	if (!pPointLights->empty()&&(omni<numOmni))
	{
		int i = 0;
		for (std::vector<PointLight*>::const_iterator it = pPointLights->begin() + omni; it != pPointLights->end(); ++it)
		{
			rc->shaderParams()->assign( "vPointLightPos", i, &XMFLOAT4((*it)->transform()->GetPosition().x,(*it)->transform()->GetPosition().y,(*it)->transform()->GetPosition().z,(*it)->getRadius()));
			rc->shaderParams()->assign( "vPointLightColor", i, &XMFLOAT4(pow((*it)->getColor().x,2.2f)*(*it)->getIntensity(),pow((*it)->getColor().y,2.2f)*(*it)->getIntensity(),pow((*it)->getColor().z,2.2f)*(*it)->getIntensity(),(*it)->getIntensity()));
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
			rc->shaderParams()->assign( "vSpotLightPos", i, &XMFLOAT4((*it)->transform()->GetPosition().x,(*it)->transform()->GetPosition().y,(*it)->transform()->GetPosition().z,(*it)->getRadius()));
			rc->shaderParams()->assign( "vSpotLightColor", i, &XMFLOAT4(pow((*it)->getColor().x,2.2f)*(*it)->getIntensity(),pow((*it)->getColor().y,2.2f)*(*it)->getIntensity(),pow((*it)->getColor().z,2.2f)*(*it)->getIntensity(),(*it)->getIntensity()));
			//rc->shaderParams()->assign( "vSpotLightColor", i, &XMFLOAT4((*it)->getColor().x*(*it)->getIntensity(),(*it)->getColor().y*(*it)->getIntensity(),(*it)->getColor().z*(*it)->getIntensity(),(*it)->getIntensity()));
			rc->shaderParams()->assign( "SpotLightViewProjection", i, &XMMatrixTranspose(XMLoadFloat4x4(&(*it)->getViewProjectionMatrix())) );
			rc->shaderParams()->assign( "iSpotLightShadowEnabled", i, (int)( (*it)->isCastingShadow() ? 1 : 0 ) );
			
			if (i==0)
			{
				rc->SetTexture( "txShadowmap", (DX11Texture2D*)(*it)->getShadowmap() );
				rc->SetTexture( "txCookie", (DX11Texture2D*)(*it)->getCookie() );
			}
			else
			{
				rc->SetTexture( "txShadowmap2", (DX11Texture2D*)(*it)->getShadowmap() );
				rc->SetTexture( "txCookie2", (DX11Texture2D*)(*it)->getCookie() );
			}
			i++;
			spot++;
			if (i>=2)
			break;
		}
	}

	rc->SetShaderset( (DX11Shaderset*)m_pShader[0] );

	if(pass>1)
	{
		rc->SetBlendState(m_pAdditiveBlendState);
		rc->SetRasterizerState(m_pAdditiveRasterizerState);
		rc->SetDepthStencilState(m_pAdditiveDepthStencilState);
	}

	if (omni>=numOmni && spot>=numSpot)
		break;
	}

	return pass;
}

UINT Material_DiffuseBumpSpecular::bind(Renderer* pRenderer, std::vector< D3D11RenderCommand_Draw* >* pRenderCommands, UINT submeshIndex, Transform* pTransform )
{
	std::vector<PointLight*>* pPointLights = pRenderer->GetAffectingPointLights(submeshIndex);
	std::vector<SpotLight*>* pSpotLights = pRenderer->GetAffectingSpotLights(submeshIndex);
	D3D11RenderCommand_Draw* rc = pRenderCommands->at(0);
	int i = 0;

	rc->SetBlendState(m_pDefaultBlendState);
	rc->SetDepthStencilState(m_pDefaultDepthStencilState);
	rc->SetRasterizerState(m_pDefaultRasterizerState);

	rc->shaderParams()->clear();	// <------ takes too long WTF

	rc->SetTexture( "txDiffuse", (DX11Texture2D*)m_pDiffuse );
	rc->SetTexture( "txNormal", (DX11Texture2D*)m_pNormal );
	rc->SetTexture( "txSpecular", (DX11Texture2D*)m_pSpecular );
	
	if ( pSpotLights->empty() && pPointLights->empty() )
	{
		rc->shaderParams()->assign( "ViewProjection", 0, &XMMatrixTranspose(m_pRenderSystem->GetCamera()->getView()*m_pRenderSystem->GetCamera()->getProjection()) );
		rc->shaderParams()->assign( "World", 0, &XMMatrixTranspose(pTransform->getXMMatrix()) );
		rc->SetShaderset( (DX11Shaderset*)m_pShaderNoLight );
		return 1;
	}
	else
	{
		rc->shaderParams()->assign( "SpecularParams", 0, &XMFLOAT4( m_SpecIntensity, m_SpecPower, 0.0f ,0.0f) );

		rc->shaderParams()->assign( "ViewProjection", 0, &XMMatrixTranspose(m_pRenderSystem->GetCamera()->getView()*m_pRenderSystem->GetCamera()->getProjection()) );
		rc->shaderParams()->assign( "vEyePos", 0, &XMFLOAT4(m_pRenderSystem->GetCamera()->GetPosition().x,m_pRenderSystem->GetCamera()->GetPosition().y,m_pRenderSystem->GetCamera()->GetPosition().z,0.0f) );
		rc->shaderParams()->assign( "NormalMatrix", 0, &pTransform->GetMatrix().transpose() ) ;
		rc->shaderParams()->assign( "World", 0, &XMMatrixTranspose(pTransform->getXMMatrix()) );
	}

	if (!pPointLights->empty())
	{
		for (std::vector<PointLight*>::const_iterator it = pPointLights->begin(); it != pPointLights->end(); ++it)
		{
			rc->shaderParams()->assign( "vPointLightPos", i, &XMFLOAT4((*it)->transform()->GetPosition().x,(*it)->transform()->GetPosition().y,(*it)->transform()->GetPosition().z,(*it)->getRadius()));
			rc->shaderParams()->assign( "vPointLightColor", i, &XMFLOAT4((*it)->getColor().x*(*it)->getIntensity(),(*it)->getColor().y*(*it)->getIntensity(),(*it)->getColor().z*(*it)->getIntensity(),(*it)->getIntensity()));
			i++;
			if (i>=2)
				break;
		}
		if (pSpotLights->empty())
		{
			rc->SetShaderset( (DX11Shaderset*)m_pShader[1] );
			return 1;
		}
	}

	if (!pSpotLights->empty())
	{
		i = 0;
		for (std::vector<SpotLight*>::const_iterator it = pSpotLights->begin(); it != pSpotLights->end(); ++it)
		{
			rc->shaderParams()->assign( "vSpotLightPos", i, &XMFLOAT4((*it)->transform()->GetPosition().x,(*it)->transform()->GetPosition().y,(*it)->transform()->GetPosition().z,(*it)->getRadius()));
			rc->shaderParams()->assign( "vSpotLightColor", i, &XMFLOAT4((*it)->getColor().x*(*it)->getIntensity(),(*it)->getColor().y*(*it)->getIntensity(),(*it)->getColor().z*(*it)->getIntensity(),(*it)->getIntensity()));
			rc->shaderParams()->assign( "SpotLightViewProjection", i, &XMMatrixTranspose(XMLoadFloat4x4(&(*it)->getViewProjectionMatrix())) );
			rc->shaderParams()->assign( "iSpotLightShadowEnabled", i, (int)( (*it)->isCastingShadow() ? 1 : 0 ) );
			
			if (i==0)
			{
				rc->SetTexture( "txShadowmap", (DX11Texture2D*)(*it)->getShadowmap() );
				rc->SetTexture( "txCookie", (DX11Texture2D*)(*it)->getCookie() );
			}
			else
			{
				rc->SetTexture( "txShadowmap2", (DX11Texture2D*)(*it)->getShadowmap() );
				rc->SetTexture( "txCookie2", (DX11Texture2D*)(*it)->getCookie() );
			}
			i++;
			if (i>=2)
			break;
		}
		if (pPointLights->empty())
		{
			rc->SetShaderset( (DX11Shaderset*)m_pShader[2] );
			return 1;
		}
	}

	rc->SetShaderset( (DX11Shaderset*)m_pShader[0] );

	return 1;
}

UINT Material_BlinnPhong::bind(Renderer* pRenderer, std::vector< D3D11RenderCommand_Draw* >* pRenderCommands, UINT submeshIndex, Transform* pTransform )
{
	std::vector<PointLight*>* pPointLights = pRenderer->GetAffectingPointLights(submeshIndex);
	std::vector<SpotLight*>* pSpotLights = pRenderer->GetAffectingSpotLights(submeshIndex);

	int numOmni = pPointLights->size();
	int numSpot = pSpotLights->size();

	int pass = 0, omni = 0, spot = 0;

	while(1)
	{
	pass++;
	D3D11RenderCommand_Draw* rc = pRenderCommands->at(pass-1);

	rc->SetBlendState(m_pDefaultBlendState);
	rc->SetDepthStencilState(m_pDefaultDepthStencilState);
	rc->SetRasterizerState(m_pDefaultRasterizerState);

	rc->SetTexture( "txDiffuse", (DX11Texture2D*)m_pDiffuse );
	rc->SetTexture( "txNormal", (DX11Texture2D*)m_pNormal );
	rc->SetTexture( "txSpecular", (DX11Texture2D*)m_pSpecular );

	rc->shaderParams()->clear();	// <------ takes too long WTF
	rc->shaderParams()->assign( "SpecularParams", 0, &XMFLOAT4( m_SpecIntensity, m_SpecPower, 0.0f ,0.0f) );

	rc->shaderParams()->assign( "ViewProjection", 0, &XMMatrixTranspose(m_pRenderSystem->GetCamera()->getView()*m_pRenderSystem->GetCamera()->getProjection()) );
	rc->shaderParams()->assign( "vEyePos", 0, &XMFLOAT4(m_pRenderSystem->GetCamera()->GetPosition().x,m_pRenderSystem->GetCamera()->GetPosition().y,m_pRenderSystem->GetCamera()->GetPosition().z,0.0f) );
	rc->shaderParams()->assign( "NormalMatrix", 0, &pTransform->GetMatrix().transpose() ) ;
	rc->shaderParams()->assign( "World", 0, &XMMatrixTranspose(pTransform->getXMMatrix()) );

	if ( pSpotLights->empty() && pPointLights->empty() )
	{
		rc->SetShaderset( (DX11Shaderset*)m_pShaderNoLight );
		break;
	}
	
	if (!pPointLights->empty()&&(omni<numOmni))
	{
		int i = 0;
		for (std::vector<PointLight*>::const_iterator it = pPointLights->begin() + omni; it != pPointLights->end(); ++it)
		{
			rc->shaderParams()->assign( "vPointLightPos", i, &XMFLOAT4((*it)->transform()->GetPosition().x,(*it)->transform()->GetPosition().y,(*it)->transform()->GetPosition().z,(*it)->getRadius()));
			rc->shaderParams()->assign( "vPointLightColor", i, &XMFLOAT4(pow((*it)->getColor().x,2.2f)*(*it)->getIntensity(),pow((*it)->getColor().y,2.2f)*(*it)->getIntensity(),pow((*it)->getColor().z,2.2f)*(*it)->getIntensity(),(*it)->getIntensity()));
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
			rc->shaderParams()->assign( "vSpotLightPos", i, &XMFLOAT4((*it)->transform()->GetPosition().x,(*it)->transform()->GetPosition().y,(*it)->transform()->GetPosition().z,(*it)->getRadius()));
			rc->shaderParams()->assign( "vSpotLightColor", i, &XMFLOAT4(pow((*it)->getColor().x,2.2f)*(*it)->getIntensity(),pow((*it)->getColor().y,2.2f)*(*it)->getIntensity(),pow((*it)->getColor().z,2.2f)*(*it)->getIntensity(),(*it)->getIntensity()));
			//rc->shaderParams()->assign( "vSpotLightColor", i, &XMFLOAT4((*it)->getColor().x*(*it)->getIntensity(),(*it)->getColor().y*(*it)->getIntensity(),(*it)->getColor().z*(*it)->getIntensity(),(*it)->getIntensity()));
			rc->shaderParams()->assign( "SpotLightViewProjection", i, &XMMatrixTranspose(XMLoadFloat4x4(&(*it)->getViewProjectionMatrix())) );
			rc->shaderParams()->assign( "iSpotLightShadowEnabled", i, (int)( (*it)->isCastingShadow() ? 1 : 0 ) );
			
			if (i==0)
			{
				rc->SetTexture( "txShadowmap", (DX11Texture2D*)(*it)->getShadowmap() );
				rc->SetTexture( "txCookie", (DX11Texture2D*)(*it)->getCookie() );
			}
			else
			{
				rc->SetTexture( "txShadowmap2", (DX11Texture2D*)(*it)->getShadowmap() );
				rc->SetTexture( "txCookie2", (DX11Texture2D*)(*it)->getCookie() );
			}
			i++;
			spot++;
			if (i>=2)
			break;
		}
	}

	rc->SetShaderset( (DX11Shaderset*)m_pShader[0] );

	if(pass>1)
	{
		rc->SetBlendState(m_pAdditiveBlendState);
		rc->SetRasterizerState(m_pAdditiveRasterizerState);
		rc->SetDepthStencilState(m_pAdditiveDepthStencilState);
	}

	if (omni>=numOmni && spot>=numSpot)
		break;
	}

	return pass;
}

////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////

UINT Material_DiffuseDetailbump::bind(Renderer* pRenderer, RenderCommand* pRenderCommands[], SubmeshRenderData* pRenderData, Transform* pTransform )
{
	std::vector<PointLight*>* pPointLights = &pRenderData->AffectingPointLights;
	std::vector<SpotLight*>* pSpotLights = &pRenderData->AffectingSpotLights;

	int numOmni = pPointLights->size();
	int numSpot = pSpotLights->size();

	int pass = 0, omni = 0, spot = 0;

	while(1)
	{
	pass++;
	D3D11RenderCommand_Draw* rc = dynamic_cast<D3D11RenderCommand_Draw*>(pRenderCommands[pass-1]);

	rc->SetBlendState(m_pDefaultBlendState);
	rc->SetDepthStencilState(m_pDefaultDepthStencilState);
	rc->SetRasterizerState(m_pDefaultRasterizerState);

	rc->SetTexture( "txDiffuse", (DX11Texture2D*)m_pDiffuse );
	rc->SetTexture( "txNormal", (DX11Texture2D*)m_pNormal );
	rc->SetTexture( "txDetailNormal", (DX11Texture2D*)m_pDetailNormal );

	rc->shaderParams()->clear();	// <------ takes too long WTF
	rc->shaderParams()->assign( "DetailNormalStrength", m_DetailNormalStrength );
	rc->shaderParams()->assign( "DetailTiling", m_DetailTiling );
	rc->shaderParams()->assign( "SpecularParams", 0, &XMFLOAT4( m_SpecIntensity, m_SpecPower, 0.0f ,0.0f) );

	rc->shaderParams()->assign( "ViewProjection", 0, &XMMatrixTranspose(m_pRenderSystem->GetCamera()->getView()*m_pRenderSystem->GetCamera()->getProjection()) );
	rc->shaderParams()->assign( "vEyePos", 0, &XMFLOAT4(m_pRenderSystem->GetCamera()->GetPosition().x,m_pRenderSystem->GetCamera()->GetPosition().y,m_pRenderSystem->GetCamera()->GetPosition().z,0.0f) );
	rc->shaderParams()->assign( "NormalMatrix", 0, &pTransform->GetMatrix().transpose() ) ;
	rc->shaderParams()->assign( "World", 0, &XMMatrixTranspose(pTransform->getXMMatrix()) );
	
	if ( pSpotLights->empty() && pPointLights->empty() )
	{
		rc->SetShaderset( (DX11Shaderset*)m_pShaderNoLight );
		break;
	}
	
	if (!pPointLights->empty()&&(omni<numOmni))
	{
		int i = 0;
		for (std::vector<PointLight*>::const_iterator it = pPointLights->begin() + omni; it != pPointLights->end(); ++it)
		{
			rc->shaderParams()->assign( "vPointLightPos", i, &XMFLOAT4((*it)->transform()->GetPosition().x,(*it)->transform()->GetPosition().y,(*it)->transform()->GetPosition().z,(*it)->getRadius()));
			rc->shaderParams()->assign( "vPointLightColor", i, &XMFLOAT4(pow((*it)->getColor().x,2.2f)*(*it)->getIntensity(),pow((*it)->getColor().y,2.2f)*(*it)->getIntensity(),pow((*it)->getColor().z,2.2f)*(*it)->getIntensity(),(*it)->getIntensity()));
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
			rc->shaderParams()->assign( "vSpotLightPos", i, &XMFLOAT4((*it)->transform()->GetPosition().x,(*it)->transform()->GetPosition().y,(*it)->transform()->GetPosition().z,(*it)->getRadius()));
			rc->shaderParams()->assign( "vSpotLightColor", i, &XMFLOAT4(pow((*it)->getColor().x,2.2f)*(*it)->getIntensity(),pow((*it)->getColor().y,2.2f)*(*it)->getIntensity(),pow((*it)->getColor().z,2.2f)*(*it)->getIntensity(),(*it)->getIntensity()));
			//rc->shaderParams()->assign( "vSpotLightColor", i, &XMFLOAT4((*it)->getColor().x*(*it)->getIntensity(),(*it)->getColor().y*(*it)->getIntensity(),(*it)->getColor().z*(*it)->getIntensity(),(*it)->getIntensity()));
			rc->shaderParams()->assign( "SpotLightViewProjection", i, &XMMatrixTranspose(XMLoadFloat4x4(&(*it)->getViewProjectionMatrix())) );
			rc->shaderParams()->assign( "iSpotLightShadowEnabled", i, (int)( (*it)->isCastingShadow() ? 1 : 0 ) );
			
			if (i==0)
			{
				rc->SetTexture( "txShadowmap", (DX11Texture2D*)(*it)->getShadowmap() );
				rc->SetTexture( "txCookie", (DX11Texture2D*)(*it)->getCookie() );
			}
			else
			{
				rc->SetTexture( "txShadowmap2", (DX11Texture2D*)(*it)->getShadowmap() );
				rc->SetTexture( "txCookie2", (DX11Texture2D*)(*it)->getCookie() );
			}
			i++;
			spot++;
			if (i>=2)
			break;
		}
	}

	rc->SetShaderset( (DX11Shaderset*)m_pShader[0] );

	if(pass>1)
	{
		rc->SetBlendState(m_pAdditiveBlendState);
		rc->SetRasterizerState(m_pAdditiveRasterizerState);
		rc->SetDepthStencilState(m_pAdditiveDepthStencilState);
	}

	if (omni>=numOmni && spot>=numSpot)
		break;
	}
	
	return pass;
}


UINT Material_BlinnPhong::bind(Renderer* pRenderer, RenderCommand* pRenderCommands[], SubmeshRenderData* pRenderData, Transform* pTransform )
{
	std::vector<PointLight*>* pPointLights = &pRenderData->AffectingPointLights;
	std::vector<SpotLight*>* pSpotLights = &pRenderData->AffectingSpotLights;

	int numOmni = pPointLights->size();
	int numSpot = pSpotLights->size();

	int pass = 0, omni = 0, spot = 0;

	while(1)
	{
	pass++;
	D3D11RenderCommand_Draw* rc = dynamic_cast<D3D11RenderCommand_Draw*>(pRenderCommands[pass-1]);

	rc->SetBlendState(m_pDefaultBlendState);
	rc->SetDepthStencilState(m_pDefaultDepthStencilState);
	rc->SetRasterizerState(m_pDefaultRasterizerState);

	rc->SetTexture( "txDiffuse", (DX11Texture2D*)m_pDiffuse );
	rc->SetTexture( "txNormal", (DX11Texture2D*)m_pNormal );
	rc->SetTexture( "txSpecular", (DX11Texture2D*)m_pSpecular );

	rc->shaderParams()->clear();	// <------ takes too long WTF
	rc->shaderParams()->assign( "SpecularParams", 0, &XMFLOAT4( m_SpecIntensity, m_SpecPower, 0.0f ,0.0f) );

	rc->shaderParams()->assign( "ViewProjection", 0, &XMMatrixTranspose(m_pRenderSystem->GetCamera()->getView()*m_pRenderSystem->GetCamera()->getProjection()) );
	rc->shaderParams()->assign( "vEyePos", 0, &XMFLOAT4(m_pRenderSystem->GetCamera()->GetPosition().x,m_pRenderSystem->GetCamera()->GetPosition().y,m_pRenderSystem->GetCamera()->GetPosition().z,0.0f) );
	rc->shaderParams()->assign( "NormalMatrix", 0, &pTransform->GetMatrix().transpose() ) ;
	rc->shaderParams()->assign( "World", 0, &XMMatrixTranspose(pTransform->getXMMatrix()) );

	if ( pSpotLights->empty() && pPointLights->empty() )
	{
		rc->SetShaderset( (DX11Shaderset*)m_pShaderNoLight );
		break;
	}
	
	if (!pPointLights->empty()&&(omni<numOmni))
	{
		int i = 0;
		for (std::vector<PointLight*>::const_iterator it = pPointLights->begin() + omni; it != pPointLights->end(); ++it)
		{
			rc->shaderParams()->assign( "vPointLightPos", i, &XMFLOAT4((*it)->transform()->GetPosition().x,(*it)->transform()->GetPosition().y,(*it)->transform()->GetPosition().z,(*it)->getRadius()));
			rc->shaderParams()->assign( "vPointLightColor", i, &XMFLOAT4(pow((*it)->getColor().x,2.2f)*(*it)->getIntensity(),pow((*it)->getColor().y,2.2f)*(*it)->getIntensity(),pow((*it)->getColor().z,2.2f)*(*it)->getIntensity(),(*it)->getIntensity()));
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
			rc->shaderParams()->assign( "vSpotLightPos", i, &XMFLOAT4((*it)->transform()->GetPosition().x,(*it)->transform()->GetPosition().y,(*it)->transform()->GetPosition().z,(*it)->getRadius()));
			rc->shaderParams()->assign( "vSpotLightColor", i, &XMFLOAT4(pow((*it)->getColor().x,2.2f)*(*it)->getIntensity(),pow((*it)->getColor().y,2.2f)*(*it)->getIntensity(),pow((*it)->getColor().z,2.2f)*(*it)->getIntensity(),(*it)->getIntensity()));
			//rc->shaderParams()->assign( "vSpotLightColor", i, &XMFLOAT4((*it)->getColor().x*(*it)->getIntensity(),(*it)->getColor().y*(*it)->getIntensity(),(*it)->getColor().z*(*it)->getIntensity(),(*it)->getIntensity()));
			rc->shaderParams()->assign( "SpotLightViewProjection", i, &XMMatrixTranspose(XMLoadFloat4x4(&(*it)->getViewProjectionMatrix())) );
			rc->shaderParams()->assign( "iSpotLightShadowEnabled", i, (int)( (*it)->isCastingShadow() ? 1 : 0 ) );
			
			if (i==0)
			{
				rc->SetTexture( "txShadowmap", (DX11Texture2D*)(*it)->getShadowmap() );
				rc->SetTexture( "txCookie", (DX11Texture2D*)(*it)->getCookie() );
			}
			else
			{
				rc->SetTexture( "txShadowmap2", (DX11Texture2D*)(*it)->getShadowmap() );
				rc->SetTexture( "txCookie2", (DX11Texture2D*)(*it)->getCookie() );
			}
			i++;
			spot++;
			if (i>=2)
			break;
		}
	}

	rc->SetShaderset( (DX11Shaderset*)m_pShader[0] );

	if(pass>1)
	{
		rc->SetBlendState(m_pAdditiveBlendState);
		rc->SetRasterizerState(m_pAdditiveRasterizerState);
		rc->SetDepthStencilState(m_pAdditiveDepthStencilState);
	}

	if (omni>=numOmni && spot>=numSpot)
		break;
	}

	return pass;
}
*/