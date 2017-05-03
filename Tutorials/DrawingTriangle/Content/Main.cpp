#include "pch.h"
#include "Common\DeviceResources.h"
#include "App.h"

using namespace DrawingTriangle;
using namespace Windows::ApplicationModel::Core;

ref class Direct3DApplicationSource sealed : Windows::ApplicationModel::Core::IFrameworkViewSource
{
public:
	virtual Windows::ApplicationModel::Core::IFrameworkView^ CreateView();
};

IFrameworkView^ Direct3DApplicationSource::CreateView()
{
	return ref new App();
}

// main 함수는 IFrameworkView 클래스 초기화에만 사용됩니다.
[Platform::MTAThread]
int main(Platform::Array<Platform::String^>^)
{
	auto direct3DApplicationSource = ref new Direct3DApplicationSource();
	CoreApplication::Run(direct3DApplicationSource);
	return 0;
}

