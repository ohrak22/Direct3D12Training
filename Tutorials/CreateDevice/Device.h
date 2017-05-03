#pragma once

#include <d3d12.h>

using namespace Microsoft::WRL;
using namespace Windows::UI::Core;

namespace CreateDevice
{
	class Device
	{
	public:
		Device(DXGI_FORMAT backBufferFormat = DXGI_FORMAT_B8G8R8A8_UNORM, DXGI_FORMAT depthBufferFormat = DXGI_FORMAT_D32_FLOAT);
		void SetWindow(Windows::UI::Core::CoreWindow^ window);
		void Present();

	private:
		void CreateDeviceResources();
		void GetHardwareAdapter(IDXGIAdapter1** ppAdapter);
		void MoveToNextFrame();

		static const UINT								g_frameCount = 3;		// 3중 버퍼링을 사용합니다.
		UINT											m_currentFrame;

		// Direct3D 개체입니다.
		ComPtr<ID3D12Device>							m_d3dDevice;
		ComPtr<IDXGIFactory4>							m_dxgiFactory;
		ComPtr<IDXGISwapChain3>							m_swapChain;
		ComPtr<ID3D12Resource>							m_renderTargets[g_frameCount];
		ComPtr<ID3D12DescriptorHeap>					m_rtvHeap;
		ComPtr<ID3D12CommandQueue>						m_commandQueue;
		ComPtr<ID3D12CommandAllocator>					m_commandAllocators[g_frameCount];
		DXGI_FORMAT										m_backBufferFormat;
		D3D12_VIEWPORT									m_screenViewport;
		UINT											m_rtvDescriptorSize;
		bool											m_deviceRemoved;

		// CPU/GPU 동기화.
		ComPtr<ID3D12Fence>								m_fence;
		UINT64											m_fenceValues[g_frameCount];
		HANDLE											m_fenceEvent;

		// 창에 대한 캐시된 참조입니다.
		Platform::Agile<Windows::UI::Core::CoreWindow>	m_window;

		// 캐시된 장치 속성입니다.
		Windows::Foundation::Size						m_outputSize;

	};

}
