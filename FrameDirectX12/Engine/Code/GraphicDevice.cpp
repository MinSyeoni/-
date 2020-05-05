#include "GraphicDevice.h"
#pragma warning(disable:4996)

USING(Engine)
IMPLEMENT_SINGLETON(CGraphicDevice)


CGraphicDevice::CGraphicDevice()
{
	for (int i = 0; i < m_iSwapChainBufferCount; i++)
	{
		m_ppSwapChainBuffer[i] = nullptr;
	}

	ZeroMemory(&m_Viewport, sizeof(D3D12_VIEWPORT));
	ZeroMemory(&m_ScissorRect, sizeof(D3D12_RECT));

	ZeroMemory(&m_ResourceBarrier, sizeof(D3D12_RESOURCE_BARRIER));
	m_PresentParameters.DirtyRectsCount = 0;
	m_PresentParameters.pDirtyRects		= nullptr;
	m_PresentParameters.pScrollRect		= nullptr;
	m_PresentParameters.pScrollOffset	= nullptr;
}


CGraphicDevice::~CGraphicDevice()
{
}

HRESULT CGraphicDevice::Ready_GraphicDevice(HWND hWnd,
											HANDLE hHandle,
											WINMODE eMode, 
											const _uint & iWidth, 
											const _uint & iHeight)
{
	FAILED_CHECK_RETURN(Create_GraphicDevice(iWidth, iHeight), E_FAIL);
	FAILED_CHECK_RETURN(Create_CommandQueueAndList(), E_FAIL);
	FAILED_CHECK_RETURN(Create_SwapChain(hWnd, eMode, iWidth, iHeight), E_FAIL);
	FAILED_CHECK_RETURN(Create_RtvAndDsvDescriptorHeaps(), E_FAIL);
	FAILED_CHECK_RETURN(OnResize(iWidth, iHeight), E_FAIL);
	FAILED_CHECK_RETURN(Create_RootSig(),E_FAIL);
	FAILED_CHECK_RETURN(Create_11On12GraphicDevice(), E_FAIL);

	return S_OK;
}

HRESULT CGraphicDevice::Render_Begin(const _rgba& vRGBA)
{
	/*____________________________________________________________________
	- ��� �Ҵ��ڿ� ��� ����Ʈ �缳��.
	- �缳���� GPU�� ���� ��� ��ϵ��� ��� ó���� �Ŀ� �Ͼ��.
	______________________________________________________________________*/
	FAILED_CHECK_RETURN(m_pCommandAllocator->Reset(), E_FAIL);
	FAILED_CHECK_RETURN(m_pCommandList->Reset(m_pCommandAllocator, m_pPipelineState), E_FAIL);

	/*____________________________________________________________________
	- ����Ʈ�� ���� �簢���� �����Ѵ�. 
	- ��� ���(CommandList)�� �缳���� ������ �̵鵵 �缳���ؾ� �Ѵ�.
	______________________________________________________________________*/
	m_pCommandList->RSSetViewports(1, &m_Viewport);
	m_pCommandList->RSSetScissorRects(1, &m_ScissorRect);


	/*____________________________________________________________________
	Indicate a state transition on the resource usage.
	- �ڿ� �뵵�� ���õ� �������̸� Direct3D�� �����Ѵ�.
	- ���� ���� Ÿ�ٿ� ���� ������Ʈ�� �����⸦ ��ٸ���. 
	- ������Ʈ�� ������ ���� Ÿ�� ������ ���´� 
	  ������Ʈ ����(D3D12_RESOURCE_STATE_PRESENT)���� ���� Ÿ�� ����(D3D12_RESOURCE_STATE_RENDER_TARGET)�� �ٲ� ���̴�.
	______________________________________________________________________*/
	m_pCommandList->ResourceBarrier(1,
									&CD3DX12_RESOURCE_BARRIER::Transition(m_ppSwapChainBuffer[m_iCurrBackBuffer],
									D3D12_RESOURCE_STATE_PRESENT, 
									D3D12_RESOURCE_STATE_RENDER_TARGET));

	/*____________________________________________________________________
	 Clear the back buffer and depth buffer.
	- ������ ���� Ÿ�ٿ� �ش��ϴ� �������� CPU �ּ�(�ڵ�)�� ���.
	- ���ϴ� �������� ���� Ÿ��(��)�� �����.
	______________________________________________________________________*/
	_float arrClearColor[4] = { vRGBA.x, vRGBA.y, vRGBA.z, vRGBA.w };
	m_pCommandList->ClearRenderTargetView(CD3DX12_CPU_DESCRIPTOR_HANDLE(m_pRTV_Heap->GetCPUDescriptorHandleForHeapStart(),
																		m_iCurrBackBuffer,
																		m_uiRTV_DescriptorSize),
										  arrClearColor,
										  0,
										  nullptr);

	/*____________________________________________________________________
	- ����-���ٽ� �������� CPU �ּҸ� ���.
	- ���ϴ� ������ ����-���ٽ�(��)�� �����.
	______________________________________________________________________*/
	m_pCommandList->ClearDepthStencilView(m_pDSV_Heap->GetCPUDescriptorHandleForHeapStart(),
										  D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 
										  1.0f,
										  0,
										  0, 
										  nullptr);

	/*____________________________________________________________________
	Specify the buffers we are going to render to.
	- ������ ����� ��ϵ� ���� Ÿ�� ���۵��� �����Ѵ�.
	- RTV(������)�� DSV(������)�� ���-���� �ܰ�(OM)�� �����Ѵ�.
	- OMSetRenderTargets : �������� ����� ���� Ÿ�ٰ� ����,���ٽ� ���۸� ���������ο� ���´�.
	______________________________________________________________________*/
	m_pCommandList->OMSetRenderTargets(1, 
									   &CD3DX12_CPU_DESCRIPTOR_HANDLE(m_pRTV_Heap->GetCPUDescriptorHandleForHeapStart(),
																	  m_iCurrBackBuffer,
																	  m_uiRTV_DescriptorSize), 
									   true,
									   &m_pDSV_Heap->GetCPUDescriptorHandleForHeapStart());


	/* ---------------- ������ �ڵ�� ���⿡ �߰��� ���̴�. ---------------- */


	return S_OK;
}
HRESULT CGraphicDevice::Render_TextBegin()
{
	/*__________________________________________________________________________________________________________
	- ���� ��� D3D12 ��� ����� �����Ͽ� 3D ����� �������� ����, �� ���� UI�� �������Ѵ�.
	�ٸ� ���ÿ����� ��� ����� �ݱ� ���� �Ϲ������� ���ҽ� �庮�� �����Ͽ� ������ ��� ���¿��� ���� ���·� �� ���۸� ��ȯ�մϴ�.
	�׷��� �� ���ÿ����� D2D�� ����Ͽ� �� ���۷� ��� �������ؾ� �ϹǷ� �ش� ���ҽ� �庮�� �����մϴ�.

	- �� ������ ���ε� ���ҽ��� ���� �� ������ ��� ���¸� "IN" ���·� �����ϰ�, ���� ���¸� "OUT" ���·� ����.
	(m_pWrappedBackBuffers�� PRESENT ���·� �����Ͽ� ResourceBarrier - PRESENT ��ü.)
	____________________________________________________________________________________________________________*/
	m_p11On12Device->AcquireWrappedResources(&m_pWrappedBackBuffers[m_iCurrBackBuffer], 1);

	m_pD2D_Context->SetTarget(m_pD2DRenderTargets[m_iCurrBackBuffer]);
	m_pD2D_Context->BeginDraw();
	m_pD2D_Context->SetTransform(D2D1::Matrix3x2F::Identity());

	return S_OK;
}

