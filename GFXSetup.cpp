#include <d3d11_4.h>
#include <d2d1_3.h>
#include <dwrite_3.h>
#include <windowsx.h>
#include <commctrl.h>
#include <string>
#include <vector>

#include <unordered_map>
#include <assert.h>
#include <sstream>
#include "GameTimer.h"
#include "D2DText.h"
#include "D3DBase.h"
#include "GFXSetup.h"
#include "resource.h"

void CComboBox::ADD_ITEM(std::wstring item, UINT value) {
	int index = 0;
	int ret = (int)SendMessage(hWnd, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(item.c_str()));
	if (ret == CB_ERR) {
		MessageBox(NULL, L"CB_ADDSTRING returns CB_ERR", L"Wrong", MB_ICONINFORMATION);
	} else if (ret == CB_ERRSPACE) {
		MessageBox(NULL, L"CB_ADDSTRING not enough space to add", L"Wrong", MB_ICONINFORMATION);
	} else {
		index = ret;
	}
	if (SendMessage(hWnd, CB_SETITEMDATA, index, static_cast<LPARAM>(value)) == CB_ERR) {
		MessageBox(NULL, L"CB_SETITEMDATA returns CB_ERR", L"Wrong", MB_ICONINFORMATION);
	}
}

void CComboBox::SET_DEFAULT_CUROR_POSITION(int index) {
	if (SendMessage(hWnd, CB_SETCURSEL, index, 0) == CB_ERR) {//last parameter 0 is not used
		MessageBox(NULL, L"CB_SETCURSEL returns CB_ERR", L"Wrong", MB_ICONINFORMATION);
	} else {
		selected_item = index;
	}
}
void CComboBox::UPDATE_SELECTED_ITEM() {
	int Item_index = (int)SendMessage(hWnd, CB_GETCURSEL, 0, 0); //last two 0 must be 0, not used
	if (Item_index != CB_ERR) {
		selected_item = (int)SendMessage(hWnd, CB_GETITEMDATA, Item_index, 0);
	} else {
		MessageBox(NULL, L"CB_GETITEMDATA returns CB_ERR", L"Wrong", MB_ICONINFORMATION);
	}

}
GraphicPropertiesSetup::GraphicPropertiesSetup(HINSTANCE hIns, HWND hwnd) : hInstance(hIns), hWnd(hwnd) {
	/*
		DXGI_FORMAT_R8G8B8A8_UNORM
		DXGI_FORMAT_R8G8B8A8_UNORM_SRGB
		DXGI_FORMAT_B8G8R8A8_UNORM
		DXGI_FORMAT_B8G8R8A8_UNORM
		DXGI_FORMAT_R10G10B10A2_UNORM
		DXGI_FORMAT_R16G16B16A16_FLOAT
		DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM (rare)
		R11B11G10
	*/
	//D3D11_MAX_MULTISAMPLE_SAMPLE_COUNT
	formats = { L"DXGI_FORMAT_R8G8B8A8_UNORM" , L"DXGI_FORMAT_R8G8B8A8_UNORM_SRGB", L"DXGI_FORMAT_B8G8R8A8_UNORM" };
	wapis = { L"DirectX 11", L"DirectX 12", L"Vulkan", L"OpenGL" };
	apis = { "DirectX 11", "DirectX 12", "Vulkan", "OpenGL" };
	texturefilters = { L"Point Filtering", L"Bilinear Filtering", L"Trilinear Filtering", L"Anisotropic Filtering" };
	screens = { L"Exclusive Full Screen",L"Full Screen Flip Mode",L"Windowed" };
	buffer_count = { L"Double Buffering", L"Triple Buffering" };

	//D3D9 For bilinear you should add SetSamplerState(D3DSAMP_MIPFILTER, D3DTEXF_POINT).
	//	SetSamplerState(D3DSAMP_MIPFILTER, D3DTEXF_LINEAR) will give you trilinear.
	/*
	D3D11_SAMPLER_DESC desc;
	desc.Filter = D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT; // bilinear
	desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR; // trilinear
	desc.Filter = D3D11_FILTER_ANISOTROPIC; // anisotropic
	// no such thing as "linear" for 2D textures
	// ...
	device->CreateSamplerState(&desc, &sampler);
	context->PSSetSamplers(0, 1, &sampler);
	//https://docs.microsoft.com/en-us/visualstudio/debugger/graphics/point-bilinear-trilinear-and-anisotropic-texture-filtering-variants?view=vs-2019

	*/



	MSAA = { 1,2,4,8 };
	AF = { 1,2,4,8,16 };
	pAdapterList = nullptr;
	pOutputList = nullptr;
	pAPIList = nullptr;
	pColorFormatList = nullptr;
	pDisplayModeList = nullptr;
	pMSAAList = nullptr;
	pAFList = nullptr;
	pTextureFilterList = nullptr;
	pScreenList = nullptr;
	pBufferCountList = nullptr;
	pMSAACheck = nullptr;
	pVSync = nullptr;
	bool apply = false;

	mapping = { {L"DXGI_FORMAT_R8G8B8A8_UNORM" ,DXGI_FORMAT_R8G8B8A8_UNORM}, {L"DXGI_FORMAT_R8G8B8A8_UNORM_SRGB",DXGI_FORMAT_R8G8B8A8_UNORM_SRGB},{L"DXGI_FORMAT_B8G8R8A8_UNORM",DXGI_FORMAT_B8G8R8A8_UNORM} };
	//ret = { 0 };
	ret.apply = DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_DirectX_Setup), hWnd, reinterpret_cast<DLGPROC>(GraphicPropertiesSetupDialogProc), reinterpret_cast<LPARAM>(this));
}

