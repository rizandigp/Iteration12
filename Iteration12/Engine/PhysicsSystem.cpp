#pragma once

#include "PhysicsSystem.h"
#include "Math.h"

// Havok keycode
#include <Common/Base/keycode.cxx>
#include <Common/Base/Config/hkProductFeatures.cxx>

void PhysicsSystem::initHavok()
{
	initMemory();
	initPhysicalWorld();
	initVDB();
}

void PhysicsSystem::initMemory() 
{
 
    #if defined(HK_COMPILER_HAS_INTRINSICS_IA32) && HK_CONFIG_SIMD ==  HK_CONFIG_SIMD_ENABLED

    // Flush all denormal/subnormal numbers to zero.
    // Operations on denormals are very slow, 
 
    // up to 100 times slower than normal numbers.
    _MM_SET_FLUSH_ZERO_MODE(_MM_FLUSH_ZERO_ON);
    #endif
 
    // Initialize the base system including our memory system
    // Allocate 5MB of physics solver buffer.
    hkMemoryRouter* memoryRouter = hkMemoryInitUtil::initChecking( hkMallocAllocator::m_defaultMallocAllocator, hkMemorySystem::FrameInfo( 5000* 1024 ) );   
	hkBaseSystem::init( memoryRouter, onHavokError );
 
 
    // We can cap the number of threads used - here we use the maximum for whatever multithreaded platform we are running on. This variable is
    // set in the following code sections.
    int totalNumThreadsUsed;
 
    // Get the number of physical threads available on the system
    hkHardwareInfo hwInfo;
    //hkGetHardwareInfo(hwInfo);
	hwInfo.calcNumHardwareThreads();
    totalNumThreadsUsed = hwInfo.getNumHardwareThreads();
 
    // We use one less than this for our thread pool, because we must also use this thread for our simulation
    hkCpuThreadPoolCinfo threadPoolCinfo;
    threadPoolCinfo.m_numThreads = totalNumThreadsUsed - 1;
 
    // This line enables timers collection, by allocating 200 Kb per thread.  If you leave this at its default (0),
    // timer collection will not be enabled.
    threadPoolCinfo.m_timerBufferPerThreadAllocation = 200000;
    m_pThreadPool = new hkCpuThreadPool( threadPoolCinfo );
 
    // We also need to create a Job queue. This job queue will be used by all Havok modules to run multithreaded work.
    // Here we only use it for physics.
    hkJobQueueCinfo info;
    info.m_jobQueueHwSetup.m_numCpuThreads = totalNumThreadsUsed;
    m_pJobQueue = new hkJobQueue(info);
 
    // Enable monitors for this thread.
    // Monitors have been enabled for thread pool threads already (see above comment).
    hkMonitorStream::getInstance().resize(200000);
}

void PhysicsSystem::initPhysicalWorld() 
{
	hkpWorldCinfo WorldInfo;
    WorldInfo.m_simulationType = hkpWorldCinfo::SIMULATION_TYPE_MULTITHREADED;
    WorldInfo.m_broadPhaseBorderBehaviour = hkpWorldCinfo::BROADPHASE_BORDER_FIX_ENTITY;
	WorldInfo.m_gravity = hkVector4( 0.0f, 0.0f, -9.8f );
	WorldInfo.setupSolverInfo( hkpWorldCinfo::SOLVER_TYPE_8ITERS_MEDIUM );

    m_pWorld = new hkpWorld(WorldInfo);
    m_pWorld->m_wantDeactivation = true;
	m_pWorld->lock();
    hkpAgentRegisterUtil::registerAllAgents( m_pWorld->getCollisionDispatcher() );
    m_pWorld->registerWithJobQueue( m_pJobQueue );
	m_pWorld->unlock();
}

void PhysicsSystem::initVDB() 
{
    hkArray<hkProcessContext*> contexts;
    {
		m_pWorld->markForWrite();
        m_pContext = new hkpPhysicsContext();
        hkpPhysicsContext::registerAllPhysicsProcesses();
        m_pContext->addWorld(m_pWorld);
        contexts.pushBack(m_pContext);
        m_pWorld->unmarkForWrite();
    }

    m_pVDB = new hkVisualDebugger(contexts);
    m_pVDB->serve();
}

void PhysicsSystem::deinitHavok()
{ 
    m_pWorld->markForWrite();
    m_pWorld->removeReference();
 
    delete m_pJobQueue;
 
    // Clean up the thread pool
    m_pThreadPool->removeReference();
 
    //deinitVDB();
 
    hkBaseSystem::quit();
    hkMemoryInitUtil::quit();
}

void PhysicsSystem::deinitVDB()
{
	m_pVDB->removeReference();
	m_pContext->removeReference();
}

void PhysicsSystem::step(float deltaTime)
{
	// Multithreaded stepping
	m_pWorld->stepMultithreaded(m_pJobQueue, m_pThreadPool, deltaTime);

	// Single threaded stepping
	//m_pWorld->stepDeltaTime( deltaTime );

	// Step the visual debugger
	stepVDB();

	// Clear timing
	hkMonitorStream::getInstance().reset();
    m_pThreadPool->clearTimerData();
}

void PhysicsSystem::stepVDB() 
{
    m_pContext->syncTimers(m_pThreadPool);
    m_pVDB->step();
}

hkVisualDebugger* PhysicsSystem::getVisualDebugger()
{
	return m_pVDB;
}

hkpPhysicsContext* PhysicsSystem::getContext()
{
	return m_pContext;
}

hkpWorld* PhysicsSystem::getWorld()
{
	return m_pWorld;
}

hkpRigidBody* PhysicsSystem::createBoxRigidBody( Vector3 halfExtents, float mass, float friction, float restitution )
{
	float convexRadius = 0.05f;
	float halfLength = clamp(halfExtents.x-convexRadius, convexRadius,halfExtents.x);
	float halfWidth = clamp(halfExtents.y-convexRadius, convexRadius,halfExtents.y);
	float halfHeight = clamp(halfExtents.z-convexRadius, convexRadius,halfExtents.z);
	
	hkVector4 hExtents( halfLength, halfWidth, halfHeight );
	hkpBoxShape* shape = new hkpBoxShape( hExtents, convexRadius );
	
	hkpRigidBodyCinfo info;
	info.m_shape = shape;
	info.m_friction = friction;
	info.m_restitution = restitution;

	//info.m_position = hkVector4( position.x, position.y, position.z );
	if (mass>0)
	{
		info.m_motionType = hkpMotion::MOTION_DYNAMIC;

		hkMassProperties massProperties;
		hkpInertiaTensorComputer::computeBoxVolumeMassProperties( hExtents, mass, massProperties );
		info.setMassProperties( massProperties );
	}
	else
		info.m_motionType = hkpMotion::MOTION_FIXED;

	hkpRigidBody* rigidBody = new hkpRigidBody(info);
	shape->removeReference();

	return rigidBody;
}

void onHavokError(const char* msg, void* userArgGivenToInit)
{
	DEBUG_OUTPUT( "[Havok]" );
	DEBUG_OUTPUT( msg );
	DEBUG_OUTPUT( "\n" );
}