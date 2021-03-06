#pragma once

#include "Prerequisites.h"
#include "Entity.h"
#include "Vector2.h"
#include "Vector3.h"


class FPPlayerEntity : public Entity
{
public:
	FPPlayerEntity( RenderSystem* pRenderSystem );
	void OnAddToScene( Scene* ptr );
	void Update(float deltaTime);
	inline Camera3D* GetCamera()		{ return m_pCamera; };
	inline SpotLight* GetFlashlight()	{ return m_pSpotlight; };

protected:
	Camera3D*	m_pCamera;
	SpotLight*	m_pSpotlight;
	bool m_bSpotlightOn;
	bool m_bMouseDragged;
	POINT m_ptLastMousePos;
	Vector3 m_DirEuler;
	Vector3 m_PointingEulerDir;

private:
	float m_TimeSinceLastShot;
	UINT m_ShotEntityId;
	float m_FlashlightButtonCooldown;
};