GraphicPropertiesSetup::~GraphicPropertiesSetup() {

	delete[] pDisplayModes;
	pDisplayModes = nullptr;

	for (auto item : Outputs) {
		item->Release();
	}

	for (auto item : Adapters) {
		item->Release();
	}

	pBufferCountList->CLEAR();
	delete pBufferCountList;
	pBufferCountList = nullptr;

	delete pVSync;
	pVSync = nullptr;

	delete pMSAACheck;
	pMSAACheck = nullptr;

	pScreenList->CLEAR();
	delete pScreenList;
	pScreenList = nullptr;

	pTextureFilterList->CLEAR();
	delete pTextureFilterList;
	pTextureFilterList = nullptr;

	delete pAFList;
	pAFList = nullptr;

	delete pMSAAList;
	pMSAAList = nullptr;

	pDisplayModeList->CLEAR();
	delete pDisplayModeList;
	pDisplayModeList = nullptr;

	pColorFormatList->CLEAR();
	delete pColorFormatList;
	pColorFormatList = nullptr;

	pAPIList->CLEAR();
	delete pAPIList;
	pAPIList = nullptr;

	pOutputList->CLEAR();
	delete pOutputList;
	pOutputList = nullptr;

	pAdapterList->CLEAR();
	delete pAdapterList;
	pAdapterList = nullptr;
}
void GraphicPropertiesSetup::UpdateAllCurrentSelectedValues() {
	ret.api = pAPIList->GET_SELECTED_ITEM() == -1 ? "" : apis[pAPIList->GET_SELECTED_ITEM()];
	ret.pAdapter = pAPIList->GET_SELECTED_ITEM() == -1 ? nullptr : Adapters[pAdapterList->GET_SELECTED_ITEM()];
	ret.pOutput = pOutputList->GET_SELECTED_ITEM() == -1 ? nullptr : Outputs[pOutputList->GET_SELECTED_ITEM()];
	ret.CurrentSelectedDisplayModeIndex = pDisplayModeList->GET_SELECTED_ITEM() == -1 ? 0 : pDisplayModeList->GET_SELECTED_ITEM();
	ret.format = pColorFormatList->GET_SELECTED_ITEM() == -1 ? DXGI_FORMAT() : mapping[formats[pColorFormatList->GET_SELECTED_ITEM()]];
	ret.MSAA = MSAA[pMSAAList->GET_BAR_POSITION()];
	ret.AF = AF[pAFList->GET_BAR_POSITION()];
	ret.filtering = pTextureFilterList->GET_SELECTED_ITEM() == -1 ? 0 : pTextureFilterList->GET_SELECTED_ITEM();
	ret.screen = pScreenList->GET_SELECTED_ITEM() == -1 ? 0 : pScreenList->GET_SELECTED_ITEM();
	ret.bMSAA = pMSAACheck->ISCHECK() == BST_CHECKED ? true : false;
	ret.VSync = pVSync->ISCHECK() == BST_CHECKED ? true : false;
	ret.BufferCount = pBufferCountList->GET_SELECTED_ITEM() == -1 ? 0 : pBufferCountList->GET_SELECTED_ITEM() + 2;
	ret.pDisplayModes = pDisplayModes;
	ret.mTotalDisplayModes = mTotalDisplayModes;

}
LRESULT CALLBACK GraphicPropertiesSetup::GraphicPropertiesSetupDialogProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	if (msg == WM_INITDIALOG) {
		InitCommonControlsEx(nullptr);
		SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(reinterpret_cast<GraphicPropertiesSetup*>(lParam)));
		return reinterpret_cast<GraphicPropertiesSetup*>(lParam)->DialogProc(hwnd, WM_INITDIALOG, wParam, 0);
	} else {
		GraphicPropertiesSetup* currentThis = reinterpret_cast<GraphicPropertiesSetup*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
		if (!currentThis)
			return DefWindowProc(hwnd, msg, wParam, lParam);
		else
			return currentThis->DialogProc(hwnd, msg, wParam, lParam);
	}
}