HRESULT CGraphicDevice::Render_TextEnd()
{
	/*__________________________________________________________________________________________________________
	- ����������, 11On12 ����̽����� ����� ��� ����� ���� ID3D12CommandQueue�� �����ϱ� ����
	ID3D11DeviceContext���� Flush�� ȣ���ؾ� �մϴ�.
	____________________________________________________________________________________________________________*/
	m_pD2D_Context->EndDraw();
	m_pD2D_Context->SetTarget(nullptr);

	m_p11On12Device->ReleaseWrappedResources(&m_pWrappedBackBuffers[m_iCurrBackBuffer], 1);

	m_p11Context->Flush();

	return S_OK;
}
HRESULT CGraphicDevice::Render_End()
{
	/*____________________________________________________________________
	 Indicate a state transition on the resource usage.
	- �ڿ� �뵵�� ���õ� ���� ���̸� Direct3D�� �����Ѵ�.
	- ���� ���� Ÿ�ٿ� ���� �������� �����⸦ ��ٸ���. 
	- GPU�� ���� Ÿ��(����)�� �� �̻� ������� ������ ���� Ÿ���� ���´� 
	  ������Ʈ ����(D3D12_RESOURCE_STATE_PRESENT)�� �ٲ� ���̴�.
	______________________________________________________________________*/
	/*__________________________________________________________________________________________________________
	- ����ü���� ������Ʈ�Ѵ�. (�ĸ� ���ۿ� ���� ���۸� ��ȯ)
	- ������Ʈ�� �ϸ� ���� ���� Ÿ��(�ĸ����)�� ������
	  ������۷� �Ű�����, ���� Ÿ�� �ε����� �ٲ� ���̴�.
	____________________________________________________________________________________________________________*/
	FAILED_CHECK_RETURN(m_pSwapChain->Present(0, 0), E_FAIL);

	m_iCurrBackBuffer = (m_iCurrBackBuffer + 1) % m_iSwapChainBufferCount;

	/*__________________________________________________________________________________________________________
	- GPU�� ��� ��� ����Ʈ�� ������ �� ���� ��ٸ���.
	____________________________________________________________________________________________________________*/
	FAILED_CHECK_RETURN(Wait_ForGpuComplete(), E_FAIL);
	return S_OK;
}

CD3DX12_CPU_DESCRIPTOR_HANDLE CGraphicDevice::Get_DepthBuffer_handle()
{
	return CD3DX12_CPU_DESCRIPTOR_HANDLE(m_pDSV_Heap->GetCPUDescriptorHandleForHeapStart());
}

HRESULT CGraphicDevice::BackBufferSettingBegin()
{

	m_pCommandList->ResourceBarrier(1,
		&CD3DX12_RESOURCE_BARRIER::Transition(m_ppSwapChainBuffer[m_iCurrBackBuffer],
			D3D12_RESOURCE_STATE_RENDER_TARGET,
			D3D12_RESOURCE_STATE_PRESENT));

	return S_OK;
}

HRESULT CGraphicDevice::BackBufferSettingEnd()
{

	m_pCommandList->ResourceBarrier(1,
		&CD3DX12_RESOURCE_BARRIER::Transition(m_ppSwapChainBuffer[m_iCurrBackBuffer],
			D3D12_RESOURCE_STATE_PRESENT,
			D3D12_RESOURCE_STATE_RENDER_TARGET));

	m_pCommandList->OMSetRenderTargets(1,
		&CD3DX12_CPU_DESCRIPTOR_HANDLE(m_pRTV_Heap->GetCPUDescriptorHandleForHeapStart(),
			m_iCurrBackBuffer,
			m_uiRTV_DescriptorSize),
		true,
		&m_pDSV_Heap->GetCPUDescriptorHandleForHeapStart());




	return S_OK;
}

HRESULT CGraphicDevice::Create_GraphicDevice(const _uint & iWidth, const _uint & iHeight)
{
#if defined(_DEBUG)
	ID3D12Debug* pDebugController = nullptr;
	D3D12GetDebugInterface(__uuidof(ID3D12Debug), (void**)&pDebugController);
	pDebugController->EnableDebugLayer();
#endif

	/*____________________________________________________________________
	DXGI ���丮�� �����Ѵ�.
	______________________________________________________________________*/
	// FAILED_CHECK_RETURN(CreateDXGIFactory1(IID_PPV_ARGS(&m_pFactory)), E_FAIL);
	ThrowIfFailed(CreateDXGIFactory1(IID_PPV_ARGS(&m_pFactory)));

	/*______________	IDXGIAdapter1 *pd3dAdapter = NULL;

	for (UINT i = 0; DXGI_ERROR_NOT_FOUND != m_pdxgiFactory->EnumAdapters1(i, &pd3dAdapter); i++)
	{
		DXGI_ADAPTER_DESC1 dxgiAdapterDesc;
		pd3dAdapter->GetDesc1(&dxgiAdapterDesc);
		if (dxgiAdapterDesc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) continue;
		if (SUCCEEDED(D3D12CreateDevice(pd3dAdapter, D3D_FEATURE_LEVEL_12_0, _uuidof(ID3D12Device), (void **)&m_pd3dDevice))) break;
	}______________________________________________________
	 Try to create hardware device.
	______________________________________________________________________*/
	IDXGIAdapter1 *pd3dAdapter = nullptr;

	//�ÿ��� ����߿���
	m_pFactory->EnumAdapters1(1, &pd3dAdapter);
	HRESULT hResult = D3D12CreateDevice(pd3dAdapter,             // default adapter
										D3D_FEATURE_LEVEL_12_0,
										IID_PPV_ARGS(&m_pGraphicDevice));

	/*____________________________________________________________________
	 Fallback to WARP device.
	Ư�� ���� 12.0�� �����ϴ� �ϵ���� ����̽��� ������ �� ������ WARP ����̽��� ����.
	______________________________________________________________________*/
	if(FAILED(hResult))
	{
		IDXGIAdapter* pWarpAdapter;
		ThrowIfFailed(m_pFactory->EnumWarpAdapter(IID_PPV_ARGS(&pWarpAdapter)));

		ThrowIfFailed(D3D12CreateDevice(pWarpAdapter,
										D3D_FEATURE_LEVEL_11_0,
										IID_PPV_ARGS(&m_pGraphicDevice)));
	}


	/*____________________________________________________________________
	�潺�� ����ȭ�� ���� �̺�Ʈ ��ü�� �����Ѵ�. (�̺�Ʈ ��ü�� �ʱⰪ�� FALSE�̴�).
	�̺�Ʈ�� ����Ǹ� (Signal)�̺�Ʈ�� ���� �ڵ������� FALSE�� �ǵ��� �����Ѵ�.
	______________________________________________________________________*/
	ThrowIfFailed(m_pGraphicDevice->CreateFence(0,
												D3D12_FENCE_FLAG_NONE,
												IID_PPV_ARGS(&m_pFence)));

	m_uiRTV_DescriptorSize			= m_pGraphicDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	m_uiDSV_DescriptorSize			= m_pGraphicDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
	m_uiCBV_SRV_UAV_DescriptorSize	= m_pGraphicDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);


	/*____________________________________________________________________
	4X MSAA ǰ�� ���� ���� ����.
	______________________________________________________________________*/
	D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS MSAA_QualityLevels;
	ZeroMemory(&MSAA_QualityLevels, sizeof(D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS));

	MSAA_QualityLevels.Format			= DXGI_FORMAT_R8G8B8A8_UNORM;
	MSAA_QualityLevels.SampleCount		= 4;	//Msaa4x ���� ���ø�
	MSAA_QualityLevels.Flags			= D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE;
	MSAA_QualityLevels.NumQualityLevels = 0;

	// ����̽��� �����ϴ� ���� ������ ǰ�� ������ Ȯ��.
	FAILED_CHECK_RETURN(m_pGraphicDevice->CheckFeatureSupport(D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS,		// ���� ǥ��ȭ ���.
										  &MSAA_QualityLevels,												// ��� ���� ������ ������ ����ü ������.
										  sizeof(D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS)), E_FAIL);	// ����ü�� ũ��.

	m_uiMSAA4X_QualityLevels = MSAA_QualityLevels.NumQualityLevels;

	//���� ������ ǰ�� ������ 1���� ũ�� ���� ���ø��� Ȱ��ȭ.
	m_bIsMSAA4X_Enable = (m_uiMSAA4X_QualityLevels > 1) ? true : false;


