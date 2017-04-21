#include "pch.h"
#include "App.h"

#include <ppltasks.h>

using namespace CreateWindow;

using namespace concurrency;
using namespace Windows::ApplicationModel;
using namespace Windows::ApplicationModel::Core;
using namespace Windows::ApplicationModel::Activation;
using namespace Windows::UI::Core;
using namespace Windows::UI::Input;
using namespace Windows::System;
using namespace Windows::Foundation;
using namespace Windows::Graphics::Display;

using Microsoft::WRL::ComPtr;



App::App()
{
	m_windowClosed = false;

}

// IFrameworkView를 만들 때 호출되는 첫 번째 메서드입니다.
void App::Initialize(CoreApplicationView^ applicationView)
{
}

// CoreWindow 개체가 생성(또는 다시 생성)될 때 호출됩니다.
void App::SetWindow(CoreWindow^ window)
{
	window->Closed += ref new TypedEventHandler<CoreWindow^, CoreWindowEventArgs^>(this, &App::OnWindowClosed);
}

// 장면 리소스를 초기화하거나 이전에 저장한 응용 프로그램 상태를 로드합니다.
void App::Load(Platform::String^ entryPoint)
{
}

// 이 메서드는 창이 활성화된 후 호출됩니다.
void App::Run()
{
	while (!m_windowClosed)
	{
	}
}

// IFrameworkView에 필요합니다.
// 종료 이벤트는 Uninitialize를 호출하지 않습니다. Uninitialize는
// 앱이 전경에 있는 동안 IFrameworkView 클래스가 삭제되는 경우에 호출됩니다.
void App::Uninitialize()
{
}

void App::OnWindowClosed(CoreWindow^ sender, CoreWindowEventArgs^ args)
{
	m_windowClosed = true;
}