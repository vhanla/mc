// Attribution
// Most of the useful code in here comes from Microsofts
// animation example source, except for the contents of
// onPostUpdate which are subject to the license below

/////////////////////////////////////////////////////////////////////////////////////////////
// Portions COPYRIGHT 2012 D. D. Steiner                                                            //
//                                                                                         //
// This work is licensed under a                                                           //
// Creative Commons Attribution-NonCommercial-ShareAlike 3.0 Unported License.             //
// Terms of this license may be found at http://creativecommons.org/licenses/by-nc-sa/3.0/ //
// All other rights reserved.                                                              //
/////////////////////////////////////////////////////////////////////////////////////////////

#include <UIAnimation.h>
#include "UIAnimationHelper.h"
#include "mc.h"
#include "MainWindow.h"
#include "ZoomWindow.h"
#include "McProps.h"

template <class T> void SafeRelease(T **ppT)
{
	if (*ppT)
	{
		(*ppT)->Release();
		*ppT = NULL;
	}
}

class McAnimation
{
public:
	McAnimation();
	HRESULT Initialize();
	HRESULT doAnimate(HWND, double);
	~McAnimation();
private:
	IUIAnimationManager *animationManager;
	IUIAnimationTimer *animationTimer;
	IUIAnimationTransitionLibrary *transitionLibrary;

	IUIAnimationVariable *animationVariable;
public:
	static HWND getAnimatedHwnd()
	{
		return animatedHwnd;
	}

	static IUIAnimationVariable *getVariable()
	{
		return variable;
	}
private:
	static void setAnimatedInfo(HWND _animatedHwnd, IUIAnimationVariable *_variable)
	{
		animatedHwnd = _animatedHwnd;
		variable = _variable;
	}
	static HWND animatedHwnd;
	static IUIAnimationVariable *variable;
};

class CTimerEventHandler :
	public CUIAnimationTimerEventHandlerBase<CTimerEventHandler>
{
public:
	static HRESULT CreateInstance(
		IUIAnimationTimerEventHandler **ppTimerEventHandler) throw()
	{
		CTimerEventHandler *timerEventHandler;
		HRESULT hr = CUIAnimationCallbackBase::CreateInstance(
			ppTimerEventHandler, &timerEventHandler);
		return hr;
	}

	IFACEMETHODIMP OnPreUpdate()
	{
		return S_OK;
	}

	IFACEMETHODIMP	OnPostUpdate()
	{
		HWND hwnd = McAnimation::getAnimatedHwnd();

		double value;
		McAnimation::getVariable()->GetValue(&value);

		MC *mc = MC::getMC();
		if (mc->getMainWindow()->getHwnd() == hwnd)
			mc->getMainWindow()->calcFrame(value);
		else if (mc->getZoomWindow()->getHwnd() == hwnd)
			mc->getZoomWindow()->calcFrame(value);

		return InvalidateRect(hwnd, NULL, FALSE);
	}

	IFACEMETHODIMP OnRenderingTooSlow(UINT32 fps)
	{
		return S_OK;
	}

protected:

	CTimerEventHandler()
	{
	}
};
