#include <d3d11_2.h>
#include <unordered_map>
#include "DDSTextureLoader.h"
#include "TextureMangement.h"


TextureMangement::~TextureMangement() {
	for (auto item : mTextureMapSRV) {
		if (item.second) {
			item.second->Release();
			item.second = nullptr;
		}
	}
}

void TextureMangement::Init(ID3D11Device* device) {
	md3dDevice = device;
}

ID3D11ShaderResourceView* TextureMangement::LoadTexture(std::string filename) {
	ID3D11ShaderResourceView* MapSRV;
	if (mTextureMapSRV.find(filename) == mTextureMapSRV.cend()) {
		ID3D11Resource* tex = nullptr;
		std::wstring name;
		name.assign(filename.cbegin(), filename.cend());
		DirectX::CreateDDSTextureFromFile(md3dDevice, name.c_str(),&tex,&MapSRV);
		mTextureMapSRV[filename] = MapSRV;
		if (tex) {
			tex->Release();
			tex = nullptr;
		}
	} else {
		MapSRV = mTextureMapSRV.at(filename);
	}
	return MapSRV;
}