#pragma once

#include "pch.h"

using namespace Windows::ApplicationModel::Core;
using namespace Windows::ApplicationModel::Activation;
using namespace Windows::UI::Core;

namespace CreateWindow
{
	// 응용 프로그램의 주 진입점입니다. 응용 프로그램을 Windows 셸과 연결하고 응용 프로그램 수명 주기 이벤트를 처리합니다.
	ref class App sealed : public IFrameworkView
	{
	private:
		bool m_windowClosed;
		bool m_windowVisible;

	public:
		App();

		// IFrameworkView 메서드.
		virtual void Initialize(CoreApplicationView^ applicationView);
		virtual void SetWindow(CoreWindow^ window);
		virtual void Load(Platform::String^ entryPoint);
		virtual void Run();
		virtual void Uninitialize();

	protected:
		void OnActivated(CoreApplicationView^ applicationView, IActivatedEventArgs^ args);
		void OnSuspending(Platform::Object^ sender, Windows::ApplicationModel::SuspendingEventArgs^ args);
		void OnResuming(Object^ sender, Object^ args);
		void OnWindowSizeChanged(CoreWindow^ sender, WindowSizeChangedEventArgs^ args);
		void OnVisibilityChanged(CoreWindow^ sender, VisibilityChangedEventArgs^ args);
		void OnWindowClosed(CoreWindow^ sender, CoreWindowEventArgs^ args);

		void OnPointerPressed(CoreWindow^ Window, PointerEventArgs^ Args);
		void OnPointerWheelChanged(CoreWindow ^ Window, PointerEventArgs ^ Args);
		void OnKeyDown(CoreWindow^ Window, KeyEventArgs^ Args);
		void OnKeyUp(CoreWindow^ Window, KeyEventArgs^ Args);
	

	};

	
}
