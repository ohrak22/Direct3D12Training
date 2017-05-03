#include "pch.h"
#include "App.h"

#include <ppltasks.h>

using namespace CreateDevice;

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

// main 함수는 IFrameworkView 클래스 초기화에만 사용됩니다.
[Platform::MTAThread]
int main(Platform::Array<Platform::String^>^)
{
	auto direct3DApplicationSource = ref new Direct3DApplicationSource();
	CoreApplication::Run(direct3DApplicationSource);
	return 0;
}

IFrameworkView^ Direct3DApplicationSource::CreateView()
{
	return ref new App();
}

App::App() :
	m_windowClosed(false),
	m_windowVisible(true)
{
}

// IFrameworkView를 만들 때 호출되는 첫 번째 메서드입니다.
void App::Initialize(CoreApplicationView^ applicationView)
{
	// 응용 프로그램 수명 주기의 이벤트 처리기를 등록합니다. 이 예에는 Activated가 포함되므로
	// CoreWindow를 활성 상태로 만들고 창에서 렌더링을 시작할 수 있습니다.
	applicationView->Activated += ref new TypedEventHandler<CoreApplicationView^, IActivatedEventArgs^>(this, &App::OnActivated);
	CoreApplication::Suspending += ref new EventHandler<SuspendingEventArgs^>(this, &App::OnSuspending);
	m_device = std::make_shared<Device>();
}

// CoreWindow 개체가 생성(또는 다시 생성)될 때 호출됩니다.
void App::SetWindow(CoreWindow^ window)
{
	window->VisibilityChanged += ref new TypedEventHandler<CoreWindow^, VisibilityChangedEventArgs^>(this, &App::OnVisibilityChanged);
	window->Closed += ref new TypedEventHandler<CoreWindow^, CoreWindowEventArgs^>(this, &App::OnWindowClosed);

	DisplayInformation^ currentDisplayInformation = DisplayInformation::GetForCurrentView();
	m_device->SetWindow(window);
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
		if (m_windowVisible)
		{
			CoreWindow::GetForCurrentThread()->Dispatcher->ProcessEvents(CoreProcessEventsOption::ProcessAllIfPresent);

			m_device->Present();
		}
		else
		{
			CoreWindow::GetForCurrentThread()->Dispatcher->ProcessEvents(CoreProcessEventsOption::ProcessOneAndAllPending);
		}
	}
}

// IFrameworkView에 필요합니다.
// 종료 이벤트는 Uninitialize를 호출하지 않습니다. Uninitialize는
// 앱이 전경에 있는 동안 IFrameworkView 클래스가 삭제되는 경우에 호출됩니다.
void App::Uninitialize()
{
}

// 응용 프로그램 수명 주기 이벤트 처리기입니다.
void App::OnActivated(CoreApplicationView^ applicationView, IActivatedEventArgs^ args)
{
	// CoreWindow가 활성화되어야 Run()이 시작됩니다.
	CoreWindow::GetForCurrentThread()->Activate();
}
void App::OnSuspending(Platform::Object^ sender, SuspendingEventArgs^ args)
{
	// 지연을 요청한 후 응용 프로그램 상태를 비동기적으로 저장합니다. 지연이 계속되는 것은
	// 응용 프로그램이 일시 중단 중인 작업을 수행하는 중이라는 의미입니다.
	// 지연이 무기한 지속되지 않을 수도 있습니다. 약 5초 후.
	//앱이 강제로 종료됩니다.
	SuspendingDeferral^ deferral = args->SuspendingOperation->GetDeferral();

	create_task([this, deferral]()
	{
		deferral->Complete();
	});
}

void App::OnVisibilityChanged(CoreWindow^ sender, VisibilityChangedEventArgs^ args)
{
	m_windowVisible = args->Visible;
}
void App::OnWindowClosed(CoreWindow^ sender, CoreWindowEventArgs^ args)
{
	m_windowClosed = true;
}

