/////////////////////////////////////////////////////////////////////////////////////////////
// COPYRIGHT 2012 D. D. Steiner                                                            //
//                                                                                         //
// This work is licensed under a                                                           //
// Creative Commons Attribution-NonCommercial-ShareAlike 3.0 Unported License.             //
// Terms of this license may be found at http://creativecommons.org/licenses/by-nc-sa/3.0/ //
// All other rights reserved.                                                              //
/////////////////////////////////////////////////////////////////////////////////////////////

#include "mc.h"
#include "WindowList.h"
#include "WindowItem.h"
#include "BgWindow.h"
#include "MainWindow.h"
#include "ThumbWindow.h"
#include "ZoomWindow.h"

WindowItem::WindowItem(
	HWND	_appHwnd,
	DWORD	_appPid,
	DWORD	_appTid,
	BOOL	_isDesktop,
	BOOL	_isMinimized,
	BOOL	_isMaximized,
	char *	_className,
	char*	_processName,
	RECT*	_normalRect,
	RECT*	_contentRect)
{
	useAlpha = FALSE;
	alpha = 255;

	appHwnd = _appHwnd;
	appPid = _appPid;
	appTid = _appTid;

	isDesktop = _isDesktop;
	isMinimized = _isMinimized;
	isMaximized = _isMaximized;
	className = new string(_className);
	processName = new string(_processName);
	normalRect.set(_normalRect);
	currentRect.set(_normalRect);
	contentRect.set(_contentRect);

	if (isDesktop)
		zOrder = 100000;
	else
		zOrder = MC::getZorder(appHwnd);

	thumbnail = NULL;
	thumbLocation = WIL_NO_WINDOW;
	thumbWindow = NULL;
	thumbHwnd = 0;

	haveFocus = FALSE;
}

WindowItem::~WindowItem()
{

	if (thumbnail) unregisterThumbnail();
	if (thumbWindow) MC::getMC()->disposeThumbWindow(thumbWindow->getHwnd());
	delete className;
	delete processName;
}

HRESULT WindowItem::registerThumbnail(wiThumbLocation _thumbLocation)
{
	return registerThumbnail(TRUE, _thumbLocation, FALSE);
}

HRESULT WindowItem::registerThumbnail(wiThumbLocation _thumbLocation, BOOL _useAlpha)
{
	return registerThumbnail(TRUE, _thumbLocation, _useAlpha);
}

HRESULT WindowItem::registerThumbnail(BOOL _unreg, wiThumbLocation _thumbLocation)
{
	return registerThumbnail(_unreg, _thumbLocation, FALSE);
}

HRESULT WindowItem::registerThumbnail(BOOL _unreg, wiThumbLocation _thumbLocation, BOOL _useAlpha)
{
	useAlpha = _useAlpha;

	if (_unreg && (thumbnail != NULL))
	{
		DwmUnregisterThumbnail(thumbnail);
		thumbnail = NULL;
	}

	if (!thumbWindow && !isDesktop)
		return GENERIC_HRESULT_FAILURE;

	HWND _displayHwnd;

	switch (_thumbLocation)
	{
	case WIL_MAIN_WINDOW:

		_displayHwnd = MC::getMC()->getMainWindow()->getHwnd();
		break;

	case WIL_ZOOM_WINDOW:

		_displayHwnd = MC::getMC()->getZoomWindow()->getHwnd();
		break;

	case WIL_THUMB_WINDOW:

		_displayHwnd = thumbWindow->getHwnd();
		break;

	case WIL_BG_WINDOW:

		_displayHwnd = MC::getMC()->getBgWindow()->getHwnd();
		break;

	default:

		return GENERIC_HRESULT_FAILURE;
	}

	thumbLocation = _thumbLocation;
	HRESULT hr = DwmRegisterThumbnail(_displayHwnd, appHwnd, &thumbnail);
	return hr;
}

void WindowItem::unregisterThumbnail(HTHUMBNAIL _thumbnail)
{
	if (_thumbnail)
		DwmUnregisterThumbnail(_thumbnail);
	else
		if (thumbnail)
		{
			DwmUnregisterThumbnail(thumbnail);
			thumbnail = NULL;
			thumbLocation = WIL_NO_WINDOW;
		}
}

