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
#include <GdiPlusBase.h>
#include "BaseWindow.h"
#include "WindowList.h"
#include "WindowItem.h"
#include "mc.h"

using namespace Gdiplus;

class BgWindow : public BaseWindow<BgWindow>
{
public:
	~BgWindow() { DestroyWindow(getHwnd()); }

	PCWSTR  ClassName() const { return L"Dummy"; }
	DWORD runThread() { return 0; }
	void calcFrame(double value) {}

	// ATTRIBUTION: From code.logos.com, modified slightly

	IStream * BgWindow::CreateStreamOnResource(LPCTSTR lpName, LPCTSTR lpType)
	{
		IStream * ipStream = NULL;
		HGLOBAL hgblResourceData = NULL;

		HRSRC hrsrc = FindResource(NULL, lpName, lpType);
		if (hrsrc)
		{
			DWORD dwResourceSize = SizeofResource(NULL, hrsrc);
			HGLOBAL hglbImage = LoadResource(NULL, hrsrc);
			if (hglbImage)
			{
				LPVOID pvSourceResourceData = LockResource(hglbImage);
				if (pvSourceResourceData)
				{
					hgblResourceData = GlobalAlloc(GMEM_MOVEABLE, dwResourceSize);
					if (hgblResourceData)
					{
						LPVOID pvResourceData = GlobalLock(hgblResourceData);
						if (pvResourceData)
						{
							CopyMemory(pvResourceData, pvSourceResourceData, dwResourceSize);
							GlobalUnlock(hgblResourceData);
							if (SUCCEEDED(CreateStreamOnHGlobal(hgblResourceData, TRUE, &ipStream)))
								hgblResourceData = NULL;
						}
					}
				}
			}
		}

		if (hgblResourceData)
			GlobalFree(hgblResourceData);

		return ipStream;
	}

	LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		switch (uMsg)
		{
		case WM_CREATE:
		{
			McRect *r = &MC::getMC()->getDisplayGeometry()->dtR;
			SetWindowPos(getHwnd(), HWND_TOP,
				r->left, r->top, r->getWidth(), r->getHeight(),
				SWP_SHOWWINDOW);
		}
		return 0;

		case WM_DESTROY:
			return 0;

		case WM_PAINT:
		{
			MC *mc = MC::getMC();

			POINT *offsets = &mc->getDisplayGeometry()->mdOffset;

			McRect mR(&mc->getDisplayGeometry()->mdR);
			mR.offset(offsets->x, offsets->y);

			PAINTSTRUCT ps;
			HDC hdc = BeginPaint(m_hwnd, &ps);

			PaintDesktop(hdc);
			HBITMAP  bitm = LoadBitmap(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_BITM));
			HBRUSH b = CreatePatternBrush(bitm);
			FillRect(hdc, &mR, b);

			// Very crude way of getting something like a drop shadow
			// around the desktop rectangle.

			Graphics g(hdc);

			McRect mr(mc->getWindowList()->getDesktopItem()->getThumbRect());
			mr.offset(offsets->x, offsets->y);
			for (int i = 0;i<5;i++)
			{
				int alpha = 45 - i * 9;
				SolidBrush brush(Color(alpha, 50, 50, 50));
				mr.offset(-1, -1);
				mr.right += 2;
				mr.bottom += 2;
				Gdiplus::Rect r(mr.left, mr.top, mr.getWidth(), mr.getHeight());
				g.FillRectangle(&brush, r);
			}

			DeleteObject(b);
			DeleteObject(bitm);

			EndPaint(m_hwnd, &ps);
		}
		return 0;

		default:
			break;
		}
		return DefWindowProc(m_hwnd, uMsg, wParam, lParam);
	}
};