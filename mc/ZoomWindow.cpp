/////////////////////////////////////////////////////////////////////////////////////////////
// COPYRIGHT 2012 D. D. Steiner                                                            //
//                                                                                         //
// This work is licensed under a                                                           //
// Creative Commons Attribution-NonCommercial-ShareAlike 3.0 Unported License.             //
// Terms of this license may be found at http://creativecommons.org/licenses/by-nc-sa/3.0/ //
// All other rights reserved.                                                              //
/////////////////////////////////////////////////////////////////////////////////////////////

#include <Windows.h>
#include <GdiPlus.h>
#include <dwmapi.h>
#include "mc.h"
#include "MainWindow.h"
#include "ThumbWindow.h"
#include "ZoomWindow.h"
#include "WindowList.h"
#include "WindowItem.h"
#include "mcAnimation.h"

using namespace Gdiplus;

ZoomWindow::ZoomWindow()
{
	mode = MC_ZOOM_NONE;
	zwEvent = CreateEvent(NULL, TRUE, FALSE, TEXT("zoom thread"));
	zwThread = CreateThread(NULL, 0, BaseWindow::ThreadProc, this, 0, &zwThreadId);
}

ZoomWindow::~ZoomWindow()
{
	DestroyWindow(getHwnd());
	TerminateThread(zwThread, 0);
	CloseHandle(zwEvent);
	CloseHandle(zwThread);
}

LRESULT ZoomWindow::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_CREATE:

		return 0;

	case WM_SYSKEYUP:
	case WM_KEYUP:
	case WM_RBUTTONUP:
	case WM_LBUTTONDOWN:
	case WM_MBUTTONDOWN:

		doZoom(MC_ZOOM_DOWN, thumbOwner, &startRect, &endRect);
		return 0;

	case WM_USER:

		if (wParam == WMMC_HIDE)
		{
			HTHUMBNAIL oldThumb = thumbOwner->getThumbnail();
			thumbOwner->registerThumbnail(FALSE, WIL_THUMB_WINDOW);
			thumbOwner->showThumbnail();
			thumbOwner->showThumbWindow();
			if (oldThumb) thumbOwner->unregisterThumbnail(oldThumb);
			ShowWindow(getHwnd(), SW_HIDE);
		}
		return 0;

	case WM_DESTROY:
		return 0;

	case WM_PAINT:

		if (mode != MC_ZOOM_NONE)
		{
			PAINTSTRUCT ps;
			HDC hdc = BeginPaint(m_hwnd, &ps);
			onPaint(hdc);
			EndPaint(m_hwnd, &ps);
		}
		return 0;

	default:
		return DefWindowProc(m_hwnd, uMsg, wParam, lParam);

	}

	return TRUE;
}

void ZoomWindow::onPaint(HDC hdc)
{
	thumbOwner->showThumbnail();
}

void ZoomWindow::doZoom(int _mode, WindowItem *_thumbOwner, McRect *_startRect, McRect *_endRect)
{
	if (mode != MC_ZOOM_NONE)
		return;

	MC *mc = MC::getMC();
	McDisplay *geometry = mc->getDisplayGeometry();

	mode = _mode;
	thumbOwner = _thumbOwner;
	startRect.set(_startRect);
	endRect.set(_endRect);
	thumbOwner->getCurRect()->set(&startRect);

	if (mode == MC_ZOOM_UP)
	{
		McRect *r = &MC::getMC()->getDisplayGeometry()->dtR;

		SetWindowPos(getHwnd(), HWND_TOP,
			r->left, r->top, r->getWidth(), r->getHeight(), SWP_SHOWWINDOW);
		ShowWindow(getHwnd(), SW_SHOW);
		HTHUMBNAIL oldT = thumbOwner->getThumbnail();
		thumbOwner->registerThumbnail(FALSE, WIL_ZOOM_WINDOW);
		thumbOwner->showThumbnail();
		thumbOwner->closeThumbWindow();
		if (oldT) thumbOwner->unregisterThumbnail(oldT);
		MC::setState(MCS_Zoom);
	}
	else
		MC::setState(MCS_Display);
	MC::getMC()->getAnimation()->doAnimate(getHwnd(), (mode == MC_ZOOM_UP) ? 1.0 : 0.0);
}

DWORD ZoomWindow::runThread()
{
	while (TRUE)
	{
		DWORD hr = WaitForSingleObject(zwEvent, INFINITE);
		switch (hr)
		{
		case WAIT_OBJECT_0:
			break;
		default:
			return 0;
		}

		if (mode == MC_ZOOM_UP)
			Sleep(25); // Very important to let thumbnail update finish

		ResetEvent(zwEvent);

		if (mode == MC_ZOOM_DOWN)
			PostMessage(getHwnd(), WM_USER, WMMC_HIDE, NULL);

		mode = MC_ZOOM_NONE;

	}
	return 0;
}

void ZoomWindow::calcFrame(double value)
{
	thumbOwner->getCurRect()->interpolate(&startRect, &endRect, value);
	if (fabs(value) < 0.00000001 || fabs(1.0 - value) < 0.00000001)
		SetEvent(zwEvent);
}