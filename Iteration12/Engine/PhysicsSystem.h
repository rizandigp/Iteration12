#pragma once

#include "Debug.h"
#include "Vector3.h"

// Havok headers
// Math and base includes
#include <Common/Base/hkBase.h>
#include <Common/Base/System/hkBaseSystem.h>
#include <Common/Base/System/Hardware/hkHardwareInfo.h>
#include <Common/Base/System/Error/hkDefaultError.h>
#include <Common/Base/Memory/System/hkMemorySystem.h>
#include <Common/Base/Memory/System/Util/hkMemoryInitUtil.h>
#include <Common/Base/Memory/Allocator/Malloc/hkMallocAllocator.h>
//#include <Common/Base/Thread/Job/ThreadPool/Cpu/hkCpuJobThreadPool.h>
#include <Common/Base/Thread/Pool/hkCpuThreadPool.h>
#include <Physics2012/Dynamics/World/hkpWorld.h>
#include <Physics2012/Collide/Dispatch/hkpAgentRegisterUtil.h>

// Visual Debugger includes
#include <Common/Visualize/hkVisualDebugger.h>
#include <Physics2012/Utilities/VisualDebugger/hkpPhysicsContext.h>

// Headers for shapes & rigid body
#include <Physics2012\Collide\Shape\Convex\Box\hkpBoxShape.h>
#include <Physics2012\Collide\Shape\Convex\Sphere\hkpSphereShape.h>
#include <Physics2012\Collide\Shape\Misc\Transform\hkpTransformShape.h>
#include <Physics2012\Dynamics\Entity\hkpRigidBody.h>
#include <Physics2012\Utilities\Dynamics\Inertia\hkpInertiaTensorComputer.h>
#include <Physics2012\Dynamics\World\hkpSimulationIsland.h>

// Raycast objects
#include <Physics2012\Collide\Query\CastUtil\hkpWorldRayCastInput.h>
#include <Physics2012\Collide\Query\CastUtil\hkpWorldRayCastOutput.h>
#include <Physics2012\Collide/Query/Multithreaded/RayCastQuery/hkpRayCastQueryJobs.h> 

// For raycasthitcollector
#include <Physics2012/Collide/Query/Collector/RayCollector/hkpClosestRayHitCollector.h>

// Loading & serialization
#include <Common/Serialize/Util/hkLoader.h>
#include <Common/Serialize/Util/hkRootLevelContainer.h>
#include <Physics2012/Utilities/Serialize/hkpPhysicsData.h>

// Utilities
#include <Physics2012/Utilities/Collide/ShapeUtils/ShapeScaling/hkpShapeScalingUtility.h>

// we're not using any product apart from Havok Physics.
#undef HK_FEATURE_PRODUCT_AI
#undef HK_FEATURE_PRODUCT_ANIMATION
#undef HK_FEATURE_PRODUCT_CLOTH
#undef HK_FEATURE_PRODUCT_DESTRUCTION
#undef HK_FEATURE_PRODUCT_BEHAVIOR
  
// Also we're not using any serialization/versioning so we don't need any of these.
#define HK_EXCLUDE_FEATURE_SerializeDeprecatedPre700
#define HK_EXCLUDE_FEATURE_RegisterVersionPatches
#define HK_EXCLUDE_FEATURE_MemoryTracker

// Havok error messages
void onHavokError(const char* msg, void* userArgGivenToInit);

class PhysicsSystem
{
public:
	void InitHavok();
	void DeinitHavok();

	void Step(float deltaTime);
	hkpWorld* GetWorld();
	hkVisualDebugger* GetVisualDebugger();
	hkpPhysicsContext* GetContext();

	hkpRigidBody* CreateBoxRigidBody( Vector3 halfExtents, float mass, float friction = 0.5f, float restitution = 0.35f  );

protected:
	void InitMemory();
	void InitPhysicalWorld();
	void InitVDB();
	void DeinitVDB();
	void StepVDB();

	hkpWorld* m_pWorld;
	hkVisualDebugger* m_pVDB;
	hkpPhysicsContext* m_pContext;

	hkCpuThreadPool* m_pThreadPool;
	hkJobQueue* m_pJobQueue;
};