#include <d3d11_2.h>
#include <DirectXMath.h>
#include <assert.h>
#include <string>
#include <vector>
#include <tuple>
#include <typeinfo>
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
#include "GeometryGenerator.h"
using namespace DirectX;





XMMATRIX InverseTranspose(CXMMATRIX M)
{
	// Inverse-transpose is just applied to normals.  So zero out 
	// translation row so that it doesn't get into our inverse-transpose
	// calculation--we don't want the inverse-transpose of the translation.
	XMMATRIX A = M;
	A.r[3] = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);

	XMVECTOR det = XMMatrixDeterminant(A);
	return XMMatrixTranspose(XMMatrixInverse(&det, A));
}




Object3D::Object3D(const std::string& modelFileName, const std::string& texturePath, TextureMangement& texMgr, ID3D11VertexShader* vshader, ID3D11PixelShader* pshader, ID3D11InputLayout* layout, std::string typestring)
: VertexCountBytes(0), VertexOffsetBytes(0), IndexOffset(0),IndexCount(0), vs(vshader), ps(pshader), InputLayout(layout), typesize(0), vertextype(typestring) {
	std::vector<M3dMaterial> mats;
	M3DLoader m3dloader;
	if (vertextype == "PosNormalTex") {
		typesize = sizeof(PosNormalTex);
	} else if (vertextype == "PosNormalTexTan") {
		typesize = sizeof(PosNormalTexTan);
	} else if (vertextype == "PosNormalTexTanAnimation") {
		typesize = sizeof(PosNormalTexTanAnimation);
	} else {
		std::string msg = "The vertex type has not been supported";
		std::wstring wmsg;
		wmsg.assign(msg.cbegin(), msg.cend());
		MessageBox(0, wmsg.c_str(), 0, 0);
	}

	size_t VertexCount = 0;
	m3dloader.LoadM3d(modelFileName, Vertices, Indices, Subsets, mats, Animations, vertextype, VertexCount);
	VertexCountBytes = VertexCount * typesize;
	SubsetCount = mats.size();
	IndexCount = Indices.size();
	for (auto i = 0; i < SubsetCount; ++i) {
		Mat.push_back(mats[i].Mat);
		ID3D11ShaderResourceView* diffuseMapSRV;
		ID3D11ShaderResourceView* normalMapSRV;
		diffuseMapSRV = texMgr.LoadTexture(texturePath + mats[i].DiffuseMapName);
		DiffuseMapSRV.push_back(diffuseMapSRV);
		normalMapSRV = texMgr.LoadTexture(texturePath + mats[i].NormalMapName);
		NormalMapSRV.push_back(normalMapSRV);
	}
}