void GraphicPropertiesSetup::PopulateAdapterList(CComboBox* pcb) {
	IDXGIFactory1* pFactory;
	HRESULT hr = CreateDXGIFactory1(__uuidof(IDXGIFactory), (void**)(&pFactory));
	UINT i = 0;
	IDXGIAdapter1* pAdapter;
	while (pFactory->EnumAdapters1(i, &pAdapter) != DXGI_ERROR_NOT_FOUND) {
		DXGI_ADAPTER_DESC1 desc1;
		pAdapter->GetDesc1(&desc1);
		pcb->ADD_ITEM(desc1.Description, i);
		Adapters.push_back(pAdapter);
		++i;
	}
	if (i >= 1) {
		pcb->SET_DEFAULT_CUROR_POSITION(0);

	}
	pcb->SETFOCUS();
	pFactory->Release();
}


void GraphicPropertiesSetup::PopulateOutputList(CComboBox* pcb) {

	UINT i = 0;
	IDXGIOutput* pOutput;
	if (pAdapterList->GET_SELECTED_ITEM() != -1) {
		IDXGIAdapter1* pAdapter = Adapters[pAdapterList->GET_SELECTED_ITEM()];
		while (pAdapter->EnumOutputs(i, &pOutput) != DXGI_ERROR_NOT_FOUND) {
			DXGI_OUTPUT_DESC Desc;
			pOutput->GetDesc(&Desc);
			pcb->ADD_ITEM(Desc.DeviceName, i);
			Outputs.push_back(pOutput);
			++i;
		}

		if (i >= 1) {
			pcb->SET_DEFAULT_CUROR_POSITION(0);

		}
		pcb->SETFOCUS();
	}
}


void GraphicPropertiesSetup::PopulateAPIList(CComboBox* pcb) {

	for (UINT i = 0; i < wapis.size(); ++i) {
		pcb->ADD_ITEM(wapis[i], i);
	}

	if (wapis.size() >= 1) {
		pcb->SET_DEFAULT_CUROR_POSITION(0);

	}
	pcb->SETFOCUS();
}


void GraphicPropertiesSetup::PopulateColorFormatList(CComboBox* pcb) {

	for (UINT i = 0; i < formats.size(); ++i) {
		pcb->ADD_ITEM(formats[i], i);
	}

	if (formats.size() >= 1) {
		pcb->SET_DEFAULT_CUROR_POSITION(0);

	}
	pcb->SETFOCUS();
}

void  GraphicPropertiesSetup::PopulateTextureFilteringList(CComboBox* pcb) {
	for (UINT i = 0; i < texturefilters.size(); ++i) {
		pcb->ADD_ITEM(texturefilters[i], i);
		OutputDebugString((texturefilters[i] + L" " + std::to_wstring(i) + L"\n").c_str());
	}

	if (texturefilters.size() >= 1) {
		pcb->SET_DEFAULT_CUROR_POSITION(1);

	}
	pcb->SETFOCUS();
}

