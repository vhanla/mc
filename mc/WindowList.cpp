/////////////////////////////////////////////////////////////////////////////////////////////
// COPYRIGHT 2012 D. D. Steiner                                                            //
//                                                                                         //
// This work is licensed under a                                                           //
// Creative Commons Attribution-NonCommercial-ShareAlike 3.0 Unported License.             //
// Terms of this license may be found at http://creativecommons.org/licenses/by-nc-sa/3.0/ //
// All other rights reserved.                                                              //
/////////////////////////////////////////////////////////////////////////////////////////////
#include <Windows.h>
#include <Psapi.h>

#include "mc.h"
#include "WindowList.h"
#include "WindowItem.h"
#include "MainWindow.h"
#include "ThumbWindow.h"
#include "ZoomWindow.h"
#include "McProps.h"
#include "McPiles.h"
#include "McAnimation.h"

#include <winuser.h>

WindowList::WindowList()
{
	McDisplay *geometry = MC::getMC()->getDisplayGeometry();
	McRect  mdT(geometry->mdR);
	mdT.offset(&geometry->mdOffset);
	desktopItem =
		new WindowItem(GetShellWindow(), 0, 0, TRUE, FALSE, FALSE, "_DTclass", "_DTprocess",
			&geometry->mdR, &mdT);

	focusItem = NULL;

	minWin = GetSystemMetrics(SM_CYMIN);

	int pad = GetSystemMetrics(SM_CXPADDEDBORDER);
	borderSize.x = pad + GetSystemMetrics(SM_CXSIZEFRAME);
	borderSize.y = pad + GetSystemMetrics(SM_CYSIZEFRAME);

	buildWindowList();
}

WindowList::~WindowList()
{
	if (desktopItem) delete desktopItem;
	for (ITEM_IT it = itemList.begin();it != itemList.end();it++)
		delete *(it);
}

void WindowList::logAllWindows()
{
	for (ITEM_IT it = itemList.begin();it != itemList.end();it++)
	{
		WindowItem *item = *it;
		MC::Log("\"%s\" WIN %x, %x PID %ld[%ld] CLASS %s  ",
			item->getProgName()->data(),
			item->getAppHwnd(), item->getThumbHwnd(),
			item->getAppPid(), item->getAppTid(),
			item->getClassName()->data());
		if (item->getIsMinimized()) MC::Log("MINIMIZED ");
		if (item->getIsMaximized()) MC::Log("MAXIMIZED ");
		MC::Log("\n");

		MC::Log("    pile=%d ", 1 + item->getPileNumber(), MC::getMC()->getNPiles());
		MC::Log("   origin (%ld, %ld) width %ld height %ld  [Z=%d]  ",
			item->getLeft(), item->getTop(), item->getWidth(), item->getHeight(), item->getZorder());
		MC::Log("   content (%ld, %ld) width %ld height %ld\n",
			item->getContentRect()->left, item->getContentRect()->top,
			item->getContentRect()->getWidth(), item->getContentRect()->getHeight());
	}
}

void WindowList::layoutDesktop(McState mode)
{
	MC *mc = MC::getMC();

	LONG dWidth = desktopItem->getNormalRect()->getWidth();
	LONG tWidth = (LONG)(desktopItem->getNormalRect()->getWidth() * .0725);

	double dtPadding = mc->getProperties()->getDesktopPadding();

	McRect *mR = &mc->getDisplayGeometry()->mdR;
	McRect *nR = desktopItem->getNormalRect();
	McRect tRect((LONG)(dtPadding * nR->getWidth()),
		(LONG)(dtPadding * nR->getHeight()),
		(LONG)(nR->getWidth()*(1.0 - dtPadding)),
		(LONG)(nR->getHeight()*(1.0 - dtPadding)));

	desktopItem->setThumbRect(&tRect);

	switch (mode)
	{
	case MCS_TransitionUp:
		desktopItem->setEndRect(desktopItem->getNormalRect());
		desktopItem->setStartRect(&tRect);
		break;
	case MCS_TransitionDown:
		desktopItem->setEndRect(&tRect);
		desktopItem->setStartRect(desktopItem->getNormalRect());
		break;
	case MCS_TransitionOver:
		desktopItem->setEndRect(&tRect);
		desktopItem->setStartRect(&tRect);
		break;
	default:
		break;
	}
}

void WindowList::createWindows()
{
	for (ITEM_IT it = itemList.begin();it != itemList.end();it++)
		(*it)->createThumbWindow();
}

