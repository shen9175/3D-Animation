#include <d3d11.h>
#include <DirectXMath.h>
#include <windowsx.h>
#include <string>
#include <vector>
#include <unordered_map>
#include <assert.h>
#include <sstream>
#include "GameTimer.h"
#include "D3DBase.h"
#include "Camera.h"
#include "Animation.h"
#include "TextureMangement.h"
#include "ConstantBuffer.h"
#include "ShaderMangement.h"
#include "LoadM3d.h"
#include "GeometryGenerator.h"
#include "Object3D.h"

//LRESULT CALLBACK MainProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)



D3DBase::D3DBase(HINSTANCE hInstance, bool enableMSAA = false, UINT8 MSAAcount = 4, int width = 800, int height = 600, D3D_DRIVER_TYPE type = D3D_DRIVER_TYPE_HARDWARE)
:
mhAppHandle(hInstance),
mMainWnd(nullptr),
mAppPaused(false),
mMinimized(false),
mMaximized(false),
mResizing(false),
mMsaaQuality(0),
mMSAACount(MSAAcount),
mClientWidth(width),
mClientHeight(height),
mEnableMsaa(enableMSAA),

md3dDevice(nullptr),
md3dImmediateContext(nullptr),
mSwapChain(nullptr),
mDepthStencilBuffer(nullptr),
mRenderTargetView(nullptr),
mDepthStencilView(nullptr),
mWireframeRS(nullptr),
mSolid(nullptr),
mMainWndCaption(L"D3D11 Application"),
md3dDriverType(type)
{
	ZeroMemory(&mScreenViewPort,sizeof(D3D11_VIEWPORT));
}

D3DBase::~D3DBase()
{
	if (mRenderTargetView)
	{
	mRenderTargetView->Release();
	mRenderTargetView = nullptr;
	}
	if (mDepthStencilView)
	{
	mDepthStencilView->Release();
	mDepthStencilView = nullptr;
	}
	if (mWireframeRS)
	{
		mWireframeRS->Release();
		mWireframeRS = nullptr;
	}
	if (mSolid)
	{
		mSolid->Release();
		mSolid = nullptr;
	}
	if (mSwapChain)
	{
	mSwapChain->Release();
	mSwapChain = nullptr;
	}

	if (mDepthStencilBuffer)
	{
	mDepthStencilBuffer->Release();
	mDepthStencilBuffer = nullptr;
	}
	if (md3dImmediateContext)
	{
	md3dImmediateContext->ClearState();
	md3dImmediateContext->Release();
	md3dImmediateContext = nullptr;
	}
	if (md3dDevice)
	{
	md3dDevice->Release();
	md3dDevice = nullptr;
	}
	if (pFactory) {
		pFactory->Release();
		pFactory = nullptr;
	}
}

int D3DBase::run()
{
	MSG msg = { 0 };//? what's this?
	mTimer.Reset();
	while (msg.message != WM_QUIT)
	{
		if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))//if program got msg from OS, process the msg, the second param must be 0 not mMainWnd or you will not get WM_QUIT message and hang there when quit.
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else//process the game
		{
			mTimer.Tick();
			if (!mAppPaused)
			{
				CalculateFrameStats();
				Update(mTimer.DeltaTime());
				Draw();
			}
			else
			{
				//do nothing 
				//to save CPU time, use sleep function. if just use ;, the CPU will 100% overload on one core over and over in this msg loop
				Sleep(100);//sleep 100ms over and over in the msg loop, if sleep time is too long, the program response will be lagged.
			}
		}
	}
	return (int)msg.wParam;
}

bool D3DBase::Init()
{
	if (!InitMainWindow())
		return false;
	
	if (!InitDirect3D())
		return false;

	return true;
}

