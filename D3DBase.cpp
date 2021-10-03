#include <d3d11_4.h>
#include <d2d1_3.h>
#include <dwrite_3.h>
#include <dxgi1_6.h>
#include <DirectXMath.h>
#include <Commctrl.h>
#include <windowsx.h>
#include <string>
#include <vector>
#include <deque>
#include <queue>
#include <unordered_map>
#include <assert.h>
#include <sstream>
#include "GameTimer.h"
#include "D2DText.h"
#include "D3DBase.h"
#include "Camera.h"
#include "Animation.h"
#include "TextureMangement.h"
#include "ConstantBuffer.h"
#include "ShaderMangement.h"
#include "LoadM3d.h"
#include "GeometryGenerator.h"
#include "Object3D.h"
#include "GFXSetup.h"
#include "resource.h"
#include "dxerr.h"



//LRESULT CALLBACK MainProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
#if defined(DEBUG) || defined(_DEBUG)
#ifndef HR
#define HR(x) { \
	HRESULT hr = (x);\
	if (FAILED(hr)) {\
		DXTrace(__FILEW__, (DWORD)__LINE__, hr, L#x, true);\
	}\
	}
#endif
#else
#ifndef HR
#define HR(x) (x)
#endif
#endif



D3DBase11::D3DBase11(HINSTANCE hInstance, bool enableMSAA = false, UINT8 MSAAcount = 4, int width = 800, int height = 600, D3D_DRIVER_TYPE type = D3D_DRIVER_TYPE_HARDWARE)
	:
	mhAppHandle(hInstance),
	mMainWnd(nullptr),
	mAppPaused(false),
	mMinimized(false),
	mMaximized(false),
	mResizing(false),
	vsync(false),
	mMsaaQuality(0),
	mMSAACount(MSAAcount),
	mAFCount(0),
	mClientWidth(width),
	mClientHeight(height),
	mEnableMsaa(enableMSAA),
	mBufferCount(2),
	Screen(2),
	TextureFilteringType(1),
	currentlyInFullscreen(false),
	currentModeIndex(0),

	md3dDevice(nullptr),
	md3dImmediateContext(nullptr),
	mSwapChain(nullptr),
	mDepthStencilBuffer(nullptr),
	mRenderTargetView(nullptr),
	mDepthStencilView(nullptr),
	mMainWndCaption(L"D3D11 Application"),
	md3dDriverType(type),
	format(DXGI_FORMAT_R8G8B8A8_UNORM),
	software_rasterizer(nullptr) {
	ZeroMemory(&mScreenViewPort, sizeof(D3D11_VIEWPORT));
}

D3DBase11::~D3DBase11() {
	if (pD2DText) {
		
		delete pD2DText;
		pD2DText = nullptr;
	}

	if (pDisplayModes) {
		delete[] pDisplayModes;
		pDisplayModes = nullptr;
	}

	if (mSwapChain) {
		mSwapChain->SetFullscreenState(false, nullptr);
	}
	if (mRenderTargetView) {
		mRenderTargetView->Release();
		mRenderTargetView = nullptr;
	}
	if (mDepthStencilView) {
		mDepthStencilView->Release();
		mDepthStencilView = nullptr;
	}
	if (mSwapChain) {
		mSwapChain->Release();
		mSwapChain = nullptr;
	}

	if (mDepthStencilBuffer) {
		mDepthStencilBuffer->Release();
		mDepthStencilBuffer = nullptr;
	}

	

	if (md3dImmediateContext) {
		md3dImmediateContext->ClearState();
		md3dImmediateContext->Release();
		md3dImmediateContext = nullptr;
	}
	if (md3dDevice) {
		md3dDevice->Release();
		md3dDevice = nullptr;
	}
}

int D3DBase11::run() {
	MSG msg = { 0 };
	mTimer.Reset();
	while (msg.message != WM_QUIT) {
		if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))//if program got msg from OS, process the msg, the second param must be 0 not mMainWnd or you will not get WM_QUIT message and hang there when quit.
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		} else//process the game
		{
			mTimer.Tick();
			if (!mAppPaused) {
				CalculateFrameStats();
				Update(mTimer.DeltaTime());
				Draw();
			} else {
				//do nothing 
				//to save CPU time, use sleep function. if just use ;, the CPU will 100% overload on one core over and over in this msg loop
				Sleep(100);//sleep 100ms over and over in the msg loop, if sleep time is too long, the program response will be lagged.
			}
		}
	}
	return (int)msg.wParam;
}