void WindowList::buildWindowList()
{
	EnumWindows(myWindowEnumCallback, (LPARAM)this);
}


void WindowList::handleNextWindow(HWND hwnd)
{
	// SET OF RULES THAT END UP VERY VERY CLOSE TO WHAT DWM DOES IN WIN/TAB DISPLAY

	wchar_t wbuff[1000];
	char	className[1000];

	if (0 < GetClassName(hwnd, wbuff, 1000))
		MC::wchar2char(wbuff, className);
	else
		sprintf_s(className, 1000, "<CN UNKNOWN>");

	if (!IsWindowVisible(hwnd))
		return;

	if (strstr(className, "SideBar_HTMLHostWindow")) return;

	DWORD threadId, processId;
	HINSTANCE hi = (HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE);
	threadId = GetWindowThreadProcessId(hwnd, &processId);

	WINDOWINFO info = {};
	GetWindowInfo(hwnd, &info);
	DWORD style = info.dwStyle;

	// Process name used for piling.  Not alway available if the 
	// window has elevated privledges but you don't

	char pbuff[1000], *pName = pbuff;

	HANDLE handle = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processId);
	if (0 < GetProcessImageFileName(handle, wbuff, 1000))
		MC::wchar2char(wbuff, pbuff);
	else
		sprintf_s(pbuff, 1000, "<FN UNKNOWN>");
	CloseHandle(handle);

	unsigned int i, j = strlen(pbuff);
	for (i = 0;i<j;i++)
	{
		if (pbuff[i] == '\\') pName = &pbuff[i + 1];
		if (pbuff[i] == '.') pbuff[i] = 0;
	}

	if (0 == (style & (WS_TILEDWINDOW | WS_CHILD))) return;

	if (0 == (style & (WS_POPUP | WS_BORDER | WS_DLGFRAME | WS_SYSMENU)))
		return;

	if (0 == (style & (WS_GROUP | WS_BORDER | WS_TABSTOP | WS_THICKFRAME | WS_SYSMENU)))
		return;

	MC *mc = MC::getMC();

	if (mc->getProperties()->getIsExcluded(pName))
		return;

	McDisplay *geometry = mc->getDisplayGeometry();

	McRect normalWindow;
	McRect contentWindow;

	WINDOWPLACEMENT wp;
	GetWindowPlacement(hwnd, &wp);

	if ((style & WS_MINIMIZE) != 0)
	{
		if ((wp.flags&WPF_RESTORETOMAXIMIZED))
		{
			McRect *mR = &geometry->mdR;
			style |= WS_MAXIMIZE;
			info.rcWindow.left = mR->left - borderSize.x;
			info.rcWindow.top = mR->top - borderSize.y;
			info.rcWindow.right = mR->right + borderSize.x;
			info.rcWindow.bottom = mR->bottom + borderSize.y;
		}
		else
		{
			info.rcWindow.left = wp.rcNormalPosition.left;
			info.rcWindow.top = wp.rcNormalPosition.top;
			info.rcWindow.right = wp.rcNormalPosition.right;
			info.rcWindow.bottom = wp.rcNormalPosition.bottom;
		}
	}

	normalWindow.set(&info.rcWindow);
	contentWindow.set(0, 0, normalWindow.getWidth(), normalWindow.getHeight());

	HMONITOR mon = MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST);
	MONITORINFO minfo;
	minfo.cbSize = sizeof(MONITORINFO);
	GetMonitorInfo(mon, &minfo);
	McRect mdwR(&minfo.rcWork);

	if (normalWindow.getWidth() == geometry->mdwR.getWidth() &&
		normalWindow.getHeight() == geometry->mdwR.getHeight())
	{
		style |= WS_MAXIMIZE;
		normalWindow.set(&minfo.rcMonitor);    // rcMonitor instead of rcWork is deliberate
		contentWindow.set(&minfo.rcMonitor);
		contentWindow.offset(-contentWindow.left, -contentWindow.top);
	}
	else if ((style & WS_MAXIMIZE) != 0)
	{
		McRect appR(&info.rcWindow);

		int xadj = borderSize.x;
		int yadj = borderSize.y;

		normalWindow.left = appR.left + xadj;
		normalWindow.top = appR.top + yadj;
		normalWindow.right = appR.right - xadj;
		normalWindow.bottom = appR.bottom - yadj;

		contentWindow.left = xadj;
		contentWindow.top = yadj;
		contentWindow.right = (appR.right - appR.left) - xadj;
		contentWindow.bottom = (appR.bottom - appR.top) - yadj;
	}

	if (normalWindow.getWidth() < minWin || normalWindow.getHeight() < minWin)
		return;

	WindowItem *item = new WindowItem(
		hwnd,
		processId,
		threadId,
		FALSE,
		(style & WS_ICONIC) != 0,
		(style & WS_MAXIMIZE) != 0,
		className,
		pName,
		&normalWindow,
		&contentWindow);

	itemList.push_front(item);
}

