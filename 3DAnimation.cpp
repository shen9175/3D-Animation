#include <d3d11_4.h>
#include <d2d1_3.h>
#include <dwrite_3.h>
#include <DirectXMath.h>
#include <queue>
#include <deque>
#include <unordered_map>
#include <sstream>
#include <string>
#include "GameTimer.h"
#include "D2DText.h"
#include "D3DBase.h"
#include "Camera.h"
#include "ConstantBuffer.h"
#include "TextureMangement.h"
#include "ShaderMangement.h"
#include "Animation.h"
#include "LoadM3d.h"
#include "GeometryGenerator.h"
#include "Object3D.h"
#include "3DAnimation.h"


D3DAnimation::D3DAnimation(HINSTANCE hInstance, bool enableMSAA, UINT8 MSAAcount, int width, int height, D3D_DRIVER_TYPE type) : D3DBase11(hInstance, enableMSAA, MSAAcount, width, height, type){
	mMainWndCaption = L"3D Animation Demo";
	//Cam.SetPosition(10.0f, 14.0f, -24.0f);
	Cam.SetPosition(0.0f, 4.0f, -15.0f);
}

D3DAnimation::~D3DAnimation() {
	if (list) {
		delete list;
		list = nullptr;
	}
}

bool D3DAnimation::Init() {
	if (!D3DBase11::Init()) {
		return false;
	}

	mLastMousePos.x = 0;
	mLastMousePos.y = 0;

	BOX_DESC box = { 1.0f, 1.0f, 1.0f, 3 };
	GRID_DESC grid = { 160.0f, 240.0f, 480, 320 };//{20.0f, 30.0f, 60, 40}
	SPHERE_DESC sphere = { 0.5f, 20, 20 };
	CYLINDER_DESC cylinder = { 0.5f, 0.3f, 3.0f, 20, 20 };

	std::vector<PosNormalTexTan> wallvertices = {
	{ {-3.5f, 0.0f, 0.0f}, {0.0f, 0.0f, -1.0f}, {0.0f, 2.0f}, {0.0f, 0.0f, 0.0f, 0.0f} },
	{ {-3.5f, 4.0f, 0.0f}, {0.0f, 0.0f, -1.0f}, {0.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f} },
	{ {-2.5f, 4.0f, 0.0f}, {0.0f, 0.0f, -1.0f}, {0.5f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f} },
	{ {-2.5f, 0.0f, 0.0f}, {0.0f, 0.0f, -1.0f}, {0.5f, 2.0f}, {0.0f, 0.0f, 0.0f, 0.0f} },

	{ {2.5f, 0.0f, 0.0f}, {0.0f, 0.0f, -1.0f}, {0.0f, 2.0f}, {0.0f, 0.0f, 0.0f, 0.0f} },
	{ {2.5f, 4.0f, 0.0f}, {0.0f, 0.0f, -1.0f}, {0.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f} },
	{ {7.5f, 4.0f, 0.0f}, {0.0f, 0.0f, -1.0f}, {2.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f} },
	{ {7.5f, 0.0f, 0.0f}, {0.0f, 0.0f, -1.0f}, {2.0f, 2.0f}, {0.0f, 0.0f, 0.0f, 0.0f} },

	{ {-3.5f, 4.0f, 0.0f}, {0.0f, 0.0f, -1.0f}, {0.0f, 1.0f}, {0.0f, 0.0f, 0.0f, 0.0f} },
	{ {-3.5f, 6.0f, 0.0f}, {0.0f, 0.0f, -1.0f}, {0.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f} },
	{  {7.5f, 6.0f, 0.0f}, {0.0f, 0.0f, -1.0f}, {6.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f} },
	{  {7.5f, 4.0f, 0.0f}, {0.0f, 0.0f, -1.0f}, {6.0f, 1.0f}, {0.0f, 0.0f, 0.0f, 0.0f} }
	};
	std::vector<unsigned int> wallindices;
	wallindices.push_back(0);
	wallindices.push_back(1);
	wallindices.push_back(2);
	wallindices.push_back(0);
	wallindices.push_back(2);
	wallindices.push_back(3);

	wallindices.push_back(4);
	wallindices.push_back(5);
	wallindices.push_back(6);
	wallindices.push_back(4);
	wallindices.push_back(6);
	wallindices.push_back(7);

	wallindices.push_back(8);
	wallindices.push_back(9);
	wallindices.push_back(10);
	wallindices.push_back(8);
	wallindices.push_back(10);
	wallindices.push_back(11);

	std::vector<PosNormalTexTan> mirrorvertices;
	mirrorvertices.push_back({ {-2.5f, 0.0f, 0.0f}, {0.0f, 0.0f, -1.0f}, {0.0f, 1.0f}, {0.0f, 0.0f, 0.0f, 0.0f}});
	mirrorvertices.push_back({ {-2.5f, 4.0f, 0.0f}, {0.0f, 0.0f, -1.0f}, {0.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f}});
	mirrorvertices.push_back({ {2.5f, 4.0f, 0.0f}, {0.0f, 0.0f, -1.0f}, {1.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f}});
	mirrorvertices.push_back({ {2.5f, 0.0f, 0.0f}, {0.0f, 0.0f, -1.0f},  {1.0f, 1.0f}, {0.0f, 0.0f, 0.0f, 0.0f}});
	std::vector<unsigned int> mirrorindices;
	mirrorindices.push_back(0);
	mirrorindices.push_back(1);
	mirrorindices.push_back(2);
	mirrorindices.push_back(0);
	mirrorindices.push_back(2);
	mirrorindices.push_back(3);

	Material mGridMat;
	Material mBoxMat;
	Material mCylinderMat;
	Material mSphereMat;
	Material mRoomMat;
	Material mMirrorMat;

	mMirrorMat.Ambient = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	mMirrorMat.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 0.5f);
	mMirrorMat.Specular = XMFLOAT4(0.4f, 0.4f, 0.4f, 16.0f);


	mGridMat.Ambient  = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
	mGridMat.Diffuse  = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
	mGridMat.Specular = XMFLOAT4(0.8f, 0.8f, 0.8f, 16.0f);

	mCylinderMat.Ambient  = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	mCylinderMat.Diffuse  = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	mCylinderMat.Specular = XMFLOAT4(0.8f, 0.8f, 0.8f, 16.0f);

	mSphereMat.Ambient  = XMFLOAT4(0.6f, 0.8f, 0.9f, 1.0f);
	mSphereMat.Diffuse  = XMFLOAT4(0.6f, 0.8f, 0.9f, 1.0f);
	mSphereMat.Specular = XMFLOAT4(0.9f, 0.9f, 0.9f, 16.0f);

	mBoxMat.Ambient  = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	mBoxMat.Diffuse  = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	mBoxMat.Specular = XMFLOAT4(0.8f, 0.8f, 0.8f, 16.0f);


	mRoomMat.Ambient = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	mRoomMat.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	mRoomMat.Specular = XMFLOAT4(0.4f, 0.4f, 0.4f, 16.0f);

	fogenable = true;
	renderWire = false;
	XMFLOAT4X4 I;
	XMFLOAT4X4 rotation;
	XMStoreFloat4x4(&rotation, XMMatrixRotationY(XM_PI));
	XMStoreFloat4x4(&I, XMMatrixIdentity());
	//instanceName;translation;rotation;scale;modelName;modelFilename;texturePath;VShaderName;PShaderName;LayoutName; vertexTypeName;
	
	std::vector<ObjectInstance_M3dModelList_DESC> objlist = {
		{"player", {0.0f, 0.0f, -8.0f}, rotation, {0.05f, 0.05f, -0.05f}, {1.0f, 1.0f, 1.0f}, "soldier", ".\\Models\\soldier.m3d", ".\\Textures\\","BasicAnimationVS","FixFunctionLightPS","PosNormalTexTanSkinned","PosNormalTexTanAnimation", {false,{0.0f, 0.0f, 0.0f, 0.0f}}}
	};
	std::vector<ObjectInstance_BOXLIST_DESC> boxlist = {
		{ "podium",{0.0f, 0.5f, 0.0f}, I, {3.0f,1.0f,3.0f}, {1.0f, 1.0f, 1.0f}, "box", mBoxMat, ".\\Textures\\", "stone.dds", "stones_nmap.dds", "BasicVS","FixFunctionLightPS","PosNormTexTan","PosNormalTexTan",box} 
	};
	
	std::vector<ObjectInstance_GRIDLIST_DESC> gridlist = {
		{ "land", {0.0f, 0.0f,0.0f}, I, {1.0f,1.0f,1.0f}, {48.0f, 64.0f, 1.0f}, "grid", mGridMat, ".\\Textures\\", "floor.dds", "floor_nmap.dds", "BasicVS","FixFunctionLightPS","PosNormTexTan","PosNormalTexTan",grid, {false,{0.0f, 0.0f, 0.0f, 0.0f}}}
	};
	
	std::vector<ObjectInstance_Custom_DESC> customlist = {
		{"wallinstance1", {-2.0f, 0.0f, 15.0f}, I, {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f, 1.0f}, "wall", mRoomMat, ".\\Textures\\","brick01.dds","bricks_nmap.dds", "BasicVS","FixFunctionLightPS","PosNormTexTan","PosNormalTexTan", wallvertices, wallindices},
		{"wallinstance2", {2.0f, 0.0f, -15.0f}, rotation, {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f, 1.0f}, "wall", mRoomMat, ".\\Textures\\","brick01.dds","bricks_nmap.dds", "BasicVS","FixFunctionLightPS","PosNormTexTan","PosNormalTexTan", wallvertices, wallindices},
		{"mirrorinstance1", {-2.0f, 0.0f, 15.0f}, I, {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f, 1.0f}, "mirror", mMirrorMat, ".\\Textures\\","ice.dds","bricks_nmap.dds", "BasicVS","FixFunctionLightPS","PosNormTexTan","PosNormalTexTan",mirrorvertices, mirrorindices, {true,{0.0f, 0.0f, 1.0f, 0.0f}}},
		{"mirrorinstance2", {2.0f, 0.0f, -15.0f}, rotation, {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f, 1.0f}, "mirror", mMirrorMat, ".\\Textures\\","ice.dds","bricks_nmap.dds", "BasicVS","FixFunctionLightPS","PosNormTexTan","PosNormalTexTan",mirrorvertices, mirrorindices, {true,{0.0f, 0.0f, 1.0f, 0.0f}}}
	};
	mDirLights[0].Ambient  = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
	mDirLights[0].Diffuse  = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	mDirLights[0].Specular = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	mDirLights[0].Direction = XMFLOAT3(0.57735f, -0.57735f, 0.57735f);
	dirLightsw[0] = true;
	mDirLights[1].Ambient  = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	mDirLights[1].Diffuse  = XMFLOAT4(0.20f, 0.20f, 0.20f, 1.0f);
	mDirLights[1].Specular = XMFLOAT4(0.25f, 0.25f, 0.25f, 1.0f);
	mDirLights[1].Direction = XMFLOAT3(-0.57735f, -0.57735f, 0.57735f);
	dirLightsw[1] = true;
	mDirLights[2].Ambient  = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	mDirLights[2].Diffuse  = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
	mDirLights[2].Specular = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	mDirLights[2].Direction = XMFLOAT3(0.0f, -0.707f, -0.707f);
	dirLightsw[2] = true;
	mSpotLights[0].Ambient = XMFLOAT4(0.2f, 1.0f, 0.2f, 1.0f);
	mSpotLights[0].Diffuse = XMFLOAT4(0.5f, 1.0f, 0.5f, 1.0f);
	mSpotLights[0].Specular = XMFLOAT4(0.5f, 1.0f, 0.5f, 1.0f);
	mSpotLights[0].Position = XMFLOAT3(0.0f, 5.0f, 0.0f);
	mSpotLights[0].Range = 25.0f;
	mSpotLights[0].Att = XMFLOAT3(0.0f, 0.15f, 0.0f);
	XMFLOAT3 dir;
	XMStoreFloat3(&dir, XMVector3Normalize(XMLoadFloat3(&XMFLOAT3(0.0f, -5.0f, -100.0f))));
	mSpotLights[0].Direction = dir;
	mSpotLights[0].Spot = 56.0f;
	SpotLightsw[0] = true;
	mSpotLights[1].Ambient = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	mSpotLights[1].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	mSpotLights[1].Specular = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	mSpotLights[1].Position = XMFLOAT3(0.0f, 2.5f, -100.0f);
	mSpotLights[1].Range = 15.0f;
	mSpotLights[1].Att = XMFLOAT3(0.0f, 0.15f, 0.0f);
	mSpotLights[1].Direction = XMFLOAT3(0.0f, 0.0f, -1.0f);
	mSpotLights[1].Spot = 50.0f;
	SpotLightsw[1] = true;
	std::vector<ObjectInstance_SPHERELIST_DESC> spherelist;
	std::vector<ObjectInstance_CYLINDERLIST_DESC> cylinderlist;
	for(int i = 0; i < 5; ++i)
	{
		ObjectInstance_CYLINDERLIST_DESC item1 = { "pillar" + std::to_string(i * 2 + 1), {-5.0f, 1.5f, -10.0f + i*5.0f}, I, {1.0f,1.0f,1.0f}, {1.0f, 1.0f, 1.0f}, "cylinder", mCylinderMat, ".\\Textures\\", "bricks.dds", "bricks_nmap.dds", "BasicVS","FixFunctionLightPS","PosNormTexTan","PosNormalTexTan", cylinder};
		ObjectInstance_CYLINDERLIST_DESC item2 = { "pillar" + std::to_string(i * 2 + 2), {+5.0f, 1.5f, -10.0f + i*5.0f}, I, {1.0f,1.0f,1.0f}, {1.0f, 1.0f, 1.0f}, "cylinder", mCylinderMat, ".\\Textures\\", "bricks.dds", "bricks_nmap.dds", "BasicVS","FixFunctionLightPS","PosNormTexTan","PosNormalTexTan",cylinder};
		cylinderlist.push_back(item1);
		cylinderlist.push_back(item2);
		ObjectInstance_SPHERELIST_DESC item3 = { "ball" + std::to_string(i * 2 + 1), {-5.0f, 3.5f, -10.0f + i*5.0f}, I, {1.0f,1.0f,1.0f}, {1.0f, 1.0f, 1.0f}, "sphere", mCylinderMat, ".\\Textures\\", "stone.dds", "stones_nmap.dds", "BasicVS","FixFunctionLightPS","PosNormTexTan","PosNormalTexTan",sphere};
		ObjectInstance_SPHERELIST_DESC item4 = { "ball" + std::to_string(i * 2 + 2), {+5.0f, 3.5f, -10.0f + i*5.0f}, I, {1.0f,1.0f,1.0f}, {1.0f, 1.0f, 1.0f}, "sphere", mCylinderMat, ".\\Textures\\", "stone.dds", "stones_nmap.dds", "BasicVS","FixFunctionLightPS","PosNormTexTan","PosNormalTexTan",sphere};
		spherelist.push_back(item3);
		spherelist.push_back(item4);

		mPointLights[i * 2].Ambient = XMFLOAT4(1.f, 0.9f, 0.3f, 1.0f);
		mPointLights[i * 2].Diffuse = XMFLOAT4(1.f, 0.9f, 0.3f, 1.0f);
		mPointLights[i * 2].Specular = XMFLOAT4(1.f, 0.9f, 0.3f, 1.0f);
		mPointLights[i * 2].Position = XMFLOAT3(-5.0f, 3.5f, -10.0f + i * 5.0f);
		mPointLights[i * 2].Att = XMFLOAT3(0.0f, 0.0f, 0.9f);
		mPointLights[i * 2].Range = 4.0f;
		pointLightsw[i * 2] = true;
		mPointLights[i * 2 + 1].Ambient = XMFLOAT4(1.f, 0.9f, 0.3f, 1.0f);
		mPointLights[i * 2 + 1].Diffuse = XMFLOAT4(1.f, 0.9f, 0.3f, 1.0f);
		mPointLights[i * 2 + 1].Specular = XMFLOAT4(1.f, 0.9f, 0.3f, 1.0f);
		mPointLights[i * 2 + 1].Position = XMFLOAT3(+5.0f, 3.5f, -10.0f + i * 5.0f);
		mPointLights[i * 2 + 1].Att = XMFLOAT3(0.0f, 0.9f, 0.0f);
		mPointLights[i * 2 + 1].Range = 4.0f;
		pointLightsw[i * 2 + 1] = true;
	}
	
	//list = new Object3DList(this);
	
	list = new Object3DList(this,objlist);
	list->AddBoxist(boxlist);
	list->AddGridList(gridlist);
	list->AddCylinderList(cylinderlist);
	list->AddSphereList(spherelist);
	
	list->AddCustomList(customlist);
	
	//list->AddM3dModelList(objlist);
	list->Init();
	Cam.SetLens(0.25f * XM_PI, getAspectRatio(), 1.0f, 1000.0f);
	return true;
}