Object3D::Object3D(const Material&  material, const std::string& texturePath, const std::string& diffuseMap, const std::string& normalMap, TextureMangement& texMgr, ID3D11VertexShader* vshader, ID3D11PixelShader* pshader, ID3D11InputLayout* layout, std::string typestring, const LAND_DESC& desc)
: VertexCountBytes(0), VertexOffsetBytes(0), IndexOffset(0),IndexCount(0), vs(vshader), ps(pshader), InputLayout(layout), typesize(0), vertextype(typestring){
	GeometryGenerator gen;
	size_t VertexCount = 0;
	gen.CreateLand(desc, Vertices, Indices, VertexCount);
	ObjectInitialHelper(VertexCount, material, texturePath, diffuseMap, normalMap, texMgr);
}
Object3D::Object3D(const Material&  material, const std::string& texturePath, const std::string& diffuseMap, const std::string& normalMap, TextureMangement& texMgr, ID3D11VertexShader* vshader, ID3D11PixelShader* pshader, ID3D11InputLayout* layout, std::string typestring, const GRID_DESC& desc)
	: VertexCountBytes(0), VertexOffsetBytes(0), IndexOffset(0),IndexCount(0), vs(vshader), ps(pshader), InputLayout(layout), typesize(0), vertextype(typestring){
	GeometryGenerator gen;
	size_t VertexCount = 0;
	gen.CreateGrid(desc, Vertices, Indices, VertexCount);
	ObjectInitialHelper(VertexCount, material, texturePath, diffuseMap, normalMap, texMgr);
}
Object3D::Object3D(const Material&  material, const std::string& texturePath, const std::string& diffuseMap, const std::string& normalMap, TextureMangement& texMgr, ID3D11VertexShader* vshader, ID3D11PixelShader* pshader, ID3D11InputLayout* layout, std::string typestring, const CYLINDER_DESC& desc)
	: VertexCountBytes(0), VertexOffsetBytes(0), IndexOffset(0),IndexCount(0), vs(vshader), ps(pshader), InputLayout(layout), typesize(0), vertextype(typestring){
	GeometryGenerator gen;
	size_t VertexCount = 0;
	gen.CreateCylinder(desc, Vertices, Indices, VertexCount);
	ObjectInitialHelper(VertexCount, material, texturePath, diffuseMap, normalMap, texMgr);
}
Object3D::Object3D(const Material&  material, const std::string& texturePath, const std::string& diffuseMap, const std::string& normalMap, TextureMangement& texMgr, ID3D11VertexShader* vshader, ID3D11PixelShader* pshader, ID3D11InputLayout* layout, std::string typestring, const GEOSPHERE_DESC& desc)
	: VertexCountBytes(0), VertexOffsetBytes(0), IndexOffset(0),IndexCount(0), vs(vshader), ps(pshader), InputLayout(layout), typesize(0), vertextype(typestring){
	GeometryGenerator gen;
	size_t VertexCount = 0;
	gen.CreateGeosphere(desc, Vertices, Indices, VertexCount);
	ObjectInitialHelper(VertexCount, material, texturePath, diffuseMap, normalMap, texMgr);
}
Object3D::Object3D(const Material&  material, const std::string& texturePath, const std::string& diffuseMap, const std::string& normalMap, TextureMangement& texMgr, ID3D11VertexShader* vshader, ID3D11PixelShader* pshader, ID3D11InputLayout* layout, std::string typestring, const SPHERE_DESC& desc)
	: VertexCountBytes(0), VertexOffsetBytes(0), IndexOffset(0),IndexCount(0), vs(vshader), ps(pshader), InputLayout(layout), typesize(0), vertextype(typestring){
	GeometryGenerator gen;
	size_t VertexCount = 0;
	gen.CreateSphere(desc, Vertices, Indices, VertexCount);
	ObjectInitialHelper(VertexCount, material, texturePath, diffuseMap, normalMap, texMgr);
}
Object3D::Object3D(const Material&  material, const std::string& texturePath, const std::string& diffuseMap, const std::string& normalMap, TextureMangement& texMgr, ID3D11VertexShader* vshader, ID3D11PixelShader* pshader, ID3D11InputLayout* layout, std::string typestring, const BOX_DESC& desc)
	: VertexCountBytes(0), VertexOffsetBytes(0), IndexOffset(0),IndexCount(0), vs(vshader), ps(pshader), InputLayout(layout), typesize(0), vertextype(typestring){
	GeometryGenerator gen;
	size_t VertexCount = 0;
	gen.CreateBox(desc, Vertices, Indices, VertexCount);
	ObjectInitialHelper(VertexCount, material, texturePath, diffuseMap, normalMap, texMgr);
}
void Object3D::ObjectInitialHelper(const size_t& VertexCount, const Material& material, const std::string& texturePath, const std::string& diffuseMap, const std::string& normalMap, TextureMangement& texMgr) {
	if (vertextype == "PosNormalTex") {
		typesize = sizeof(PosNormalTex);
	} else if (vertextype == "PosNormalTexTan") {
		typesize = sizeof(PosNormalTexTan);
	} else if (vertextype == "PosNormalTexTanAnimation") {
		typesize = sizeof(PosNormalTexTanAnimation);
	} else {
		std::string msg = "The vertex type has not been supported";
		std::wstring wmsg;
		wmsg.assign(msg.cbegin(), msg.cend());
		MessageBox(0, wmsg.c_str(), 0, 0);
	}
	VertexCountBytes = VertexCount * typesize;
	std::vector<M3dMaterial> mats;
	M3dMaterial item;
	item.DiffuseMapName = diffuseMap;
	item.NormalMapName = normalMap;
	item.Mat = material;
	item.AlphaClip = true;
	item.EffectTypeName = "none";
	mats.push_back(item);

	SubsetCount = mats.size();
	IndexCount = Indices.size();
	Subset sbset;
	sbset.Id = 0;
	sbset.VertexStart = 0;
	sbset.VertexCount = static_cast<UINT>(VertexCount);
	sbset.FaceStart = 0;
	sbset.FaceCount = static_cast<UINT>(IndexCount / 3);
	Subsets.push_back(sbset);
	for (auto i = 0; i < SubsetCount; ++i) {
		Mat.push_back(mats[i].Mat);
		ID3D11ShaderResourceView* diffuseMapSRV;
		ID3D11ShaderResourceView* normalMapSRV;
		diffuseMapSRV = texMgr.LoadTexture(texturePath + mats[i].DiffuseMapName);
		DiffuseMapSRV.push_back(diffuseMapSRV);
		normalMapSRV = texMgr.LoadTexture(texturePath + mats[i].NormalMapName);
		NormalMapSRV.push_back(normalMapSRV);
	}
}