#if defined(_DEBUG)
	Log_Adapters();
	Engine::Safe_Release(pDebugController);
#endif

	return S_OK;
}

HRESULT CGraphicDevice::Create_CommandQueueAndList()
{
	D3D12_COMMAND_QUEUE_DESC QueueDesc = {};
	QueueDesc.Type	= D3D12_COMMAND_LIST_TYPE_DIRECT;
	QueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	ThrowIfFailed(m_pGraphicDevice->CreateCommandQueue(&QueueDesc, IID_PPV_ARGS(&m_pCommandQueue)));

	ThrowIfFailed(m_pGraphicDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_pCommandAllocator)));

	ThrowIfFailed(m_pGraphicDevice->CreateCommandList(0,
													  D3D12_COMMAND_LIST_TYPE_DIRECT,
													  m_pCommandAllocator,	// Associated command allocator
													  nullptr,				// Initial PipelineStateObject
													  IID_PPV_ARGS(&m_pCommandList)));

	// Start off in a closed state.  This is because the first time we refer 
	// to the command list we will Reset it, and it needs to be closed before
	// calling Reset.
	m_pCommandList->Close();


	ThrowIfFailed(m_pGraphicDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_pCommandAllocatorThread)));

	ThrowIfFailed(m_pGraphicDevice->CreateCommandList(0,
		D3D12_COMMAND_LIST_TYPE_DIRECT,
		m_pCommandAllocatorThread,	// Associated command allocator
		nullptr,				// Initial PipelineStateObject
		IID_PPV_ARGS(&m_pCommandListThread)));

	m_pCommandListThread->Close();

	return S_OK;
}

HRESULT CGraphicDevice::Create_SwapChain(HWND hWnd, WINMODE eMode, const _uint& iWidth, const _uint& iHeight)
{
	// Release the previous swapchain we will be recreating.
    DXGI_SWAP_CHAIN_DESC SwapChainDesc;
    SwapChainDesc.BufferDesc.Width						= iWidth;
    SwapChainDesc.BufferDesc.Height						= iHeight;
    SwapChainDesc.BufferDesc.RefreshRate.Numerator		= 60;
    SwapChainDesc.BufferDesc.RefreshRate.Denominator	= 1;
    SwapChainDesc.BufferDesc.Format						= DXGI_FORMAT_R8G8B8A8_UNORM;
    SwapChainDesc.BufferDesc.ScanlineOrdering			= DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
    SwapChainDesc.BufferDesc.Scaling					= DXGI_MODE_SCALING_UNSPECIFIED;
    SwapChainDesc.SampleDesc.Count						= m_bIsMSAA4X_Enable ? 4 : 1;
    SwapChainDesc.SampleDesc.Quality					= m_bIsMSAA4X_Enable ? (m_uiMSAA4X_QualityLevels - 1) : 0;
    SwapChainDesc.BufferUsage							= DXGI_USAGE_RENDER_TARGET_OUTPUT;
    SwapChainDesc.BufferCount							= m_iSwapChainBufferCount;
    SwapChainDesc.OutputWindow							= hWnd;
    SwapChainDesc.Windowed								= eMode;
	SwapChainDesc.SwapEffect							= DXGI_SWAP_EFFECT_FLIP_DISCARD;
    SwapChainDesc.Flags									= DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	// Note: Swap chain uses queue to perform flush.
    ThrowIfFailed(m_pFactory->CreateSwapChain(m_pCommandQueue,
											  &SwapChainDesc, 
											  &m_pSwapChain));

	return S_OK;
}

HRESULT CGraphicDevice::Create_RtvAndDsvDescriptorHeaps()
{
	D3D12_DESCRIPTOR_HEAP_DESC RTV_HeapDesc;
	RTV_HeapDesc.NumDescriptors	= m_iSwapChainBufferCount;
	RTV_HeapDesc.Type			= D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	RTV_HeapDesc.Flags			= D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	RTV_HeapDesc.NodeMask		= 0;

	ThrowIfFailed(m_pGraphicDevice->CreateDescriptorHeap(&RTV_HeapDesc, IID_PPV_ARGS(&m_pRTV_Heap)));


	D3D12_DESCRIPTOR_HEAP_DESC DSV_HeapDesc;
	DSV_HeapDesc.NumDescriptors	= 1;
	DSV_HeapDesc.Type			= D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	DSV_HeapDesc.Flags			= D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	DSV_HeapDesc.NodeMask		= 0;
	ThrowIfFailed(m_pGraphicDevice->CreateDescriptorHeap(&DSV_HeapDesc, IID_PPV_ARGS(&m_pDSV_Heap)));

	return S_OK;
}

HRESULT CGraphicDevice::Create_RootSig()
{
	Create_ColorObjectRoot();
	Create_TextureRoot();
	Create_MeshRoot();
	Create_LightRoot();
	Create_BlendRoot();
	Create_ShadowDepthRoot();
	Create_TerrainRoot();
	Create_DownSampleRoot();
	Create_BlurRoot();
	Create_DistortRoot();
	Create_SSAORoot();

	return S_OK;
}

HRESULT CGraphicDevice::Create_ColorObjectRoot()
{

	CD3DX12_ROOT_PARAMETER slotRootParameter[1];

	slotRootParameter[0].InitAsConstantBufferView(0);

	auto staticSamplers = GetStaticSamplers();

	CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(1, slotRootParameter,
		(UINT)staticSamplers.size(), staticSamplers.data(),
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	// create a root signature with a single slot which points to a descriptor range consisting of a single constant buffer
	ComPtr<ID3DBlob> serializedRootSig = nullptr;
	ComPtr<ID3DBlob> errorBlob = nullptr;
	HRESULT hr = D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1,
		serializedRootSig.GetAddressOf(), errorBlob.GetAddressOf());

	if (errorBlob != nullptr)
	{
		::OutputDebugStringA((char*)errorBlob->GetBufferPointer());
	}
	ThrowIfFailed(hr);

	ThrowIfFailed(m_pGraphicDevice->CreateRootSignature(
		0,
		serializedRootSig->GetBufferPointer(),
		serializedRootSig->GetBufferSize(),
		IID_PPV_ARGS(&m_arrSig[(UINT)ROOT_SIG_TYPE::INPUT_OBJECT])));

	return S_OK;

}

HRESULT CGraphicDevice::Create_TextureRoot()
{
	CD3DX12_DESCRIPTOR_RANGE texTable;
	texTable.Init(
		D3D12_DESCRIPTOR_RANGE_TYPE_SRV,
		1,  // number of descriptors
		0); // register t0

	// Root parameter can be a table, root descriptor or root constants.
	CD3DX12_ROOT_PARAMETER slotRootParameter[2];

	slotRootParameter[0].InitAsDescriptorTable(1, &texTable, D3D12_SHADER_VISIBILITY_PIXEL);
	slotRootParameter[1].InitAsConstantBufferView(0);

	auto staticSamplers = GetStaticSamplers();

	CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(2, slotRootParameter,
		(UINT)staticSamplers.size(), staticSamplers.data(),
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	// create a root signature with a single slot which points to a descriptor range consisting of a single constant buffer
	ComPtr<ID3DBlob> serializedRootSig = nullptr;
	ComPtr<ID3DBlob> errorBlob = nullptr;
	HRESULT hr = D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1,
		serializedRootSig.GetAddressOf(), errorBlob.GetAddressOf());

	if (errorBlob != nullptr)
	{
		::OutputDebugStringA((char*)errorBlob->GetBufferPointer());
	}
	ThrowIfFailed(hr);

	ThrowIfFailed(m_pGraphicDevice->CreateRootSignature(
		0,
		serializedRootSig->GetBufferPointer(),
		serializedRootSig->GetBufferSize(),
		IID_PPV_ARGS(&m_arrSig[(UINT)ROOT_SIG_TYPE::INPUT_TEXTURE])));

	return S_OK;
}

