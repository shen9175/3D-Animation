#include <d3d11_4.h>
#include <d2d1_3.h>
#include <dwrite_3.h>
#include <DirectXMath.h>
#include <vector>
#include <queue>
#include <deque>
#include <unordered_map>
#include "GameTimer.h"
#include "D2DText.h"
#include "D3DBase.h"
#include "Camera.h"
#include "TextureMangement.h"
#include "Animation.h"
#include "LoadM3d.h"
#include "ConstantBuffer.h"
#include "ShaderMangement.h"
#include "GeometryGenerator.h"
#include "Object3D.h"
#include "3DAnimation.h"
#include "resource.h"
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d2d1.lib")
#pragma comment(lib, "dwrite.lib")
#pragma comment(lib, "Comctl32.lib")


#if defined(DEBUG) || defined(_DEBUG)
#ifndef HR
#define HR(x) { \
	hr = (x);\
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




int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR szCmdLine, int iCmdShow) {

#if defined(DEBUG)|defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif
	D3DAnimation a(hInstance, 1, 4, 800, 600, D3D_DRIVER_TYPE_HARDWARE);
	if (!a.Init())
		return 0;
	return a.run();
	//int abc = DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_DirectX_Setup), NULL, reinterpret_cast<DLGPROC>(DialogProc), 0);

}


