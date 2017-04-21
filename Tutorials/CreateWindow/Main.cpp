#include "pch.h"
#include "ViewSource.h"

using namespace CreateWindow;
using namespace Windows::ApplicationModel::Core;

[Platform::MTAThread]
int main(Platform::Array<Platform::String^>^)
{
	auto viewSource = ref new ViewSource();
	CoreApplication::Run(viewSource);
	return 0;
}