void Object3DInstance::MoveTo(CXMMATRIX translation, CXMMATRIX scale, CXMMATRIX rotation) {
	XMMATRIX world = XMLoadFloat4x4(&mWorld);
	world *= translation * scale * rotation;
	XMStoreFloat4x4(&mWorld, world);
	XMStoreFloat3(&mUP, XMVector3Transform(XMLoadFloat3(&mUP), world));
	XMStoreFloat3(&mForward, XMVector3Transform(XMLoadFloat3(&mForward), world));
	XMStoreFloat3(&mRight, XMVector3Transform(XMLoadFloat3(&mRight), world));	
}

void Object3DInstance::Update(float dt) {
	TimePos += dt;
	//ClipName = Some character control update function; it should change dynamically in real game with multiple clips
	if (!FinalTransforms.empty()) {
		Model->Animations.GetFinalTransfroms(ClipName, TimePos, FinalTransforms);
		if (TimePos > Model->Animations.GetClipEndTime(ClipName)) {
			TimePos = 0.0f;
		}
		if (TimePos < Model->Animations.GetClipStartTime(ClipName)) {
			TimePos = Model->Animations.GetClipEndTime(ClipName);
		}		
	}
}

void Object3DInstance::ObjectInstanceDraw(ID3D11Device* md3dDevice, ID3D11DeviceContext* md3dImmediateContext, ID3D11Buffer* VB, ID3D11Buffer* IB, Camera& Cam, ConstantBuffer<PerObjectCB>& mPerObjectCB, std::unordered_map <unsigned short, ConstantBufferVector<XMFLOAT4X4>*> & mPerAnimationCB) {
	md3dImmediateContext->IASetInputLayout(Model->InputLayout);
	md3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	UINT stride = static_cast<UINT>(Model->typesize);
	UINT offset = static_cast<UINT>(Model->VertexOffsetBytes);
	md3dImmediateContext->IASetVertexBuffers(0, 1, &VB, &stride, &offset);
	md3dImmediateContext->IASetIndexBuffer(IB, DXGI_FORMAT_R16_UINT, 0);

	//bind VS/PS shaders to VS/PS pipeline
	//shader should use per object
	md3dImmediateContext->VSSetShader(Model->vs, 0, 0);
	md3dImmediateContext->PSSetShader(Model->ps, 0, 0);

	XMMATRIX world = XMLoadFloat4x4(&mWorld);
	XMMATRIX InvTranspose = InverseTranspose(world);
	//XMMATRIX worldViewProj = world * Cam.ViewProj();

	//all matrix need to transpose to switch from row-major to column-major
	XMStoreFloat4x4(&mPerObjectCB.Data.World, XMMatrixTranspose(world));
	XMStoreFloat4x4(&mPerObjectCB.Data.WorldInvTranspose, XMMatrixTranspose(InvTranspose));
	XMStoreFloat4x4(&mPerObjectCB.Data.TexTransform, XMMatrixTranspose(XMLoadFloat4x4(&mTexTransform)));
	

	//if the model has bone animation data, then transfor all bone final transformation matrix to video card animation constant buffer
	if (!FinalTransforms.empty()) {
		unsigned short size = static_cast<unsigned short>(FinalTransforms.size());
		//first to find if the same bones size constant buffer has already been created, it not create it, or just use it.
		if (mPerAnimationCB.find(size) == mPerAnimationCB.cend()) {
			mPerAnimationCB[size] = new ConstantBufferVector<XMFLOAT4X4>;
			mPerAnimationCB.at(size)->Data.resize(size);//resize the ConstantBuffer's vector member before Initialize(create the ConstantBuffer according to the size)
			mPerAnimationCB.at(size)->Initialize(md3dDevice);//create the constant buffer on video card
		}
		ConstantBufferVector<XMFLOAT4X4>* cb = mPerAnimationCB.at(size);
		ID3D11Buffer* animationCB = cb->getBuffer();
		//bine this animation constant buffer to VS/PS shaders
		md3dImmediateContext->VSSetConstantBuffers(2, 1, &animationCB);
		//md3dImmediateContext->PSSetConstantBuffers(2, 1, &animationCB);
		for (auto i = 0; i < size; ++i) {
			XMStoreFloat4x4(&(mPerAnimationCB.at(size)->Data)[i], XMMatrixTranspose(XMLoadFloat4x4(&FinalTransforms[i])));
			//XMStoreFloat4x4(&(mPerAnimationCB.at(size)->Data)[i], XMLoadFloat4x4(&FinalTransforms[i]));
		}
		cb->ApplyChanges(md3dImmediateContext);//finally copy all animation transformation matrix into video card constant buffer
	}

	//a whole model has different material part, each material part is a subset of the whole model
	for (auto i = 0; i < Model->SubsetCount; ++i) {
	//for (auto i = 4; i <= 4; ++i) {
		mPerObjectCB.Data.Mat = Model->Mat[i];////non matrix just asignment, no need matrix transpose
		mPerObjectCB.ApplyChanges(md3dImmediateContext);//copy to all matrix, data of constant buffer to video card RAM to the shader part
		md3dImmediateContext->PSSetShaderResources(0, 1, &Model->DiffuseMapSRV[i]);
		//md3dImmediateContext->PSSetShaderResources(0, 1, &Model->NormalMapSRV[i]);
		md3dImmediateContext->DrawIndexed(Model->Subsets[i].FaceCount * 3, static_cast<UINT>(Model->IndexOffset + Model->Subsets[i].FaceStart * 3), 0);//vertex offset is set in offset when binding vertextbuffer IASetVertexBuffers
	}

}

