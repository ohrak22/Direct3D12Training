#include "pch.h"
#include "App.h"
#include "ViewSource.h"

using namespace CreateWindow;
using namespace Windows::ApplicationModel::Core;

IFrameworkView^ ViewSource::CreateView()
{
	return ref new App();
}