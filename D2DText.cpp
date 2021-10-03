#include <d2d1_3.h>
#include <d3d11_4.h>
#include <dwrite_3.h>
#include "D2DText.h"
#include "dxerr.h"


#if defined(DEBUG) || defined(_DEBUG)
#ifndef HR
#define HR(x) { \
	HRESULT hr = (x);\
	if (FAILED(hr)) {\
		DXTrace(__FILEW__, (DWORD)__LINE__, hr, L#x, true);\
	}\
	}
#endif
#else
#ifndef HR
#define HR(x) (x)
#endif
#endif

D2DText::~D2DText() {

	if (mDWriteTextFormat) {
		mDWriteTextFormat->Release();
		mDWriteTextFormat = nullptr;
	}
	if (mDWriteTextLayout) {
		mDWriteTextLayout->Release();
		mDWriteTextLayout = nullptr;
	}
	if (yellowBrush) {
		yellowBrush->Release();
		yellowBrush = nullptr;
	}

	if (whiteBrush) {
		whiteBrush->Release();
		whiteBrush = nullptr;
	}

	if (blackBrush) {
		blackBrush->Release();
		blackBrush = nullptr;
	}

	if (mdwriteFactory) {
		mdwriteFactory->Release();
		mdwriteFactory = nullptr;
	}

	if (md2dContext) {
		md2dContext->Release();
		md2dContext = nullptr;
	}

	if (md2dDevice) {
		md2dDevice->Release();
		md2dDevice = nullptr;
	}

	if (md2dFactory) {
		md2dFactory->Release();
		md2dFactory = nullptr;
	}

}


void D2DText::CreateD2DResources() {
	D2D1_FACTORY_OPTIONS options;
#if defined(DEBUG) || defined(_DEBUG)
	options.debugLevel = D2D1_DEBUG_LEVEL_INFORMATION;
#else
	options.debugLevel = D2D1_DEBUG_LEVEL_NONE;
#endif
	HR(D2D1CreateFactory(D2D1_FACTORY_TYPE_MULTI_THREADED, __uuidof(ID2D1Factory2), &options, (void**)&md2dFactory));
	IDXGIDevice* dxgiDevice = 0;
	HR(md3dDevice->QueryInterface(__uuidof(IDXGIDevice), (void**)&dxgiDevice));

	HR(md2dFactory->CreateDevice(dxgiDevice, &md2dDevice));

	if (dxgiDevice) {
		dxgiDevice->Release();
		dxgiDevice = nullptr;
	}

	D2D1_DEVICE_CONTEXT_OPTIONS deviceOptions = D2D1_DEVICE_CONTEXT_OPTIONS_NONE;
	md2dDevice->CreateDeviceContext(deviceOptions, &md2dContext);
	md2dContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Yellow), &yellowBrush);
	md2dContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Black), &blackBrush);
	md2dContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::White), &whiteBrush);


	// set up text formats
	DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), reinterpret_cast<IUnknown**>(&mdwriteFactory));
	

}

void D2DText::CreateRenderTarget() {
	// Direct2D needs the dxgi version of the backbuffer surface pointer.
	IDXGISurface* DXGIBackBuffer;
	mSwapChain->GetBuffer(0, __uuidof(IDXGISurface), reinterpret_cast<void**>(&DXGIBackBuffer));

	ID2D1Bitmap1* targetBitmap;
	D2D1_BITMAP_PROPERTIES1 bp;
	bp.pixelFormat.format = mformat;
	bp.pixelFormat.alphaMode = D2D1_ALPHA_MODE_IGNORE;
	bp.dpiX = 96.0f;
	bp.dpiY = 96.0f;
	bp.bitmapOptions = D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW;
	bp.colorContext = nullptr;

	md2dContext->CreateBitmapFromDxgiSurface(DXGIBackBuffer, &bp, &targetBitmap);
	md2dContext->SetTarget(targetBitmap);
	if (targetBitmap) {
		targetBitmap->Release();
		targetBitmap = nullptr;
	}

	if (DXGIBackBuffer) {
		DXGIBackBuffer->Release();
		DXGIBackBuffer = nullptr;
	}

}

void D2DText::ClearRenderTarget() {
	if (md2dContext) {
		md2dContext->SetTarget(nullptr);
	}
}


void D2DText::CreateTextLayout(WCHAR const* string,	UINT32 stringLength, FLOAT maxWidth, FLOAT maxHeight) {
	mdwriteFactory->CreateTextLayout(string, stringLength, mDWriteTextFormat, maxWidth, maxHeight,&mDWriteTextLayout);
}
void D2DText::ClearTextLayout() {
	if (mDWriteTextLayout) {
		mDWriteTextLayout->Release();
		mDWriteTextLayout = nullptr;
	}
}
void D2DText::CreateTextFormat(WCHAR const* fontFamilyName, IDWriteFontCollection* fontCollection,	DWRITE_FONT_WEIGHT fontWeight,	DWRITE_FONT_STYLE fontStyle, DWRITE_FONT_STRETCH fontStretch, FLOAT fontSize, WCHAR const* localeName) {
	mdwriteFactory->CreateTextFormat(fontFamilyName, fontCollection, fontWeight, fontStyle, fontStretch, fontSize, localeName, &mDWriteTextFormat);
}

void D2DText::ClearTextFormat() {
	if (mDWriteTextFormat) {
		mDWriteTextFormat->Release();
		mDWriteTextFormat = nullptr;
	}
}


void D2DText::DrawTextLayout(D2D1_POINT_2F origin) {
	if (mDWriteTextLayout) {
		md2dContext->BeginDraw();
		md2dContext->DrawTextLayout(origin, mDWriteTextLayout, yellowBrush);
		md2dContext->EndDraw();
	}
}


