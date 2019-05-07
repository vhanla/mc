/////////////////////////////////////////////////////////////////////////////////////////////
// COPYRIGHT 2012 D. D. Steiner                                                            //
//                                                                                         //
// This work is licensed under a                                                           //
// Creative Commons Attribution-NonCommercial-ShareAlike 3.0 Unported License.             //
// Terms of this license may be found at http://creativecommons.org/licenses/by-nc-sa/3.0/ //
// All other rights reserved.                                                              //
/////////////////////////////////////////////////////////////////////////////////////////////
#pragma once
#include <Windows.h>
#include <list>

using namespace std;

class WindowItem;

class WindowList
{
public:
	WindowList();
	~WindowList();

	void createWindows();
	void logAllWindows();

	void initializeRects(McState);

	WindowItem *getFirstItem();
	WindowItem *getNextItem();
	WindowItem *getLastItem();
	WindowItem *getPrevItem();

	WindowItem *getDesktopItem();

	WindowItem *getItemAbove(WindowItem*);
	WindowItem *getItemBelow(WindowItem*);
	WindowItem *getPileItemAbove(WindowItem*);
	WindowItem *getPileItemBelow(WindowItem*);

	void insertItemBefore(WindowItem *_insert, WindowItem *_before);
	void insertItemOnTop(WindowItem *_intert);

	void changeFocus(WindowItem *_focusitem = NULL);
	WindowItem *getFocusItem() { return focusItem; }
	void setFocusItem(WindowItem *);

	void takeFocus();
	void loseFocus();

	void keyPress(WPARAM key);

	void nextPile(int delta);

	void killApp(WindowItem *);
	void reinitLayout();
private:
	void handleNextWindow(HWND hwnd);
	void handleNextDesktop(LPTSTR lpszDesktop);

	void layoutDesktop(McState);

	void nextWindow(int delta);

	list<WindowItem*> itemList;
	list<WindowItem*>::iterator forwardIter;
	list<WindowItem*>::reverse_iterator backwardIter;

	void buildWindowList();

	POINT borderSize;

	WindowItem* desktopItem;
	WindowItem*	focusItem;

	BOOL isProcessed;

	int minWin;

	// STATIC STUFF
private:
	static BOOL CALLBACK myWindowEnumCallback(HWND hwnd, LPARAM obj)
	{
		WindowList *pThis = (WindowList *)obj;
		if (pThis)
			pThis->handleNextWindow(hwnd);
		return TRUE;
	}
};