#include <d3d11_4.h>
#include <fstream>
#include <vector>
#include <tuple>
#include <unordered_map>
#include <assert.h>
#include "ConstantBuffer.h"
#include "ShaderMangement.h"

Shaders::Shaders(ID3D11Device* mdevice, UINT filtering_type, UINT AF_Count) : device(mdevice){
	Init(filtering_type, AF_Count);
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
	if (RenderStateMgr) {
		delete RenderStateMgr;
		RenderStateMgr = nullptr;
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
void Shaders::Init(UINT filtering_type, UINT AF_Count) {
	AF = AF_Count;
	filtering = filtering_type;
	RenderStateMgr = new RenderStates(device);
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

	/*
	//https://docs.microsoft.com/en-us/visualstudio/debugger/graphics/point-bilinear-trilinear-and-anisotropic-texture-filtering-variants?view=vs-2019


	Point Texture Filtering :
	D3D11_SAMPLER_DESC sampler_description;
	sampler_description.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
	d3d_device->CreateSamplerState(&sampler_desc, &sampler);
	d3d_context->PSSetSamplers(0, 1, &sampler);


	Bilinear Texture Filtering :
	D3D11_SAMPLER_DESC sampler_description;
	sampler_description.Filter = D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT;
	d3d_device->CreateSamplerState(&sampler_desc, &sampler);
	d3d_context->PSSetSamplers(0, 1, &sampler);


	Trilinear Texture Filtering :
	D3D11_SAMPLER_DESC sampler_description;
	sampler_description.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	d3d_device->CreateSamplerState(&sampler_desc, &sampler);
	d3d_context->PSSetSamplers(0, 1, &sampler);

	Anisotropic Texture Filtering :
	D3D11_SAMPLER_DESC sampler_description;
	sampler_description.Filter = D3D11_FILTER_ANISOTROPIC;
	sampler_description.MaxAnisotropy = 16;
	d3d_device->CreateSamplerState(&sampler_desc, &sampler);
	d3d_context->PSSetSamplers(0, 1, &sampler);
	*/

	D3D11_SAMPLER_DESC mSampleDesc;
	mSampleDesc.MaxAnisotropy = 0;
	if (filtering == 0) {
		mSampleDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
	} else if (filtering == 1) {
		mSampleDesc.Filter = D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT;
	} else if (filtering == 2) {
		mSampleDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	} else if (filtering == 3) {
		mSampleDesc.Filter = D3D11_FILTER_ANISOTROPIC;
		mSampleDesc.MaxAnisotropy = AF;
	}

	mSampleDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	mSampleDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	mSampleDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	mSampleDesc.MipLODBias = 0;
	mSampleDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	mSampleDesc.BorderColor[0] = 1.0f;
	mSampleDesc.BorderColor[1] = 1.0f;
	mSampleDesc.BorderColor[2] = 1.0f;
	mSampleDesc.BorderColor[3] = 1.0f;
	mSampleDesc.MinLOD = -FLT_MAX;
	mSampleDesc.MaxLOD = FLT_MAX;
	device->CreateSamplerState(&mSampleDesc, &SampleState);


	//https://stackoverflow.com/questions/14888402/accessing-visual-studio-macros-from-source-code
	// to use "..\$Platform\$Configuration\*.cso need to use preprocessor:
	//go to Project Properties->Configuration Properties->C / C++->Preprocessor->Preprocessor Definitions to define CONFIGURATION=$(Configuration) ...

	std::string path = "..\\";
	path += PLATFORM;
	path += "\\";
	path += CONFIGURATION;
	path += "\\";

	//shadername, shader blob filename, layoutname, layout pointer
	std::vector<std::tuple<std::string, std::string, std::string, const D3D11_INPUT_ELEMENT_DESC*, size_t>> vslist = {
		std::make_tuple("BasicAnimationVS", path + "BasicAnimationVS.cso", "PosNormalTexTanSkinned", PosNormalTexTanSkinned, sizeof(PosNormalTexTanSkinned)),
		std::make_tuple("BasicVS", path + "BasicVS.cso", "PosNormTexTan", PosNormalTexTan, sizeof(PosNormalTexTan))
	};
	std::vector<std::pair<std::string, std::string>> pslist = {
		{"FixFunctionLightPS", path + "FixFunctionLightPS.cso"}
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

RenderStates::RenderStates(ID3D11Device* device) {

	D3D11_RASTERIZER_DESC rsDesc;
	ZeroMemory(&rsDesc, sizeof(D3D11_RASTERIZER_DESC));
	rsDesc.FillMode = D3D11_FILL_SOLID;
	rsDesc.CullMode = D3D11_CULL_NONE;
	rsDesc.FrontCounterClockwise = false;
	rsDesc.DepthClipEnable = true;
	device->CreateRasterizerState(&rsDesc, &SolidRS);

	ZeroMemory(&rsDesc, sizeof(D3D11_RASTERIZER_DESC));
	rsDesc.FillMode = D3D11_FILL_WIREFRAME;
	rsDesc.CullMode = D3D11_CULL_NONE;
	rsDesc.FrontCounterClockwise = false;
	rsDesc.DepthClipEnable = true;
	device->CreateRasterizerState(&rsDesc, &WireframeRS);

	D3D11_RASTERIZER_DESC cullClockwiseDesc;
	ZeroMemory(&cullClockwiseDesc, sizeof(D3D11_RASTERIZER_DESC));
	cullClockwiseDesc.FillMode = D3D11_FILL_SOLID;
	cullClockwiseDesc.CullMode = D3D11_CULL_BACK;
	cullClockwiseDesc.FrontCounterClockwise = true;
	cullClockwiseDesc.DepthClipEnable = true;
	device->CreateRasterizerState(&cullClockwiseDesc, &CullClockwiseRS);

	
	D3D11_DEPTH_STENCIL_DESC mirrorMarkDesc;
	mirrorMarkDesc = { 0 };
	mirrorMarkDesc.DepthEnable = true;
	mirrorMarkDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;//render mirror only to stencil buffer, disable write to depth buffer
	mirrorMarkDesc.DepthFunc = D3D11_COMPARISON_LESS;
	mirrorMarkDesc.StencilEnable = true;
	mirrorMarkDesc.StencilReadMask = 0xff;//default
	mirrorMarkDesc.StencilWriteMask = 0xff;//default
	mirrorMarkDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;//default
	mirrorMarkDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;//default
	mirrorMarkDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_REPLACE;
	mirrorMarkDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;//default
	mirrorMarkDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;//default
	mirrorMarkDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;//default
	mirrorMarkDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_REPLACE;
	mirrorMarkDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;//default
	device->CreateDepthStencilState(&mirrorMarkDesc, &MarkMirrorDSS);


	D3D11_DEPTH_STENCIL_DESC mirrorMarkinmirrorDesc;
	mirrorMarkinmirrorDesc = { 0 };
	mirrorMarkinmirrorDesc.DepthEnable = true;
	mirrorMarkinmirrorDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;//render mirror only to stencil buffer, disable write to depth buffer
	mirrorMarkinmirrorDesc.DepthFunc = D3D11_COMPARISON_LESS;
	mirrorMarkinmirrorDesc.StencilEnable = true;
	mirrorMarkinmirrorDesc.StencilReadMask = 0xff;//default
	mirrorMarkinmirrorDesc.StencilWriteMask = 0xff;//default
	mirrorMarkinmirrorDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;//default
	mirrorMarkinmirrorDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;//default
	mirrorMarkinmirrorDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_INCR_SAT;
	mirrorMarkinmirrorDesc.FrontFace.StencilFunc = D3D11_COMPARISON_EQUAL;//default
	mirrorMarkinmirrorDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;//default
	mirrorMarkinmirrorDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;//default
	mirrorMarkinmirrorDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_INCR_SAT;
	mirrorMarkinmirrorDesc.BackFace.StencilFunc = D3D11_COMPARISON_EQUAL;//default
	device->CreateDepthStencilState(&mirrorMarkinmirrorDesc, &MarkMirrorinMirrorDSS);

	D3D11_DEPTH_STENCIL_DESC clearStencilDesc;
	clearStencilDesc = { 0 };
	clearStencilDesc.DepthEnable = false;
	clearStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;//render mirror only to stencil buffer, disable write to depth buffer
	clearStencilDesc.DepthFunc = D3D11_COMPARISON_LESS;
	clearStencilDesc.StencilEnable = true;
	clearStencilDesc.StencilReadMask = 0xff;//default
	clearStencilDesc.StencilWriteMask = 0xff;//default
	clearStencilDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;//default
	clearStencilDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;//default
	clearStencilDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_DECR_SAT;
	clearStencilDesc.FrontFace.StencilFunc = D3D11_COMPARISON_EQUAL;//default
	clearStencilDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;//default
	clearStencilDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;//default
	clearStencilDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_DECR_SAT;
	clearStencilDesc.BackFace.StencilFunc = D3D11_COMPARISON_EQUAL;//default
	device->CreateDepthStencilState(&clearStencilDesc, &ClearStencilDSS);

	D3D11_DEPTH_STENCIL_DESC mirrorReflectionDesc;
	mirrorReflectionDesc = { 0 };
	mirrorReflectionDesc.DepthEnable = true;
	mirrorReflectionDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	mirrorReflectionDesc.DepthFunc = D3D11_COMPARISON_LESS;
	mirrorReflectionDesc.StencilEnable = true;
	mirrorReflectionDesc.StencilReadMask = 0xff;
	mirrorReflectionDesc.StencilWriteMask = 0xff;
	mirrorReflectionDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	mirrorReflectionDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	mirrorReflectionDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	mirrorReflectionDesc.FrontFace.StencilFunc = D3D11_COMPARISON_EQUAL;
	mirrorReflectionDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	mirrorReflectionDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	mirrorReflectionDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	mirrorReflectionDesc.BackFace.StencilFunc = D3D11_COMPARISON_EQUAL;
	device->CreateDepthStencilState(&mirrorReflectionDesc, &ReflectionMirrorDSS);

	D3D11_DEPTH_STENCIL_DESC noDoubleBlendDesc;
	noDoubleBlendDesc = { 0 };
	noDoubleBlendDesc.DepthEnable = true;
	noDoubleBlendDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	noDoubleBlendDesc.DepthFunc = D3D11_COMPARISON_LESS;
	noDoubleBlendDesc.StencilEnable = true;
	noDoubleBlendDesc.StencilReadMask = 0xff;
	noDoubleBlendDesc.StencilWriteMask = 0xff;
	noDoubleBlendDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	noDoubleBlendDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	noDoubleBlendDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_INCR;
	noDoubleBlendDesc.FrontFace.StencilFunc = D3D11_COMPARISON_EQUAL;
	noDoubleBlendDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	noDoubleBlendDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	noDoubleBlendDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_INCR;
	noDoubleBlendDesc.BackFace.StencilFunc = D3D11_COMPARISON_EQUAL;
	device->CreateDepthStencilState(&noDoubleBlendDesc, &NoDoubleBlendDSS);

	D3D11_BLEND_DESC TransparentDesc;
	TransparentDesc = { 0 };
	TransparentDesc.AlphaToCoverageEnable = false;
	TransparentDesc.IndependentBlendEnable = false;
	TransparentDesc.RenderTarget[0].BlendEnable = true;
	TransparentDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	TransparentDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	TransparentDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	TransparentDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	TransparentDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	TransparentDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	TransparentDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	device->CreateBlendState(&TransparentDesc, &TransparentBS);

	D3D11_BLEND_DESC alphaToCoverageDesc;
	alphaToCoverageDesc = { 0 };
	alphaToCoverageDesc.AlphaToCoverageEnable = true;
	alphaToCoverageDesc.IndependentBlendEnable = false;
	alphaToCoverageDesc.RenderTarget[0].BlendEnable = false;
	alphaToCoverageDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	device->CreateBlendState(&alphaToCoverageDesc, &AlphaToCoverageBS);

	D3D11_BLEND_DESC NoRenderTargetWritesDesc;
	NoRenderTargetWritesDesc = { 0 };
	NoRenderTargetWritesDesc.AlphaToCoverageEnable = false;
	NoRenderTargetWritesDesc.IndependentBlendEnable = false;
	NoRenderTargetWritesDesc.RenderTarget[0].BlendEnable = false;
	NoRenderTargetWritesDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
	NoRenderTargetWritesDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ZERO;
	NoRenderTargetWritesDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	NoRenderTargetWritesDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	NoRenderTargetWritesDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	NoRenderTargetWritesDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	NoRenderTargetWritesDesc.RenderTarget[0].RenderTargetWriteMask = 0;
	device->CreateBlendState(&NoRenderTargetWritesDesc, &NoRenderTargetWritesBS);
}

RenderStates::~RenderStates() {
	if (WireframeRS) {
		WireframeRS->Release();
		WireframeRS = nullptr;
	}
	if (SolidRS) {
		SolidRS->Release();
		SolidRS = nullptr;
	}
	if (CullClockwiseRS) {
		CullClockwiseRS->Release();
		CullClockwiseRS = nullptr;
	}
	if (MarkMirrorDSS) {
		MarkMirrorDSS->Release();
		MarkMirrorDSS = nullptr;
	}
	if (MarkMirrorinMirrorDSS) {
		MarkMirrorinMirrorDSS->Release();
		MarkMirrorinMirrorDSS = nullptr;
	}
	if (ClearStencilDSS) {
		ClearStencilDSS->Release();
		ClearStencilDSS = nullptr;
	}
	if (ReflectionMirrorDSS) {
		ReflectionMirrorDSS->Release();
		ReflectionMirrorDSS = nullptr;
	}
	if (NoDoubleBlendDSS) {
		NoDoubleBlendDSS->Release();
		NoDoubleBlendDSS = nullptr;
	}
	if (AlphaToCoverageBS) {
		AlphaToCoverageBS->Release();
		AlphaToCoverageBS = nullptr;
	}
	if (TransparentBS) {
		TransparentBS->Release();
		TransparentBS = nullptr;
	}
	if (NoRenderTargetWritesBS) {
		NoRenderTargetWritesBS->Release();
		NoRenderTargetWritesBS = nullptr;
	}
}