void  GraphicPropertiesSetup::PopulateScreenList(CComboBox* pcb) {
	for (UINT i = 0; i < screens.size(); ++i) {
		pcb->ADD_ITEM(screens[i], i);
	}

	if (screens.size() >= 1) {
		pcb->SET_DEFAULT_CUROR_POSITION(2);

	}
	pcb->SETFOCUS();
}

void GraphicPropertiesSetup::PopulateBufferCountList(CComboBox* pcb) {
	for (UINT i = 0; i < buffer_count.size(); ++i) {
		pcb->ADD_ITEM(buffer_count[i], i);
	}

	if (buffer_count.size() >= 1) {
		pcb->SET_DEFAULT_CUROR_POSITION(0);

	}
	pcb->SETFOCUS();
}

void GraphicPropertiesSetup::PopulateDisplayModeList(CComboBox* pcb) {
	UINT num = 0;
	UINT flags = 0;
	if (pOutputList->GET_SELECTED_ITEM() != -1) {
		IDXGIOutput* pOutput = Outputs[pOutputList->GET_SELECTED_ITEM()];
		DXGI_FORMAT format = mapping[formats[pColorFormatList->GET_SELECTED_ITEM()]];
		pOutput->GetDisplayModeList(format, flags, &num, 0);
		mTotalDisplayModes = num;
		pDisplayModes = new DXGI_MODE_DESC[num];
		pOutput->GetDisplayModeList(format, flags, &num, pDisplayModes);
		for (UINT i = 0; i < num; ++i) {
			std::wstring mode = std::to_wstring(pDisplayModes[i].Width) + L" x " + std::to_wstring(pDisplayModes[i].Height) + L" x " + std::to_wstring((pDisplayModes[i].RefreshRate.Numerator / static_cast<float>(pDisplayModes[i].RefreshRate.Denominator)));
			pcb->ADD_ITEM(mode, i);
		}

		if (num >= 1) {
			pcb->SET_DEFAULT_CUROR_POSITION(num/2);

		}
		pcb->SETFOCUS();
	}
}