bool D3DBase::InitMainWindow()
{
	WNDCLASS wc;
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = MainWndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = sizeof(D3DBase*);
	wc.hInstance = mhAppHandle;
	wc.hIcon = LoadIcon(0, IDI_APPLICATION);
	wc.hCursor = LoadCursor(0, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(NULL_BRUSH);
	wc.lpszMenuName = 0;
	wc.lpszClassName = L"D3DWndClassName";

	RegisterClass(&wc);
	//if (!RegisterClass(&wc))
	{
		//MessageBox(0, L"RegisterClass Failed", 0, 0);
		//return false;
	}
	RECT R = { 0, 0, mClientWidth, mClientHeight };
	AdjustWindowRect(&R, WS_OVERLAPPEDWINDOW, false);
	int width = R.right - R.left;
	int heigth = R.bottom - R.top;
	mMainWnd = CreateWindow(L"D3DWndClassName", mMainWndCaption.c_str(), WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, width, heigth, 0, 0, mhAppHandle, this);//put "this" pointer in the createwindow
	if (mMainWnd == nullptr)
	{
		MessageBox(0, L"CreateWindow Failed", 0, 0);
		return false;
	}
	ShowWindow(mMainWnd, SW_SHOW);
	UpdateWindow(mMainWnd);

	return true;
}


LRESULT CALLBACK D3DBase::MainWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (msg == WM_NCCREATE)
	{
		//retrieve "this" pointer which has been passed by CreateWindow() last parameter: "this" pointer is stored in the first member of CREATESTRUCT lpCreateParams
		SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(reinterpret_cast<LPCREATESTRUCT>(lParam)->lpCreateParams));
		return reinterpret_cast<D3DBase*>(reinterpret_cast<LPCREATESTRUCT>(lParam)->lpCreateParams)->MsgProc(hwnd, msg, wParam, lParam);
	}
	else
	{
		D3DBase* curretnThis=reinterpret_cast<D3DBase*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
		if (!curretnThis)
			return DefWindowProc(hwnd, msg, wParam, lParam);
		else
			return curretnThis->MsgProc(hwnd, msg, wParam, lParam);
	}
		
}
LRESULT D3DBase::MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_ACTIVATE:
		if (LOWORD(wParam) == WA_INACTIVE)
		{
			mAppPaused = true;
			mTimer.Pause();
		}
		else
		{
			mAppPaused = false;
			mTimer.Resume();
		}
		return 0;
		
	case WM_SIZE:
		mClientWidth = LOWORD(lParam);
		mClientHeight = HIWORD(lParam);
		if (md3dDevice)
		{
			if (wParam == SIZE_MINIMIZED)
			{
				mAppPaused = true;
				mMinimized = true;
				mMaximized = false;
			}
			else if (wParam == SIZE_MAXIMIZED)
			{
				mAppPaused = false;
				mMinimized = false;
				mMaximized = true;
				OnResize();
			}
			else if (wParam == SIZE_RESTORED)
			{
				if (mMinimized)
				{
					mAppPaused = false;
					mMinimized = false;
					OnResize();
				}
				else if (mMaximized)
				{
					mAppPaused = false;
					mMinimized = false;
					OnResize();
				}
				else if (mResizing)
				{
					//wait until finishing resizing
				}
				else
				{
					OnResize();
				}
			}
		}
		return 0;

	case WM_ENTERSIZEMOVE:
		mAppPaused = true;
		mResizing = true;
		mTimer.Pause();
		return 0;
	case WM_EXITSIZEMOVE:
		mAppPaused = false;
		mResizing = false;
		mTimer.Resume();
		OnResize(); 
		return 0;
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	case WM_MENUCHAR:
		return MAKELRESULT(0, MNC_CLOSE);
	case WM_GETMINMAXINFO:
		reinterpret_cast<MINMAXINFO*>(lParam)->ptMinTrackSize.x = 200;
		reinterpret_cast<MINMAXINFO*>(lParam)->ptMinTrackSize.y = 200;
		return 0;
	case WM_KEYDOWN:
		OnKeyDown(wParam);
		return 0;
	case WM_SYSKEYUP:
		OnKeyDown(wParam);
		return 0;
	case WM_LBUTTONDOWN:
	case WM_MBUTTONDOWN:
	case WM_RBUTTONDOWN:
		OnMouseDown(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		return 0;
	case WM_LBUTTONUP:
	case WM_MBUTTONUP:
	case WM_RBUTTONUP:
		OnMouseUp(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		return 0;
	case WM_MOUSEMOVE:
		OnMouseMove(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		return 0;
	}
	return DefWindowProc(hwnd, msg, wParam, lParam);
}

bool D3DBase::InitDirect3D()
{
	unsigned int createDeviceFlags = 0;
#if defined(DEBUG) || defined(_DEBUG)
		createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif


		HRESULT hr;
		hr = CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)(&pFactory) );



		UINT i = 0; 
		IDXGIAdapter * pAdapter; 
		std::vector <IDXGIAdapter*> vAdapters; 
		while(pFactory->EnumAdapters(i, &pAdapter) != DXGI_ERROR_NOT_FOUND) 
		{ 
			vAdapters.push_back(pAdapter); 
			++i; 
		} 
		pFactory->Release();

		
		//see remark: https://msdn.microsoft.com/en-us/library/ff476082(v=vs.85).aspx
		//if you specify the DGXIFactory pointer, DRIVER_TYPE input must be UNKNOW, software must be null.

		D3D_FEATURE_LEVEL featureLevel;
		md3dDriverType = D3D_DRIVER_TYPE_UNKNOWN;
		hr = D3D11CreateDevice(vAdapters[0], md3dDriverType, 0, createDeviceFlags, 0, 0, D3D11_SDK_VERSION, &md3dDevice, &featureLevel, &md3dImmediateContext);
		//hr = D3D11CreateDevice(0, md3dDriverType, 0, createDeviceFlags, 0, 0, D3D11_SDK_VERSION, &md3dDevice, &featureLevel, &md3dImmediateContext);
		if (FAILED(hr))
		{
			MessageBox(0, L"D3D11CreateDevice Failed", 0, 0);
			return false;
		}
		if (featureLevel != D3D_FEATURE_LEVEL_11_0)
		{
			MessageBox(0, L"Direct3D Feature Level 11 unsupported.", 0, 0);
			return false;
		}
		md3dDevice->CheckMultisampleQualityLevels(DXGI_FORMAT_R8G8B8A8_UNORM, mMSAACount, &mMsaaQuality);
		assert(mMsaaQuality > 0);

		DXGI_SWAP_CHAIN_DESC sd;
		sd.BufferDesc.Width = mClientWidth;
		sd.BufferDesc.Height = mClientHeight;
		sd.BufferDesc.RefreshRate.Numerator = 60;
		sd.BufferDesc.RefreshRate.Denominator = 1;
		sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
		sd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

		if (mEnableMsaa)
		{
			sd.SampleDesc.Count = mMSAACount;
			sd.SampleDesc.Quality = mMsaaQuality - 1;
		}
		else
		{
			sd.SampleDesc.Count = 1;
			sd.SampleDesc.Quality = 0;
		}

		sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		sd.BufferCount = 1;
		sd.OutputWindow = mMainWnd;
		sd.Windowed = true;
		sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
		sd.Flags = 0;
		IDXGIDevice* dxgiDevice = 0;
		md3dDevice->QueryInterface(__uuidof(IDXGIDevice), (void**)&dxgiDevice);
		IDXGIAdapter* dxgiAdapter = 0;
		dxgiDevice->GetParent(__uuidof(IDXGIAdapter), (void**)&dxgiAdapter);
		IDXGIFactory* dxgiFactory = 0;
		dxgiAdapter->GetParent(__uuidof(IDXGIFactory), (void**)&dxgiFactory);
		dxgiFactory->CreateSwapChain(md3dDevice, &sd, &mSwapChain);
		if (dxgiDevice)
		{
			dxgiDevice->Release();
			dxgiDevice = nullptr;
		}
		if (dxgiAdapter)
		{
			dxgiAdapter->Release();
			dxgiAdapter = nullptr; 
		}
		if (dxgiFactory)
		{
			dxgiFactory->Release();
			dxgiFactory = nullptr;
		}

		D3D11_RASTERIZER_DESC rsDesc;
		ZeroMemory(&rsDesc, sizeof(D3D11_RASTERIZER_DESC));
		rsDesc.FillMode = D3D11_FILL_SOLID;
		rsDesc.CullMode = D3D11_CULL_NONE;
		rsDesc.FrontCounterClockwise = false;
		rsDesc.DepthClipEnable = true;
		md3dDevice->CreateRasterizerState(&rsDesc, &mSolid);

		ZeroMemory(&rsDesc, sizeof(D3D11_RASTERIZER_DESC));
		rsDesc.FillMode = D3D11_FILL_WIREFRAME;
		rsDesc.CullMode = D3D11_CULL_NONE;
		rsDesc.FrontCounterClockwise = false;
		rsDesc.DepthClipEnable = true;
		md3dDevice->CreateRasterizerState(&rsDesc, &mWireframeRS);
		D3DBase::OnResize();
		return true;
}

