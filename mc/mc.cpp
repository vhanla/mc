/////////////////////////////////////////////////////////////////////////////////////////////
// COPYRIGHT 2012 D. D. Steiner                                                            //
//                                                                                         //
// This work is licensed under a                                                           //
// Creative Commons Attribution-NonCommercial-ShareAlike 3.0 Unported License.             //
// Terms of this license may be found at http://creativecommons.org/licenses/by-nc-sa/3.0/ //
// All other rights reserved.                                                              //
/////////////////////////////////////////////////////////////////////////////////////////////


// https://github.com/pauldotknopf/WindowsSDK7-Samples/blob/master/multimedia/WindowsAnimation/GridLayout/UIAnimationHelper.h

//#define UNICODE
#include <Windows.h>

#include "mc.h"
#include "ZoomWindow.h"
#include "ThumbWindow.h"
#include "MainWindow.h"
#include "BgWindow.h"
#include "WindowList.h"
#include "WindowItem.h"
#include "mcAnimation.h"
#include "McPiles.h"
#include "McProps.h"

#include <time.h>

#pragma comment( lib, "Dwmapi.lib" )	// Needed to use the dwm
#pragma comment( lib, "psapi.lib" )		// Needed to get path of executable for a window
#pragma comment( lib, "gdiplus.lib" )	// Needed to paint the background window and zoom window
#pragma comment( lib, "Msimg32.lib" )	// For painting the zoom window

MC *MC::mc = NULL;
McState MC::state = MCS_Idle;
FILE *MC::logFile = NULL;

#define _LISTEN_CLASS_NAME L"_MC_LISTEN_CLASS"
#define _LISTEN_WINDOW_NAME L"_MC_LISTEN_WINDOW"
#define _MAX_CYCLE_COUNT 9500

BOOL _runcycle();
void _runcycles();
void _runlistenloop(HINSTANCE);
void _signalinstance();
LRESULT CALLBACK _windowproc(HWND hwnd, UINT uMsg, WPARAM, LPARAM);

BOOL _incycle = FALSE;

int  _cyclecount = 0;
int  _messagecount = 0;


int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR pCmdLine, int nCmdShow)
{
	// This mutex prevents multiple instances from running at the same time.
	// If another instance is running we'll post a message simulating it's
	// hot key response.  Not pretty, but effective.

	CreateMutex(NULL, TRUE, (LPCWSTR)"_MC_MUTEX_QWERTY");
	if (GetLastError() == ERROR_ALREADY_EXISTS)
	{
		_signalinstance();
		return 0;
	}

	// Look, if composition is off this tool can't work because it depends
	// on transparency and the dwm.   So turn it on.
#pragma warning(disable : 4995)
	DwmEnableComposition(DWM_EC_ENABLECOMPOSITION);

	CoInitialize(NULL);
	srand((unsigned)(time(NULL) + hInstance));
	GdiplusStartupInput gdiplusStartupInput;
	ULONG_PTR gdiplusToken;
	GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

	// Determine if we're starting in resident or one-time mode

	char command_buffer[2000];
	MC::wchar2char(pCmdLine, command_buffer);
	if (strstr(command_buffer, "-resident"))
		_runlistenloop(hInstance);	// Resident
	else if (strstr(command_buffer, "-onetime"))
		_runcycles();					// Single
	else
	{
		_runcycles();					// Run once ...
		_runlistenloop(hInstance);	// then stay resident
	}
	GdiplusShutdown(gdiplusToken);

	CoUninitialize();

	return 0;
}

void _runlistenloop(HINSTANCE hInstance)
{
	WNDCLASS wc = { 0 };
	wc.lpfnWndProc = _windowproc;
	wc.hInstance = hInstance;
	wc.lpszClassName = _LISTEN_CLASS_NAME;
	RegisterClass(&wc);

	HWND hwnd = CreateWindow(
		_LISTEN_CLASS_NAME,
		_LISTEN_WINDOW_NAME,
		WS_CHILD,
		0, 0, 0, 0,
		HWND_MESSAGE,
		NULL,
		hInstance,
		NULL);

	MSG msg = {};
	while ((_cyclecount < _MAX_CYCLE_COUNT) && GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}

LRESULT CALLBACK _windowproc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	_messagecount++;

	switch (uMsg)
	{
	case WM_CREATE:

		if (!RegisterHotKey(hwnd, HOTKEY_CTAB, MOD_CONTROL | MOD_NOREPEAT, VK_TAB))
		{
			OutputDebugString(L"Could not register hot key.\n");
			exit(99);
		}

		return 0;

	case WM_USER:
	case WM_HOTKEY:

		if (_incycle)		// Never double down
			return 0;
		_incycle = TRUE;

		// Turn off hotkey monitoring while running, and turn it back on when done.
		// While running a separate trap of the hot key is one way to restore the
		// normal desktop view.

		UnregisterHotKey(hwnd, HOTKEY_CTAB);
		_runcycles();
		if (!RegisterHotKey(hwnd, HOTKEY_CTAB, MOD_CONTROL | MOD_NOREPEAT, VK_TAB))
		{
			exit(99);
		}
		_incycle = FALSE;
		return 0;

	default:

		return DefWindowProc(hwnd, uMsg, wParam, lParam);
	}
}