void D3DAnimation::OnResize() {
	D3DBase11::OnResize();
	Cam.SetLens(0.25f * XM_PI, getAspectRatio(), 1.0f, 1000.0f);
}

void D3DAnimation::Update(float dt) {
	if( GetAsyncKeyState('W') & 0x8000 )
		Cam.Walk(10.0f * dt);

	if( GetAsyncKeyState('S') & 0x8000 )
		Cam.Walk(-10.0f * dt);

	if( GetAsyncKeyState('A') & 0x8000 )
		Cam.Strafe(-10.0f * dt);

	if( GetAsyncKeyState('D') & 0x8000 )
		Cam.Strafe(10.0f * dt);
	/*
	for (int i = 0; i < 10; ++i) {
		if (GetAsyncKeyState(VK_F1 + i) & 0x8000 ) {
			if (pointLightsw[i] == true) {
				pointLightsw[i] = false;
			} else {
				pointLightsw[i] = true;
			}
		}
	}
	for (int i = 0; i < 2; ++i) {
		if (GetAsyncKeyState('4' + i) & 0x8000 ) {
			if (SpotLightsw[i] == true) {
				SpotLightsw[i] = false;
			} else {
				SpotLightsw[i] = true;
			}
		}
	}
	for (int i = 0; i < 3; ++i) {
		if ((GetAsyncKeyState('1' + i) & 0x8000) && (!GetKeyState('1' + i) &0x8000)) {
			if (dirLightsw[i] == true) {
				dirLightsw[i] = false;
			} else {
				dirLightsw[i] = true;
			}
		}

	}
	*/
	Object3DInstance* player = list->getPlayer("player");
	if (player) {
		if (GetAsyncKeyState(VK_UP) & 0x8000) {
			XMMATRIX world = XMLoadFloat4x4(&(list->getPlayer("player")->mWorld));
			XMMATRIX translation;
			XMFLOAT4X4 offset = { 1.0f, 0.0f, 0.0f, 0.0f,0.0f, 1.0f, 0.0f, 0.0f,0.0f, 0.0f,1.0f, 0.0f,0.0f, 0.0f, -45.0f * dt, 1.0f };
			translation = XMLoadFloat4x4(&offset);
			XMStoreFloat4x4(&list->getPlayer("player")->mWorld, translation * world);
			//list->getPlayer("player")->mWorld._43 -= 2.5f * dt;
			list->Update(dt);
		} else if (GetAsyncKeyState(VK_DOWN) & 0x8000) {
			XMMATRIX world = XMLoadFloat4x4(&(list->getPlayer("player")->mWorld));
			XMMATRIX translation;
			XMFLOAT4X4 offset = { 1.0f, 0.0f, 0.0f, 0.0f,0.0f, 1.0f, 0.0f, 0.0f,0.0f, 0.0f,1.0f, 0.0f,0.0f, 0.0f, 45.0f * dt, 1.0f };
			translation = XMLoadFloat4x4(&offset);
			XMStoreFloat4x4(&list->getPlayer("player")->mWorld, translation * world);
			list->Update(-dt);
		} else if (GetAsyncKeyState(VK_LEFT) & 0x8000) {
			XMMATRIX world = XMLoadFloat4x4(&(list->getPlayer("player")->mWorld));
			world = XMMatrixRotationY(0.004f * XM_PI) * world;
			XMStoreFloat4x4(&(list->getPlayer("player")->mWorld), world);
			list->Update(dt);
		} else if (GetAsyncKeyState(VK_RIGHT) & 0x8000) {
			XMMATRIX world = XMLoadFloat4x4(&(list->getPlayer("player")->mWorld));
			world = XMMatrixRotationY(-0.004f * XM_PI) * world;
			XMStoreFloat4x4(&(list->getPlayer("player")->mWorld), world);
			list->Update(dt);
		} else {
			list->Update(0);
		}
	}
		Cam.UpdateViewMatrix();


	/*
	XMFLOAT3 eye = Cam.GetPosition();
	XMFLOAT3 focus = Cam.GetLook();
	std::wostringstream outs;
	outs.precision(6);
	outs << mMainWndCaption << L"                                                              " << L"eyeX = " << eye.x << L"  eyeY = " << eye.y << L"    eyeZ = "<<eye.z << L"   focusX = "<<focus.x <<L"   focusY = "<<focus.y <<L"    focusZ = "<< focus.z;
	SetWindowText(mMainWnd, outs.str().c_str());*/
}

