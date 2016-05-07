// Iteration12.cpp : Defines the entry point for the application.
//

#pragma warning( disable : 4996 ) // warning C4996: 'itoa': The POSIX name for this item is deprecated. Instead, use the ISO C++ conformant name: _itoa. See online help for details.

#include "stdafx.h"
#include "Iteration12.h"
#include "Engine\Engine.h"
#include <boost\thread.hpp>

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;								// current instance
TCHAR szTitle[MAX_LOADSTRING];					// The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];			// the main window class name
HWND hWnd;

// App-related globals
Timer					globalTimer;
RenderSystem*			pRenderSystem;
PhysicsSystem*			pPhysicsSystem;
Scene					scene;
PlayerEntity*           pPlayer;
FPPlayerEntity*			pFPPlayer;
PointLight*				pLight1, *pLight2, *pLight3, *pLight4;
SpotLight*				pSpotlight1, *pSpotlight2, *pSpotlight3;
Camera3D*				pCamera;
Entity*					pTestEntity;

// Forward declarations of functions included in this code module:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);

HRESULT InitWindow( HINSTANCE hInstance, int nCmdShow );
HRESULT InitDevice();
void CleanupDevice();
void DoFrame();

class RenderThreadWork
{
public:
	RenderThreadWork( RenderSystem* ptr ) : m_pRenderSystem(ptr)	{};
	void operator()()
	{
		m_pRenderSystem->ConsumeRenderQueue( false );
	}

	RenderSystem* m_pRenderSystem;
};

int APIENTRY _tWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

 	// TODO: Place code here.
	//MSG msg;
	HACCEL hAccelTable;

	// Initialize global strings
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_ITERATION12, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// Perform application initialization:
	if (FAILED(InitWindow(hInstance, nCmdShow)))
	{
		return FALSE;
	}
	if( FAILED( InitDevice() ) )
    {
        CleanupDevice();
        return 0;
    }

	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_ITERATION12));

	// Start render thread
	RenderThreadWork worker(pRenderSystem);
	boost::thread renderThread(worker);

	// Main message loop
	MSG msg = {0};
    while( WM_QUIT != msg.message )
    {
        if( PeekMessage( &msg, NULL, 0, 0, PM_REMOVE ) )
        {
            TranslateMessage( &msg );
            DispatchMessage( &msg );
        }
        else
        {
            DoFrame();
        }
    }/*
	// Main message loop:
	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			DoFrame();
		}
	}*/

	return (int) msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