void _runcycles()
{
	BOOL runAgain = TRUE;
	while (runAgain)
		runAgain = _runcycle();
}

BOOL _runcycle()
{
	MC *mc = MC::getMC();
	mc->start();

	if (0 == mc->createMainWindow(L"MC3"))	exit(98);
	if (0 == mc->createZoomWindow())			exit(98);

	mc->getMainWindow()->start();

	// Note this message loop runs inside another message loop when
	// running in resident mode.   Either elegant or a kludge, hard
	// to know.

	MSG msg = {};
	while ((mc->getState() != MCS_Finishing) && GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	BOOL rval = mc->getRunAgain();
	delete mc;

	_cyclecount++;

	return rval;

}

void _signalinstance()
{
	HWND hwnd = FindWindow(_LISTEN_CLASS_NAME, _LISTEN_WINDOW_NAME);

	if (hwnd == NULL)
		MC::Log("Unable to locate running instance.\n");
	else
	{
		SetForegroundWindow(hwnd);
		PostMessage(hwnd, WM_USER, NULL, NULL);
	}
}

MC::MC()
{
	runAgain = FALSE;
	mainWindow = NULL;
	mcAnimation = NULL;
	bgWindow = NULL;
	thumbWindows = NULL;
	zoomWindow = NULL;
	properties = NULL;
	thumbCount = 0;
	thumbArraySize = 0;

	logFile = NULL;

	SYSTEMTIME time;
	GetLocalTime(&time);

	MC::Log("McSoft Desktop Window Organizer (Windows %x) at %d/%d/%0d %d:%02d:%02d\n\n",
		WINVER,
		time.wMonth, time.wDay, time.wYear,
		time.wHour, time.wMinute, time.wSecond);

	// Create the wrapper to access window's animation interface, which
	// is efficient compared to simple threading implementations.

	mcAnimation = new McAnimation();
	HRESULT hr = mcAnimation->Initialize();
	if (!SUCCEEDED(hr))
	{
		MC::Log("COULD NOT INITIALIZE ANIMATION\n");
		exit(96);
	}

	desktopWindow = GetDesktopWindow();

	POINT pt;
	pt.x = pt.y = 1;
	HMONITOR monitor = MonitorFromPoint(pt, MONITOR_DEFAULTTONEAREST);
	MONITORINFO minfo;
	minfo.cbSize = sizeof(MONITORINFO);
	GetMonitorInfo(monitor, &minfo);

	WINDOWPLACEMENT wp;
	GetWindowPlacement(desktopWindow, &wp);

	// When debugging leave non-primary monitors unaffected so that you
	// can operate the debugger.

#if DEBUGGING
	mdGeometry.dtR.set(&minfo.rcMonitor);
#else
	mdGeometry.dtR.set(&wp.rcNormalPosition);
#endif

	mdGeometry.mdR.set(&minfo.rcMonitor);
	mdGeometry.mdwR.set(&minfo.rcWork);

	Log("Desktop Rect: %d,%d  %d,%d\n", mdGeometry.dtR.left, mdGeometry.dtR.top,
		mdGeometry.dtR.right, mdGeometry.dtR.bottom);

	Log("Monitor Rect: %d,%d %d,%d\n", mdGeometry.mdR.left, mdGeometry.mdR.top,
		mdGeometry.mdR.right, mdGeometry.mdR.bottom);

	Log("Working Rect: %d,%d %d,%d\n", mdGeometry.mdwR.left, mdGeometry.mdwR.top,
		mdGeometry.mdwR.right, mdGeometry.mdwR.bottom);


	mdGeometry.mdAspect = ((double)mdGeometry.mdR.getWidth()) / ((double)mdGeometry.mdR.getHeight());
	mdGeometry.mdOffset.x = mdGeometry.mdR.left - mdGeometry.dtR.left;
	mdGeometry.mdOffset.y = mdGeometry.mdR.top - mdGeometry.dtR.top;
}

void MC::start()
{
	properties = new McProps();
	properties->read();

	windowList = new WindowList();

	SystemParametersInfo(SPI_GETWORKAREA, 1, &savedWorkArea, NULL);
	SystemParametersInfo(SPI_SETWORKAREA, 1, &mdGeometry.mdR, NULL);

	// The taskbar and start button can only interfere with the display,
	// so hide them

	ShowWindow(FindWindow(L"Shell_TrayWnd", L""), SW_HIDE);
	ShowWindow(FindWindow(L"Button", L"Start"), SW_HIDE);

	McPiles *piles = new McPiles();
	piles->createPiles();

	piles->layoutPiles();
	nPiles = piles->getNextPileNumber();
	piles->positionThumbs();
	windowList->createWindows();
	windowList->logAllWindows();

	delete piles;

}

MC::~MC()
{
	SystemParametersInfo(SPI_SETWORKAREA, 1, &savedWorkArea, 0);

	ShowWindow(FindWindow(L"Shell_TrayWnd", L""), SW_SHOW);
	ShowWindow(FindWindow(L"Button", L"Start"), SW_SHOW);
	if (windowList) delete windowList;
	if (properties) delete properties;
	if (mcAnimation) delete mcAnimation;
	if (thumbWindows) delete thumbWindows;

	disposeMainWindow();
	disposeZoomWindow();

	if (logFile)
	{
		fclose(logFile);
		logFile = NULL;
	}

	mc = NULL;
	state = MCS_Idle;
}

HWND MC::createMainWindow(PCWSTR name)
{
	if (mainWindow)
		return mainWindow->getHwnd();

	bgWindow = new BgWindow();
	bgWindow->Create(name, &mdGeometry.dtR);

	setGlobalDwmAttributes(bgWindow->getHwnd());

	mainWindow = new MainWindow(&mdGeometry.mdR);
	mainWindow->Create(name, &mdGeometry.mdR);
	PostMessage(mainWindow->getHwnd(), WM_SETCURSOR, (WPARAM)desktopWindow, NULL);

	setGlobalDwmAttributes(mainWindow->getHwnd());

	return mainWindow->getHwnd();
}

MainWindow* MC::getMainWindow()
{
	return mainWindow;
}

BgWindow* MC::getBgWindow()
{
	return bgWindow;
}

void MC::disposeMainWindow()
{
	delete mainWindow;
	mainWindow = NULL;
	delete bgWindow;
	bgWindow = NULL;
}

HWND MC::createThumbWindow(McRect *thumbR, WindowItem *item)
{
	if (thumbCount == thumbArraySize)
	{
		ThumbWindow **newT = new ThumbWindow*[thumbArraySize + ARRAY_ALLOC_INCREMENT];
		for (int i = 0;i<thumbCount;i++)
			newT[i] = thumbWindows[i];
		if (thumbArraySize > 0)
			delete thumbWindows;
		thumbArraySize += ARRAY_ALLOC_INCREMENT;
		thumbWindows = newT;
	}


	thumbWindows[thumbCount] = new ThumbWindow(item);
	thumbWindows[thumbCount]->Create(NULL, thumbR);

	// Thumbnail windows throw a shadow and have
	// a frosted glass backing, accomplished by the DWM call.
	// and by modifying the window style to WS_POPUP

	int policy = DWMNCRP_ENABLED;
	MARGINS marg = { -1 };
	DwmSetWindowAttribute(thumbWindows[thumbCount]->getHwnd(), DWMWA_NCRENDERING_POLICY, &policy, sizeof(int));
	DwmExtendFrameIntoClientArea(thumbWindows[thumbCount]->getHwnd(), &marg);

	LONG newStyle = WS_CHILD | WS_POPUP;
	SetWindowLongPtr(thumbWindows[thumbCount]->getHwnd(), GWL_STYLE, newStyle);

	setGlobalDwmAttributes(thumbWindows[thumbCount]->getHwnd());

	return thumbWindows[thumbCount++]->getHwnd();
}

void MC::setGlobalDwmAttributes(HWND hwnd)
{
	unsigned pva1 = DWMFLIP3D_EXCLUDEBELOW;
	DwmSetWindowAttribute(hwnd, DWMWA_FLIP3D_POLICY, &pva1, sizeof(pva1));
}

ThumbWindow* MC::getThumbWindow(HWND hwnd)
{
	for (int i = 0;i<thumbCount;i++)
		if (hwnd == thumbWindows[i]->getHwnd())
			return thumbWindows[i];
	return NULL;
}

void	MC::disposeThumbWindow(HWND hwnd)
{
	int i;
	for (i = 0;i<thumbCount;i++)
		if (hwnd == thumbWindows[i]->getHwnd())
			break;

	if (i < thumbCount)
	{
		delete thumbWindows[i];
		for (int j = i;j<thumbCount - 1;j++)
			thumbWindows[j] = thumbWindows[j + 1];
		thumbCount--;
	}
}

HWND MC::createZoomWindow()
{
	if (zoomWindow)
		return zoomWindow->getHwnd();

	zoomWindow = new ZoomWindow();
	zoomWindow->Create(NULL, this->mdGeometry.dtR);

	setGlobalDwmAttributes(zoomWindow->getHwnd());
	return zoomWindow->getHwnd();
}

void	MC::disposeZoomWindow()
{
	if (zoomWindow) delete zoomWindow;
	zoomWindow = NULL;
}

void	MC::setVanishingPoint(McRect *start, McRect *vanish)
{
	// Minimized windows have to vanish somewhere, and the animation
	// into the windows taskbar is something I've never liked.   So, like
	// old soldiers they just get smaller and fade away.

	vanish->left = vanish->right = (start->left + start->right) / 2;
	vanish->top = vanish->bottom = (start->top + start->bottom) / 2;
}






