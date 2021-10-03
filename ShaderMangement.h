#ifndef SHADER_MANGEMENT_H
#define SHADER_MANGEMENT_H

class RenderStates {
public:
	RenderStates(ID3D11Device* device);
	~RenderStates();
	ID3D11RasterizerState* Solid() const { return SolidRS; }
	ID3D11RasterizerState* WireFrame() const{ return WireframeRS; }
	ID3D11RasterizerState* CullClockwise() const { return CullClockwiseRS; }
	ID3D11DepthStencilState* MarkMirror() const { return MarkMirrorDSS; }
	ID3D11DepthStencilState* MarkMirrorinMirror() const { return MarkMirrorinMirrorDSS; }
	ID3D11DepthStencilState* ClearStencil() const { return ClearStencilDSS; }
	ID3D11DepthStencilState* ReflectionMirror() const { return ReflectionMirrorDSS; }
	ID3D11DepthStencilState* NoDoubleBlend() const { return NoDoubleBlendDSS; }
	ID3D11BlendState* AlphaToCoverage() const { return AlphaToCoverageBS; }
	ID3D11BlendState* Transparent() const { return TransparentBS; }
	ID3D11BlendState* NoRenderTargetWrite() const { return NoRenderTargetWritesBS; }
private:
	ID3D11RasterizerState* SolidRS;
	ID3D11RasterizerState* WireframeRS;
	ID3D11RasterizerState* CullClockwiseRS;
	ID3D11DepthStencilState* MarkMirrorDSS;
	ID3D11DepthStencilState* MarkMirrorinMirrorDSS;
	ID3D11DepthStencilState* ClearStencilDSS;
	ID3D11DepthStencilState* ReflectionMirrorDSS;
	ID3D11DepthStencilState* NoDoubleBlendDSS;
	ID3D11BlendState* TransparentBS;
	ID3D11BlendState* AlphaToCoverageBS;
	ID3D11BlendState* NoRenderTargetWritesBS;
};

class Shaders {
public:
	Shaders(ID3D11Device* mdevice, UINT filtering_type, UINT AF_Count);
	~Shaders();
	ID3D11VertexShader* GetVertexShader(const std::string& name) { return VertexShaders.at(name); }
	ID3D11PixelShader* GetPixelShader(const std::string& name) { return PixelShaders.at(name); }
	ID3D11InputLayout* GetInputLayouts(const std::string& name) { return InputLayouts.at(name); }
	ID3D11SamplerState* GetSamplerState() const { return SampleState; }
	RenderStates* GetRenderStates() const { return RenderStateMgr; }
private:
	void LoadShaderBlob(const std::string& filename, char*& blob, unsigned int& size);
	void CreateVertexShader(const std::string& shadername, const std::string& filename, const std::string& layoutname, const D3D11_INPUT_ELEMENT_DESC desc[], UINT layout_size);
	void CreatePixelShader(const std::string& name, const std::string& filename);
	void Init(UINT filtering_type, UINT AF_Count);
	std::unordered_map<std::string, ID3D11VertexShader*> VertexShaders;
	std::unordered_map<std::string, ID3D11PixelShader*> PixelShaders;
	std::unordered_map<std::string, ID3D11InputLayout*> InputLayouts;
	ID3D11SamplerState* SampleState;
	RenderStates* RenderStateMgr;
	ID3D11Device* device;
	UINT AF;
	UINT filtering;
};
#endif