HRESULT CGraphicDevice::Create_MeshRoot()
{
	CD3DX12_DESCRIPTOR_RANGE texTable[4];
	texTable[0].Init(
		D3D12_DESCRIPTOR_RANGE_TYPE_SRV,
		1,  // number of descriptors
		0); // register t0
	texTable[1].Init(
		D3D12_DESCRIPTOR_RANGE_TYPE_SRV,
		1,  // number of descriptors
		1); // register t1
	texTable[2].Init(
		D3D12_DESCRIPTOR_RANGE_TYPE_SRV,
		1,  // number of descriptors
		2); // register t2
	texTable[3].Init(
		D3D12_DESCRIPTOR_RANGE_TYPE_SRV,
		1,  // number of descriptors
		3); // register t3
	// Root parameter can be a table, root descriptor or root constants.
	CD3DX12_ROOT_PARAMETER slotRootParameter[7];

	slotRootParameter[0].InitAsDescriptorTable(1, &texTable[0], D3D12_SHADER_VISIBILITY_PIXEL);
	slotRootParameter[1].InitAsDescriptorTable(1, &texTable[1], D3D12_SHADER_VISIBILITY_PIXEL);
	slotRootParameter[2].InitAsDescriptorTable(1, &texTable[2], D3D12_SHADER_VISIBILITY_PIXEL);
	slotRootParameter[3].InitAsDescriptorTable(1, &texTable[3], D3D12_SHADER_VISIBILITY_PIXEL);
	slotRootParameter[4].InitAsConstantBufferView(0);
	slotRootParameter[5].InitAsConstantBufferView(1);
	slotRootParameter[6].InitAsConstantBufferView(2);

	auto staticSamplers = GetStaticSamplers();

	CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(7, slotRootParameter,
		(UINT)staticSamplers.size(), staticSamplers.data(),
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	// create a root signature with a single slot which points to a descriptor range consisting of a single constant buffer
	ComPtr<ID3DBlob> serializedRootSig = nullptr;
 	ComPtr<ID3DBlob> errorBlob = nullptr;
	HRESULT hr = D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1,
		serializedRootSig.GetAddressOf(), errorBlob.GetAddressOf());

	if (errorBlob != nullptr)
	{
		::OutputDebugStringA((char*)errorBlob->GetBufferPointer());
	}
	ThrowIfFailed(hr);

	ThrowIfFailed(m_pGraphicDevice->CreateRootSignature(
		0,
		serializedRootSig->GetBufferPointer(),
		serializedRootSig->GetBufferSize(),
		IID_PPV_ARGS(&m_arrSig[(UINT)ROOT_SIG_TYPE::INPUT_MESH])));

	return S_OK;
}

HRESULT CGraphicDevice::Create_LightRoot()
{
	CD3DX12_DESCRIPTOR_RANGE texTable[3];
	texTable[0].Init(
		D3D12_DESCRIPTOR_RANGE_TYPE_SRV,
		1,  // number of descriptors
		0); // register t0

	texTable[1].Init(
		D3D12_DESCRIPTOR_RANGE_TYPE_SRV,
		1,  // number of descriptors
		1); // register t0

	texTable[2].Init(
		D3D12_DESCRIPTOR_RANGE_TYPE_SRV,
		1,  // number of descriptors
		2); // register t0

	// Root parameter can be a table, root descriptor or root constants.
	CD3DX12_ROOT_PARAMETER slotRootParameter[5];

	slotRootParameter[0].InitAsDescriptorTable(1, &texTable[0], D3D12_SHADER_VISIBILITY_PIXEL);
	slotRootParameter[1].InitAsDescriptorTable(1, &texTable[1], D3D12_SHADER_VISIBILITY_PIXEL);
	slotRootParameter[2].InitAsDescriptorTable(1, &texTable[2], D3D12_SHADER_VISIBILITY_PIXEL);
	slotRootParameter[3].InitAsConstantBufferView(0);
	slotRootParameter[4].InitAsConstantBufferView(1);
	auto staticSamplers = GetStaticSamplers();

	CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(5, slotRootParameter,
		(UINT)staticSamplers.size(), staticSamplers.data(),
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	// create a root signature with a single slot which points to a descriptor range consisting of a single constant buffer
	ComPtr<ID3DBlob> serializedRootSig = nullptr;
	ComPtr<ID3DBlob> errorBlob = nullptr;
	HRESULT hr = D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1,
		serializedRootSig.GetAddressOf(), errorBlob.GetAddressOf());

	if (errorBlob != nullptr)
	{
		::OutputDebugStringA((char*)errorBlob->GetBufferPointer());
	}
	ThrowIfFailed(hr);

	ThrowIfFailed(m_pGraphicDevice->CreateRootSignature(
		0,
		serializedRootSig->GetBufferPointer(),
		serializedRootSig->GetBufferSize(),
		IID_PPV_ARGS(&m_arrSig[(UINT)ROOT_SIG_TYPE::INPUT_LIGHT])));

	return S_OK;
}

HRESULT CGraphicDevice::Create_BlendRoot()
{
	CD3DX12_DESCRIPTOR_RANGE texTable[7];

	texTable[0].Init(
		D3D12_DESCRIPTOR_RANGE_TYPE_SRV,
		1,  // number of descriptors
		0); // register t0

	texTable[1].Init(
		D3D12_DESCRIPTOR_RANGE_TYPE_SRV,
		1,  // number of descriptors
		1); // register t0

	texTable[2].Init(
		D3D12_DESCRIPTOR_RANGE_TYPE_SRV,
		1,  // number of descriptors
		2); // register t0

	texTable[3].Init(
		D3D12_DESCRIPTOR_RANGE_TYPE_SRV,
		1,  // number of descriptors
		3); // register t0
	texTable[4].Init(
		D3D12_DESCRIPTOR_RANGE_TYPE_SRV,
		1,  // number of descriptors
		4); // register t0
	texTable[5].Init(
		D3D12_DESCRIPTOR_RANGE_TYPE_SRV,
		1,  // number of descriptors
		5); // register t0
	// Root parameter can be a table, root descriptor or root constants.
	texTable[6].Init(
		D3D12_DESCRIPTOR_RANGE_TYPE_SRV,
		1,  // number of descriptors
		6); // register t0
	// Root parameter can be a table, root descriptor or root constants.
	CD3DX12_ROOT_PARAMETER slotRootParameter[7];

	slotRootParameter[0].InitAsDescriptorTable(1, &texTable[0], D3D12_SHADER_VISIBILITY_PIXEL);
	slotRootParameter[1].InitAsDescriptorTable(1, &texTable[1], D3D12_SHADER_VISIBILITY_PIXEL);
	slotRootParameter[2].InitAsDescriptorTable(1, &texTable[2], D3D12_SHADER_VISIBILITY_PIXEL);
	slotRootParameter[3].InitAsDescriptorTable(1, &texTable[3], D3D12_SHADER_VISIBILITY_PIXEL);
	slotRootParameter[4].InitAsDescriptorTable(1, &texTable[4], D3D12_SHADER_VISIBILITY_PIXEL);
	slotRootParameter[5].InitAsDescriptorTable(1, &texTable[5], D3D12_SHADER_VISIBILITY_PIXEL);
	slotRootParameter[6].InitAsDescriptorTable(1, &texTable[6], D3D12_SHADER_VISIBILITY_PIXEL);

	auto staticSamplers = GetStaticSamplers();

	CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(7, slotRootParameter,
		(UINT)staticSamplers.size(), staticSamplers.data(),
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	// create a root signature with a single slot which points to a descriptor range consisting of a single constant buffer
	ComPtr<ID3DBlob> serializedRootSig = nullptr;
	ComPtr<ID3DBlob> errorBlob = nullptr;
	HRESULT hr = D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1,
		serializedRootSig.GetAddressOf(), errorBlob.GetAddressOf());

	if (errorBlob != nullptr)
	{
		::OutputDebugStringA((char*)errorBlob->GetBufferPointer());
	}
	ThrowIfFailed(hr);

	ThrowIfFailed(m_pGraphicDevice->CreateRootSignature(
		0,
		serializedRootSig->GetBufferPointer(),
		serializedRootSig->GetBufferSize(),
		IID_PPV_ARGS(&m_arrSig[(UINT)ROOT_SIG_TYPE::INPUT_BLEND])));
	return S_OK;
}

