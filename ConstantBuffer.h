#ifndef CONSTANTBUFFER_H
#define CONSTANTBUFFER_H
template<typename T> class ConstantBuffer {
	public:
		T Data;
		ConstantBuffer() : mInitialized(false), mBuffer(nullptr){}
		//ConstantBuffer(const ConstantBuffer<T>& rhs) {}
		~ConstantBuffer() { if (mBuffer) { mBuffer->Release(); } }
		//ConstantBuffer<T>& operator= (const ConstantBuffer<T>& rhs) {}
		HRESULT Initialize(ID3D11Device* device) {
			HRESULT hr;
			D3D11_BUFFER_DESC desc;
			desc.Usage = D3D11_USAGE_DYNAMIC;
			desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
			desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
			desc.MiscFlags = 0;
			desc.ByteWidth = static_cast<UINT>(sizeof(T) + 16 - (sizeof(T) % 16));//make sure the value can be divide by 16
			desc.StructureByteStride = 0;
			hr = device->CreateBuffer(&desc, 0, &mBuffer);
			mInitialized = true;
			return hr;
		}
		void ApplyChanges(ID3D11DeviceContext* dc) {
			assert(mInitialized);
			D3D11_MAPPED_SUBRESOURCE mappedResource;
			dc->Map(mBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
			CopyMemory(mappedResource.pData, &Data, sizeof(T));
			dc->Unmap(mBuffer, 0);
		}
		ID3D11Buffer* getBuffer() const { return mBuffer; }
	private:
		ID3D11Buffer* mBuffer;
		bool mInitialized;
};

template<typename T> class ConstantBufferVector {
public:
	std::vector<T> Data;
	ConstantBufferVector() : mInitialized(false), mBuffer(nullptr){}
	//ConstantBuffer(const ConstantBuffer<T>& rhs) {}
	~ConstantBufferVector() { if (mBuffer) { mBuffer->Release(); } }
	//ConstantBuffer<T>& operator= (const ConstantBuffer<T>& rhs) {}
	HRESULT Initialize(ID3D11Device* device) {
		HRESULT hr;
		D3D11_BUFFER_DESC desc;
		desc.Usage = D3D11_USAGE_DYNAMIC;
		desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		desc.MiscFlags = 0;
		desc.ByteWidth = static_cast<UINT>(sizeof(T) * Data.size() + 16 - (sizeof(T) * Data.size() % 16));//make sure the value can be divide by 16
		desc.StructureByteStride = 0;
		hr = device->CreateBuffer(&desc, 0, &mBuffer);
		mInitialized = true;
		return hr;
	}
	void ApplyChanges(ID3D11DeviceContext* dc) {
		assert(mInitialized);
		D3D11_MAPPED_SUBRESOURCE mappedResource;
		T* look = new T[Data.size()];
		for (auto i = 0U; i < Data.size(); ++i) {
			look[i] = Data[i];
		}
		char* put = (char*)look;
		dc->Map(mBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
		CopyMemory(mappedResource.pData, put/*&Data[0]*/, sizeof(T) * Data.size());
		dc->Unmap(mBuffer, 0);
		delete[]look;
	}
	ID3D11Buffer* getBuffer() const { return mBuffer; }
private:
	ID3D11Buffer* mBuffer;
	bool mInitialized;
};

#endif
