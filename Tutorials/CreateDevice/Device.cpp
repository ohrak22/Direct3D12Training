#include "pch.h"
#include "Device.h"
#include "../Common/DirectXHelper.h"

using namespace CreateDevice;
using namespace Microsoft::WRL;
using namespace Windows::Foundation;
using namespace Windows::Graphics::Display;
using namespace Windows::UI::Core;
using namespace Windows::UI::Xaml::Controls;
using namespace Platform;
using namespace DX;

// Device�� �������Դϴ�.
Device::Device(DXGI_FORMAT backBufferFormat, DXGI_FORMAT depthBufferFormat) :
	m_currentFrame(0),
	m_screenViewport(),
	m_rtvDescriptorSize(0),
	m_fenceEvent(0),
	m_backBufferFormat(backBufferFormat),
	m_depthBufferFormat(depthBufferFormat),
	m_fenceValues{},
	m_d3dRenderTargetSize(),
	m_outputSize(),
	m_logicalSize(),
	m_nativeOrientation(DisplayOrientations::None),
	m_currentOrientation(DisplayOrientations::None),
	m_dpi(-1.0f),
	m_effectiveDpi(-1.0f),
	m_deviceRemoved(false)
{
	CreateDeviceResources();
}

// Direct3D ��ġ�� �����ϰ� �ش� ��ġ�� ���� �ڵ� �� ��ġ ���ؽ�Ʈ�� �����մϴ�.
void Device::CreateDeviceResources()
{
#if defined(_DEBUG)
	// ������Ʈ�� ����� ���� ���� ��� SDK ���̾ ���� ������� �����ϵ��� �����ϼ���.
	{
		ComPtr<ID3D12Debug> debugController;
		if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
		{
			debugController->EnableDebugLayer();
		}
	}
#endif

	DX::ThrowIfFailed(CreateDXGIFactory1(IID_PPV_ARGS(&m_dxgiFactory)));

	ComPtr<IDXGIAdapter1> adapter;
	GetHardwareAdapter(&adapter);

	// Direct3D 12 API ��ġ ��ü�� ����ϴ�.
	HRESULT hr = D3D12CreateDevice(
		adapter.Get(),					// �ϵ���� ������Դϴ�.
		D3D_FEATURE_LEVEL_11_0,			// �� ���� ������ �� �ִ� �ִ� ��� �����Դϴ�.
		IID_PPV_ARGS(&m_d3dDevice)		// ������� Direct3D ��ġ�� ��ȯ�մϴ�.
	);

#if defined(_DEBUG)
	if (FAILED(hr))
	{
		// �ʱ�ȭ�� �����ϸ� WARP ��ġ�� ��ü�˴ϴ�.
		// WARP�� ���� �ڼ��� ������ ������ �����ϼ���. 
		// http://go.microsoft.com/fwlink/?LinkId=286690

		ComPtr<IDXGIAdapter> warpAdapter;
		DX::ThrowIfFailed(m_dxgiFactory->EnumWarpAdapter(IID_PPV_ARGS(&warpAdapter)));

		hr = D3D12CreateDevice(warpAdapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&m_d3dDevice));
	}
#endif

	DX::ThrowIfFailed(hr);

	// ��� ť�� ����ϴ�.
	D3D12_COMMAND_QUEUE_DESC queueDesc = {};
	queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

	DX::ThrowIfFailed(m_d3dDevice->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_commandQueue)));
	NAME_D3D12_OBJECT(m_commandQueue);

	// ������ ��� �信 ���� ������ ���� ����ϴ�.
	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
	rtvHeapDesc.NumDescriptors = g_frameCount;
	rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	DX::ThrowIfFailed(m_d3dDevice->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&m_rtvHeap)));
	NAME_D3D12_OBJECT(m_rtvHeap);

	m_rtvDescriptorSize = m_d3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	for (UINT n = 0; n < g_frameCount; n++)
	{
		DX::ThrowIfFailed(
			m_d3dDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_commandAllocators[n]))
		);
	}

	// ����ȭ ��ü�� ����ϴ�.
	DX::ThrowIfFailed(m_d3dDevice->CreateFence(m_fenceValues[m_currentFrame], D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence)));
	m_fenceValues[m_currentFrame]++;

	m_fenceEvent = CreateEventEx(nullptr, FALSE, FALSE, EVENT_ALL_ACCESS);
}