HRESULT CGraphicDevice::Create_TerrainRoot()
{
	CD3DX12_DESCRIPTOR_RANGE texTable[3];
	texTable[0].Init(
		D3D12_DESCRIPTOR_RANGE_TYPE_SRV,
		1,  // number of descriptors
		0); // register t0
	texTable[1].Init(
		D3D12_DESCRIPTOR_RANGE_TYPE_SRV,
		1,  // number of descriptors
		1); // register t1
	texTable[2].Init(
		D3D12_DESCRIPTOR_RANGE_TYPE_SRV,
		1,  // number of descriptors
	    2); // register t2
	// Root parameter can be a table, root descriptor or root constants.
	CD3DX12_ROOT_PARAMETER slotRootParameter[5];

	slotRootParameter[0].InitAsDescriptorTable(1, &texTable[0], D3D12_SHADER_VISIBILITY_PIXEL);
	slotRootParameter[1].InitAsDescriptorTable(1, &texTable[1], D3D12_SHADER_VISIBILITY_PIXEL);
	slotRootParameter[2].InitAsDescriptorTable(1, &texTable[2], D3D12_SHADER_VISIBILITY_PIXEL);
	slotRootParameter[3].InitAsConstantBufferView(0);
	slotRootParameter[4].InitAsConstantBufferView(1);

	auto staticSamplers = GetStaticSamplers();

	CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(5, slotRootParameter,
		(UINT)staticSamplers.size(), staticSamplers.data(),
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	// create a root signature with a single slot which points to a descriptor range consisting of a single constant buffer
	ComPtr<ID3DBlob> serializedRootSig = nullptr;
	ComPtr<ID3DBlob> errorBlob = nullptr;
	HRESULT hr = D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1,
		serializedRootSig.GetAddressOf(), errorBlob.GetAddressOf());

	if (errorBlob != nullptr)
	{
		::OutputDebugStringA((char*)errorBlob->GetBufferPointer());
	}
	ThrowIfFailed(hr);

	ThrowIfFailed(m_pGraphicDevice->CreateRootSignature(
		0,
		serializedRootSig->GetBufferPointer(),
		serializedRootSig->GetBufferSize(),
		IID_PPV_ARGS(&m_arrSig[(UINT)ROOT_SIG_TYPE::INPUT_TERRAIN])));

	return S_OK;
}

HRESULT CGraphicDevice::Create_ShadowDepthRoot()
{
	
	// Root parameter can be a table, root descriptor or root constants.
	CD3DX12_ROOT_PARAMETER slotRootParameter[2];

	slotRootParameter[0].InitAsConstantBufferView(0);
	slotRootParameter[1].InitAsConstantBufferView(1);


	CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(2, slotRootParameter,
		0, nullptr,
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	// create a root signature with a single slot which points to a descriptor range consisting of a single constant buffer
	ComPtr<ID3DBlob> serializedRootSig = nullptr;
	ComPtr<ID3DBlob> errorBlob = nullptr;
	HRESULT hr = D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1,
		serializedRootSig.GetAddressOf(), errorBlob.GetAddressOf());

	if (errorBlob != nullptr)
	{
		::OutputDebugStringA((char*)errorBlob->GetBufferPointer());
	}
	ThrowIfFailed(hr);

	ThrowIfFailed(m_pGraphicDevice->CreateRootSignature(
		0,
		serializedRootSig->GetBufferPointer(),
		serializedRootSig->GetBufferSize(),
		IID_PPV_ARGS(&m_arrSig[(UINT)ROOT_SIG_TYPE::INPUT_SHADOWDEPTH])));

	return S_OK;
}

HRESULT CGraphicDevice::Create_DownSampleRoot()
{
	CD3DX12_DESCRIPTOR_RANGE texTable[2];
	texTable[0].Init(
		D3D12_DESCRIPTOR_RANGE_TYPE_SRV,
		1,  // number of descriptors
		0); // register t0
	texTable[1].Init(
		D3D12_DESCRIPTOR_RANGE_TYPE_SRV,
		1,  // number of descriptors
		1); // register t0

	// Root parameter can be a table, root descriptor or root constants.
	CD3DX12_ROOT_PARAMETER slotRootParameter[3];

	slotRootParameter[0].InitAsDescriptorTable(1, &texTable[0], D3D12_SHADER_VISIBILITY_PIXEL);
	slotRootParameter[1].InitAsDescriptorTable(1, &texTable[1], D3D12_SHADER_VISIBILITY_PIXEL);
	slotRootParameter[2].InitAsConstantBufferView(0);

	auto staticSamplers = GetStaticSamplers();

	CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(3, slotRootParameter,
		(UINT)staticSamplers.size(), staticSamplers.data(),
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	// create a root signature with a single slot which points to a descriptor range consisting of a single constant buffer
	ComPtr<ID3DBlob> serializedRootSig = nullptr;
	ComPtr<ID3DBlob> errorBlob = nullptr;
	HRESULT hr = D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1,
		serializedRootSig.GetAddressOf(), errorBlob.GetAddressOf());

	if (errorBlob != nullptr)
	{
		::OutputDebugStringA((char*)errorBlob->GetBufferPointer());
	}
	ThrowIfFailed(hr);

	ThrowIfFailed(m_pGraphicDevice->CreateRootSignature(
		0,
		serializedRootSig->GetBufferPointer(),
		serializedRootSig->GetBufferSize(),
		IID_PPV_ARGS(&m_arrSig[(UINT)ROOT_SIG_TYPE::INPUT_DOWNSAMPLE])));

	return S_OK;
}

HRESULT CGraphicDevice::Create_BlurRoot()
{
	CD3DX12_DESCRIPTOR_RANGE texTable[1];
	texTable[0].Init(
		D3D12_DESCRIPTOR_RANGE_TYPE_SRV,
		1,  // number of descriptors
		0); // register t0

	// Root parameter can be a table, root descriptor or root constants.
	CD3DX12_ROOT_PARAMETER slotRootParameter[3];

	slotRootParameter[0].InitAsDescriptorTable(1, &texTable[0], D3D12_SHADER_VISIBILITY_PIXEL);
	slotRootParameter[1].InitAsConstantBufferView(0);
	slotRootParameter[2].InitAsConstantBufferView(1);

	auto staticSamplers = GetStaticSamplers();

	CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(3, slotRootParameter,
		(UINT)staticSamplers.size(), staticSamplers.data(),
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	// create a root signature with a single slot which points to a descriptor range consisting of a single constant buffer
	ComPtr<ID3DBlob> serializedRootSig = nullptr;
	ComPtr<ID3DBlob> errorBlob = nullptr;
	HRESULT hr = D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1,
		serializedRootSig.GetAddressOf(), errorBlob.GetAddressOf());

	if (errorBlob != nullptr)
	{
		::OutputDebugStringA((char*)errorBlob->GetBufferPointer());
	}
	ThrowIfFailed(hr);

	ThrowIfFailed(m_pGraphicDevice->CreateRootSignature(
		0,
		serializedRootSig->GetBufferPointer(),
		serializedRootSig->GetBufferSize(),
		IID_PPV_ARGS(&m_arrSig[(UINT)ROOT_SIG_TYPE::INPUT_BLUR])));

	return S_OK;
}

