#ifndef TEXTUREMANGEMENT_H
#define TEXTUREMANGEMENT_H

class TextureMangement {
public:
	TextureMangement() : md3dDevice(0) {};
	~TextureMangement();
	void Init(ID3D11Device* device);
	ID3D11ShaderResourceView* LoadTexture(std::string filename);
private:
	ID3D11Device* md3dDevice;
	std::unordered_map<std::string, ID3D11ShaderResourceView*> mTextureMapSRV;
};
#endif
