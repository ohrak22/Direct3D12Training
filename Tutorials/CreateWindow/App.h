#pragma once

#include "pch.h"

namespace CreateWindow
{
	// 응용 프로그램의 주 진입점입니다. 응용 프로그램을 Windows 셸과 연결하고 응용 프로그램 수명 주기 이벤트를 처리합니다.
	ref class App sealed : public Windows::ApplicationModel::Core::IFrameworkView
	{
	public:
		App();

		// IFrameworkView 메서드.
		virtual void Initialize(Windows::ApplicationModel::Core::CoreApplicationView^ applicationView);
		virtual void SetWindow(Windows::UI::Core::CoreWindow^ window);
		virtual void Load(Platform::String^ entryPoint);
		virtual void Run();
		virtual void Uninitialize();

	protected:
		void OnWindowClosed(Windows::UI::Core::CoreWindow^ sender, Windows::UI::Core::CoreWindowEventArgs^ args);

	private:
		bool m_windowClosed;

	};

	
}
