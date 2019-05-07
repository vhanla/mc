/////////////////////////////////////////////////////////////////////////////////////////////
// COPYRIGHT 2012 D. D. Steiner                                                            //
//                                                                                         //
// This work is licensed under a                                                           //
// Creative Commons Attribution-NonCommercial-ShareAlike 3.0 Unported License.             //
// Terms of this license may be found at http://creativecommons.org/licenses/by-nc-sa/3.0/ //
// All other rights reserved.                                                              //
/////////////////////////////////////////////////////////////////////////////////////////////

#include <windows.h>
#include <dwmapi.h>
#include "mc.h"
#include "MainWindow.h"
#include "ThumbWindow.h"
#include "ZoomWindow.h"
#include "WindowList.h"
#include "WindowItem.h"

ThumbWindow::ThumbWindow(WindowItem *_owner)
{
	owner = _owner;
	haveMouse = FALSE;
}

ThumbWindow::~ThumbWindow()
{
	DestroyWindow(getHwnd());
}

LRESULT ThumbWindow::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	SYSTEMTIME tm;
	GetSystemTime(&tm);
	switch (uMsg)
	{
	case WM_SETFOCUS:

	{
		MC::getMC()->getWindowList()->setFocusItem(owner);
		HWND fg = GetForegroundWindow();
		if (fg != getHwnd())
			SetForegroundWindow(getHwnd());
	}
	return 0;

	case WM_KILLFOCUS:

		MC::getMC()->getWindowList()->setFocusItem(NULL);
		return 0;

	case WM_PAINT:
		if (owner->getIsDesktop())
		{
			PAINTSTRUCT ps;
			HDC hdc = BeginPaint(m_hwnd, &ps);
			EndPaint(m_hwnd, &ps);
		}
		return 0;

	case WM_MOUSEMOVE:

		if (!haveMouse)
		{
			TRACKMOUSEEVENT tme;
			tme.cbSize = sizeof(TRACKMOUSEEVENT);
			tme.dwFlags = TME_LEAVE | TME_HOVER;
			tme.hwndTrack = getHwnd();
			tme.dwHoverTime = 50;
			TrackMouseEvent(&tme);
			haveMouse = TRUE;
		}
		return 0;

	case WM_MOUSEHOVER:
		if (haveMouse)
			MC::getMC()->getWindowList()->changeFocus(owner);
		return 0;

	case WM_MOUSELEAVE:
		haveMouse = FALSE;
		return 0;

	case WM_LBUTTONUP:
		onSelectWindow((wParam&MK_CONTROL) != NULL);
		return 0;

	case WM_RBUTTONDOWN:
		onZoom();
		return 0;

	case WM_MBUTTONDOWN:
		if ((wParam&MK_CONTROL) != NULL)
			PostMessage(MC::getMC()->getMainWindow()->getHwnd(), WM_KEYDOWN, VK_KILLKEY, NULL);
		else
			onMoveToBottom();
		return 0;

	case WM_KEYDOWN:
		if (wParam == VK_KILLKEY)
			PostMessage(MC::getMC()->getMainWindow()->getHwnd(), WM_KEYDOWN, VK_KILLKEY, NULL);
		else
			MC::getMC()->getWindowList()->keyPress(wParam);
		return 0;

	case WM_USER:

		if (wParam == WMMC_FOCUS)
		{
			MC::getMC()->getWindowList()->changeFocus(owner);
			MC::getMC()->getWindowList()->nextPile(1);
			MC::getMC()->getWindowList()->nextPile(-1);
			return 0;
		}

	default:
		return DefWindowProc(m_hwnd, uMsg, wParam, lParam);
	}

	return TRUE;
}

void ThumbWindow::onMoveToBottom()
{
	WindowList *wl = MC::getMC()->getWindowList();
	WindowItem *itemBelow = wl->getItemBelow(owner);
	if (itemBelow == NULL)
		return;
	owner->loseFocus();
	owner->pushToBottom();
	MC::getMC()->getWindowList()->reinitLayout();
}

void ThumbWindow::onSelectWindow(BOOL isCtrl)
{
	MC *mc = MC::getMC();
	mc->getWindowList()->changeFocus(owner);
	if (isCtrl)
		mc->getMainWindow()->onMinimizeAll(FALSE);
	owner->raiseToTop();
	owner->setStartRect(owner->getNormalRect());

	PostMessage(mc->getMainWindow()->getHwnd(), WM_USER, WMMC_TRANSITIONUP, (LPARAM)owner);
}

void ThumbWindow::onZoom()
{
	MC *mc = MC::getMC();
	McRect *mdR = &mc->getDisplayGeometry()->mdR;

	mc->getWindowList()->changeFocus(owner);
	McRect normal(owner->getNormalRect());

	if (normal.getWidth() > mdR->getWidth())
		normal.scale(((double)mdR->getWidth()) / normal.getWidth());
	if (normal.getHeight() > mdR->getHeight())
		normal.scale(((double)mdR->getHeight()) / normal.getHeight());

	McRect zoomRect(
		(mdR->getWidth() - normal.getWidth()) / 2,
		(mdR->getHeight() - normal.getHeight()) / 2,
		normal.getWidth(),
		normal.getHeight());
	zoomRect.right += zoomRect.left;
	zoomRect.bottom += zoomRect.top;

	mc->getZoomWindow()->doZoom(MC_ZOOM_UP, owner, owner->getThumbRect(), &zoomRect);
}