bool D3DBase11::Init() {


	if (!InitMainWindow())
		return false;

	//ShowWindow(mMainWnd, SW_HIDE);
	GraphicPropertiesSetup gps(mhAppHandle, mMainWnd);
	if (!gps.GetSelectedProperties().apply) {
		return false;
	}

	mMSAACount = gps.GetSelectedProperties().MSAA;
	mAFCount = gps.GetSelectedProperties().AF;
	format = gps.GetSelectedProperties().format;
	mEnableMsaa = gps.GetSelectedProperties().bMSAA;
	Screen = gps.GetSelectedProperties().screen;
	//OutputDebugString((L"Screen=" + std::to_wstring(Screen) + L"\n").c_str());
	TextureFilteringType = gps.GetSelectedProperties().filtering;
	//OutputDebugString((L"Texture Filtering Type = " + std::to_wstring(TextureFilteringType)+L"\n").c_str());
	vsync = gps.GetSelectedProperties().VSync;
	mBufferCount = gps.GetSelectedProperties().BufferCount;
	//OutputDebugString((L"Buffer Count=" + std::to_wstring(mBufferCount) + L"\n").c_str());
	mTotalDisplayModes = gps.GetSelectedProperties().mTotalDisplayModes;
	pDisplayModes = new DXGI_MODE_DESC[mTotalDisplayModes];
	for (UINT i = 0; i < mTotalDisplayModes; ++i) {
		pDisplayModes[i] = gps.GetSelectedProperties().pDisplayModes[i];
	}
	currentModeIndex = gps.GetSelectedProperties().CurrentSelectedDisplayModeIndex;
	mClientWidth = pDisplayModes[currentModeIndex].Width;
	mClientHeight = pDisplayModes[currentModeIndex].Height;
	RECT R;
	GetClientRect(GetDesktopWindow(), &R);
	MoveWindow(mMainWnd, (R.right - mClientWidth) / 2 < 0 ? 0: (R.right - mClientWidth) / 2, (R.bottom-mClientHeight) / 2 < 0 ? 0 : (R.bottom - mClientHeight) / 2, mClientWidth, mClientHeight, 0);
	ShowWindow(mMainWnd, SW_SHOW);

	if (!InitDirect3D(gps.GetSelectedProperties().pAdapter))
		return false;

	return true;
}