void Object3DList::FrameDraw() {
	
		 
	float backgroundcolor[4] = { 0,0,0,1 };//{ 0.098f, 0.098f, 0.439f, 1.000f  };

	//clear backbuffer/stencil buffer to prepare to new draw
	app->md3dImmediateContext->ClearRenderTargetView(app->mRenderTargetView, reinterpret_cast<const float*>(backgroundcolor));
	app->md3dImmediateContext->ClearDepthStencilView(app->mDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	//bind PerFrame/PerObject const buffer to VS/PS pipeline
	ID3D11Buffer* cbuffers[2] = { mPerFrameCB.getBuffer(), mPerObjectCB.getBuffer() };
	app->md3dImmediateContext->VSSetConstantBuffers(0, 2, cbuffers);
	app->md3dImmediateContext->PSSetConstantBuffers(0, 2, cbuffers);
	
	//bind sampler state to PS pipe line
	ID3D11SamplerState* ss = shaderMgr->GetSamplerState();
	app->md3dImmediateContext->PSSetSamplers(0, 1, &ss);
	



	
	//update perFrame constant buffer
	for (unsigned i = 0; i < mControlCB.Data.DirLightCount; ++i) {
		if (app->dirLightsw[i]) {
			mPerFrameCB.Data.DirLights[i] = app->mDirLights[i];
		} else {
			mPerFrameCB.Data.DirLights[i].Ambient =  { 0,0,0,1 };
			mPerFrameCB.Data.DirLights[i].Diffuse =  { 0,0,0,1 };
			mPerFrameCB.Data.DirLights[i].Specular = { 0,0,0,1 };
		}
	}
	for (unsigned i = 0; i < mControlCB.Data.PointLightCount; ++i) {
		if (app->pointLightsw[i]) {
			mPerFrameCB.Data.PointLights[i] = app->mPointLights[i];
		} else {
			mPerFrameCB.Data.PointLights[i].Ambient =  { 0,0,0,1 };
			mPerFrameCB.Data.PointLights[i].Diffuse =  { 0,0,0,1 };
			mPerFrameCB.Data.PointLights[i].Specular = { 0,0,0,1 };
		}
	}
	for (unsigned i = 0; i < mControlCB.Data.SpotLightCount; ++i) {
		if (app->SpotLightsw[i]) {
			mPerFrameCB.Data.SpotLights[i] = app->mSpotLights[i];
		} else {
			mPerFrameCB.Data.SpotLights[i].Ambient =  { 0,0,0,1 };
			mPerFrameCB.Data.SpotLights[i].Diffuse =  { 0,0,0,1 };
			mPerFrameCB.Data.SpotLights[i].Specular = { 0,0,0,1 };
		}
	}
	if (app->SpotLightsw[0]) {
		//0 is fixed position
		XMVECTOR spotlight0pos = XMLoadFloat3(&app->mSpotLights[0].Position);
		XMVECTOR playerpos = XMLoadFloat3(&XMFLOAT3(instancelist.at("player")->mWorld._41, instancelist.at("player")->mWorld._42, instancelist.at("player")->mWorld._43));
		XMVECTOR DirVector = playerpos - spotlight0pos;
		DirVector = XMVector3Normalize(DirVector);
		XMFLOAT3 dir0;
		XMStoreFloat3(&dir0, DirVector);
		mPerFrameCB.Data.SpotLights[0].Direction = dir0;
	}
	if (app->SpotLightsw[1]) {
		mPerFrameCB.Data.SpotLights[1].Position = XMFLOAT3(instancelist.at("player")->mWorld._41, instancelist.at("player")->mWorld._42 + 3.5f, instancelist.at("player")->mWorld._43);
		XMFLOAT4 look = XMFLOAT4(0.0f, 0.0f, -1.0f, 0.0f);
		XMFLOAT4 dir;
		XMStoreFloat4(&dir, XMVector4Normalize(XMVector4Transform(XMLoadFloat4(&look), XMLoadFloat4x4(&instancelist.at("player")->mWorld))));
		mPerFrameCB.Data.SpotLights[1].Direction = XMFLOAT3(dir.x, dir.y, dir.z);
	}

	mPerFrameCB.Data.EyePosW = app->Cam.GetPosition();
	XMStoreFloat4x4(&mPerFrameCB.Data.ViewProj, XMMatrixTranspose(app->Cam.ViewProj()));

		mPerFrameCB.Data.gFogStart = 10.0f;
		mPerFrameCB.Data.gFogRange = 60.0f;
		mPerFrameCB.Data.gFogColor = XMFLOAT4(0.65f, 0.65f, 0.65f, 1.0f);
		mPerFrameCB.ApplyChanges(app->md3dImmediateContext);

		mControlCB.Data.DirLightCount = 3;
		mControlCB.Data.PointLightCount = 10;
		mControlCB.Data.SpotLightCount = 2;
		mControlCB.Data.UseTexture = true;
		mControlCB.Data.AlphaClip = false;
		if (app->fogenable) {
			mControlCB.Data.FogEnable = true;
		} else {
			mControlCB.Data.FogEnable = false;
		}
		mControlCB.Data.ReflectionEnable = false;
		mControlCB.ApplyChanges(app->md3dImmediateContext);
		ID3D11Buffer* cb = mControlCB.getBuffer();
		app->md3dImmediateContext->PSSetConstantBuffers(3, 1, &cb);






	//draw every object instance
	for (auto item : instancelist) {
		item.second->ObjectInstanceDraw(app->md3dDevice, app->md3dImmediateContext, mVB, mIB, app->Cam, mPerObjectCB, PerAnimationCB);
	}

	//flip the drawn backbuffer to the frontend screen to display it.
	app->mSwapChain->Present(0, 0);
}
void Object3DList::Update(float dt) {
	for (auto item : instancelist) {
		item.second->Update(dt);}
}

Object3DList::Object3DList(D3DAnimation* d3dptr, std::vector<ObjectInstance_M3dModelList_DESC>objectlist)
	: app(d3dptr),	mLastVertiesOffsetBytes(0),	mLastVertiesCountBytes(0), mLastIndicesOffset(0), mLastIndicesCount(0), mTotalVertexCountBytes(0), mTotalIndexCount(0) {
	shaderMgr = new Shaders(app->md3dDevice);
	texMgr.Init(app->md3dDevice);
	for (auto item : objectlist) {
		if (modellist.find(item.modelName) == modellist.cend()) {
			Object3D* temp = new Object3D(
				item.modelFilename,
				item.texturePath,
				texMgr,
				shaderMgr->GetVertexShader(item.VShaderName),
				shaderMgr->GetPixelShader(item.PShaderName),
				shaderMgr->GetInputLayouts(item.LayoutName),
				item.vertexTypeName/*vertex type name*/
				);		
			AddObjectList(temp, item.modelName);
		}
		AddInstanceList(item.instanceName, item.modelName, item.translation, item.rotation, item.scale);
	}
}

void Object3DList::AddObjectList(Object3D* temp, const std::string& modelname) {
	temp->VertexOffsetBytes = mLastVertiesOffsetBytes + mLastVertiesCountBytes;
	mLastVertiesCountBytes = temp->VertexCountBytes;
	mLastVertiesOffsetBytes = temp->VertexOffsetBytes;
	temp->IndexOffset = mLastIndicesOffset + mLastIndicesCount;
	mLastIndicesCount = temp->IndexCount;
	mLastIndicesOffset = temp->IndexOffset;
	mTotalVertexCountBytes += mLastVertiesCountBytes,
		mTotalIndexCount += mLastIndicesCount;
	temp->vsarray.resize(temp->VertexCountBytes);

	std::copy(static_cast<char*>(temp->Vertices), static_cast<char*>(temp->Vertices) + temp->VertexCountBytes, temp->vsarray.begin());
	modellist[modelname] = temp;
	//delete[]temp->Vertices;
	//temp->Vertices = nullptr;
}
void Object3DList::AddInstanceList(const std::string& instanceName, const std::string& modelname, const XMFLOAT3& translation, const XMFLOAT4X4& rotation, const XMFLOAT3& scale) {
	if (instancelist.find(instanceName) == instancelist.cend()) {
		Object3DInstance* temp = new Object3DInstance;
		temp->Model = modellist.at(modelname);
		temp->MoveTo(XMMatrixTranslation(translation.x, translation.y, translation.z), XMMatrixScaling(scale.x, scale.y, scale.z), XMLoadFloat4x4(&rotation));
		if (temp->Model->Animations.BoneCount() > 0) {
			temp->FinalTransforms.resize(temp->Model->Animations.BoneCount());
		}
		instancelist[instanceName] = temp;
	} else {
		std::string msg = "There is already \"" + instanceName + "\" in the Object Instance list!";
		std::wstring wmsg;
		wmsg.assign(msg.cbegin(), msg.cend());
		MessageBox(0, wmsg.c_str(), 0, 0);
	}
}
void Object3DList::AddLandList(std::vector<ObjectInstance_LANDLIST_DESC> objectlist) {
	for (auto item : objectlist) {
		if (modellist.find(item.modelname) == modellist.cend()) {
			Object3D* temp = new Object3D(
				item.material,
				item.texturePath,
				item.diffuseMap,
				item.normalMap,
				texMgr,
				shaderMgr->GetVertexShader(item.VShaderName),
				shaderMgr->GetPixelShader(item.PShaderName),
				shaderMgr->GetInputLayouts(item.LayoutName),
				item.vertexTypeName,
				item.desc
				);		
			AddObjectList(temp, item.modelname);
		}
		AddInstanceList(item.instanceName, item.modelname, item.translation, item.rotation, item.scale);
	}
}
void Object3DList::AddGridList(std::vector<ObjectInstance_GRIDLIST_DESC> objectlist) {
	for (auto item : objectlist) {
		if (modellist.find(item.modelname) == modellist.cend()) {
			Object3D* temp = new Object3D(
				item.material,
				item.texturePath,
				item.diffuseMap,
				item.normalMap,
				texMgr,
				shaderMgr->GetVertexShader(item.VShaderName),
				shaderMgr->GetPixelShader(item.PShaderName),
				shaderMgr->GetInputLayouts(item.LayoutName),
				item.vertexTypeName,
				item.desc
				);		
			AddObjectList(temp, item.modelname);
		}
		AddInstanceList(item.instanceName, item.modelname, item.translation, item.rotation, item.scale);
	}
}
void Object3DList::AddCylinderList(std::vector<ObjectInstance_CYLINDERLIST_DESC> objectlist) {
	for (auto item : objectlist) {
		if (modellist.find(item.modelname) == modellist.cend()) {
			Object3D* temp = new Object3D(
				item.material,
				item.texturePath,
				item.diffuseMap,
				item.normalMap,
				texMgr,
				shaderMgr->GetVertexShader(item.VShaderName),
				shaderMgr->GetPixelShader(item.PShaderName),
				shaderMgr->GetInputLayouts(item.LayoutName),
				item.vertexTypeName,
				item.desc
				);		
			AddObjectList(temp, item.modelname);
		}
		AddInstanceList(item.instanceName, item.modelname, item.translation, item.rotation, item.scale);
	}
}
void Object3DList::AddGeoSphereList(std::vector<ObjectInstance_GEOSPHERELIST_DESC> objectlist) {
	for (auto item : objectlist) {
		if (modellist.find(item.modelname) == modellist.cend()) {
			Object3D* temp = new Object3D(
				item.material,
				item.texturePath,
				item.diffuseMap,
				item.normalMap,
				texMgr,
				shaderMgr->GetVertexShader(item.VShaderName),
				shaderMgr->GetPixelShader(item.PShaderName),
				shaderMgr->GetInputLayouts(item.LayoutName),
				item.vertexTypeName,
				item.desc
				);		
			AddObjectList(temp, item.modelname);
		}
		AddInstanceList(item.instanceName, item.modelname, item.translation, item.rotation, item.scale);
	}
}
void Object3DList::AddSphereList(std::vector<ObjectInstance_SPHERELIST_DESC> objectlist) {
	for (auto item : objectlist) {
		if (modellist.find(item.modelname) == modellist.cend()) {
			Object3D* temp = new Object3D(
				item.material,
				item.texturePath,
				item.diffuseMap,
				item.normalMap,
				texMgr,
				shaderMgr->GetVertexShader(item.VShaderName),
				shaderMgr->GetPixelShader(item.PShaderName),
				shaderMgr->GetInputLayouts(item.LayoutName),
				item.vertexTypeName,
				item.desc
				);		
			AddObjectList(temp, item.modelname);
		}
		AddInstanceList(item.instanceName, item.modelname, item.translation, item.rotation, item.scale);
	}
}
void Object3DList::AddBoxist(std::vector<ObjectInstance_BOXLIST_DESC> objectlist) {
	for (auto item : objectlist) {
		if (modellist.find(item.modelname) == modellist.cend()) {
			Object3D* temp = new Object3D(
				item.material,
				item.texturePath,
				item.diffuseMap,
				item.normalMap,
				texMgr,
				shaderMgr->GetVertexShader(item.VShaderName),
				shaderMgr->GetPixelShader(item.PShaderName),
				shaderMgr->GetInputLayouts(item.LayoutName),
				item.vertexTypeName,
				item.desc
				);		
			AddObjectList(temp, item.modelname);
		}
		AddInstanceList(item.instanceName, item.modelname, item.translation, item.rotation, item.scale);
	}
}

Object3DList::~Object3DList() {
	if (shaderMgr) {
		delete shaderMgr;
		shaderMgr = nullptr;
	}
	if (mVB) {
		mVB->Release();
		mVB = nullptr;
	}
	if (mIB) {
		mIB->Release();
		mIB = nullptr;
	}
	for (auto item : PerAnimationCB) {
		if (item.second) {
			delete item.second;
			item.second = nullptr;
		}
	}
	for (auto item : modellist) {
		if (item.second) {
			delete item.second;
			item.second = nullptr;
		}
	}
	for (auto item : instancelist) {
		if (item.second) {
			delete item.second;
			item.second = nullptr;
		}
	}
}
void Object3DList::Init() {
	CreateVideoRAMBuffer();

	//initialize PerFrame/PerObject contant buffer
	mPerFrameCB.Initialize(app->md3dDevice);
	mPerObjectCB.Initialize(app->md3dDevice);
	mControlCB.Initialize(app->md3dDevice);
	//app->md3dImmediateContext->RSSetState(app->mWireframeRS);

}

void Object3DList::CreateVideoRAMBuffer() {
	if (mVB) {
		mVB->Release();
		mVB = nullptr;
	}
	std::vector<char> vertices;
	std::vector<unsigned short> indices;
	for (auto item : modellist) {
		vertices.insert(vertices.cend(), item.second->vsarray.cbegin(), item.second->vsarray.cend());
		item.second->vsarray.clear();
		indices.insert(indices.cend(), item.second->Indices.cbegin(), item.second->Indices.cend());
	}

	D3D11_BUFFER_DESC vbd;
	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = static_cast<UINT>(mTotalVertexCountBytes);
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	vbd.MiscFlags = 0;
	vbd.StructureByteStride = 0;
	D3D11_SUBRESOURCE_DATA vinitData;
	vinitData.pSysMem = &vertices[0];
	app->md3dDevice->CreateBuffer(&vbd, &vinitData, &mVB);

	/*
	char* look = new char[mTotalVertexCountBytes];
	for (unsigned long i = 0; i < mTotalVertexCountBytes; ++i) {
	look[i] = vertices[i];
	}
	PosNormalTexTanAnimation* ppp = (PosNormalTexTanAnimation*)look;
	std::vector<PosNormalTexTanAnimation> qqq;
	unsigned number = mTotalVertexCountBytes / sizeof(PosNormalTexTanAnimation);
	for (auto i = 0; i < number; ++i) {
	qqq.push_back(ppp[i]);
	}
	delete[]look;
	*/
	if (mIB) {
		mIB->Release();
		mIB = nullptr;
	}
	D3D11_BUFFER_DESC ibd;
	ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.ByteWidth = static_cast<UINT>(sizeof(unsigned short)*mTotalIndexCount);
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;
	ibd.MiscFlags = 0;
	ibd.StructureByteStride = 0;
	D3D11_SUBRESOURCE_DATA iinitData;
	iinitData.pSysMem = &indices[0];
	app->md3dDevice->CreateBuffer(&ibd, &iinitData, &mIB);
}