// â ũ�Ⱑ ����� ������ �̷��� ���ҽ��� �ٽ� ������ �մϴ�.
void Device::CreateWindowSizeDependentResources()
{
	// ��� ���� GPU �۾��� �Ϸ�� ������ ��ٸ��ϴ�.
	WaitForGpu();

	// ���� â ũ�� ���� �������� ����� ������ fence ���� ������Ʈ�մϴ�.
	for (UINT n = 0; n < g_frameCount; n++)
	{
		m_renderTargets[n] = nullptr;
		m_fenceValues[n] = m_fenceValues[m_currentFrame];
	}

	UpdateRenderTargetSize();

	// ���� ü���� �ʺ�� ���̴� â�� ���� ���� �ʺ� �� ���̸�
	// �������� �ؾ� �մϴ�. â�� ���� ������ �ƴ� ��쿡��
	// ġ���� �ݴ�� �ؾ� �մϴ�.
	DXGI_MODE_ROTATION displayRotation = ComputeDisplayRotation();

	bool swapDimensions = displayRotation == DXGI_MODE_ROTATION_ROTATE90 || displayRotation == DXGI_MODE_ROTATION_ROTATE270;
	m_d3dRenderTargetSize.Width = swapDimensions ? m_outputSize.Height : m_outputSize.Width;
	m_d3dRenderTargetSize.Height = swapDimensions ? m_outputSize.Width : m_outputSize.Height;

	UINT backBufferWidth = lround(m_d3dRenderTargetSize.Width);
	UINT backBufferHeight = lround(m_d3dRenderTargetSize.Height);

	if (m_swapChain != nullptr)
	{
		// ���� ü���� �̹� ������ ��� ũ�⸦ �����մϴ�.
		HRESULT hr = m_swapChain->ResizeBuffers(g_frameCount, backBufferWidth, backBufferHeight, m_backBufferFormat, 0);

		if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET)
		{
			// � �����ε� ��ġ�� ���ŵ� ��� �� ��ġ�� ���� ü���� ������ �մϴ�.
			m_deviceRemoved = true;

			// �� �޼��带 ��� �������� ������. Device�� ���ŵǰ� �ٽ� ��������ϴ�.
			return;
		}
		else
		{
			DX::ThrowIfFailed(hr);
		}
	}
	else
	{
		// �׷��� ������ ���� Direct3D ��ġ�� ������ ����͸� ����Ͽ� �� �׸��� ����ϴ�.
		//DXGI_SCALING scaling = DisplayMetrics::SupportHighResolutions ? DXGI_SCALING_NONE : DXGI_SCALING_STRETCH;
		DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};

		swapChainDesc.Width = backBufferWidth;						// â�� ũ�⸦ ����ϴ�.
		swapChainDesc.Height = backBufferHeight;
		swapChainDesc.Format = m_backBufferFormat;
		swapChainDesc.Stereo = false;
		swapChainDesc.SampleDesc.Count = 1;							// ���� ���ø��� ������� �ʽ��ϴ�.
		swapChainDesc.SampleDesc.Quality = 0;
		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.BufferCount = g_frameCount;					// 3�� ���۸��� ����Ͽ� ��� �ð��� �ּ�ȭ�մϴ�.
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;	// ��� Windows ���Ϲ��� ���� _FLIP_ SwapEffects�� ����ؾ� �մϴ�.
		swapChainDesc.Flags = 0;
		//swapChainDesc.Scaling = scaling;
		swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_IGNORE;

		ComPtr<IDXGISwapChain1> swapChain;
		DX::ThrowIfFailed(
			m_dxgiFactory->CreateSwapChainForCoreWindow(
				m_commandQueue.Get(),								// ���� ü�ο��� DirectX 12�� ��� ť�� ���� ������ �ʿ��մϴ�.
				reinterpret_cast<IUnknown*>(m_window.Get()),
				&swapChainDesc,
				nullptr,
				&swapChain
			)
		);

		DX::ThrowIfFailed(swapChain.As(&m_swapChain));
	}

	// ���� ü�� �� ������ ������ ��� �並 ����ϴ�.
	m_currentFrame = m_swapChain->GetCurrentBackBufferIndex();
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvDescriptor(m_rtvHeap->GetCPUDescriptorHandleForHeapStart());
	for (UINT n = 0; n < g_frameCount; n++)
	{
		DX::ThrowIfFailed(m_swapChain->GetBuffer(n, IID_PPV_ARGS(&m_renderTargets[n])));
		m_d3dDevice->CreateRenderTargetView(m_renderTargets[n].Get(), nullptr, rtvDescriptor);
		rtvDescriptor.Offset(m_rtvDescriptorSize);

		WCHAR name[25];
		if (swprintf_s(name, L"m_renderTargets[%u]", n) > 0)
		{
			DX::SetName(m_renderTargets[n].Get(), name);
		}
	}

	// ��ü â�� ������� �ϱ� ���� 3D ������ ����Ʈ�� �����մϴ�.
	m_screenViewport = { 0.0f, 0.0f, m_d3dRenderTargetSize.Width, m_d3dRenderTargetSize.Height, 0.0f, 1.0f };
}

