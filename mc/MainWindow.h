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
#include "BaseWindow.h"

class ZoomWindow;

class MainWindow : public BaseWindow<MainWindow>
{
public:
	MainWindow(McRect *);
	~MainWindow();

	PCWSTR  ClassName() const { return L"Main Window Class"; }
	LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
	DWORD runThread();
	void calcFrame(double value);

	void start();
	void finish();

	void startZoom(HWND, HWND, McRect *, McRect *);

	void onMinimizeAll(BOOL = TRUE);
	void onNormalizeAll();
private:
	McRect desktopRect;

	void onTransitionDown();
	void onTransitionUp();
	void onTransitionOver();
	void onDisplay();
	void onDestroy();

	WindowItem *activeApp;

	void onPaint(HDC, RECT*);

	void setThumbnailDimensions();

	HANDLE mwEvent;
	HANDLE mwThread;
	DWORD  mwThreadId;
};