#pragma once

// Disable pesky warnings
#pragma warning( disable : 4238 ) // "nonstandard extension used : class rvalue used as lvalue"
#pragma warning( disable : 4018 ) // "'<' : signed/unsigned mismatch"

#include <d3dx9math.h>
#include "tbb\tbb.h"
#include "tbb\scalable_allocator.h"
#include <assert.h>
#include "Debug.h"
#include <map>
#include <unordered_map>
#include <vector>
#include <queue>
#include <string>
#include <math.h>

class BufferLayout;
class Camera3D;
class CameraFPS;
class ConstantBufferData;
class D3D10IndexBuffer;
class D3D10Texture2D;
class D3D10VertexBuffer;
class D3D10GeometryChunk;
class D3D10IndexBuffer;
class DX11GeometryChunk;
class D3D11IndexBuffer;
class D3D11RenderCommand_BeginFrame;
class D3D11RenderCommand_ClearTexture;
class D3D11RenderCommand_Draw;
class D3D11RenderCommand_EndFrame;
class D3D11RenderCommand_RenderTarget;
class D3D11RenderCommand_SharedCB;
class DX11RenderDispatcher;
class DX11Shaderset;
class DX11Texture2D;
class D3D11VertexBuffer;
class DX11Material;
class DX11RenderCommand;
class DX11Renderer;
class GenericRenderCommand;
class GeometryChunk;
class Entity;
class FFT;
class FFTWater;
class IndexBuffer;
class Light;
class Material;
class Material_Diffuse;
class Material_DiffuseBump;
class Material_DiffuseBumpSpecular;
class Material_DiffuseDetailbump;
class Matrix3x3;
class Matrix4x4;
class Mesh;
class MeshLoader;
class MyEntity;
class PhysicsSystem;
class Plane;
class PointLight;
class Quaternion;
class Ray;
class RenderCommand;
class RenderCommandAllocator;
class RenderDispatcher;
class Renderer;
class RenderOutput;
class RenderSystem;
class Scene;
class Shaderset;
class ShaderParamBlock;
class ShaderParams;
class SpotLight;
class Submesh;
class Texture;
class Texture2D;
class Texture3D;
class TextureCube;
class Timer;
class Transform;
class Vector3;
class Vector4;
class VertexBuffer;

struct RenderCommandBlock;
struct RenderViewport;
struct ShadersetDescription;
struct ShaderVariable;
struct SubmeshRenderData;
struct VertexElement;

class GameEntity;
class MyEntity;
class Entity_Prop;
class Entity_StaticProp;
class Entity_Water;


#ifndef SAFE_RELEASE
#define SAFE_RELEASE(p)      { if (p) { (p)->Release(); (p)=NULL; } };
#endif

#ifndef SAFE_DELETE
#define SAFE_DELETE( obj )	 { if(obj); delete obj; };
#endif

#ifndef PUREVIRTUAL
#define PUREVIRTUAL = 0
#endif