void D3DBase::OnResize()
{
	assert(md3dImmediateContext);
	assert(md3dDevice);
	assert(mSwapChain);

	if (mRenderTargetView)
	{
		mRenderTargetView->Release();
		mRenderTargetView = nullptr;
	}
	if (mDepthStencilView)
	{
		mDepthStencilView->Release();
		mDepthStencilView = nullptr;
	}
	if (mDepthStencilBuffer)
	{
		mDepthStencilBuffer->Release();
		mDepthStencilBuffer = nullptr;
	}

	mSwapChain->ResizeBuffers(1, mClientWidth, mClientHeight, DXGI_FORMAT_B8G8R8A8_UNORM, 0);
	ID3D11Texture2D* backBuffer;
	mSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&backBuffer));
	md3dDevice->CreateRenderTargetView(backBuffer, 0, &mRenderTargetView);

	if (backBuffer)
	{
	backBuffer->Release();
	backBuffer = nullptr;
	}


	D3D11_TEXTURE2D_DESC depthStecilDesc;
	depthStecilDesc.Width = mClientWidth;
	depthStecilDesc.Height = mClientHeight;
	depthStecilDesc.MipLevels = 1;
	depthStecilDesc.ArraySize = 1;
	depthStecilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;

	if (mEnableMsaa)
	{
		depthStecilDesc.SampleDesc.Count = mMSAACount;
		depthStecilDesc.SampleDesc.Quality = mMsaaQuality-1;
	}
	else
	{
		depthStecilDesc.SampleDesc.Count = 1;
		depthStecilDesc.SampleDesc.Quality = 0;
	}

	depthStecilDesc.Usage = D3D11_USAGE_DEFAULT;
	depthStecilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthStecilDesc.CPUAccessFlags = 0;
	depthStecilDesc.MiscFlags = 0;

	md3dDevice->CreateTexture2D(&depthStecilDesc, 0, &mDepthStencilBuffer);
	md3dDevice->CreateDepthStencilView(mDepthStencilBuffer, 0, &mDepthStencilView);
	md3dImmediateContext->OMSetRenderTargets(1, &mRenderTargetView, mDepthStencilView);


	mScreenViewPort.TopLeftX = 0;
	mScreenViewPort.TopLeftY = 0;
	mScreenViewPort.Width = static_cast<float>(mClientWidth);
	mScreenViewPort.Height = static_cast<float>(mClientHeight);
	mScreenViewPort.MinDepth = 0.0f;
	mScreenViewPort.MaxDepth = 1.0f;

	md3dImmediateContext->RSSetViewports(1, &mScreenViewPort);

}

void D3DBase::CalculateFrameStats()
{
	static int frameCnt = 0;
	static float timeElapsed = 0.0f;
	frameCnt++;
	if (mTimer.TotalTime() - timeElapsed >= 1.0f)
	{
		float fps = static_cast<float>(frameCnt);
		float mspf = 1000.0f / fps;
		std::wostringstream outs;
		outs.precision(6);
		outs << mMainWndCaption << L"  " << L"FPS: " << fps << L"  " << L"Frame Time: " << mspf << L"(ms)";
		SetWindowText(mMainWnd, outs.str().c_str());

		frameCnt = 0;
		timeElapsed += 1.0f;
		
	}
}

