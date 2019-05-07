/////////////////////////////////////////////////////////////////////////////////////////////
// COPYRIGHT 2012 D. D. Steiner                                                            //
//                                                                                         //
// This work is licensed under a                                                           //
// Creative Commons Attribution-NonCommercial-ShareAlike 3.0 Unported License.             //
// Terms of this license may be found at http://creativecommons.org/licenses/by-nc-sa/3.0/ //
// All other rights reserved.                                                              //
/////////////////////////////////////////////////////////////////////////////////////////////

#include <windows.h>
#include <WindowsX.h>
#include <dwmapi.h>
#include "mc.h"
#include "MainWindow.h"
#include "BgWindow.h"
#include "ThumbWindow.h"
#include "ZoomWindow.h"
#include "WindowList.h"
#include "WindowItem.h"
#include "mcAnimation.h"

MainWindow::MainWindow(McRect *_desktopRect) : BaseWindow()
{
	desktopRect.set(_desktopRect);
	mwEvent = CreateEvent(NULL, TRUE, FALSE, TEXT("transition animation thread"));
	mwThread = CreateThread(NULL, 0, BaseWindow::ThreadProc, this, 0, &mwThreadId);
	activeApp = NULL;
}

MainWindow::~MainWindow()
{
	DestroyWindow(getHwnd());
	TerminateThread(mwThread, 0);
	CloseHandle(mwThread);
	CloseHandle(mwEvent);
}

void MainWindow::start()
{
	PostMessage(getHwnd(), WM_USER, WMMC_TRANSITIONDOWN, NULL);
}

LRESULT MainWindow::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{

	case WM_CREATE:
	{
		McRect *r = &MC::getMC()->getDisplayGeometry()->dtR;

		SetWindowPos(getHwnd(), HWND_TOP,
			r->left, r->top, r->getWidth(), r->getHeight(),
			SWP_SHOWWINDOW);

		RegisterHotKey(getHwnd(), HOTKEY_CTAB, MOD_CONTROL | MOD_NOREPEAT, VK_TAB);
		//if ( !RegisterHotKey( getHwnd(), HOTKEY_ESCAPE, MOD_NOREPEAT, VK_ESCAPE );
	}

	return 0;

	case WM_DESTROY:

		ShowWindow(getHwnd(), SW_HIDE);
		return 0;

	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(m_hwnd, &ps);
		onPaint(hdc, &ps.rcPaint);
		EndPaint(m_hwnd, &ps);
	}
	return 0;

	case WM_HOTKEY:

		if (MC::getMC()->getState() == MCS_Display)
		{
			UnregisterHotKey(getHwnd(), HOTKEY_CTAB);
			UnregisterHotKey(getHwnd(), HOTKEY_ESCAPE);
			PostMessage(m_hwnd, WM_USER, WMMC_TRANSITIONUP, NULL);
			return 0;
		}
		return DefWindowProc(m_hwnd, uMsg, wParam, lParam);

	case WM_LBUTTONUP:

		if (MC::getMC()->getState() == MCS_Display)
		{
			if (MC::getMC()->getWindowList()->getDesktopItem()->getThumbRect()->contains(
				GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)))
				onMinimizeAll();
			else
				PostMessage(m_hwnd, WM_USER, WMMC_TRANSITIONUP, (LPARAM)MC::getMC()->getWindowList()->getFirstItem());
			return 0;
		}
		return DefWindowProc(m_hwnd, uMsg, wParam, lParam);

	case WM_KEYDOWN:
		MC::getMC()->getWindowList()->keyPress(wParam);
		break;

	case WM_USER:

		switch (wParam)
		{
		case WMMC_TRANSITIONDOWN:

			if (MC::getMC()->getState() == MCS_Idle)
			{
				onTransitionDown();
				return 0;
			}
			break;

		case WMMC_TRANSITIONUP:

			if (MC::getMC()->getState() == MCS_Display)
			{
				activeApp = (WindowItem *)lParam;
				onTransitionUp();
				return 0;
			}

			break;

		case WMMC_TRANSITIONOVER:

			onTransitionOver();
			return 0;

		case WMMC_DISPLAY:

			onDisplay();
			return 0;

		case WMMC_DESTROY:

			onDestroy();
			return 0;

		default:
			break;
		}
		return DefWindowProc(m_hwnd, uMsg, wParam, lParam);

	default:
		return DefWindowProc(m_hwnd, uMsg, wParam, lParam);
	}

	return DefWindowProc(m_hwnd, uMsg, wParam, lParam);;
}

