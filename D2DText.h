#ifndef D2DTEXT_H
#define D2DTEXT_H



class D2DText {
public:
	D2DText(ID3D11Device* d3dDevice, IDXGISwapChain* SwapChain, DXGI_FORMAT format) :md3dDevice(d3dDevice), mSwapChain(SwapChain), mformat(format) { CreateD2DResources(); }
	~D2DText();
	void CreateRenderTarget();
	void ClearRenderTarget();
	void CreateTextLayout(WCHAR const* string, UINT32 stringLength, FLOAT maxWidth, FLOAT maxHeight);
	void ClearTextLayout();
	void CreateTextFormat(WCHAR const* fontFamilyName, IDWriteFontCollection* fontCollection, DWRITE_FONT_WEIGHT fontWeight, DWRITE_FONT_STYLE fontStyle, DWRITE_FONT_STRETCH fontStretch, FLOAT fontSize, WCHAR const* localeName);
	void ClearTextFormat();
	void SetTextAlignMent(DWRITE_TEXT_ALIGNMENT textAlignment) { mDWriteTextFormat->SetTextAlignment(textAlignment); }
	void SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT paragraphAlignment) { mDWriteTextFormat->SetParagraphAlignment(paragraphAlignment); }
	void DrawTextLayout(D2D1_POINT_2F origin);
private:
	void CreateD2DResources();

ID3D11Device* md3dDevice;
IDXGISwapChain* mSwapChain;
DXGI_FORMAT mformat;

IDWriteFactory* mdwriteFactory;
ID2D1Factory2* md2dFactory;
ID2D1Device1* md2dDevice;
ID2D1DeviceContext1* md2dContext;

ID2D1SolidColorBrush* yellowBrush;
ID2D1SolidColorBrush* whiteBrush;
ID2D1SolidColorBrush* blackBrush;


IDWriteTextFormat* mDWriteTextFormat;
IDWriteTextLayout* mDWriteTextLayout;

};
#endif

