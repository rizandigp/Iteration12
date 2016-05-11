#pragma once

#include "DX11Renderer.h"
#include "DX11Material.h"
#include "DX11Shaderset.h"


DX11Renderer::DX11Renderer() 	
{ 
	m_Mesh = NULL; 
}

void DX11Renderer::SetRenderSystem( RenderSystem*	renderSystem )
{
	m_RenderSystem = renderSystem;
	m_ShadowmapShader = m_RenderSystem->LoadShaderset( L"Shaders/Shadowmap.hlsl", "VS", "PS", SM_5_0 );
	m_RSMShader = m_RenderSystem->LoadShaderset( L"Shaders/RSM.hlsl", "VS", "PS", SM_5_0 );
}

// TODO : optimize implementation
void DX11Renderer::SetMesh( Mesh* mesh )
{
	if (m_Mesh!=mesh && mesh)
	{
		m_Mesh = mesh;
		
		// Delete all debug render commands
		while (!m_DebugRenderCommands.empty())
		{
			delete m_DebugRenderCommands.back();  m_DebugRenderCommands.pop_back();
		}

		// Reset render data
		m_SubmeshRenderData.clear();

		for( int i=0; i<m_Mesh->GetNumberOfSubmeshes(); i++ )
		{
			SubmeshRenderData renderData;
			renderData.bVisible = true;
			m_SubmeshRenderData.push_back(renderData);

			Submesh* submesh = m_Mesh->GetSubmesh( i );

			for (int multipass = 0; multipass < 4; multipass++)
			{
				// Main render commands
				DX11RenderCommand_Draw* command = new DX11RenderCommand_Draw();
				command->SetGeometryChunk( (DX11GeometryChunk*)submesh->GetGeometryChunk() );
				m_SubmeshRenderCommands.push_back(std::vector<DX11RenderCommand_Draw*>());
				m_SubmeshRenderCommands[i].push_back( command );
			}

			// Render commands for shadowmap rendering
			DX11RenderCommand_Draw* command = new DX11RenderCommand_Draw();
			command->SetShaderset( (DX11Shaderset*)m_ShadowmapShader );
			command->SetGeometryChunk( (DX11GeometryChunk*)submesh->GetGeometryChunk() );
			m_ShadowmapRenderCommands.push_back( command );

			// Render commands for debug OBB drawing
			DX11RenderCommand_Draw* rcDebug = new DX11RenderCommand_Draw();
			rcDebug->SetGeometryChunk( (DX11GeometryChunk*)m_RenderSystem->CreateBoxWireframeMesh( Vector3(2.0f, 2.0f, 2.0f) )->GetSubmesh(0)->GetGeometryChunk() );
			rcDebug->SetShaderset( (DX11Shaderset*)m_RenderSystem->LoadShaderset( L"Shaders/OneColor.hlsl", "VS", "PS", SM_5_0 ) );
			m_DebugRenderCommands.push_back( rcDebug );
		}
	}

	m_Mesh = mesh;
}

// TODO : optimize implementation
void DX11Renderer::Render( Transform* transform )
{
	if (m_Mesh)
	{
		for( int i=0; i<m_Mesh->GetNumberOfSubmeshes(); i++ )
		{
			if (m_SubmeshRenderData[i].bVisible)
			{
				Submesh* submesh = m_Mesh->GetSubmesh( i );
	
				Timer matTimer;
				UINT passes = 1;
				if (submesh->GetMaterial())
						passes = submesh->GetMaterial()->Bind( this, (RenderCommand**)&m_SubmeshRenderCommands[i][0], &m_SubmeshRenderData[i], transform );
				// else
				// "No material" shader
	
				m_RenderSystem->t_material += matTimer.GetMiliseconds();
					
				for( int ii=0; ii<passes; ii++ )
				{
					m_RenderSystem->Submit( m_SubmeshRenderCommands[i].at(ii) );
					m_RenderSystem->drawcalls++;
				}
			}
		}
	}
}

void DX11Renderer::Render( Transform* transform, Material* material )
{
	if (m_Mesh && material)
	{
		for( int i=0; i<m_Mesh->GetNumberOfSubmeshes(); i++ )
		{
			if (m_SubmeshRenderData[i].bVisible)
			{
				Submesh* submesh = m_Mesh->GetSubmesh( i );
	
				Timer matTimer;
				UINT passes = 1;

				passes = material->Bind( this, (RenderCommand**)&m_SubmeshRenderCommands[i][0], &m_SubmeshRenderData[i], transform );
	
				m_RenderSystem->t_material += matTimer.GetMiliseconds();
					
				for( int ii=0; ii<passes; ii++ )
				{
					m_RenderSystem->Submit( m_SubmeshRenderCommands[i].at(ii) );
					m_RenderSystem->drawcalls++;
				}
			}
		}
	}
}