HRESULT CGraphicDevice::Create_DistortRoot()
{
	CD3DX12_DESCRIPTOR_RANGE texTable[2];
	texTable[0].Init(
		D3D12_DESCRIPTOR_RANGE_TYPE_SRV,
		1,  // number of descriptors
		0); // register t0
	texTable[1].Init(
		D3D12_DESCRIPTOR_RANGE_TYPE_SRV,
		1,  // number of descriptors
		1); // register t0

	// Root parameter can be a table, root descriptor or root constants.
	CD3DX12_ROOT_PARAMETER slotRootParameter[4];

	slotRootParameter[0].InitAsDescriptorTable(1, &texTable[0], D3D12_SHADER_VISIBILITY_PIXEL);
	slotRootParameter[1].InitAsDescriptorTable(1, &texTable[1], D3D12_SHADER_VISIBILITY_PIXEL);
	slotRootParameter[2].InitAsConstantBufferView(0);
	slotRootParameter[3].InitAsConstantBufferView(1);

	auto staticSamplers = GetStaticSamplers();

	CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(4, slotRootParameter,
		(UINT)staticSamplers.size(), staticSamplers.data(),
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	// create a root signature with a single slot which points to a descriptor range consisting of a single constant buffer
	ComPtr<ID3DBlob> serializedRootSig = nullptr;
	ComPtr<ID3DBlob> errorBlob = nullptr;
	HRESULT hr = D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1,
		serializedRootSig.GetAddressOf(), errorBlob.GetAddressOf());

	if (errorBlob != nullptr)
	{
		::OutputDebugStringA((char*)errorBlob->GetBufferPointer());
	}
	ThrowIfFailed(hr);

	ThrowIfFailed(m_pGraphicDevice->CreateRootSignature(
		0,
		serializedRootSig->GetBufferPointer(),
		serializedRootSig->GetBufferSize(),
		IID_PPV_ARGS(&m_arrSig[(UINT)ROOT_SIG_TYPE::INPUT_DISTORT])));

	return S_OK;
}

HRESULT CGraphicDevice::Create_SSAORoot()
{
	CD3DX12_DESCRIPTOR_RANGE texTable[2];
	texTable[0].Init(
		D3D12_DESCRIPTOR_RANGE_TYPE_SRV,
		1,  // number of descriptors
		0); // register t0
	texTable[1].Init(
		D3D12_DESCRIPTOR_RANGE_TYPE_SRV,
		1,  // number of descriptors
		1); // register t0

	// Root parameter can be a table, root descriptor or root constants.
	CD3DX12_ROOT_PARAMETER slotRootParameter[2];

	slotRootParameter[0].InitAsDescriptorTable(1, &texTable[0], D3D12_SHADER_VISIBILITY_PIXEL);
	slotRootParameter[1].InitAsDescriptorTable(1, &texTable[1], D3D12_SHADER_VISIBILITY_PIXEL);

	auto staticSamplers = GetStaticSamplers();

	CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(2, slotRootParameter,
		(UINT)staticSamplers.size(), staticSamplers.data(),
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	// create a root signature with a single slot which points to a descriptor range consisting of a single constant buffer
	ComPtr<ID3DBlob> serializedRootSig = nullptr;
	ComPtr<ID3DBlob> errorBlob = nullptr;
	HRESULT hr = D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1,
		serializedRootSig.GetAddressOf(), errorBlob.GetAddressOf());

	if (errorBlob != nullptr)
	{
		::OutputDebugStringA((char*)errorBlob->GetBufferPointer());
	}
	ThrowIfFailed(hr);

	ThrowIfFailed(m_pGraphicDevice->CreateRootSignature(
		0,
		serializedRootSig->GetBufferPointer(),
		serializedRootSig->GetBufferSize(),
		IID_PPV_ARGS(&m_arrSig[(UINT)ROOT_SIG_TYPE::INPUT_SSAO])));

	return S_OK;
}

HRESULT CGraphicDevice::OnResize(const _uint& iWidth, const _uint& iHeight)
{
	assert(m_pGraphicDevice);
	assert(m_pSwapChain);
	assert(m_pCommandAllocator);

	// Flush before changing any resources.
	Wait_ForGpuComplete();

	ThrowIfFailed(m_pCommandList->Reset(m_pCommandAllocator, nullptr));

	// Release the previous resources we will be recreating.
	for (int i = 0; i < m_iSwapChainBufferCount; ++i)
		m_ppSwapChainBuffer[i]	= nullptr;
	m_pDepthStencilBuffer		= nullptr;

	// Resize the swap chain.
	ThrowIfFailed(m_pSwapChain->ResizeBuffers(m_iSwapChainBufferCount,
											  iWidth, iHeight,
											  DXGI_FORMAT_R8G8B8A8_UNORM,
											  DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH));

	
	m_iCurrBackBuffer = 0;

	CD3DX12_CPU_DESCRIPTOR_HANDLE RTV_HeapHandle(m_pRTV_Heap->GetCPUDescriptorHandleForHeapStart());
	for (_uint i = 0; i < m_iSwapChainBufferCount; i++)
	{
		ThrowIfFailed(m_pSwapChain->GetBuffer(i, IID_PPV_ARGS(&m_ppSwapChainBuffer[i])));

		m_pGraphicDevice->CreateRenderTargetView(m_ppSwapChainBuffer[i], nullptr, RTV_HeapHandle);

		RTV_HeapHandle.Offset(1, m_uiRTV_DescriptorSize);
	}

	// Create the depth/stencil buffer and view.
	D3D12_RESOURCE_DESC DepthStencilDesc;
	DepthStencilDesc.Dimension			= D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	DepthStencilDesc.Alignment			= 0;
	DepthStencilDesc.Width				= iWidth;
	DepthStencilDesc.Height				= iHeight;
	DepthStencilDesc.DepthOrArraySize	= 1;
	DepthStencilDesc.MipLevels			= 1;
	DepthStencilDesc.Format				= DXGI_FORMAT_D24_UNORM_S8_UINT;
	DepthStencilDesc.SampleDesc.Count	= m_bIsMSAA4X_Enable ? 4 : 1;
	DepthStencilDesc.SampleDesc.Quality = m_bIsMSAA4X_Enable ? (m_uiMSAA4X_QualityLevels - 1) : 0;
	DepthStencilDesc.Layout				= D3D12_TEXTURE_LAYOUT_UNKNOWN;
	DepthStencilDesc.Flags				= D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	D3D12_CLEAR_VALUE optClear;
	optClear.Format						= DXGI_FORMAT_D24_UNORM_S8_UINT;
	optClear.DepthStencil.Depth			= 1.0f;
	optClear.DepthStencil.Stencil		= 0;
	ThrowIfFailed(m_pGraphicDevice->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
															D3D12_HEAP_FLAG_NONE,
															&DepthStencilDesc,
															D3D12_RESOURCE_STATE_COMMON,
															&optClear,
															IID_PPV_ARGS(&m_pDepthStencilBuffer)));

	// Create descriptor to mip level 0 of entire resource using the format of the resource.
	m_pGraphicDevice->CreateDepthStencilView(m_pDepthStencilBuffer,
											 nullptr, 
											 m_pDSV_Heap->GetCPUDescriptorHandleForHeapStart());

	// Transition the resource from its initial state to be used as a depth buffer.
	m_pCommandList->ResourceBarrier(1,
									&CD3DX12_RESOURCE_BARRIER::Transition(m_pDepthStencilBuffer,
									D3D12_RESOURCE_STATE_COMMON,
									D3D12_RESOURCE_STATE_DEPTH_WRITE));

	// Execute the resize commands.
	ThrowIfFailed(m_pCommandList->Close());
	ID3D12CommandList* cmdsLists[] = { m_pCommandList };
	m_pCommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

	// Wait until resize is complete.
	Wait_ForGpuComplete();

	// Update the viewport transform to cover the client area.
	m_Viewport.TopLeftX = 0;
	m_Viewport.TopLeftY = 0;
	m_Viewport.Width	= static_cast<_float>(iWidth);
	m_Viewport.Height	= static_cast<_float>(iHeight);
	m_Viewport.MinDepth = 0.0f;
	m_Viewport.MaxDepth = 1.0f;

	m_ScissorRect		= { 0, 0, (LONG)iWidth, (LONG)iHeight };

	return S_OK;
}


