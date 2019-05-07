/////////////////////////////////////////////////////////////////////////////////////////////
// COPYRIGHT 2012 D. D. Steiner                                                            //
//                                                                                         //
// This work is licensed under a                                                           //
// Creative Commons Attribution-NonCommercial-ShareAlike 3.0 Unported License.             //
// Terms of this license may be found at http://creativecommons.org/licenses/by-nc-sa/3.0/ //
// All other rights reserved.                                                              //
/////////////////////////////////////////////////////////////////////////////////////////////
#pragma once

template <class DERIVED_TYPE>
class BaseWindow
{
public:
	static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		DERIVED_TYPE *pThis = NULL;

		if (uMsg == WM_NCCREATE)
		{
			CREATESTRUCT* pCreate = (CREATESTRUCT*)lParam;
			pThis = (DERIVED_TYPE*)pCreate->lpCreateParams;
			SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)pThis);

			pThis->m_hwnd = hwnd;
		}
		else
		{
			pThis = (DERIVED_TYPE*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
		}
		if (pThis)
		{
			return pThis->HandleMessage(uMsg, wParam, lParam);
		}
		else
		{
			return DefWindowProc(hwnd, uMsg, wParam, lParam);
		}
	}

	BaseWindow() : m_hwnd(NULL) {}

	BOOL Create(PCWSTR lpWindowName, McRect rect)
	{
		WNDCLASS wc = { 0 };

		wc.style = CS_DROPSHADOW;
		wc.lpfnWndProc = DERIVED_TYPE::WindowProc;
		wc.hInstance = GetModuleHandle(NULL);
		wc.lpszClassName = ClassName();

		RegisterClass(&wc);

		m_hwnd = CreateWindowEx(
			WS_EX_TOPMOST | WS_EX_TRANSPARENT | WS_EX_COMPOSITED | WS_EX_TOOLWINDOW,
			ClassName(),
			lpWindowName,
			WS_CHILD,
			rect.left, rect.top,
			rect.getWidth(), rect.getHeight(),
			GetDesktopWindow(),
			NULL,
			GetModuleHandle(NULL),
			this
		);

		return (m_hwnd ? TRUE : FALSE);
	}

	static DWORD WINAPI ThreadProc(LPVOID obj)
	{
		if (obj)
		{
			DERIVED_TYPE *pThis = (DERIVED_TYPE *)obj;
			return pThis->runThread();
		}
		return 1;
	}

	HWND getHwnd() const { return m_hwnd; }

protected:

	virtual ~BaseWindow()
	{
	}

	virtual PCWSTR  ClassName() const = 0;
	virtual LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam) = 0;
	virtual DWORD runThread() = 0;

	HWND m_hwnd;
};

