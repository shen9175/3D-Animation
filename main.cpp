#include <d3d11_2.h>
#include <DirectXMath.h>
#include <vector>
#include <unordered_map>
#include "GameTimer.h"
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



int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInstance, PSTR cmdLine, int showCmd)
{
#if defined(DEBUG)|defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	D3DAnimation theApp(hInstance, 1, 4, 1920, 1080, D3D_DRIVER_TYPE_HARDWARE);
	if (!theApp.Init())
		return 0;
	return theApp.run();
}