HRESULT CGraphicDevice::Wait_ForGpuComplete()
{
	/*____________________________________________________________________
	 Advance the fence value to mark commands up to this fence point.
	- Cpu �潺�� ����.
	- ���� Fence ���������� ��ɵ��� ǥ���ϵ��� ��Ÿ�� ���� ������Ų��.
	______________________________________________________________________*/
	m_uiCurrentFence++;

	/*____________________________________________________________________
	 Add an instruction to the command queue to set a new fence point.  Because we 
	 are on the GPU timeline, the new fence point won't be set until the GPU finishes
	 processing all the commands prior to this Signal().

	- Cpu�� �潺�� ���� �����ϴ� ����� ��� ť�� �߰�.
	- �� Fence ������ �����ϴ� ���(Signal)�� ��� ��⿭�� �߰��Ѵ�.
	 ���� �츮�� GPU �ð��� �� �����Ƿ�, �� Fence ������ 
	 GPU�� Signal()��ɱ����� ��� ����� ó���ϱ� �������� �������� �ʴ´�.
	______________________________________________________________________*/
	ThrowIfFailed(m_pCommandQueue->Signal(m_pFence, m_uiCurrentFence));

	/*____________________________________________________________________
	 Wait until the GPU has completed commands up to this fence point.
	- GPU�� �� Fence ���������� ��ɵ��� �Ϸ��� �� ���� ��ٸ���.
	______________________________________________________________________*/
	if (m_pFence->GetCompletedValue() < m_uiCurrentFence)
	{
		HANDLE eventHandle = CreateEventEx(nullptr, nullptr, false, EVENT_ALL_ACCESS);

		// GPU�� ���� Fence ������ ���������� �̺�Ʈ�� �ߵ��Ѵ�.
		ThrowIfFailed(m_pFence->SetEventOnCompletion(m_uiCurrentFence, eventHandle));

		// GPU�� ���� ��Ÿ�� ������ ���������� ���ϴ� �̺�Ʈ�� ��ٸ���.
		WaitForSingleObject(eventHandle, INFINITE);
		CloseHandle(eventHandle);
	}

	return S_OK;
}

HRESULT CGraphicDevice::Begin_ResetCmdList()
{
	FAILED_CHECK_RETURN(m_pCommandList->Reset(m_pCommandAllocator, nullptr), E_FAIL);

	return S_OK;
}

HRESULT CGraphicDevice::End_ResetCmdList()
{
	FAILED_CHECK_RETURN(m_pCommandList->Close(), E_FAIL);

	ID3D12CommandList* CommandLists[] = { m_pCommandList };
	m_pCommandQueue->ExecuteCommandLists(_countof(CommandLists), CommandLists);

	Wait_ForGpuComplete();

	return S_OK;
}

HRESULT CGraphicDevice::Begin_ResetCmdListThread()
{
	FAILED_CHECK_RETURN(m_pCommandListThread->Reset(m_pCommandAllocatorThread, nullptr), E_FAIL);

	return S_OK;
}

HRESULT CGraphicDevice::End_ResetCmdListThread()
{
	FAILED_CHECK_RETURN(m_pCommandListThread->Close(), E_FAIL);

	ID3D12CommandList* CommandLists[] = { m_pCommandListThread };
	m_pCommandQueue->ExecuteCommandLists(_countof(CommandLists), CommandLists);

	Wait_ForGpuComplete();

	return S_OK;
}


void CGraphicDevice::Log_Adapters()
{
	UINT i = 0;
	IDXGIAdapter* adapter = nullptr;
	std::vector<IDXGIAdapter*> adapterList;
	while (m_pFactory->EnumAdapters(i, &adapter) != DXGI_ERROR_NOT_FOUND)
	{
		DXGI_ADAPTER_DESC desc;
		adapter->GetDesc(&desc);

		std::wstring text = L"***Adapter: ";
		text += desc.Description;
		text += L"\n";

		OutputDebugString(text.c_str());

		adapterList.push_back(adapter);

		++i;
	}

	for (size_t i = 0; i < adapterList.size(); ++i)
	{
		Log_AdapterOutputs(adapterList[i]);
		Engine::Safe_Release(adapterList[i]);
	}
}

void CGraphicDevice::Log_AdapterOutputs(IDXGIAdapter* pAdapter)
{
	UINT i = 0;
	IDXGIOutput* output = nullptr;
	while (pAdapter->EnumOutputs(i, &output) != DXGI_ERROR_NOT_FOUND)
	{
		DXGI_OUTPUT_DESC desc;
		output->GetDesc(&desc);

		std::wstring text = L"***Output: ";
		text += desc.DeviceName;
		text += L"\n";
		OutputDebugString(text.c_str());

		Log_OutputDisplayModes(output, DXGI_FORMAT_R8G8B8A8_UNORM);

		Engine::Safe_Release(output);

		++i;
	}
}

void CGraphicDevice::Log_OutputDisplayModes(IDXGIOutput* output, DXGI_FORMAT format)
{
	UINT count = 0;
	UINT flags = 0;

	// Call with nullptr to get list count.
	output->GetDisplayModeList(format, flags, &count, nullptr);

	std::vector<DXGI_MODE_DESC> modeList(count);
	output->GetDisplayModeList(format, flags, &count, &modeList[0]);

	for (auto& x : modeList)
	{
		UINT n = x.RefreshRate.Numerator;
		UINT d = x.RefreshRate.Denominator;
		std::wstring text =
			L"Width = " + std::to_wstring(x.Width) + L" " +
			L"Height = " + std::to_wstring(x.Height) + L" " +
			L"Refresh = " + std::to_wstring(n) + L"/" + std::to_wstring(d) +
			L"\n";

		::OutputDebugString(text.c_str());
	}
}

array<const CD3DX12_STATIC_SAMPLER_DESC, 1> CGraphicDevice::GetStaticSamplers()
{

	const CD3DX12_STATIC_SAMPLER_DESC linearWrap(
		0, // shaderRegister
		D3D12_FILTER_MIN_MAG_MIP_LINEAR, // filter
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_WRAP); // addressW

	                           // maxAnisotropy

	return {linearWrap};
}