// ������ ����� ũ��� ������ ����� ��ҵ��� ���θ� �����մϴ�.
void Device::UpdateRenderTargetSize()
{
	m_effectiveDpi = m_dpi;

	// �ʿ��� ������ ��� ũ�⸦ �ȼ� ������ ����մϴ�.
	m_outputSize.Width = DX::ConvertDipsToPixels(m_logicalSize.Width, m_effectiveDpi);
	m_outputSize.Height = DX::ConvertDipsToPixels(m_logicalSize.Height, m_effectiveDpi);

	// DirectX ������ ũ�⸦ 0���� ������ �ʽ��ϴ�.
	m_outputSize.Width = max(m_outputSize.Width, 1);
	m_outputSize.Height = max(m_outputSize.Height, 1);
}

// �� �޼���� CoreWindow�� ����ų� �ٽ� ���� �� ȣ��˴ϴ�.
void Device::SetWindow(CoreWindow^ window)
{
	DisplayInformation^ currentDisplayInformation = DisplayInformation::GetForCurrentView();

	m_window = window;
	m_logicalSize = Windows::Foundation::Size(window->Bounds.Width, window->Bounds.Height);
	m_nativeOrientation = currentDisplayInformation->NativeOrientation;
	m_currentOrientation = currentDisplayInformation->CurrentOrientation;
	m_dpi = currentDisplayInformation->LogicalDpi;

	CreateWindowSizeDependentResources();
}

// �� �޼���� SizeChanged �̺�Ʈ�� �̺�Ʈ ó���⿡�� ȣ��˴ϴ�.
void Device::SetLogicalSize(Windows::Foundation::Size logicalSize)
{
	if (m_logicalSize != logicalSize)
	{
		m_logicalSize = logicalSize;
		CreateWindowSizeDependentResources();
	}
}

// �� �޼���� DpiChanged �̺�Ʈ�� �̺�Ʈ ó���⿡�� ȣ��˴ϴ�.
void Device::SetDpi(float dpi)
{
	if (dpi != m_dpi)
	{
		m_dpi = dpi;

		// ���÷��� DPI�� ����� ��� â�� ���� ũ��(DIP ������ ����)�� ����Ǹ� ������Ʈ�ؾ� �մϴ�.
		m_logicalSize = Windows::Foundation::Size(m_window->Bounds.Width, m_window->Bounds.Height);

		CreateWindowSizeDependentResources();
	}
}

