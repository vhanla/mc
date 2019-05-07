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
#include <string>
#include <math.h>

#include "resource.h"

#define DEBUGGING FALSE

#define GENERIC_HRESULT_FAILURE (0x80000000|ERROR_BAD_COMMAND)

#define HOTKEY_CTAB 0x564
#define HOTKEY_ESCAPE 0x563

#define VK_KILLKEY 0x4B

class McRect : public RECT
{
public:
	McRect()
	{
		clear();
	}

	void McRect::clear()
	{
		left = right = bottom = top = 0;
	}

	McRect(RECT *r)
	{
		set(r);
	}

	void McRect::set(RECT *r)
	{
		left = r->left;
		top = r->top;
		right = r->right;
		bottom = r->bottom;
	}

	void McRect::interpolate(RECT *start, RECT *finish, double value)
	{
		left = (LONG)floor(value*start->left + (1.0 - value)*finish->left);
		top = (LONG)floor(value*start->top + (1.0 - value)*finish->top);
		right = (LONG)floor(value*start->right + (1.0 - value)*finish->right);
		bottom = (LONG)floor(value*start->bottom + (1.0 - value)*finish->bottom);
	}

	BOOL McRect::contains(long x, long y)
	{
		return (left <= x && x <= right && top <= y && y <= bottom);
	}

	McRect(LONG l, LONG t, LONG r, LONG b)
	{
		set(l, t, r, b);
	}

	void McRect::set(LONG l, LONG t, LONG r, LONG b)
	{
		left = l;
		top = t;
		right = r;
		bottom = b;
	}

	void McRect::scale(double _scale)
	{
		left = (LONG)floor(_scale*left);
		top = (LONG)floor(_scale*top);
		right = (LONG)floor(_scale*right);
		bottom = (LONG)floor(_scale*bottom);
	}

	LONG McRect::getArea()
	{
		return getWidth()*getHeight();
	}

	void McRect::offset(long _xOffset, long _yOffset)
	{
		left += _xOffset;
		right += _xOffset;
		top += _yOffset;
		bottom += _yOffset;
	}

	void McRect::offset(POINT *o)
	{
		offset(o->x, o->y);
	}

	LONG McRect::getWidth()
	{
		return right - left;
	}

	LONG McRect::getHeight()
	{
		return bottom - top;
	}

	BOOL McRect::equals(McRect *two)
	{
		return left == two->left &&
			top == two->top &&
			bottom == two->bottom &&
			right == two->right;
	}

	LONG McRect::intersection(McRect *that)
	{
		return	max(0, min(right, that->right) - max(left, that->left))
			*	max(0, min(bottom, that->bottom) - max(top, that->top));
	}
};

struct McDisplay
{
	McRect dtR;
	McRect mdR;
	McRect mdwR;
	POINT  mdOffset;
	double mdAspect;
};

enum McState
{
	MCS_Idle,
	MCS_TransitionDown,
	MCS_Display,
	MCS_TransitionUp,
	MCS_Zoom,
	MCS_TransitionOver,
	MCS_Finishing
};

#define WMMC_TRANSITIONDOWN	0x0001
#define WMMC_TRANSITIONUP	0x0002
#define WMMC_TRANSITIONOVER 0x0003
#define WMMC_DISPLAY		0x0004
#define WMMC_HIDE			0x0005
#define WMMC_FOCUS			0x0006
#define WMMC_DESTROY		0x0007
#define WMMC_EXIT			0x00FF	

class MainWindow;
class BgWindow;
class ThumbWindow;
class ZoomWindow;
class WindowItem;
class WindowList;
class McAnimation;
class McPiles;
class McProps;

#define ARRAY_ALLOC_INCREMENT 10 // kludge

class MC
{
public:
	~MC();

	HWND			createMainWindow(PCWSTR);
	MainWindow*		getMainWindow();
	BgWindow*		getBgWindow();
	void			disposeMainWindow();
	HWND			createThumbWindow(McRect*, WindowItem *);
	ThumbWindow*	getThumbWindow(HWND);
	void			disposeThumbWindow(HWND);
	HWND			createZoomWindow();
	void			disposeZoomWindow();
	BOOL			isMultipleMonitor();
	void			setVanishingPoint(McRect *start, McRect *vanish);
	McAnimation*	getAnimation() { return mcAnimation; }

	McDisplay*		getDisplayGeometry() { return &mdGeometry; }
	HWND			getDesktopWindow() { return desktopWindow; }
	ZoomWindow*		getZoomWindow() { return zoomWindow; }
	WindowList*		getWindowList() { return windowList; }
	McProps*		getProperties() { return properties; }
	int				getNPiles() { return nPiles; }
	void			setNPiles(int _npiles) { nPiles = _npiles; }

	void			setRunAgain(BOOL _runAgain) { runAgain = _runAgain; }
	BOOL			getRunAgain(void) { return runAgain; }

	void start();
private:
	MC();
	void setGlobalDwmAttributes(HWND hwnd);

	McDisplay		mdGeometry;
	MainWindow*		mainWindow;
	BgWindow *		bgWindow;
	ThumbWindow**	thumbWindows;
	int				thumbCount;
	int				thumbArraySize;
	ZoomWindow*		zoomWindow;
	HWND			desktopWindow;
	RECT			savedMdRect;
	WindowList*		windowList;
	McAnimation*	mcAnimation;
	McProps*		properties;
	int				nPiles;
	BOOL			runAgain;

	RECT			savedWorkArea;

public:
	static McState  getState() { return state; }
	static void 	setState(McState _state)
	{
		state = _state;
	}
	static MC* getMC()
	{
		if (mc == 0)
			mc = new MC();
		return mc;
	}

	static void Log(char *fmt, ...)
	{
		char cbuff[1000];
		va_list args;
		va_start(args, fmt);
		vsprintf_s(cbuff, 1000, fmt, args);
		va_end(args);
#if DEBUGGING
		static WCHAR buff[1000];
		char2wchar(cbuff, buff);
		OutputDebugString(buff);
#else
		if (logFile == NULL)
			fopen_s(&logFile, "mc.log", "w");
		fprintf(logFile, cbuff);
#endif
	}

	static WCHAR* char2wchar(char *cstr, WCHAR *wstr)
	{
		size_t cc = 0;
		mbstowcs_s(&cc, wstr, 1 + strlen(cstr), cstr, _TRUNCATE);
		return wstr;
	}

	static char *wchar2char(WCHAR *wstr, char *cstr)
	{
		size_t cc = 0;
		wcstombs_s(&cc, cstr, 1 + wcslen(wstr) + 1, wstr, _TRUNCATE);
		return cstr;
	}

	static int getZorder(HWND appHwnd)
	{
		int	zOrder = 0;
		for (HWND h = appHwnd;h;h = GetWindow(h, GW_HWNDPREV), zOrder++);
		return zOrder;
	}
private:
	static MC		*mc;
	static McState	state;
	static FILE*	logFile;
};

