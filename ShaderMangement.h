#ifndef SHADER_MANGEMENT_H
#define SHADER_MANGEMENT_H


class Shaders {
public:
	Shaders(ID3D11Device* mdevice);
	~Shaders();
	ID3D11VertexShader* GetVertexShader(const std::string& name) { return VertexShaders.at(name); }
	ID3D11PixelShader* GetPixelShader(const std::string& name) { return PixelShaders.at(name); }
	ID3D11InputLayout* GetInputLayouts(const std::string& name) { return InputLayouts.at(name); }
	ID3D11SamplerState* GetSamplerState() const { return SampleState; }
private:
	void LoadShaderBlob(const std::string& filename, char*& blob, unsigned int& size);
	void CreateVertexShader(const std::string& shadername, const std::string& filename, const std::string& layoutname, const D3D11_INPUT_ELEMENT_DESC desc[], UINT layout_size);
	void CreatePixelShader(const std::string& name, const std::string& filename);
	void Init();
	std::unordered_map<std::string, ID3D11VertexShader*> VertexShaders;
	std::unordered_map<std::string, ID3D11PixelShader*> PixelShaders;
	std::unordered_map<std::string, ID3D11InputLayout*> InputLayouts;
	ID3D11SamplerState* SampleState;
	ID3D11Device* device;
};
#endif