// �� �޼���� OrientationChanged �̺�Ʈ�� �̺�Ʈ ó���⿡�� ȣ��˴ϴ�.
void Device::SetCurrentOrientation(DisplayOrientations currentOrientation)
{
	if (m_currentOrientation != currentOrientation)
	{
		m_currentOrientation = currentOrientation;
		CreateWindowSizeDependentResources();
	}
}

// �� �޼���� DisplayContentsInvalidated �̺�Ʈ�� �̺�Ʈ ó���⿡�� ȣ��˴ϴ�.
void Device::ValidateDevice()
{
	// �⺻ ����Ͱ� ��ġ�� ������� ���Ŀ� ����ǰų� ��ġ�� ���ŵ� ���
	// D3D ��ġ�� �� �̻� ��ȿ���� �ʽ��ϴ�.

	// ���� ��ġ�� ������� ���� �⺻ ����Ϳ� ���� LUID�� �����ɴϴ�.
	DXGI_ADAPTER_DESC previousDesc;
	{
		ComPtr<IDXGIAdapter1> previousDefaultAdapter;
		DX::ThrowIfFailed(m_dxgiFactory->EnumAdapters1(0, &previousDefaultAdapter));

		DX::ThrowIfFailed(previousDefaultAdapter->GetDesc(&previousDesc));
	}

	// ��������, ���� �⺻ ����Ϳ� ���� ������ �����ɴϴ�.
	DXGI_ADAPTER_DESC currentDesc;
	{
		ComPtr<IDXGIFactory4> currentDxgiFactory;
		DX::ThrowIfFailed(CreateDXGIFactory1(IID_PPV_ARGS(&currentDxgiFactory)));

		ComPtr<IDXGIAdapter1> currentDefaultAdapter;
		DX::ThrowIfFailed(currentDxgiFactory->EnumAdapters1(0, &currentDefaultAdapter));

		DX::ThrowIfFailed(currentDefaultAdapter->GetDesc(&currentDesc));
	}

	// ����� LUID�� ��ġ���� �ʰų� ��ġ�� ���ŵǾ��ٰ� �����ϴ� ���
	// �� D3D ��ġ�� ������ �մϴ�.
	if (previousDesc.AdapterLuid.LowPart != currentDesc.AdapterLuid.LowPart ||
		previousDesc.AdapterLuid.HighPart != currentDesc.AdapterLuid.HighPart ||
		FAILED(m_d3dDevice->GetDeviceRemovedReason()))
	{
		m_deviceRemoved = true;
	}
}

// ���� ü���� �������� ȭ�鿡 ǥ���մϴ�.
void Device::Present()
{
	// ù ��° �μ��� DXGI�� VSync���� �����ϵ��� �����Ͽ� ���� ���α׷���
	// ���� VSync���� ����ϵ��� �մϴ�. �̸� ���� ȭ�鿡 ǥ�õ��� �ʴ� ��������
	// �������ϴ� �ֱ⸦ �������� ���� �� �ֽ��ϴ�.
	HRESULT hr = m_swapChain->Present(1, 0);

	// ������ ����ų� ����̹� ���׷��̵�� ���� ��ġ�� ���ŵǸ� 
	// ��� ��ġ ���ҽ��� �ٽ� ������ �մϴ�.
	if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET)
	{
		m_deviceRemoved = true;
	}
	else
	{
		DX::ThrowIfFailed(hr);

		MoveToNextFrame();
	}
}

// �Ͻ� �ߴ� ���� GPU �۾��� �Ϸ�� ������ ��ٸ��ϴ�.
void Device::WaitForGpu()
{
	// ť���� ��ȣ ����� �����մϴ�.
	DX::ThrowIfFailed(m_commandQueue->Signal(m_fence.Get(), m_fenceValues[m_currentFrame]));

	// fence�� ���� ������ ��ٸ��ϴ�.
	DX::ThrowIfFailed(m_fence->SetEventOnCompletion(m_fenceValues[m_currentFrame], m_fenceEvent));
	WaitForSingleObjectEx(m_fenceEvent, INFINITE, FALSE);

	// ���� �����ӿ� ���� fence ���� �����մϴ�.
	m_fenceValues[m_currentFrame]++;
}