void MainWindow::onTransitionUp()
{
	MC *mc = MC::getMC();

	// set the new state

	mc->setState(MCS_TransitionUp);

	// Hide each subwindow, detach the thumbnail and reattach it
	// to the mainwindow.   Schedule the animation.

	WindowList *windowList = mc->getWindowList();

	windowList->initializeRects(MCS_TransitionUp);

	HWND main = mc->getMainWindow()->getHwnd();
	for (WindowItem *item = windowList->getFirstItem();item;item = windowList->getNextItem())
	{
		HTHUMBNAIL oldThumb = item->getThumbnail();
		item->registerThumbnail(FALSE, WIL_MAIN_WINDOW, TRUE);
		item->showThumbnail();
		item->closeThumbWindow();
		SetForegroundWindow(main);
		if (oldThumb) item->unregisterThumbnail(oldThumb);
	}


	MC::getMC()->getAnimation()->doAnimate(getHwnd(), 0.0);
}

void MainWindow::onTransitionDown()
{

	MC *mc = MC::getMC();

	WindowList *windowList = mc->getWindowList();

	windowList->initializeRects(MCS_TransitionDown);

	windowList->getDesktopItem()->registerThumbnail(WIL_BG_WINDOW);
	for (WindowItem *item = windowList->getFirstItem();item;item = windowList->getNextItem())
	{
		item->registerThumbnail(WIL_MAIN_WINDOW, TRUE);
		item->showThumbnail();
	}

	mc->setState(MCS_TransitionDown);
	mc->getAnimation()->doAnimate(getHwnd(), 0.0);
}

void MainWindow::onTransitionOver()
{

	MC *mc = MC::getMC();

	WindowList *windowList = mc->getWindowList();

	windowList->initializeRects(MCS_TransitionOver);

	HWND main = mc->getMainWindow()->getHwnd();
	for (WindowItem *item = windowList->getFirstItem();item;item = windowList->getNextItem())
	{
		HTHUMBNAIL oldThumb = item->getThumbnail();
		item->registerThumbnail(FALSE, WIL_MAIN_WINDOW, TRUE);
		item->showThumbnail();
		item->closeThumbWindow();
		SetForegroundWindow(main);
		if (oldThumb) item->unregisterThumbnail(oldThumb);
	}

	mc->setState(MCS_TransitionOver);
	mc->getAnimation()->doAnimate(getHwnd(), 0.0);
}


void MainWindow::onDestroy()
{
	if (activeApp)
		activeApp->raiseApp();

	SetWindowPos(MC::getMC()->getBgWindow()->getHwnd(), HWND_BOTTOM, 0, 0, 0, 0, SWP_HIDEWINDOW);
	SetWindowPos(getHwnd(), HWND_BOTTOM, 0, 0, 0, 0, SWP_HIDEWINDOW);

	WindowList *windowList = MC::getMC()->getWindowList();
	windowList->getDesktopItem()->unregisterThumbnail();
	for (WindowItem *item = windowList->getFirstItem();item;item = windowList->getNextItem())
		item->unregisterThumbnail();


	MC::setState(MCS_Finishing);
}