//  COMMENTS:
//
//    This function and its usage are only necessary if you want this code
//    to be compatible with Win32 systems prior to the 'RegisterClassEx'
//    function that was added to Windows 95. It is important to call this function
//    so that the application will get 'well formed' small icons associated
//    with it.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ITERATION12));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= MAKEINTRESOURCE(IDC_ITERATION12);
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassEx(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   //HWND hWnd;

   hInst = hInstance; // Store instance handle in our global variable

   hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

//--------------------------------------------------------------------------------------
// Register class and create window
//--------------------------------------------------------------------------------------
HRESULT InitWindow( HINSTANCE hInstance, int nCmdShow )
{
    // Register class
    WNDCLASSEX wcex;
    wcex.cbSize = sizeof( WNDCLASSEX );
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon( hInstance, ( LPCTSTR )IDI_ITERATION12 );
    wcex.hCursor = LoadCursor( NULL, IDC_ARROW );
    wcex.hbrBackground = ( HBRUSH )( COLOR_WINDOW + 1 );
    wcex.lpszMenuName = NULL;
    wcex.lpszClassName = L"WindowClass";
    wcex.hIconSm = LoadIcon( wcex.hInstance, ( LPCTSTR )IDI_ITERATION12 );
    if( !RegisterClassEx( &wcex ) )
        return E_FAIL;

    // Create window
    hInst = hInstance;
    RECT rc = { 0, 0, 1200, 700 };
    AdjustWindowRect( &rc, WS_OVERLAPPEDWINDOW, FALSE );
    hWnd = CreateWindow( L"WindowClass", L"NG - DX11", WS_OVERLAPPEDWINDOW,
                           CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top, NULL, NULL, hInstance,
                           NULL );
    if( !hWnd )
	{
		long error = GetLastError();
        return E_FAIL;
	}

    ShowWindow( hWnd, nCmdShow );
	UpdateWindow(hWnd);

    return S_OK;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND	- process the application menu
//  WM_PAINT	- Paint the main window
//  WM_DESTROY	- post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;

	switch (message)
	{
	case WM_COMMAND:
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		// Parse the menu selections:
		switch (wmId)
		{
		case IDM_ABOUT:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		// TODO: Add any drawing code here...
		EndPaint(hWnd, &ps);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}


HRESULT InitDevice()
{
    RECT rc;
    GetClientRect( hWnd, &rc );
    UINT width = rc.right - rc.left;
    UINT height = rc.bottom - rc.top;

    UINT createDeviceFlags = 0;
#ifdef _DEBUG
    createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
	
	// Create and initialize render system
	RenderSystemConfig config;
	config.api = GRAPHICS_API_DX11;
	config.BackbufferMultisampling = MSAA_NONE;
	config.MaxAnisotropy = 4;
	config.Height = height;
	config.Width = width;
	config.RefreshRate = 60;
	config.Vsync = false;
	config.Windowed = true;
	config.OutputWindow = hWnd;
	config.DebugDevice = false;
	config.Multithreaded = true;

	pRenderSystem = new RenderSystem();
	pRenderSystem->Initialize( config );
	//pRenderSystem->SetOutputWindow( hWnd, width, height );

	// Create and initialize physics system
	pPhysicsSystem = new PhysicsSystem();
	pPhysicsSystem->InitHavok();

	// Create fps camera
	pCamera = new CameraFPS();
	pCamera->SetPosition( Vector3( 0.0f, 5.0f, 2.0f ) );
	pCamera->SetLookAt( Vector3( 0.0f, 1.0f, 0.0f ) );
	pCamera->SetUpVector( Vector3( 0.0f, 0.0f, 1.0f ) );
	pCamera->SetProjection( XM_PI*0.35f, width / (float)height, 0.1f, 5000.0f );
	pCamera->Update( 0.0f );

	// Create controllable player entity
	pPlayer = new PlayerEntity( pRenderSystem );
	pPlayer->Transformation()->SetPosition( 10.5f, 10.5f, 5.8f );

	pFPPlayer = new FPPlayerEntity( pRenderSystem );
	pFPPlayer->Transformation()->SetPosition( 0.0f, 0.0f, 2.0f );

	// Set up the scene
	scene.Init( pRenderSystem, pPhysicsSystem );
	scene.LoadFromFile( L"Levels/physics_testlevel09.txt" );
	//scene.LoadCustomObjects();
	scene.AddEntity(pFPPlayer);

	return S_OK;
}


//--------------------------------------------------------------------------------------
// Process and render one frame. Called by the main loop
//--------------------------------------------------------------------------------------
void DoFrame()
{
	Timer mainTimer;

    // Clear backbuffer and begin frame
    float ClearColor[4] = { 0.0f, 0.125f, 0.3f, 1.0f }; //red,green,blue,alpha
	pRenderSystem->BeginFrame( ClearColor, 1.0f, 0 );
	
	static double t = 0, dt = 0;
	dt=t;
	t = globalTimer.GetSeconds();
	dt = t-dt;

	if (dt > 1/30.0f)
		dt = 1/30.0f;

	Timer updateTimer;
	scene.Update( dt );
	double t_update = updateTimer.GetMiliseconds();

	scene.ParallelRenderDeferred();
	pRenderSystem->EndFrame();

	double t_main = mainTimer.GetMiliseconds();


	// Wait for render thread to finish
	// pRenderSystem->WaitForFrameToFinish();

	double t_waitrender = mainTimer.GetMiliseconds() - t_main;
	
	char str[20];

	if ( KEYDOWN('T') )
	{
		_gcvt( t_waitrender, 5, str );
		DEBUG_OUTPUT( "main thread wait: " );
		DEBUG_OUTPUT( str );
		DEBUG_OUTPUT( "ms\t" );

		_gcvt( t_main, 5, str );
		DEBUG_OUTPUT( "main thread: " );
		DEBUG_OUTPUT( str );
		DEBUG_OUTPUT( "ms\t  " );

		_gcvt( scene.t_physics, 5, str );
		DEBUG_OUTPUT( "physics: " );
		DEBUG_OUTPUT( str );
		DEBUG_OUTPUT( "ms\t  " );

		/*_gcvt( scene.t_ocean, 5, str );
		DEBUG_OUTPUT( "ocean: " );
		DEBUG_OUTPUT( str );
		DEBUG_OUTPUT( "ms\t  " );*/

		_gcvt( t_update, 5, str );
		DEBUG_OUTPUT( "Scene::update: " );
		DEBUG_OUTPUT( str );
		DEBUG_OUTPUT( "ms\t  " );

		_gcvt( pRenderSystem->GetRenderDispatcher()->t_renderthread, 5, str );
		DEBUG_OUTPUT( "render thread: " );
		DEBUG_OUTPUT( str );
		DEBUG_OUTPUT( "ms\t  " );
		/*
		_gcvt( pRenderSystem->t_renderstarve, 5, str );
		DEBUG_OUTPUT( "starve: " );
		DEBUG_OUTPUT( str );
		DEBUG_OUTPUT( "ms\t  " );
		*/
		_gcvt( pRenderSystem->t_commands, 5, str );
		DEBUG_OUTPUT( "render commands: " );
		DEBUG_OUTPUT( str );
		DEBUG_OUTPUT( "ms\t  " );

		_gcvt( pRenderSystem->GetRenderDispatcher()->t_bindparams, 6, str );
		DEBUG_OUTPUT( "Bind shader params: " );
		DEBUG_OUTPUT( str );
		DEBUG_OUTPUT( "ms\t  " );

		_gcvt( scene.t_scenerender, 6, str );
		DEBUG_OUTPUT( "Scene::render(): " );
		DEBUG_OUTPUT( str );
		DEBUG_OUTPUT( "ms\t  " );
		/*
		_gcvt( pRenderSystem->GetRenderDispatcher()->t_1, 6, str );
		DEBUG_OUTPUT( "(t_1: " );
		DEBUG_OUTPUT( str );
		DEBUG_OUTPUT( "ms\t  " );

		_gcvt( pRenderSystem->GetRenderDispatcher()->t_2, 6, str );
		DEBUG_OUTPUT( "t_2: " );
		DEBUG_OUTPUT( str );
		DEBUG_OUTPUT( "ms)\t  " );
	
		_gcvt( pRenderSystem->GetRenderDispatcher()->t_bindtextures, 6, str );
		DEBUG_OUTPUT( "Bind textures: " );
		DEBUG_OUTPUT( str );
		DEBUG_OUTPUT( "ms\t  " );

		_gcvt( pRenderSystem->GetRenderDispatcher()->t_drawcalls, 6, str );
		DEBUG_OUTPUT( "draw calls: " );
		DEBUG_OUTPUT( str );
		DEBUG_OUTPUT( "ms\t  " );
	
		_gcvt( pRenderSystem->GetRenderDispatcher()->t_BeginFrame, 6, str );
		DEBUG_OUTPUT( "begin frame: " );
		DEBUG_OUTPUT( str );
		DEBUG_OUTPUT( "ms\t  " );

		_gcvt( pRenderSystem->GetRenderDispatcher()->t_EndFrame, 6, str );
		DEBUG_OUTPUT( "end frame: " );
		DEBUG_OUTPUT( str );
		DEBUG_OUTPUT( "ms\t  " );
		*/
		_gcvt( scene.t_Cull, 5, str );
		DEBUG_OUTPUT( "Culling: " );
		DEBUG_OUTPUT( str );
		DEBUG_OUTPUT( "ms\t  " );

		_gcvt( pRenderSystem->t_material, 5, str );
		DEBUG_OUTPUT( "material system: " );
		DEBUG_OUTPUT( str );
		DEBUG_OUTPUT( "ms\t  " );

		_gcvt( pRenderSystem->t_material, 5, str );
		DEBUG_OUTPUT( "Renderer::RenderShadowmap: " );
		DEBUG_OUTPUT( str );
		DEBUG_OUTPUT( "ms\t  " );

		_gcvt( pRenderSystem->t_shaderparams, 5, str );
		DEBUG_OUTPUT( "shaderparams: " );
		DEBUG_OUTPUT( str );
		DEBUG_OUTPUT( "ms\t  " );
		/*
		_gcvt( pRenderSystem->t_shaderparams, 5, str );
		DEBUG_OUTPUT( "new shader params: " );
		DEBUG_OUTPUT( str );
		DEBUG_OUTPUT( "ms\t  " );
		
		_gcvt( pRenderSystem->t_alloc, 7, str );
		DEBUG_OUTPUT( "command alloc: " );
		DEBUG_OUTPUT( str );
		DEBUG_OUTPUT( "ms\t  " );

		_gcvt( pRenderSystem->t_dealloc, 7, str );
		DEBUG_OUTPUT( "command dealloc: " );
		DEBUG_OUTPUT( str );
		DEBUG_OUTPUT( "ms\t  " );
		/*
		_gcvt( pRenderSystem->t_queuepush, 7, str );
		DEBUG_OUTPUT( "render queue push: " );
		DEBUG_OUTPUT( str );
		DEBUG_OUTPUT( "mus\t  " );

		_gcvt( pRenderSystem->t_queuepop, 7, str );
		DEBUG_OUTPUT( "render queue pop: " );
		DEBUG_OUTPUT( str );
		DEBUG_OUTPUT( "mus\t  " );
	
		_gcvt( pRenderSystem->t_renderstarve, 5, str );
		DEBUG_OUTPUT( "render thread starve: " );
		DEBUG_OUTPUT( str );
		DEBUG_OUTPUT( "mus\t  " );
		*/
		itoa( pRenderSystem->GetRenderDispatcher()->drawcalls, str, 10 );
		DEBUG_OUTPUT( "draw calls: " );
		DEBUG_OUTPUT( str );
		DEBUG_OUTPUT( "\t  " );

		itoa( pRenderSystem->GetRenderDispatcher()->numVerts, str, 10 );
		DEBUG_OUTPUT( "verts: " );
		DEBUG_OUTPUT( str );
		DEBUG_OUTPUT( "\n  " );
		/*
		itoa( sizeof(uint64_t), str, 10 );
		DEBUG_OUTPUT( "sizeof uint64_t: " );
		DEBUG_OUTPUT( str );
		DEBUG_OUTPUT( "\n  " );*/
		/*
		int size = sizeof(D3D11RenderCommand_Draw);
		itoa( size, str, 10 );
		DEBUG_OUTPUT( "D3D11RenderCommand_Draw size: " );
		DEBUG_OUTPUT( str );
		DEBUG_OUTPUT( "\n  " );

		*/
	}
}

//--------------------------------------------------------------------------------------
// Clean up the objects we've created
//--------------------------------------------------------------------------------------
void CleanupDevice()
{
	// Shiiiieeeett
}