bool D3DBase11::InitMainWindow() {
	WNDCLASS wc;
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = MainWndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = sizeof(D3DBase11*);
	wc.hInstance = mhAppHandle;
	wc.hIcon = LoadIcon(0, IDI_APPLICATION);
	wc.hCursor = LoadCursor(0, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(NULL_BRUSH);
	wc.lpszMenuName = 0;
	wc.lpszClassName = L"D3DWndClassName";

	RegisterClass(&wc);

	RECT R = { 0, 0, mClientWidth, mClientHeight };
	AdjustWindowRect(&R, WS_OVERLAPPEDWINDOW, false);
	int width = R.right - R.left;
	int heigth = R.bottom - R.top;

	RECT Desktop;
	GetClientRect(GetDesktopWindow(), &Desktop);

	mMainWnd = CreateWindow(L"D3DWndClassName", mMainWndCaption.c_str(), WS_OVERLAPPEDWINDOW, (Desktop.right - width) / 2 < 0 ? 0 : (Desktop.right - width) / 2, (Desktop.bottom - heigth) / 2 < 0 ? 0 : (Desktop.bottom - heigth) / 2, width, heigth, 0, 0, mhAppHandle, this);//put "this" pointer in the createwindow
	if (mMainWnd == nullptr) {
		MessageBox(0, L"CreateWindow Failed", 0, 0);
		return false;
	}
	//ShowWindow(mMainWnd, SW_SHOW);
	UpdateWindow(mMainWnd);

	return true;
}


LRESULT CALLBACK D3DBase11::MainWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	if (msg == WM_NCCREATE) {
		//retrieve "this" pointer which has been passed by CreateWindow() last parameter: "this" pointer is stored in the first member of CREATESTRUCT lpCreateParams
		SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(reinterpret_cast<LPCREATESTRUCT>(lParam)->lpCreateParams));
		return reinterpret_cast<D3DBase11*>(reinterpret_cast<LPCREATESTRUCT>(lParam)->lpCreateParams)->MsgProc(hwnd, msg, wParam, lParam);
	} else {
		D3DBase11* curretnThis = reinterpret_cast<D3DBase11*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
		if (!curretnThis)
			return DefWindowProc(hwnd, msg, wParam, lParam);
		else
			return curretnThis->MsgProc(hwnd, msg, wParam, lParam);
	}

}
LRESULT D3DBase11::MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	switch (msg) {
	case WM_ACTIVATE:
		if (LOWORD(wParam) == WA_INACTIVE) {
			mAppPaused = true;
			mTimer.Pause();
		} else {
			mAppPaused = false;
			mTimer.Resume();
		}
		return 0;

	case WM_SIZE:
		mClientWidth = LOWORD(lParam);
		mClientHeight = HIWORD(lParam);
		if (md3dDevice) {
			if (wParam == SIZE_MINIMIZED) {
				mAppPaused = true;
				mMinimized = true;
				mMaximized = false;
			} else if (wParam == SIZE_MAXIMIZED) {
				mAppPaused = false;
				mMinimized = false;
				mMaximized = true;
				OnResize();
			} else if (wParam == SIZE_RESTORED) {
				if (mMinimized) {
					mAppPaused = false;
					mMinimized = false;
					OnResize();
				} else if (mMaximized) {
					mAppPaused = false;
					mMinimized = false;
					OnResize();
				} else if (mResizing) {
					//wait until finishing resizing
				} else {
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





bool D3DBase11::InitDirect3D(IDXGIAdapter* pAdapter = nullptr) {

	// This flag adds support for surfaces with a different color channel ordering than the API default.
	// You need it for compatibility with Direct2D.
	//https://docs.microsoft.com/en-us/windows/win32/direct2d/devices-and-device-contexts
	//If you don't set this D3D11_CREATE_DEVICE_BGRA_SUPPORT flag, D2D create device will failed.
	unsigned int createDeviceFlags =  D3D11_CREATE_DEVICE_BGRA_SUPPORT;
	
	
#if defined(DEBUG) || defined(_DEBUG)
	createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif





/*
#include <wrl.h>
using Microsoft::WRL::ComPtr;

ComPtr<IDXGIFactory2> m_dxgiFactory;
ComPtr<IDXGIFactory5> factory5;

HRESULT hr;
IDXGIFactory2* m_dxgiFactory;
IDXGIFactory5* factory5;
hr = m_dxgiFactory->QueryInterface(__uuidof(IDXGIFactory5), &factory5);
=>hr = m_dxgiFactory->QueryInterface(IID_PPV_ARGS(&factory5));
=>hr = m_dxgiFactory.As(&factory5);

if (SUCCEEDED(hr))

{

	hr = factory5->CheckFeatureSupport(DXGI_FEATURE_PRESENT_ALLOW_TEARING, &allowTearing, sizeof(allowTearing));

}
*/


	//see remark: https://msdn.microsoft.com/en-us/library/ff476082(v=vs.85).aspx
	//if you specify the DGXIFactory pointer, DRIVER_TYPE input must be UNKNOW, software must be null.

	D3D_FEATURE_LEVEL featureLevel;
	if (pAdapter) {
		md3dDriverType = D3D_DRIVER_TYPE_UNKNOWN;
	} else {
		if (software_rasterizer == nullptr) {
			if (md3dDriverType == D3D_DRIVER_TYPE_SOFTWARE) {
				MessageBox(0, L"No Software Rasterizer Specified!", 0, 0);
				return false;
			} else {
				md3dDriverType = D3D_DRIVER_TYPE_HARDWARE;
			}
		} else {
			md3dDriverType = D3D_DRIVER_TYPE_SOFTWARE;
		}
	}
	const D3D_FEATURE_LEVEL pFeatureLevels[9] = {
			D3D_FEATURE_LEVEL_12_1,
			D3D_FEATURE_LEVEL_12_0,
			D3D_FEATURE_LEVEL_11_1,
			D3D_FEATURE_LEVEL_11_0,
			D3D_FEATURE_LEVEL_10_1,
			D3D_FEATURE_LEVEL_10_0,
			D3D_FEATURE_LEVEL_9_3,
			D3D_FEATURE_LEVEL_9_2,
			D3D_FEATURE_LEVEL_9_1
			//D3D_FEATURE_LEVEL_1_0_CORE  --> this will cause create d3d11 device failed.
	};

	HRESULT hr;
	hr = D3D11CreateDevice(pAdapter, md3dDriverType, software_rasterizer, createDeviceFlags, pFeatureLevels, sizeof(pFeatureLevels) / sizeof(D3D_FEATURE_LEVEL), D3D11_SDK_VERSION, &md3dDevice, &featureLevel, &md3dImmediateContext);
	//hr = D3D11CreateDevice(pAdapter, md3dDriverType, software_rasterizer, createDeviceFlags, 0, 0, D3D11_SDK_VERSION, &md3dDevice, &featureLevel, &md3dImmediateContext);
	//hr = D3D11CreateDevice(0, md3dDriverType, 0, createDeviceFlags, 0, 0, D3D11_SDK_VERSION, &md3dDevice, &featureLevel, &md3dImmediateContext);
	if (FAILED(hr)) {
		MessageBox(0, L"D3D11CreateDevice Failed", 0, 0);
		return false;
	}

	if (featureLevel != D3D_FEATURE_LEVEL_11_0 && featureLevel != D3D_FEATURE_LEVEL_11_1 && featureLevel != D3D_FEATURE_LEVEL_12_0 && featureLevel != D3D_FEATURE_LEVEL_12_1) {
		MessageBox(0, L"Direct3D Feature Level 11 or above unsupported.", 0, 0);
		return false;
	}
	md3dDevice->CheckMultisampleQualityLevels(format, mMSAACount, &mMsaaQuality);
	assert(mMsaaQuality > 0);

	DXGI_SWAP_CHAIN_DESC sd;
	sd.BufferDesc = pDisplayModes[currentModeIndex];
	if (mEnableMsaa && Screen != 1) {
		sd.SampleDesc.Count = mMSAACount;
		sd.SampleDesc.Quality = mMsaaQuality - 1;
	} else {
		sd.SampleDesc.Count = 1;
		sd.SampleDesc.Quality = 0;
	}

	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.BufferCount = mBufferCount;
	sd.OutputWindow = mMainWnd;
	sd.Windowed = true;
	if (Screen == 1) {
		sd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	} else {
		sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	}
	sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;




/*
	IDXGIDevice* dxgiDevice = 0;
	HR(md3dDevice->QueryInterface(__uuidof(IDXGIDevice), (void**)&dxgiDevice));



	IDXGIAdapter* dxgiAdapter = 0;
	dxgiDevice->GetParent(__uuidof(IDXGIAdapter), (void**)&dxgiAdapter);
	*/
	IDXGIFactory7* dxgiFactory = 0;
	CreateDXGIFactory(__uuidof(IDXGIFactory7), (void**)(&dxgiFactory));
	
	//dxgiAdapter->GetParent(__uuidof(IDXGIFactory), (void**)&dxgiFactory);
	dxgiFactory->CreateSwapChain(md3dDevice, &sd, &mSwapChain);
	/*

	if (dxgiDevice)
	{
		dxgiDevice->Release();
		dxgiDevice = nullptr;
	}
	if (dxgiAdapter)
	{
		dxgiAdapter->Release();
		dxgiAdapter = nullptr; 
	}*/
	if (dxgiFactory) {
		dxgiFactory->Release();
		dxgiFactory = nullptr;
	}

	pD2DText = new D2DText(md3dDevice, mSwapChain, format);

	pD2DText->CreateTextFormat(L"Times New Roman", nullptr, DWRITE_FONT_WEIGHT_LIGHT, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, 24.0f, L"en-US");
	pD2DText->SetTextAlignMent(DWRITE_TEXT_ALIGNMENT_LEADING);
	pD2DText->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_NEAR);

	if (Screen != 2) {
		currentlyInFullscreen = true;
	} else {
		currentlyInFullscreen = false;
	}

	//currentlyInFullscreen = false;
	mSwapChain->SetFullscreenState(currentlyInFullscreen, nullptr);
	//OutputDebugString(L"before initial on resize\n");
	D3DBase11::OnResize();
	//OutputDebugString(L"after initial on resize\n");
	return true;
}

void D3DBase11::OnResize() {
	assert(md3dImmediateContext);
	assert(md3dDevice);
	assert(mSwapChain);



	// switched to windowed -> simply set fullscreen mode to false
	DXGI_MODE_DESC current = pDisplayModes[currentModeIndex];
	current.Width = mClientWidth;
	current.Height = mClientHeight;
	current.RefreshRate.Denominator = 0;
	current.RefreshRate.Numerator = 0;
	mSwapChain->ResizeTarget(&current);
	// check for fullscreen switch
	BOOL inFullscreen = false;
	mSwapChain->GetFullscreenState(&inFullscreen, nullptr);
	if (currentlyInFullscreen != inFullscreen) {
		currentlyInFullscreen = !currentlyInFullscreen; //full screen mode switch
	}
	//currentlyInFullscreen = false;
	// release and reset all resources
	mSwapChain->SetFullscreenState(currentlyInFullscreen, nullptr);

	pD2DText->ClearRenderTarget();

	//need destroy all these back buffer reference just before the ResizeBuffer, it used to be placed in the begining of this OnReSize(), but seems the instruction pointer rerun from begining after ResizeTarget and after finish this OnResize() it will come back just after ResizeTarget at BOOL inFullscree = false; This skip the previous release back buffer part.
	md3dImmediateContext->ClearState();
	if (mRenderTargetView) {
		mRenderTargetView->Release();
		mRenderTargetView = nullptr;
	}
	if (mDepthStencilView) {
		mDepthStencilView->Release();
		mDepthStencilView = nullptr;
	}
	if (mDepthStencilBuffer) {
		mDepthStencilBuffer->Release();
		mDepthStencilBuffer = nullptr;
	}
	//OutputDebugString((L"mClientWidth=" + std::to_wstring(mClientWidth) + L", mClientHeight=" + std::to_wstring(mClientHeight) + L"\n").c_str());
	mSwapChain->ResizeBuffers(mBufferCount, mClientWidth, mClientHeight, format, DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH);


	ID3D11Texture2D* backBuffer;

	if (mEnableMsaa && Screen == 1) {
		//Flip mode need to set MSAA count here:
		// Create an MSAA render target.
		D3D11_TEXTURE2D_DESC backbufferDesc;
		backbufferDesc.Format = format;
		backbufferDesc.Width = mClientWidth;
		backbufferDesc.Height = mClientHeight;
		backbufferDesc.MipLevels = 1;
		backbufferDesc.ArraySize = 1;
		backbufferDesc.SampleDesc.Count = mMSAACount;
		backbufferDesc.SampleDesc.Quality = mMsaaQuality - 1;
		backbufferDesc.Usage = D3D11_USAGE_DEFAULT;
		backbufferDesc.BindFlags = D3D11_BIND_RENDER_TARGET;
		backbufferDesc.CPUAccessFlags = 0;
		backbufferDesc.MiscFlags = 0;
		//Could use CD3D11_TEXTURE2D_DESC initialization creation backbufferDesc
		HR(md3dDevice->CreateTexture2D(&backbufferDesc, 0, &backBuffer));
		CD3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc(D3D11_RTV_DIMENSION_TEXTURE2DMS, format);
		HR(md3dDevice->CreateRenderTargetView(backBuffer, &renderTargetViewDesc, &mRenderTargetView));
	} else {
		mSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&backBuffer));
		HR(md3dDevice->CreateRenderTargetView(backBuffer, nullptr, &mRenderTargetView));
	}

	

	if (backBuffer) {
		backBuffer->Release();
		backBuffer = nullptr;
	}

	pD2DText->CreateRenderTarget();
	D3D11_TEXTURE2D_DESC depthStecilDesc;
	depthStecilDesc.Width = mClientWidth;
	depthStecilDesc.Height = mClientHeight;
	depthStecilDesc.MipLevels = 1;
	depthStecilDesc.ArraySize = 1;
	depthStecilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;

	if (mEnableMsaa) {
		depthStecilDesc.SampleDesc.Count = mMSAACount;
		depthStecilDesc.SampleDesc.Quality = mMsaaQuality - 1;
	} else {
		depthStecilDesc.SampleDesc.Count = 1;
		depthStecilDesc.SampleDesc.Quality = 0;
	}

	depthStecilDesc.Usage = D3D11_USAGE_DEFAULT;
	depthStecilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthStecilDesc.CPUAccessFlags = 0;
	depthStecilDesc.MiscFlags = 0;

	md3dDevice->CreateTexture2D(&depthStecilDesc, 0, &mDepthStencilBuffer);
	md3dDevice->CreateDepthStencilView(mDepthStencilBuffer, 0, &mDepthStencilView);
	if (Screen != 1){
		md3dImmediateContext->OMSetRenderTargets(1, &mRenderTargetView, mDepthStencilView);
	}
	


	mScreenViewPort.TopLeftX = 0;
	mScreenViewPort.TopLeftY = 0;
	mScreenViewPort.Width = static_cast<float>(mClientWidth);
	mScreenViewPort.Height = static_cast<float>(mClientHeight);
	mScreenViewPort.MinDepth = 0.0f;
	mScreenViewPort.MaxDepth = 1.0f;

	md3dImmediateContext->RSSetViewports(1, &mScreenViewPort);
	
	

}
void D3DBase11::changeResolution(bool increase) {
	if (increase) {
		// if increase is true, choose a higher resolution, if possible
		if (currentModeIndex < mTotalDisplayModes - 1) {
			currentModeIndex++;
			mClientHeight = pDisplayModes[currentModeIndex].Height;
			mClientWidth = pDisplayModes[currentModeIndex].Width;
			OnResize();
		}
	} else {
		// else choose a smaller resolution, but only if possible
		if (currentModeIndex > 0) {
			currentModeIndex--;
			mClientHeight = pDisplayModes[currentModeIndex].Height;
			mClientWidth = pDisplayModes[currentModeIndex].Width;
			OnResize();
		}
	}
}
void D3DBase11::CalculateFrameStats() {
	static int frameCnt = 0;
	static float timeElapsed = 0.0f;
	frameCnt++;
	if (mTimer.TotalTime() - timeElapsed >= 1.0f) {
		float fps = static_cast<float>(frameCnt);
		float mspf = 1000.0f / fps;
		std::wostringstream outs;
		outs.precision(6);
		outs << mMainWndCaption << L"  " << L"FPS: " << fps << L"  " << L"Frame Time: " << mspf << L"(ms)";
		SetWindowText(mMainWnd, outs.str().c_str());
		
		// create FPS information text layout
		std::wostringstream outFPS;
		outFPS.precision(6);
		outFPS << "Resolution: " << pDisplayModes[currentModeIndex].Width << " x " << pDisplayModes[currentModeIndex].Height << " @ " << pDisplayModes[currentModeIndex].RefreshRate.Numerator / pDisplayModes[currentModeIndex].RefreshRate.Denominator << " Hz" << std::endl;
		outFPS << "Mode #" << currentModeIndex + 1 << " of " << mTotalDisplayModes << std::endl;
		outFPS << "FPS: " << fps << std::endl;
		outFPS << "mSPF: " << mspf << std::endl;
		pD2DText->CreateTextLayout(outFPS.str().c_str(), (UINT32)outFPS.str().size(), static_cast<FLOAT>(mClientWidth), static_cast<FLOAT>(mClientHeight));
	
		frameCnt = 0;
		timeElapsed += 1.0f;

	}
}