void MainWindow::onDisplay()
{
	MC *mc = MC::getMC();

	if (mc->getState() != MCS_TransitionDown && mc->getState() != MCS_TransitionOver)
		return;

	mc->setState(MCS_Display);

	WindowList *windowList = mc->getWindowList();

	if (NULL == windowList->getFirstItem())
		PostMessage(getHwnd(), WM_USER, WMMC_TRANSITIONUP, NULL);

	for (WindowItem *item = windowList->getFirstItem();item;item = windowList->getNextItem())
	{
		HTHUMBNAIL oldThumb = item->getThumbnail();

		item->registerThumbnail(FALSE, WIL_THUMB_WINDOW);
		item->showThumbnail();
		item->showThumbWindow(getHwnd());

		if (oldThumb) item->unregisterThumbnail(oldThumb);
	}

	McRect *r = &mc->getDisplayGeometry()->dtR;
	if (windowList->getFirstItem() != NULL)
		SetWindowPos(getHwnd(), windowList->getFirstItem()->getThumbHwnd(),
			r->left, r->top, r->getWidth(), r->getHeight(), SWP_NOACTIVATE);

	if (windowList->getLastItem())
		PostMessage(windowList->getLastItem()->getThumbHwnd(), WM_USER, WMMC_FOCUS, NULL);
}

void MainWindow::onMinimizeAll(BOOL terminate)
{
	MC *mc = MC::getMC();
	WindowList *windowList = mc->getWindowList();

	for (WindowItem *item = windowList->getFirstItem();item;item = windowList->getNextItem())
	{
		ShowWindowAsync(item->getAppHwnd(), SW_MINIMIZE);
		item->setIsMinimized(TRUE);
	}
	if (terminate)
		PostMessage(m_hwnd, WM_USER, WMMC_TRANSITIONUP, NULL);
}

void MainWindow::onNormalizeAll()
{
	WindowList *windowList = MC::getMC()->getWindowList();

	for (WindowItem *item = windowList->getFirstItem();item;item = windowList->getNextItem())
	{
		if (item->getIsMinimized())
		{
			ShowWindowAsync(item->getAppHwnd(), SW_RESTORE);
			item->setIsMinimized(FALSE);
			item->setStartRect(item->getNormalRect());
		}
	}

	PostMessage(m_hwnd, WM_USER, WMMC_TRANSITIONUP, NULL);
}

void MainWindow::onPaint(HDC hdc, RECT *r)
{
	switch (MC::getState())
	{
	case MCS_Idle:

		return;

	case MCS_Finishing:
	case MCS_TransitionUp:
	case MCS_TransitionDown:
	case MCS_TransitionOver:

	{
		WindowList *windowList = MC::getMC()->getWindowList();
		windowList->getDesktopItem()->showThumbnail();
		for (WindowItem *item = windowList->getFirstItem(); item; item = windowList->getNextItem())
			item->showThumbnail();
	}
	break;

	default:
		break;
	}

}

DWORD MainWindow::runThread()
{
	while (TRUE)
	{
		DWORD hr = WaitForSingleObject(mwEvent, INFINITE);
		switch (hr)
		{
		case WAIT_OBJECT_0:
			break;
		default:
			return 0;
		}

		ResetEvent(mwEvent);

		switch (MC::getState())
		{
		case MCS_TransitionUp:
			PostMessage(getHwnd(), WM_USER, WMMC_DESTROY, NULL);
			break;
		case MCS_TransitionDown:
		case MCS_TransitionOver:
			Sleep(1);
			PostMessage(getHwnd(), WM_USER, WMMC_DISPLAY, NULL);
			break;
		default:
			MC::Log("***MainWindow thread, unexpected state.\n");
			break;
		}
	}
}

void MainWindow::calcFrame(double value)
{
	MC *mc = MC::getMC();
	WindowList *windowList = mc->getWindowList();
	windowList->getDesktopItem()->getCurRect()->interpolate(
		windowList->getDesktopItem()->getEndRect(),
		windowList->getDesktopItem()->getStartRect(),
		value);
	double alphaval = value;
	switch (MC::getState())
	{
	case MCS_TransitionUp:
		alphaval = 1.0 - value;
		break;
	case MCS_TransitionOver:
		alphaval = 1.0;
		break;
	default:
		alphaval = value;
		break;
	}
	for (WindowItem *item = windowList->getFirstItem();item;item = windowList->getNextItem())
	{
		item->getCurRect()->interpolate(
			item->getEndRect(),
			item->getStartRect(),
			value);
		item->setAlpha(alphaval);
	}
	if (fabs(value) < 0.00000001 || fabs(1.0 - value) < 0.00000001)
		SetEvent(mwEvent);
}