void  D3DAnimation::Draw() {
	list->FrameDraw();

}

void D3DAnimation::OnMouseDown(WPARAM btnState, int x, int y) {
	mLastMousePos.x = x;
	mLastMousePos.y = y;
	SetCapture(mMainWnd);
}


void  D3DAnimation::OnMouseUp(WPARAM btnState, int x, int y) {
	ReleaseCapture();
}
void  D3DAnimation::OnMouseMove(WPARAM btnState, int x, int y) {
	if ((btnState & MK_LBUTTON) != 0) {
		float dx = XMConvertToRadians(0.25f * static_cast<float>(x - mLastMousePos.x));
		float dy = XMConvertToRadians(0.25f * static_cast<float>(y - mLastMousePos.y));
		Cam.Pitch(dy);
		Cam.RotateY(dx);
	}
	mLastMousePos.x = x;
	mLastMousePos.y = y;
}

void D3DAnimation::OnKeyDown(WPARAM wParam) {
	switch (wParam) {
	case VK_F1:
		if (pointLightsw[0]) {
			pointLightsw[0] = false;
		} else {
			pointLightsw[0] = true;
		}
		break;
	case VK_F2:
		if (pointLightsw[1]) {
			pointLightsw[1] = false;
		} else {
			pointLightsw[1] = true;
		}
		break;
	case VK_F3:
		if (pointLightsw[2]) {
			pointLightsw[2] = false;
		} else {
			pointLightsw[2] = true;
		}
		break;
	case VK_F4:
		if (pointLightsw[3]) {
			pointLightsw[3] = false;
		} else {
			pointLightsw[3] = true;
		}
		break;
	case VK_F5:
		if (pointLightsw[4]) {
			pointLightsw[4] = false;
		} else {
			pointLightsw[4] = true;
		}
		break;
	case VK_F6:
		if (pointLightsw[5]) {
			pointLightsw[5] = false;
		} else {
			pointLightsw[5] = true;
		}
		break;
	case VK_F7:
		if (pointLightsw[6]) {
			pointLightsw[6] = false;
		} else {
			pointLightsw[6] = true;
		}
		break;
	case VK_F8:
		if (pointLightsw[7]) {
			pointLightsw[7] = false;
		} else {
			pointLightsw[7] = true;
		}
		break;
	case VK_F9:
		if (pointLightsw[8]) {
			pointLightsw[8] = false;
		} else {
			pointLightsw[8] = true;
		}
		break;
	case VK_F10:
		if (pointLightsw[9]) {
			pointLightsw[9] = false;
		} else {
			pointLightsw[9] = true;
		}
		break;
	case '1':
		if (dirLightsw[0]) {
			dirLightsw[0] = false;
		} else {
			dirLightsw[0] = true;
		}
		break;
	case '2':
		if (dirLightsw[1]) {
			dirLightsw[1] = false;
		} else {
			dirLightsw[1] = true;
		}
		break;
	case '3':
		if (dirLightsw[2]) {
			dirLightsw[2] = false;
		} else {
			dirLightsw[2] = true;
		}
		break;
	case '4':
		if (SpotLightsw[0]) {
			SpotLightsw[0] = false;
		} else {
			SpotLightsw[0] = true;
		}
		break;
	case '5':
		if (SpotLightsw[1]) {
			SpotLightsw[1] = false;
		} else {
			SpotLightsw[1] = true;
		}
		break;
	case '6':
		if (fogenable) {
			fogenable = false;
		} else {
			fogenable = true;
		}
		break;
	case '7':
		if (renderWire) {
			renderWire = false;
		} else {
			renderWire = true;
		}
		break;
	case VK_PRIOR:
		// page up -> chose higher resolution
		D3DBase11::changeResolution(true);
		break;

	case VK_NEXT:
		// page down -> chose lower resolution
		D3DBase11::changeResolution(false);
		break;
	default:
		break;
	}

}