void GraphicPropertiesSetup::PopulateMSAASliderBar(CTrackBar* ptb) {
	ptb->SET_RANGE_MIN(0);
	ptb->SET_RANGE_MAX(3);
	ptb->SET_FREQUENCY(1);
	if (!ptb->SET_TICK_MARK(1)) {
		MessageBox(NULL, L"set ticks 1 failed", L"Wrong", MB_ICONINFORMATION);
	}
	if (!ptb->SET_TICK_MARK(2)) {
		MessageBox(NULL, L"set ticks 2 failed", L"Wrong", MB_ICONINFORMATION);
	}
	/*
	UINT num = ptb->GET_TICKS_COUNT();
	UINT* pTicks = reinterpret_cast<UINT*>(ptb->GET_TICKS_ARRAY_ADDRESS());
	std::wstring final = L"num=" + std::to_wstring(num) + L" final=";
	for (UINT i = 0; i < num - 2; ++i) {
		final += std::to_wstring(pTicks[i])+L" ";
	}
	MessageBox(NULL, final.c_str(), L"Check", MB_ICONINFORMATION);
	*/
}
void GraphicPropertiesSetup::PopulateAFSliderBar(CTrackBar* ptb) {
	ptb->SET_RANGE_MIN(0);
	ptb->SET_RANGE_MAX(4);
	ptb->SET_FREQUENCY(1);
	ptb->SET_TICK_MARK(1);
	ptb->SET_TICK_MARK(2);
	ptb->SET_TICK_MARK(3);
}
LRESULT GraphicPropertiesSetup::DialogProc(HWND hDlg, UINT iMsg, WPARAM wParam, LPARAM lParam) {
	switch (iMsg) {
	case WM_INITDIALOG:
	{
		pAdapterList = new CComboBox(hDlg, IDC_Adapters);
		pOutputList = new CComboBox(hDlg, IDC_Outputs);
		pAPIList = new CComboBox(hDlg, IDC_APIs);
		pColorFormatList = new CComboBox(hDlg, IDC_COLOR_FORMATS);
		pDisplayModeList = new CComboBox(hDlg, IDC_DisplayModes);
		pMSAAList = new CTrackBar(hDlg, IDC_MSAA);
		pAFList = new CTrackBar(hDlg, IDC_AF);
		pTextureFilterList = new CComboBox(hDlg, IDC_TF);
		pScreenList = new CComboBox(hDlg, IDC_SCREEN);
		pBufferCountList = new CComboBox(hDlg, IDC_BUFFER_COUNT);
		pMSAACheck = new CCheckBox(hDlg, IDC_CHECK_MSAA);
		pVSync = new CCheckBox(hDlg, IDC_CHECK_VSYNC);
		PopulateColorFormatList(pColorFormatList);
		PopulateAPIList(pAPIList);
		PopulateAdapterList(pAdapterList);
		PopulateOutputList(pOutputList);
		PopulateDisplayModeList(pDisplayModeList);
		PopulateMSAASliderBar(pMSAAList);
		PopulateTextureFilteringList(pTextureFilterList);
		PopulateScreenList(pScreenList);
		PopulateAFSliderBar(pAFList);
		PopulateBufferCountList(pBufferCountList);
		pMSAAList->Disable();
		pAFList->Disable();
		break;
	}
	case WM_COMMAND:
	{
		switch (LOWORD(wParam)) {
		case IDOK:
		{
			if (pAdapterList->GET_SELECTED_ITEM() == -1) {
				MessageBox(NULL, L"You didn't choose any Adapter!.", L"Wrong", MB_ICONINFORMATION);
			} else {
				UpdateAllCurrentSelectedValues();
				EndDialog(hDlg, 1);
			}
		}
		break;
		case IDCANCEL:
		{
			UpdateAllCurrentSelectedValues();
			EndDialog(hDlg, 0);
		}
		break;
		case IDC_Adapters:
		{
			switch (HIWORD(wParam)) {
			case CBN_SELCHANGE:
			{
				pAdapterList->UPDATE_SELECTED_ITEM();
				pOutputList->CLEAR();
				pDisplayModeList->CLEAR();
				PopulateOutputList(pOutputList);
				PopulateDisplayModeList(pDisplayModeList);
			}
			break;
			}
		}
		break;
		case IDC_Outputs:
		{
			switch (HIWORD(wParam)) {
			case CBN_SELCHANGE:
			{
				pOutputList->UPDATE_SELECTED_ITEM();
				pDisplayModeList->CLEAR();
				PopulateDisplayModeList(pDisplayModeList);
			}
			break;
			}
		}
		break;

		case IDC_APIs:
		{
			switch (HIWORD(wParam)) {
			case CBN_SELCHANGE:
			{
				pAPIList->UPDATE_SELECTED_ITEM();
			}
			break;
			}
		}
		break;

		case IDC_COLOR_FORMATS:
		{
			switch (HIWORD(wParam)) {
			case CBN_SELCHANGE:
			{
				pColorFormatList->UPDATE_SELECTED_ITEM();
				pDisplayModeList->CLEAR();
				PopulateDisplayModeList(pDisplayModeList);
			}
			break;
			}
		}
		break;

		case IDC_DisplayModes:
		{
			switch (HIWORD(wParam)) {
			case CBN_SELCHANGE:
			{
				pDisplayModeList->UPDATE_SELECTED_ITEM();

			}
			break;
			}
		}
		break;

		case IDC_TF:
		{
			switch (HIWORD(wParam)) {
			case CBN_SELCHANGE:
			{
				pTextureFilterList->UPDATE_SELECTED_ITEM();
				if (pTextureFilterList->GET_SELECTED_ITEM() == 3) {
					pAFList->Enable();
				} else {
					pAFList->Disable();
				}

			}
			break;
			}
		}
		break;


		case IDC_SCREEN:
		{
			switch (HIWORD(wParam)) {
			case CBN_SELCHANGE:
			{
				pScreenList->UPDATE_SELECTED_ITEM();
			}
			break;
			}
		}
		break;
		case IDC_BUFFER_COUNT:
		{
			switch (HIWORD(wParam)) {
			case CBN_SELCHANGE:
			{
				pBufferCountList->UPDATE_SELECTED_ITEM();
			}
			break;
			}
		}
		break;

		case IDC_CHECK_MSAA:
		{
			switch (HIWORD(wParam)) {
			case BN_CLICKED:
			{
				if (pMSAACheck->ISCHECK() == BST_CHECKED) {
					pMSAAList->Enable();
				} else {
					pMSAAList->Disable();
				}
			}
			break;
			}
		}
		break;

		case IDC_CHECK_VSYNC:
		{
			switch (HIWORD(wParam)) {
			case BN_CLICKED:
			{
				if (pVSync->ISCHECK() == BST_CHECKED) {
					;
				} else {
					;
				}
			}
			break;
			}
		}
		break;

		}
	}
	break;

	case WM_NOTIFY:
	{
		UINT code = reinterpret_cast<LPNMHDR>(lParam)->code;
		auto ID = reinterpret_cast<LPNMHDR>(lParam)->idFrom;
		HWND msaa_compare = reinterpret_cast<LPNMHDR>(lParam)->hwndFrom;
		HWND msaa = GetDlgItem(hDlg, IDC_MSAA);
		int a = 5 + 2;
		switch (reinterpret_cast<LPNMHDR>(lParam)->code) {
		case TRBN_THUMBPOSCHANGING:
		{
			switch (reinterpret_cast<LPNMHDR>(lParam)->idFrom) {
			case IDC_MSAA:
			{
				DWORD pos = reinterpret_cast<NMTRBTHUMBPOSCHANGING*>(lParam)->dwPos;
				MessageBox(NULL, std::to_wstring(pos).c_str(), L"Slider bar of MSAA moving to...", MB_ICONINFORMATION);
			}
			break;
			case IDC_AF:
			{
				DWORD pos = reinterpret_cast<NMTRBTHUMBPOSCHANGING*>(lParam)->dwPos;
				MessageBox(NULL, std::to_wstring(pos).c_str(), L"Slider bar of AF moving to...", MB_ICONINFORMATION);
			}
			break;
			}
		}
		break;
		case NM_RELEASEDCAPTURE:
		{
			switch (reinterpret_cast<LPNMHDR>(lParam)->idFrom) {
			case IDC_MSAA:
			{
				DWORD pos = pMSAAList->GET_BAR_POSITION();
				/*
				DWORD tick_logical0 = pMSAAList->GET_TICK_LOGICAL_POSITION(0);
				DWORD tick_logical1 = pMSAAList->GET_TICK_LOGICAL_POSITION(1);
				DWORD tick_logical2 = pMSAAList->GET_TICK_LOGICAL_POSITION(2);
				DWORD tick_logical3 = pMSAAList->GET_TICK_LOGICAL_POSITION(3);
				DWORD tick_physical0 = pMSAAList->GET_TICK_PHYSICAL_POSITION(0);
				DWORD tick_physical1 = pMSAAList->GET_TICK_PHYSICAL_POSITION(1);
				DWORD tick_physical2 = pMSAAList->GET_TICK_PHYSICAL_POSITION(2);
				DWORD tick_physical3 = pMSAAList->GET_TICK_PHYSICAL_POSITION(3);
				MessageBox(NULL, std::to_wstring(pos).c_str(), L"Slider bar of MSAA moves to...", MB_ICONINFORMATION);*/
			}
			break;
			case IDC_AF:
			{
				DWORD pos = pAFList->GET_BAR_POSITION();
				/*
				DWORD tick_logical0 = pAFList->GET_TICK_LOGICAL_POSITION(0);
				DWORD tick_logical1 = pAFList->GET_TICK_LOGICAL_POSITION(1);
				DWORD tick_logical2 = pAFList->GET_TICK_LOGICAL_POSITION(2);
				DWORD tick_logical3 = pAFList->GET_TICK_LOGICAL_POSITION(3);
				DWORD tick_physical0 = pAFList->GET_TICK_PHYSICAL_POSITION(0);
				DWORD tick_physical1 = pAFList->GET_TICK_PHYSICAL_POSITION(1);
				DWORD tick_physical2 = pAFList->GET_TICK_PHYSICAL_POSITION(2);
				DWORD tick_physical3 = pAFList->GET_TICK_PHYSICAL_POSITION(3);
				MessageBox(NULL, std::to_wstring(pos).c_str(), L"Slider bar of MSAA moves to...", MB_ICONINFORMATION);*/
			}
			break;
			}
		}
		break;
		}
	}
	break;
	}
	return FALSE;
}
