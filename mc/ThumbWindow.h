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

class WindowItem;
class MainWindow;
class McRect;

class ThumbWindow : public BaseWindow<ThumbWindow>
{
public:
	ThumbWindow(WindowItem *_owner);
	~ThumbWindow();
	void setPosition(McRect *);
	PCWSTR ClassName() const { return L"Thumb Window Class"; }
	LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
	DWORD runThread() { return 0; }
	void onSelectWindow(BOOL isCtrl);
	void onMoveToBottom();
	void onZoom();
private:
	BOOL haveMouse;
	void onDestroyWindow();
	HWND topWindow;

	WindowItem *owner;
};
