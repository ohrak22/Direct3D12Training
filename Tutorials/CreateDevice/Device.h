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

		static const UINT								g_frameCount = 3;		// 3�� ���۸��� ����մϴ�.
		UINT											m_currentFrame;

		// Direct3D ��ü�Դϴ�.
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

		// CPU/GPU ����ȭ.
		ComPtr<ID3D12Fence>								m_fence;
		UINT64											m_fenceValues[g_frameCount];
		HANDLE											m_fenceEvent;

		// â�� ���� ĳ�õ� �����Դϴ�.
		Platform::Agile<Windows::UI::Core::CoreWindow>	m_window;

		// ĳ�õ� ��ġ �Ӽ��Դϴ�.
		Windows::Foundation::Size						m_outputSize;

	};

}
