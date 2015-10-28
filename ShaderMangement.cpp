#include <d3d11_2.h>
#include <fstream>
#include <vector>
#include <tuple>
#include <unordered_map>
#include "ConstantBuffer.h"
#include "ShaderMangement.h"

Shaders::Shaders(ID3D11Device* mdevice) : device(mdevice){
	Init();
}
Shaders::~Shaders() {
	for (auto item : VertexShaders) {
		if (item.second) {
			item.second->Release();
			item.second = nullptr;
		}
	}
	for (auto item : PixelShaders) {
		if (item.second) {
			item.second->Release();
			item.second = nullptr;
		}
	}
	for (auto item : InputLayouts) {
		if (item.second) {
			item.second->Release();
			item.second = nullptr;
		}
	}
	if (SampleState) {
		SampleState->Release();
		SampleState = nullptr;
	}
}
void Shaders::LoadShaderBlob(const std::string& filename, char*& blob, unsigned int& size) {
	std::ifstream fin;
	fin.open(filename, std::ios::binary);
	if (fin.fail()) {
		std::string msg = "Can't open file " + filename;
		std::wstring wmsg;
		wmsg.assign(msg.cbegin(), msg.cend());
		MessageBox(0, wmsg.c_str(), 0, 0);
	} else {
		fin.seekg(0, std::ios_base::end);
		size = static_cast<int>(fin.tellg());
		blob = new char[size];
		fin.seekg(0, std::ios::beg);
		fin.read(blob, size);
		fin.close();
	}
}
void Shaders::Init() {
	const D3D11_INPUT_ELEMENT_DESC Pos[1] = 
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0}
	};

	const D3D11_INPUT_ELEMENT_DESC Basic[3] = 
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0}
	};

	const D3D11_INPUT_ELEMENT_DESC PosNormalTexTan[4] = 
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT,    0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,       0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"TANGENT",  0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 32, D3D11_INPUT_PER_VERTEX_DATA, 0}
	};

	const D3D11_INPUT_ELEMENT_DESC PosNormalTexTanSkinned[6] = 
	{
		{"POSITION",     0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"NORMAL",       0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"TEXCOORD",     0, DXGI_FORMAT_R32G32_FLOAT,    0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"TANGENT",      0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 32, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"WEIGHTS",      0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 48, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"BONEINDICES",  0, DXGI_FORMAT_R8G8B8A8_UINT,   0, 60, D3D11_INPUT_PER_VERTEX_DATA, 0}
	};

	D3D11_SAMPLER_DESC mSampleDesc;
	mSampleDesc.Filter = D3D11_FILTER_ANISOTROPIC;
	mSampleDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	mSampleDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	mSampleDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	mSampleDesc.MipLODBias = 0;
	mSampleDesc.MaxAnisotropy = 16;
	mSampleDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	mSampleDesc.BorderColor[0] = 1.0f;
	mSampleDesc.BorderColor[1] = 1.0f;
	mSampleDesc.BorderColor[2] = 1.0f;
	mSampleDesc.BorderColor[3] = 1.0f;
	mSampleDesc.MinLOD = -FLT_MAX;
	mSampleDesc.MaxLOD = FLT_MAX;
	device->CreateSamplerState(&mSampleDesc, &SampleState);
	//shadername, shader blob filename, layoutname, layout pointer
	std::vector<std::tuple<std::string, std::string, std::string, const D3D11_INPUT_ELEMENT_DESC*, size_t>> vslist = {
		std::make_tuple("BasicAnimationVS", "..\\x64\\Debug\\BasicAnimationVS.cso", "PosNormalTexTanSkinned", PosNormalTexTanSkinned, sizeof(PosNormalTexTanSkinned)),
		std::make_tuple("BasicVS", "..\\x64\\Debug\\BasicVS.cso", "PosNormTexTan", PosNormalTexTan, sizeof(PosNormalTexTan))
	};
	std::vector<std::pair<std::string, std::string>> pslist = {
		{"FixFunctionLightPS", "..\\x64\\Debug\\FixFunctionLightPS.cso"}
	};
	for (auto item : vslist) {
		CreateVertexShader(std::get<0>(item), std::get<1>(item), std::get<2>(item), std::get<3>(item), static_cast<UINT>(std::get<4>(item)));
	}
	for (auto item : pslist) {
		CreatePixelShader(item.first, item.second);
	}
}
void Shaders::CreateVertexShader(const std::string& shadername, const std::string& filename, const std::string& layoutname, const D3D11_INPUT_ELEMENT_DESC desc[], UINT layout_size) {
	char* blob = nullptr;
	unsigned int size = 0;
	ID3D11VertexShader* vs = nullptr;
	LoadShaderBlob(filename, blob, size);
	HRESULT hr;
	if (VertexShaders.find(shadername) == VertexShaders.cend()) {
		hr = device->CreateVertexShader(blob, size, nullptr, &vs);
		VertexShaders[shadername] = vs;
	}

	if (InputLayouts.find(layoutname) == InputLayouts.cend()) {
		ID3D11InputLayout* layout = nullptr;
		hr = device->CreateInputLayout(desc, layout_size / sizeof(D3D11_INPUT_ELEMENT_DESC), blob, size, &layout);
		InputLayouts[layoutname] = layout;
	}
	if (blob) {
		delete[]blob;
		blob = nullptr;
	}
}
void Shaders::CreatePixelShader(const std::string& name, const std::string& filename) {
	char* blob = nullptr;
	unsigned int size = 0;
	ID3D11PixelShader* ps = nullptr;
	LoadShaderBlob(filename, blob, size);
	device->CreatePixelShader(blob, size, nullptr, &ps);
	if (PixelShaders.find(name) == PixelShaders.cend()) {
		PixelShaders[name] = ps;
	}
	if (blob) {
		delete[]blob;
		blob = nullptr;
	}
}