#ifndef D3DBASE_H
#define D3DBASE_H



//LRESULT CALLBACK MainWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam){ return this->MsgProc(hwnd, msg, wParam, lParam); }

class D3DBase
{

public:
	D3DBase(HINSTANCE hInstance, bool enableMSAA, UINT8 MSAAcount, int width, int height, D3D_DRIVER_TYPE type);
	virtual ~D3DBase();

	HINSTANCE getThisInstanceHandle() const { return mhAppHandle; }//return this program's pointer(void* ->handle)
	HWND getThisWindowHandle() const { return mMainWnd; }//return this program's window's pointer(void* ->handle)

	float getAspectRatio() const { return static_cast<float>(mClientWidth) / mClientHeight; }
	int run();//main program message loop!

	virtual bool Init();
	virtual void OnResize();
	virtual void Update(float dt) = 0;//this is pure virtual: no implementation code in this base class. Derived class should deal with update 3D software logic during every frame.
	virtual void Draw() = 0;//this is pure virtual: no implementation code in this base class. Derived class should implement drawing a frame from this method according to the data updated from Update().
	virtual LRESULT MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam); //Basic windows message handler to proceed basic message from OS.

	virtual void OnMouseDown(WPARAM btnState, int x, int y){}
	virtual void OnMouseUp(WPARAM btnState, int x, int y){}
	virtual void OnMouseMove(WPARAM btnState, int x, int y){}
	virtual void OnKeyDown(WPARAM wParam) {}
protected:
	static LRESULT CALLBACK MainWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
	bool InitMainWindow();
	bool InitDirect3D();
	void CalculateFrameStats();

	HINSTANCE mhAppHandle;
	HWND mMainWnd;

	bool mAppPaused;
	bool mMinimized;
	bool mMaximized;
	bool mResizing;
	UINT mMsaaQuality;
	UINT mMSAACount;

	GameTimer mTimer;
	IDXGIFactory * pFactory;
	ID3D11Device* md3dDevice;
	ID3D11DeviceContext* md3dImmediateContext;
	IDXGISwapChain* mSwapChain;
	ID3D11Texture2D* mDepthStencilBuffer;
	ID3D11RenderTargetView* mRenderTargetView;
	ID3D11DepthStencilView* mDepthStencilView;
	ID3D11RasterizerState* mWireframeRS;
	ID3D11RasterizerState* mSolid;
	D3D11_VIEWPORT mScreenViewPort;
	std::wstring mMainWndCaption;
	D3D_DRIVER_TYPE md3dDriverType;
	int mClientWidth;
	int mClientHeight;
	bool mEnableMsaa;

};










#endif