void WindowList::insertItemOnTop(WindowItem *_item)
{
	itemList.remove(_item);
	itemList.push_back(_item);
}

WindowItem *WindowList::getDesktopItem()
{
	return desktopItem;
}

// Even though the window objects are tracked with a C++ list
// object, this somewhat odd interface is used in the rest of the
// program.   Not really a good idea, and should be cleaned up.

WindowItem *WindowList::getFirstItem()
{
	forwardIter = itemList.begin();
	if (forwardIter == itemList.end())
		return NULL;
	return *forwardIter;
}

WindowItem *WindowList::getNextItem()
{
	++forwardIter;
	if (forwardIter == itemList.end())
		return NULL;
	return *forwardIter;
}

WindowItem *WindowList::getLastItem()
{
	backwardIter = itemList.rbegin();
	if (backwardIter == itemList.rend())
		return NULL;
	return *backwardIter;
}

WindowItem *WindowList::getPrevItem()
{
	++backwardIter;
	if (backwardIter == itemList.rend())
		return NULL;
	return *backwardIter;
}

WindowItem* WindowList::getItemAbove(WindowItem *target)
{
	for (list<WindowItem *>::iterator it = itemList.begin(); it != itemList.end(); it++)
		if ((*it)->getZorder() < target->getZorder())
			return (*it);
	return NULL;
}

WindowItem* WindowList::getPileItemAbove(WindowItem *target)
{
	for (list<WindowItem *>::iterator it = itemList.begin(); it != itemList.end(); it++)
		if ((*it)->getPileNumber() == target->getPileNumber() && (*it)->getZorder() < target->getZorder())
			return (*it);
	return NULL;
}

WindowItem *WindowList::getItemBelow(WindowItem *target)
{
	for (list<WindowItem *>::reverse_iterator r_it = itemList.rbegin(); r_it != itemList.rend(); r_it++)
		if ((*r_it)->getZorder() > target->getZorder())
			return (*r_it);
	return NULL;
}

WindowItem *WindowList::getPileItemBelow(WindowItem *target)
{
	for (list<WindowItem *>::reverse_iterator r_it = itemList.rbegin(); r_it != itemList.rend(); r_it++)
		if ((*r_it)->getPileNumber() == target->getPileNumber() && (*r_it)->getZorder() > target->getZorder())
			return (*r_it);
	return NULL;
}

void WindowList::insertItemBefore(WindowItem *_item, WindowItem *_target)
{
	itemList.remove(_item);
	for (ITEM_IT it = itemList.begin(); it != itemList.end(); it++)
		if ((*it) == _target)
		{
			itemList.insert(it, _item);
			break;
		}
}

void WindowList::changeFocus(WindowItem *_item)
{
	if (_item != focusItem)
	{
		if (focusItem)
			focusItem->loseFocus();
		if (_item)
			_item->takeFocus();
	}
}

