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
#include <string>
#include <dwmapi.h>

#include "mc.h"

using namespace std;

enum wiThumbLocation
{
	WIL_NO_WINDOW,
	WIL_MAIN_WINDOW,
	WIL_THUMB_WINDOW,
	WIL_ZOOM_WINDOW,
	WIL_BG_WINDOW
};

class WindowItem
{
public:
	WindowItem(
		HWND	_appHwnd,
		DWORD	_appPid,
		DWORD	_appTid,
		BOOL	_isDesktop,
		BOOL	_isMinimized,
		BOOL	_isMaximized,
		char*	_className,
		char*	_processName,
		RECT*	_normalRect,
		RECT*	_contentRect);

	~WindowItem();

	// Action Functions

	HRESULT registerThumbnail(wiThumbLocation);
	HRESULT registerThumbnail(wiThumbLocation, BOOL);
	HRESULT registerThumbnail(BOOL, wiThumbLocation);
	HRESULT registerThumbnail(BOOL, wiThumbLocation, BOOL);

	void	unregisterThumbnail(HTHUMBNAIL = NULL);
	HRESULT	showThumbnail();

	void	createThumbWindow();

	void	showThumbWindow();
	void	showThumbWindow(HWND);
	void	closeThumbWindow();
	void	placeInPile(BOOL = FALSE);
	void	pushToBottom();
	void	raiseToTop();
	void	raiseApp();

	HWND	getAppHwnd() { return appHwnd; }
	DWORD	getAppPid() { return appPid; }
	DWORD	getAppTid() { return appTid; }
	BOOL	getIsDesktop() { return isDesktop; }
	BOOL	getIsMinimized() { return isMinimized; }
	BOOL	getIsMaximized() { return isMaximized; }
	string*	getClassName() { return className; }
	string*	getProgName() { return processName; }
	McRect*	getCurRect() { return &currentRect; }
	void	setCurRect(McRect *r) { currentRect.set(r); }
	McRect*	getNormalRect() { return &normalRect; }
	McRect*	getContentRect() { return &contentRect; }
	McRect*	getEndRect() { return &endRect; }
	void	setEndRect(McRect *r) { endRect.set(r); }
	McRect*	getThumbRect() { return &thumbRect; }
	void	setThumbRect(McRect *r) { thumbRect.set(r); }
	McRect*	getStartRect() { return &startRect; }
	void	setStartRect(McRect *r) { startRect.set(r); }
	LONG	getLeft() { return currentRect.left; }
	LONG	getTop() { return currentRect.top; }
	LONG	getRight() { return currentRect.right; }
	LONG	getBottom() { return currentRect.bottom; }
	LONG	getWidth() { return currentRect.getWidth(); }
	LONG	getHeight() { return currentRect.getHeight(); }
	int		getZorder() { return zOrder; }
	HWND	getThumbHwnd() { return thumbHwnd; }
	void	setPileNumber(int _pileNumber) { pileNumber = _pileNumber; }
	int		getPileNumber() { return pileNumber; }
	void	setIsMinimized(BOOL _isMinimized) { isMinimized = _isMinimized; }

	ThumbWindow*	getThumbWindow() { return thumbWindow; }
	HTHUMBNAIL		getThumbnail() { return thumbnail; }

	void setAlpha(double);

	void setHaveFocus(BOOL _havefocus) { haveFocus = _havefocus; }
	void takeFocus();
	void loseFocus();
private:
	HWND appHwnd;
	DWORD appPid;
	DWORD appTid;

	BOOL isDesktop;
	BOOL isMinimized;
	BOOL isMaximized;

	string *className;
	string *processName;

	HTHUMBNAIL thumbnail;
	wiThumbLocation thumbLocation;
	ThumbWindow *thumbWindow;
	HWND thumbHwnd;

	int			pileNumber;

	BOOL		haveFocus;

	McRect startRect;
	McRect endRect;
	McRect currentRect;

	McRect normalRect;
	McRect thumbRect;

	McRect contentRect;

	int zOrder;
	BOOL useAlpha;
	BYTE alpha;
};

#define ITEM_IT list<WindowItem*>::iterator
