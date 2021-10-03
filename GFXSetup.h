#ifndef GFXSETUP_H
#define GFXSETUP_H



class CCheckBox {
public:
	CCheckBox(HWND parent, UINT id) : ID(id), checked(false) { hWnd = GetDlgItem(parent, ID); }
	LRESULT ISCHECK() { return SendMessage(hWnd, BM_GETCHECK, 0, 0); }
private:
	UINT ID;
	HWND hWnd;
	bool checked;
};
class CTrackBar {
public:
	CTrackBar(HWND parent, UINT id) : ID(id), selected_item(-1) { hWnd = GetDlgItem(parent, ID); }
	BOOL SET_TICK_MARK(LPARAM tic) { return static_cast<BOOL>(SendMessage(hWnd, TBM_SETTIC, 0, tic)); }
	void SET_RANGE_MAX(LPARAM max) { SendMessage(hWnd, TBM_SETRANGEMAX, true, max); }
	void SET_RANGE_MIN(LPARAM min) { SendMessage(hWnd, TBM_SETRANGEMIN, true, min); }
	UINT SET_PAGE_SIZE(LPARAM newsize) { return static_cast<UINT>(SendMessage(hWnd, TBM_SETPAGESIZE, 0, newsize)); }
	void SET_FREQUENCY(WPARAM freq) { SendMessage(hWnd, TBM_SETTICFREQ, freq, 0); }
	UINT GET_TICKS_COUNT() { return static_cast<UINT>(SendMessage(hWnd, TBM_GETNUMTICS, 0, 0)); }
	LRESULT GET_TICKS_ARRAY_ADDRESS() { return  SendMessage(hWnd, TBM_GETPTICS, 0, 0); }
	UINT GET_BAR_POSITION() { return static_cast<UINT>(SendMessage(hWnd, TBM_GETPOS, 0, 0)); }
	UINT GET_TICK_LOGICAL_POSITION(WPARAM index) { return static_cast<UINT>(SendMessage(hWnd, TBM_GETTIC, index, 0)); }//Retrieves the logical position of a tick mark in a trackbar. The logical position can be any of the integer values in the trackbar's range of minimum to maximum slider positions.
	BOOL Disable() { return EnableWindow(hWnd, false); }
	BOOL Enable() { return EnableWindow(hWnd, true); }
	UINT GET_TICK_PHYSICAL_POSITION(WPARAM index) { return static_cast<UINT>(SendMessage(hWnd, TBM_GETTICPOS, index, 0)); } //Retrieves the current physical position of a tick mark in a trackbar.
	//WPARAM index is Zero-based index identifying a tick mark. The positions of the first and last tick marks are not directly available via this message.
	//Returns the distance, in client coordinates, from the left or top of the trackbar's client area to the specified tick mark. The return value is the x-coordinate of the tick mark for a horizontal trackbar or the y-coordinate for a vertical trackbar. If wParam is not a valid index, the return value is -1.
	//Because the first and last tick marks are not available through this message, valid indexes are offset from their tick position on the trackbar. If the difference between TBM_GETRANGEMIN and TBM_GETRANGEMAX is less than two, then there is no valid index and this message will fail.
	//The following illustrates the relation between the ticks on a trackbar, the ticks available through this message, and their zero - based indexes.
	//0 1 2 3 4 5 6 7 8 9    // Tick positions seen on the trackbar.
	//  1 2 3 4 5 6 7 8      // Tick positions whose position can be identified.
	//	0 1 2 3 4 5 6 7      // Index numbers for the identifiable positions.


private:
	UINT ID;
	HWND hWnd;
	int selected_item;
};
class CComboBox {
public:
	CComboBox(HWND parent, UINT id) : ID(id), selected_item(-1) { hWnd = GetDlgItem(parent, ID); }
	void ADD_ITEM(std::wstring item, UINT value);
	void SET_DEFAULT_CUROR_POSITION(int index);
	void UPDATE_SELECTED_ITEM();
	int GET_SELECTED_ITEM() { return selected_item; }
	void SETFOCUS() { SetFocus(hWnd); }
	void CLEAR() { SendMessage(hWnd, CB_RESETCONTENT, 0, 0); selected_item = -1; }
private:
	UINT ID;
	HWND hWnd;
	int selected_item;
};

class GraphicPropertiesSetup {
public:
	GraphicPropertiesSetup(HINSTANCE hIns, HWND hWnd);
	~GraphicPropertiesSetup();
	struct return_values {
		std::string api;
		IDXGIAdapter1* pAdapter;
		IDXGIOutput* pOutput;
		DXGI_FORMAT format;
		UINT CurrentSelectedDisplayModeIndex;
		DXGI_MODE_DESC* pDisplayModes;
		UINT mTotalDisplayModes;
		UINT MSAA;
		UINT AF;
		bool bMSAA;
		UINT filtering;
		UINT screen;
		UINT BufferCount;
		bool VSync;
		bool apply;
		
	} GetSelectedProperties() { return ret; }
private:
	static LRESULT CALLBACK GraphicPropertiesSetupDialogProc(HWND hDlg, UINT iMsg, WPARAM wParam, LPARAM lParam);
	LRESULT DialogProc(HWND hDlg, UINT iMsg, WPARAM wParam, LPARAM lParam);
	void PopulateAdapterList(CComboBox* pcb);
	void PopulateOutputList(CComboBox* pcb);
	void PopulateAPIList(CComboBox* pcb);
	void PopulateColorFormatList(CComboBox* pcb);
	void PopulateDisplayModeList(CComboBox* pcb);
	void PopulateMSAASliderBar(CTrackBar* ptb);
	void PopulateTextureFilteringList(CComboBox* pcb);
	void PopulateBufferCountList(CComboBox* pcb);
	void PopulateScreenList(CComboBox* pcb);
	void PopulateAFSliderBar(CTrackBar* ptb);
	void UpdateAllCurrentSelectedValues();
	HINSTANCE hInstance;
	HWND hWnd;
	std::vector<IDXGIAdapter1*> Adapters;
	std::vector<IDXGIOutput*> Outputs;
	DXGI_MODE_DESC* pDisplayModes;
	UINT mTotalDisplayModes;
	std::vector<std::wstring> formats;
	std::vector<std::wstring> wapis;
	std::vector<std::wstring> texturefilters;
	std::vector<std::wstring> screens;
	std::vector<std::string> apis;
	std::vector<std::wstring> buffer_count;
	std::vector<UINT> MSAA;
	std::vector<UINT> AF;
	CComboBox* pAdapterList;
	CComboBox* pOutputList;
	CComboBox* pAPIList;
	CComboBox* pColorFormatList;
	CComboBox* pDisplayModeList;
	CComboBox* pTextureFilterList;
	CComboBox* pScreenList;
	CComboBox* pBufferCountList;
	CTrackBar* pMSAAList;
	CTrackBar* pAFList;
	CCheckBox* pMSAACheck;
	CCheckBox* pVSync;
	
	std::unordered_map<std::wstring, DXGI_FORMAT> mapping; //color format mapping
	struct return_values ret;



};

#endif