// ���� �������� �������ϵ��� �غ��մϴ�.
void Device::MoveToNextFrame()
{
	// ť���� ��ȣ ����� �����մϴ�.
	const UINT64 currentFenceValue = m_fenceValues[m_currentFrame];
	DX::ThrowIfFailed(m_commandQueue->Signal(m_fence.Get(), currentFenceValue));

	//������ �ε����� �̵��մϴ�.
	m_currentFrame = m_swapChain->GetCurrentBackBufferIndex();

	// ���� �������� ������ �غ� �Ǿ����� Ȯ���ϼ���.
	if (m_fence->GetCompletedValue() < m_fenceValues[m_currentFrame])
	{
		DX::ThrowIfFailed(m_fence->SetEventOnCompletion(m_fenceValues[m_currentFrame], m_fenceEvent));
		WaitForSingleObjectEx(m_fenceEvent, INFINITE, FALSE);
	}

	// ���� �����ӿ� ���� fence ���� �����մϴ�.
	m_fenceValues[m_currentFrame] = currentFenceValue + 1;
}

// �� �޼���� ���÷��� ��ġ�� �⺻ ����� ���� ���÷��� ���� ���� ȸ����
// Ȯ���մϴ�.
DXGI_MODE_ROTATION Device::ComputeDisplayRotation()
{
	DXGI_MODE_ROTATION rotation = DXGI_MODE_ROTATION_UNSPECIFIED;

	// ����: NativeOrientation�� DisplayOrientations �������� �ٸ� ���� �����ϴ���
	// Landscape �Ǵ� Portrait�� �� �� �ֽ��ϴ�.
	switch (m_nativeOrientation)
	{
	case DisplayOrientations::Landscape:
		switch (m_currentOrientation)
		{
		case DisplayOrientations::Landscape:
			rotation = DXGI_MODE_ROTATION_IDENTITY;
			break;

		case DisplayOrientations::Portrait:
			rotation = DXGI_MODE_ROTATION_ROTATE270;
			break;

		case DisplayOrientations::LandscapeFlipped:
			rotation = DXGI_MODE_ROTATION_ROTATE180;
			break;

		case DisplayOrientations::PortraitFlipped:
			rotation = DXGI_MODE_ROTATION_ROTATE90;
			break;
		}
		break;

	case DisplayOrientations::Portrait:
		switch (m_currentOrientation)
		{
		case DisplayOrientations::Landscape:
			rotation = DXGI_MODE_ROTATION_ROTATE90;
			break;

		case DisplayOrientations::Portrait:
			rotation = DXGI_MODE_ROTATION_IDENTITY;
			break;

		case DisplayOrientations::LandscapeFlipped:
			rotation = DXGI_MODE_ROTATION_ROTATE270;
			break;

		case DisplayOrientations::PortraitFlipped:
			rotation = DXGI_MODE_ROTATION_ROTATE180;
			break;
		}
		break;
	}
	return rotation;
}

// �� �޼���� Direct3D 12�� �����ϴ� ��� ������ ù ��° �ϵ���� ����͸� �����ɴϴ�.
// �׷��� ����͸� ã�� �� ���� ��� *ppAdapter�� nullptr�� �����˴ϴ�.
void Device::GetHardwareAdapter(IDXGIAdapter1** ppAdapter)
{
	ComPtr<IDXGIAdapter1> adapter;
	*ppAdapter = nullptr;

	for (UINT adapterIndex = 0; DXGI_ERROR_NOT_FOUND != m_dxgiFactory->EnumAdapters1(adapterIndex, &adapter); adapterIndex++)
	{
		DXGI_ADAPTER_DESC1 desc;
		adapter->GetDesc1(&desc);

		if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
		{
			// �⺻ ������ ����̹� ����͸� �������� ������.
			continue;
		}

		// ����Ϳ��� Direct3D 12�� ���������� ���� ��ġ�� ���� ������ �ʾҴ���
		// Ȯ���մϴ�.
		if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr)))
		{
			break;
		}
	}

	*ppAdapter = adapter.Detach();
}