void WindowList::keyPress(WPARAM key)
{
	if (MC::getState() == MCS_Display)
		switch (key)
		{
		case VK_RETURN:
			if (focusItem)
				focusItem->getThumbWindow()->onSelectWindow((0x80 & GetKeyState(VK_CONTROL)) != 0);
			break;
		case 0x4D:	// M key (Minimize all windows if CTRL)
			if (GetKeyState(VK_CONTROL) & 0x80)
				MC::getMC()->getMainWindow()->onMinimizeAll();
			break;
		case 0x4E:	// N key (Normalize all windows if CTRL)
			if (GetKeyState(VK_CONTROL) & 0x80)
				MC::getMC()->getMainWindow()->onNormalizeAll();
			break;
		case 0x42:	// B Key (Move focus window to bottom of z order if CTRL)
			if (GetKeyState(VK_CONTROL) & 0x80)
				if (focusItem)
					focusItem->getThumbWindow()->onMoveToBottom();
			break;
		case 0x5A:	// Z Key (Zoom focus window if CTRL )
			if (GetKeyState(VK_CONTROL) & 0x80)
				if (focusItem)
					focusItem->getThumbWindow()->onZoom();
			break;
		case 0x52:	// R Key (Refresh entirely)
			MC::getMC()->setRunAgain(TRUE);
			PostMessage(MC::getMC()->getMainWindow()->getHwnd(), WM_USER,
				WMMC_TRANSITIONUP, NULL);
			break;
		case VK_KILLKEY:  // K Key (Kill window if CTRL )
			if (GetKeyState(VK_CONTROL) & 0x80)
				if (focusItem)
					killApp(focusItem);
			break;
		case VK_TAB:
			if (GetKeyState(VK_SHIFT) & 0x80)
				nextPile(-1);
			else
				nextPile(+1);
			break;
		case VK_LEFT:
			nextPile(-1);		// Move focus left one pile
			break;
		case VK_RIGHT:
			nextPile(+1);		// Move focus right one pile
			break;
		case VK_UP:
			nextWindow(+1);	// Move focus up within pile
			break;
		case VK_DOWN:
			nextWindow(-1);	// Move focus down within pile
			break;
		default:
			break;
		}
	else if (MC::getState() == MCS_Zoom)
		SendMessage(MC::getMC()->getZoomWindow()->getHwnd(), WM_RBUTTONUP, NULL, NULL);
}

void WindowList::nextPile(int _delta)
{
	if (!focusItem)
		return;

	MC *mc = MC::getMC();
	int newPile = focusItem->getPileNumber() + _delta;
	if (newPile < 0)
		newPile = mc->getNPiles() - 1;
	else
		if (newPile == mc->getNPiles())
			newPile = 0;
	for (list<WindowItem*>::reverse_iterator it = itemList.rbegin(); it != itemList.rend(); it++)
	{
		WindowItem *wi = *it;
		if (wi->getPileNumber() == newPile)
		{
			changeFocus(wi);
			return;
		}
	}
}

void WindowList::nextWindow(int _delta)
{
	if (!focusItem)
		return;

	WindowItem *newItem = NULL;
	if (_delta < 0)
		newItem = getPileItemBelow(focusItem);
	else if (_delta > 0)
		newItem = getPileItemAbove(focusItem);
	if (newItem != NULL)
		changeFocus(newItem);
}

void WindowList::killApp(WindowItem *_item)
{
	MC *mc = MC::getMC();

	itemList.remove(_item);

	if (itemList.size() > 0)
		changeFocus(itemList.back());

	PostMessage(_item->getAppHwnd(), WM_CLOSE, NULL, NULL);
	delete _item;

	if (itemList.size() == 0)
	{
		PostMessage(mc->getMainWindow()->getHwnd(), WM_USER, WMMC_TRANSITIONUP, NULL);
		return;
	}
	else
		reinitLayout();
}

void WindowList::reinitLayout()
{

	MC *mc = MC::getMC();

	initializeRects(MCS_Display);

	McPiles *piles = new McPiles();
	piles->createPiles();

	piles->layoutPiles();
	mc->setNPiles(piles->getNextPileNumber());

	piles->positionThumbs();

	delete piles;

	PostMessage(mc->getMainWindow()->getHwnd(), WM_USER, WMMC_TRANSITIONOVER, NULL);
}

void WindowList::initializeRects(McState mode)
{
	layoutDesktop(mode);
	for (ITEM_IT it = itemList.begin();it != itemList.end();it++)
	{
		WindowItem *item = *it;

		switch (mode)
		{
		case MCS_TransitionDown:
			if (item->getIsMinimized())
				MC::getMC()->setVanishingPoint(item->getThumbRect(), item->getStartRect());
			else
				item->setStartRect(item->getNormalRect());
			item->setEndRect(item->getThumbRect());
			break;
		case MCS_TransitionUp:
			if (item->getIsMinimized())
				MC::getMC()->setVanishingPoint(item->getThumbRect(), item->getEndRect());
			else
				item->setEndRect(item->getNormalRect());
			item->setStartRect(item->getThumbRect());
			break;
		case MCS_Display:
			item->setStartRect(item->getThumbRect());
			break;
		case MCS_TransitionOver:
			item->setEndRect(item->getThumbRect());
			break;
		default:
			return;
		}

		item->setCurRect(item->getStartRect());

	}
}

void WindowList::setFocusItem(WindowItem *_focusitem)
{
	if (focusItem)
		focusItem->setHaveFocus(FALSE);
	focusItem = _focusitem;
	if (focusItem)
		focusItem->setHaveFocus(TRUE);
}