HRESULT CGraphicDevice::Create_11On12GraphicDevice()
{
	/*__________________________________________________________________________________________________________
	[ Create 11On12 state to enable D2D rendering on D3D12. ]

	- ù ��° �ܰ�� ID3D12Device�� ���� �Ŀ� ID3D11On12Device�� ����� ���Դϴ�.
	- �̶� API D3D11On12CreateDevice�� ���� ID3D12Device�� ���εǴ� ID3D11Device�� ����ϴ�.
	- �� API�� �ٸ� �Ű� ���� �߿��� ID3D12CommandQueue�� ����ϹǷ� 11On12 ����̽��� �ش� ����� ������ �� �ֽ��ϴ�.
	- ID3D11Device�� ��������� ���⿡�� ID3D11On12Device �������̽��� ������ �� �ֽ��ϴ�.
	- �̰��� D2D ������ ����� �⺻ ����̽� ��ü�Դϴ�.
	____________________________________________________________________________________________________________*/
	IUnknown* pRenderCommandQueue = m_pCommandQueue;

	ThrowIfFailed(D3D11On12CreateDevice(m_pGraphicDevice,					// D3D11 interop�� ����� ���� D3D12 ��ġ.
		D3D11_CREATE_DEVICE_BGRA_SUPPORT,
		nullptr,							// NULL�� �ϸ� D3D12��ġ�� ��� ����.
		0,									// ��� ���� �迭�� ũ��(����Ʈ).
		&pRenderCommandQueue,				// D3D11on12���� ����� ������ ť �迭.
		1,									// ��� ť �迭�� ũ��(����Ʈ).
		0,									// D3D12 ��ġ�� ���. 1��Ʈ�� ������ �� ����.								
		&m_p11Device,						// 11Device
		&m_p11Context,						// 11DeviceContext
		nullptr));

	ThrowIfFailed(m_p11Device->QueryInterface(&m_p11On12Device));

	/*__________________________________________________________________________________________________________
	[ Create D2D/DWrite components ]

	- D2D ���丮 �����.
	- ���� 11On12 ����̽��� �����Ƿ�, �� ����̽��� ����Ͽ� D3D11�� ����� ��ó�� D2D ���͸� �� ����̽��� ����ϴ�.
	____________________________________________________________________________________________________________*/
	ThrowIfFailed(DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(m_pDWriteFactory), (IUnknown**)&m_pDWriteFactory));
	ThrowIfFailed(D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, IID_PPV_ARGS(&m_pD2D_Factory)));


	D2D1_DEVICE_CONTEXT_OPTIONS DeviceOptions = D2D1_DEVICE_CONTEXT_OPTIONS_NONE;
	IDXGIDevice*				pDxgiDevice = nullptr;;

	ThrowIfFailed(m_p11On12Device->QueryInterface(&pDxgiDevice));

	ThrowIfFailed(m_pD2D_Factory->CreateDevice(pDxgiDevice, &m_pD2D_Device));
	Safe_Release(pDxgiDevice);

	ThrowIfFailed(m_pD2D_Device->CreateDeviceContext(DeviceOptions, &m_pD2D_Context));

	/*__________________________________________________________________________________________________________
	[ Cretae D2D RenderTarget ]

	- D3D12�� ���� ü���� �����ϹǷ�, 11On12 ����̽�(D2D ������)�� ����Ͽ� �� ���۷� �������Ϸ���,
	ID3D12Resource ������ �� ���ۿ��� ID3D11Resource ������ ���ε� ���ҽ��� ������ �մϴ�.
	- �̷��� �ϸ� 11On12 ����̽��� ����� �� �ֵ��� ID3D12Resource�� D3D11 ��� �������̽��� ����˴ϴ�.
	- ���ε� ���ҽ��� �غ��� �Ŀ��� ���������� LoadAssets �޼��忡�� D2D�� �������� ������ ��� ȭ���� ���� �� �ֽ��ϴ�.
	____________________________________________________________________________________________________________*/

	/*__________________________________________________________________________________________________________
	- Query the desktop's dpi settings, which will be used to create D2D's render targets.
	____________________________________________________________________________________________________________*/
	_float DPIx = 0.0f;
	_float DPIy = 0.0f;
	m_pD2D_Factory->GetDesktopDpi(&DPIx, &DPIy);

	D2D1_BITMAP_PROPERTIES1 bitmapProperties = D2D1::BitmapProperties1(D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW,
		D2D1::PixelFormat(DXGI_FORMAT_UNKNOWN, D2D1_ALPHA_MODE_PREMULTIPLIED),
		DPIx, DPIy);
	/*__________________________________________________________________________________________________________
	- For each buffer in the swapchain, we need to create a wrapped resource,
	and create a D2D render target object to enable D2D rendering for the UI.

	- D3D12�� ���� ü���� �����ϹǷ�, 11On12 ����̽�(D2D ������)�� ����Ͽ� �� ���۷� �������Ϸ���
	ID3D12Resource ������ �� ���ۿ��� ID3D11Resource ������ ���ε� ���ҽ��� ������ �մϴ�.
	- �̷��� �ϸ� 11On12 ����̽��� ����� �� �ֵ��� ID3D12Resource�� D3D11 ��� �������̽��� ����˴ϴ�.
	- ���ε� ���ҽ��� �غ��� �Ŀ��� ���������� LoadAssets �޼��忡�� D2D�� �������� ������ ��� ȭ���� ���� �� �ֽ��ϴ�.
	____________________________________________________________________________________________________________*/
	for (_uint i = 0; i < m_iSwapChainBufferCount; ++i)
	{
		D3D11_RESOURCE_FLAGS d3d11Flags = { D3D11_BIND_RENDER_TARGET };

		ThrowIfFailed(m_p11On12Device->CreateWrappedResource(m_ppSwapChainBuffer[i],
			&d3d11Flags,
			D3D12_RESOURCE_STATE_RENDER_TARGET,
			D3D12_RESOURCE_STATE_PRESENT,
			IID_PPV_ARGS(&m_pWrappedBackBuffers[i])));

		IDXGISurface* pSurface;
		ThrowIfFailed(m_pWrappedBackBuffers[i]->QueryInterface(&pSurface));

		m_pD2D_Context->CreateBitmapFromDxgiSurface(pSurface, &bitmapProperties, &m_pD2DRenderTargets[i]);
		Safe_Release(pSurface);
	}

	return S_OK;
}
void CGraphicDevice::Free()
{
#ifdef _DEBUG
	COUT_STR("Destroy GrpahicDevice");
#endif

	for (int i = 0; i < m_iSwapChainBufferCount; i++)
	{
		Engine::Safe_Release(m_ppSwapChainBuffer[i]);
		Engine::Safe_Release(m_pWrappedBackBuffers[i]);
		Engine::Safe_Release(m_pD2DRenderTargets[i]);

	}
	Engine::Safe_Release(m_pDepthStencilBuffer);

	Engine::Safe_Release(m_pRTV_Heap);
	Engine::Safe_Release(m_pDSV_Heap);

	Engine::Safe_Release(m_pCommandAllocator);
	Engine::Safe_Release(m_pCommandQueue);
	Engine::Safe_Release(m_pCommandList);
	Engine::Safe_Release(m_pFence);

	m_pSwapChain->SetFullscreenState(FALSE, nullptr);
	Engine::Safe_Release(m_pSwapChain);

	Engine::Safe_Release(m_pGraphicDevice);
	Engine::Safe_Release(m_pFactory);


	Engine::Safe_Release(m_p11Device);
	Engine::Safe_Release(m_p11On12Device);
	Engine::Safe_Release(m_pD2D_Device);
	Engine::Safe_Release(m_pD2D_Context);


	Engine::Safe_Release(m_pD2D_Factory);
	Engine::Safe_Release(m_pDWriteFactory);

	if (m_p11Context)
	{
		m_p11Context->ClearState();
		m_p11Context->Flush();
		Engine::Safe_Release(m_p11Context);
	}
}