void DX11Renderer::RenderShadowmap( Transform* transform, Camera3D* shadowCamera )
{
	if (m_Mesh)
	{
		for( int i=0; i<m_Mesh->GetNumberOfSubmeshes(); i++ )
		{
			if (m_SubmeshRenderData[i].bVisible)
			{
				Timer timer;
				DX11RenderCommand_Draw* command = m_ShadowmapRenderCommands[i];

				ShaderParams params;
				params.initialize( m_ShadowmapShader );

				params.setParam( "World", 0, &transform->GetMatrix().transpose() );
				params.setParam( "SpotLightViewProjection", 0, &shadowCamera->GetViewProjectionMatrix().transpose() );
				params.setParam( "SpotLightView", 0, &shadowCamera->GetViewMatrix().transpose() );
				params.setParam( "fFarPlane", 0, shadowCamera->GetFarPlane() );
			
				command->SetShaderParams( &params );
				command->SetShaderset( (DX11Shaderset*)m_ShadowmapShader );

				RenderState renderState;
				renderState.CullingMode = CULL_FRONT;
				command->SetRenderState(renderState);

				BlendState blendState;
				command->SetBlendState(blendState);

				m_RenderSystem->t_shadowmapbinding += timer.GetMiliseconds();

				m_RenderSystem->Submit( command ); 
				m_RenderSystem->drawcalls++;
			}
		}
	}
}

void DX11Renderer::RenderRSM( Transform* transform, Camera3D* shadowCamera, SpotLight* lightSource )
{
	if (m_Mesh)
	{
		for( int i=0; i<m_Mesh->GetNumberOfSubmeshes(); i++ )
		{
			if (m_SubmeshRenderData[i].bVisible)
			{
				DX11RenderCommand_Draw* command = m_ShadowmapRenderCommands[i];

				ShaderParams params;
				params.initialize( m_RSMShader );

				params.setParam( "World", 0, &transform->GetMatrix().transpose() );
				params.setParam( "SpotLightViewProjection", 0, &shadowCamera->GetViewProjectionMatrix().transpose() );
				params.setParam( "SpotLightView", 0, &shadowCamera->GetViewMatrix().transpose() );
				params.setParam( "NormalMatrix", 0, &transform->GetMatrix().transpose() ) ;
				params.setParam( "vResolution", 0, &Vector4(lightSource->GetCookie()->GetDimensions(),0.0f,0.0f) );
				params.setParam( "fFarPlane", 0, shadowCamera->GetFarPlane() );
			
				command->SetShaderParams( &params );
				command->SetShaderset( (DX11Shaderset*)m_RSMShader );
				command->SetTexture( "txCookie", lightSource->GetCookie() );

				if (dynamic_cast<Material_Deferred*>(m_Mesh->GetSubmesh(i)->GetMaterial()))
				{
					command->SetTexture( "txDiffuse", dynamic_cast<Material_Deferred*>(m_Mesh->GetSubmesh(i)->GetMaterial())->GetDiffusemap() );
					command->SetTexture( "txNormal", dynamic_cast<Material_Deferred*>(m_Mesh->GetSubmesh(i)->GetMaterial())->GetNormalmap() );
				}

				m_RenderSystem->Submit( command ); 
				m_RenderSystem->drawcalls++;
			}
		}
	}
}

void DX11Renderer::RenderOBB( Transform* transform )
{
	if (m_Mesh)
	{
		for( int i=0; i<m_Mesh->GetNumberOfSubmeshes(); i++ )
		{
			if (m_SubmeshRenderData[i].bVisible)
			{
				DX11RenderCommand_Draw* command = m_DebugRenderCommands[i];
				
				Submesh* submesh = m_Mesh->GetSubmesh( i );
				XMVECTOR extents = XMLoadFloat3( &submesh->GetGeometryChunk()->GetAABB()->Extents );
				XMVECTOR scale = XMLoadFloat3( &transform->GetScale().intoXMFLOAT3() );
				XMVECTOR offset = XMLoadFloat3( &submesh->GetGeometryChunk()->GetAABB()->Center )*scale;
				XMVECTOR position = XMVector3Rotate( offset, XMLoadFloat4(&transform->GetOrientation().intoXMFLOAT4()) );
				scale *= extents;
				position += XMLoadFloat3( &transform->GetPosition().intoXMFLOAT3() );
	
				XMMATRIX transformMatrix = XMMatrixAffineTransformation( scale, XMVectorZero(), XMLoadFloat4(&transform->GetOrientation().intoXMFLOAT4()), position );

				ShaderParams params;
				params.initialize( command->GetShaderset() );

				params.setParam( "ViewProjection", 0, &m_RenderSystem->GetCamera()->GetViewMatrix().transpose() );
				params.setParam( "World", 0, &XMMatrixTranspose(transformMatrix) );
				params.setParam( "vMeshColor", 0, &Vector4( 0.1f, 0.1f, 0.2f, 0.0f ) );

				command->SetShaderParams( &params );

				m_RenderSystem->Submit( command ); 
			}
		}
	}
}

DX11Renderer::~DX11Renderer()
{
	// Delete all render commands
	while (!m_SubmeshRenderCommands.empty())
	{
		std::vector< DX11RenderCommand_Draw* >* container = &m_SubmeshRenderCommands.back();
		while (!container->empty())
		{
			delete container->back();  container->pop_back();
		}
		m_SubmeshRenderCommands.pop_back();
	}
	while (!m_DebugRenderCommands.empty())
	{
		delete m_DebugRenderCommands.back();  m_DebugRenderCommands.pop_back();
	}
	while (!m_ShadowmapRenderCommands.empty())
	{
		delete m_ShadowmapRenderCommands.back();  m_ShadowmapRenderCommands.pop_back();
	}
}