HRESULT WindowItem::showThumbnail()
{
	if (!(thumbnail && (thumbWindow || isDesktop)))
		return GENERIC_HRESULT_FAILURE;

	DWM_THUMBNAIL_PROPERTIES props;
	props.dwFlags = DWM_TNP_VISIBLE | DWM_TNP_RECTDESTINATION | DWM_TNP_RECTSOURCE | DWM_TNP_OPACITY;
	props.fVisible = TRUE;
	if (useAlpha && isMinimized)
		props.opacity = alpha;
	else
		props.opacity = 255;

	McRect rect;

	switch (thumbLocation)
	{
	case WIL_MAIN_WINDOW:
	case WIL_ZOOM_WINDOW:
	case WIL_BG_WINDOW:
		rect.set(&currentRect);
		rect.offset(&MC::getMC()->getDisplayGeometry()->mdOffset);
		break;
	case WIL_THUMB_WINDOW:
		rect.left = rect.top = 0;
		rect.right = thumbRect.getWidth();
		rect.bottom = thumbRect.getHeight();
		break;
	default:
		return GENERIC_HRESULT_FAILURE;
	}

	props.rcDestination = rect;
	props.rcSource = contentRect;

	HRESULT hr = DwmUpdateThumbnailProperties(thumbnail, &props);

	return hr;
}

void WindowItem::createThumbWindow()
{
	if (thumbWindow || isDesktop) return;
	MC *mc = MC::getMC();
	thumbHwnd = mc->createThumbWindow(&thumbRect, this);
	thumbWindow = mc->getThumbWindow(thumbHwnd);
}

void WindowItem::closeThumbWindow()
{
	if (thumbWindow && !isDesktop) ShowWindow(thumbWindow->getHwnd(), SW_HIDE);
}

void WindowItem::showThumbWindow()
{
	if (!isDesktop)
		showThumbWindow(HWND_TOP);
}

void WindowItem::showThumbWindow(HWND hwnd)
{
	if (thumbWindow && !isDesktop)
	{
		SetWindowPos(thumbWindow->getHwnd(), hwnd,
			thumbRect.left, thumbRect.top, thumbRect.getWidth(), thumbRect.getHeight(),
			(hwnd == HWND_TOP ? SWP_SHOWWINDOW : SWP_NOACTIVATE | SWP_SHOWWINDOW));
	}
}

void WindowItem::placeInPile(BOOL reorder)
{
	if (isDesktop) return;

	WindowList *windowList = MC::getMC()->getWindowList();
	WindowItem *item = windowList->getItemAbove(this);
	if (item != NULL)
	{
		if (reorder) windowList->insertItemBefore(this, item);
		showThumbWindow(item->getThumbHwnd());
	}
	else
	{
		if (reorder) windowList->insertItemOnTop(this);
		showThumbWindow();
	}
}

void WindowItem::loseFocus()
{
	if (haveFocus)
		placeInPile();
	haveFocus = FALSE;
}

void WindowItem::pushToBottom()
{
	if (isDesktop) return;

	SetWindowPos(appHwnd, HWND_BOTTOM, normalRect.left, normalRect.top, normalRect.getWidth(), normalRect.getHeight(),
		SWP_NOACTIVATE);
	if (isMinimized)
		ShowWindowAsync(appHwnd, SW_MINIMIZE);
	WindowItem *bottomItem = MC::getMC()->getWindowList()->getFirstItem();
	zOrder = bottomItem->getZorder() + 1;
	placeInPile(TRUE);

}

void WindowItem::takeFocus()
{
	if (haveFocus) return;
	showThumbWindow();
	haveFocus = TRUE;
}

void WindowItem::raiseApp()
{
	SetFocus(getAppHwnd());
	SetWindowPos(getAppHwnd(), HWND_TOP, normalRect.left, normalRect.top,
		normalRect.getWidth(), normalRect.getHeight(), SWP_SHOWWINDOW);
	ShowWindow(getAppHwnd(), SW_SHOW);
	SetForegroundWindow(getAppHwnd());
}

void WindowItem::raiseToTop()
{
	if (isDesktop) return;

	if (isMinimized)
	{
		ShowWindowAsync(appHwnd, SW_RESTORE);
		setIsMinimized(FALSE);
	}
	WindowItem *topItem = MC::getMC()->getWindowList()->getLastItem();
	if (this != topItem)
	{
		zOrder = topItem->getZorder() - 1;
		placeInPile(TRUE);
	}

}

void WindowItem::setAlpha(double _alpha)
{
	alpha = (BYTE)((_alpha /** _alpha*/) * 255.);
}











