/////////////////////////////////////////////////////////////////////////////////////////////
// COPYRIGHT 2012 D. D. Steiner                                                            //
//                                                                                         //
// This work is licensed under a                                                           //
// Creative Commons Attribution-NonCommercial-ShareAlike 3.0 Unported License.             //
// Terms of this license may be found at http://creativecommons.org/licenses/by-nc-sa/3.0/ //
// All other rights reserved.                                                              //
/////////////////////////////////////////////////////////////////////////////////////////////
#pragma once
#include <windows.h>
#include <dwmapi.h>
#include "BaseWindow.h"

#define MC_ZOOM_NONE 0
#define MC_ZOOM_UP 1
#define MC_ZOOM_DOWN 2

class WindowItem;

class ZoomWindow : public BaseWindow<ZoomWindow>
{
public:
	ZoomWindow();
	~ZoomWindow();
	PCWSTR ClassName() const { return L"Zoom Window Class"; }
	LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
	void doZoom(int mode, WindowItem *_thumbOwner, McRect *_startRect, McRect *_endRect);
	DWORD runThread();
	void calcFrame(double value);
private:
	int mode;

	WindowItem *thumbOwner;

	HTHUMBNAIL thumbnail;
	HWND sourceWindow;
	HWND thumbWindow;

	HANDLE zwEvent;
	HANDLE zwThread;
	HANDLE zwMutex;
	DWORD  zwThreadId;

	McRect startRect;
	McRect endRect;

	void runZoom();
	void onPaint(HDC);
};