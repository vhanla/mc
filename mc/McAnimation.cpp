// Thank You Microsoft

#include "mc.h"
#include "McAnimation.h"

HWND McAnimation::animatedHwnd = NULL;
IUIAnimationVariable *McAnimation::variable = NULL;

McAnimation::McAnimation()
{

	animationManager = NULL;
	animationTimer = NULL;
	transitionLibrary = NULL;
	animationVariable = NULL;
}

McAnimation::~McAnimation()
{
	SafeRelease(&animationVariable);
	SafeRelease(&animationManager);
	SafeRelease(&animationTimer);
	SafeRelease(&transitionLibrary);
	animatedHwnd = NULL;
	variable = NULL;
}

HRESULT McAnimation::Initialize()
{
	IUIAnimationTimerUpdateHandler *timerUpdateHandler = nullptr;
	IUIAnimationTimerEventHandler  *timerEventHandler = nullptr;

	HRESULT hr = CoCreateInstance(
		CLSID_UIAnimationManager,
		NULL,
		CLSCTX_INPROC_SERVER,
		IID_PPV_ARGS(&animationManager)
	);

	// Create Animation Timer
	if (SUCCEEDED(hr))
		hr = CoCreateInstance(
			CLSID_UIAnimationTimer,
			NULL,
			CLSCTX_INPROC_SERVER,
			IID_PPV_ARGS(&animationTimer)
		);

	// Create Animation Transition Library
	if (SUCCEEDED(hr))
		hr = CoCreateInstance(
			CLSID_UIAnimationTransitionLibrary,
			NULL,
			CLSCTX_INPROC_SERVER,
			IID_PPV_ARGS(&transitionLibrary)
		);

	// Connect the animation manager to the timer.
	// UI_ANIMATION_IDLE_BEHAVIOR_DISABLE tells the timer to shut itself
	// off when there is nothing to animate.
	if (SUCCEEDED(hr))
		hr = animationManager->QueryInterface(
			IID_PPV_ARGS(&timerUpdateHandler));

	if (SUCCEEDED(hr))
	{
		hr = animationTimer->SetTimerUpdateHandler(
			timerUpdateHandler, UI_ANIMATION_IDLE_BEHAVIOR_DISABLE);
		SafeRelease(&timerUpdateHandler);
	}

	// Create and set the Timer Event Handler
	if (SUCCEEDED(hr))
		hr = CTimerEventHandler::CreateInstance(
			&timerEventHandler);

	if (SUCCEEDED(hr))
	{
		hr = animationTimer->SetTimerEventHandler(
			timerEventHandler);
		SafeRelease(&timerEventHandler);
	}

	return hr;
}

HRESULT McAnimation::doAnimate(HWND hwnd, double initialValue)
{

	SafeRelease(&animationVariable);
	HRESULT hr = animationManager->CreateAnimationVariable(initialValue, &animationVariable);

	double endingValue = (initialValue < 0.5 ? 1.0 : 0.0);

	setAnimatedInfo(hwnd, animationVariable);

	IUIAnimationStoryboard *pStoryboard = NULL;
	IUIAnimationTransition *pTransition = NULL;
	UI_ANIMATION_SECONDS secondsNow;

	if (SUCCEEDED(hr))
		hr = animationManager->CreateStoryboard(&pStoryboard);

	if (SUCCEEDED(hr))
		hr = transitionLibrary->
		CreateAccelerateDecelerateTransition(MC::getMC()->getProperties()->getAnnimationDuration(),
			endingValue, .5, .5, &pTransition);

	if (SUCCEEDED(hr))
		hr = pStoryboard->AddTransition(
			animationVariable, pTransition);

	if (SUCCEEDED(hr))
		hr = animationTimer->GetTime(&secondsNow);

	if (SUCCEEDED(hr))
		hr = pStoryboard->Schedule(secondsNow);

	SafeRelease(&pTransition);
	SafeRelease(&pStoryboard);

	return hr;
}