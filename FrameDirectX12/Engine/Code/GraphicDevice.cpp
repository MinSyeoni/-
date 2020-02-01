#include "GraphicDevice.h"

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

HRESULT CGraphicDevice::Render_End()
{
	/*____________________________________________________________________
	 Indicate a state transition on the resource usage.
	- �ڿ� �뵵�� ���õ� ���� ���̸� Direct3D�� �����Ѵ�.
	- ���� ���� Ÿ�ٿ� ���� �������� �����⸦ ��ٸ���. 
	- GPU�� ���� Ÿ��(����)�� �� �̻� ������� ������ ���� Ÿ���� ���´� 
	  ������Ʈ ����(D3D12_RESOURCE_STATE_PRESENT)�� �ٲ� ���̴�.
	______________________________________________________________________*/
	m_pCommandList->ResourceBarrier(1, 
									&CD3DX12_RESOURCE_BARRIER::Transition(m_ppSwapChainBuffer[m_iCurrBackBuffer],
									D3D12_RESOURCE_STATE_RENDER_TARGET, 
									D3D12_RESOURCE_STATE_PRESENT));




	/*____________________________________________________________________
	- ��ɵ��� ����� ��ģ��.
	- ExecuteCommandLists�� ��� ����� �����ϱ� ���� 
	  �ݵ�� �� �޼��带 �̿��ؼ� ��� ����� �ݾƾ� �Ѵ�.
	______________________________________________________________________*/
	FAILED_CHECK_RETURN(m_pCommandList->Close(), E_FAIL);

	/*____________________________________________________________________
	- ��� ����Ʈ�� ��� ť�� �߰��Ͽ� �����Ѵ�.
	______________________________________________________________________*/
	ID3D12CommandList* ppCommandLists[] = { m_pCommandList };
	m_pCommandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

	/*____________________________________________________________________
	- ����ü���� ������Ʈ�Ѵ�. (�ĸ� ���ۿ� ���� ���۸� ��ȯ)
	- ������Ʈ�� �ϸ� ���� ���� Ÿ��(�ĸ����)�� ������ 
	  ������۷� �Ű�����, ���� Ÿ�� �ε����� �ٲ� ���̴�.
	______________________________________________________________________*/
	FAILED_CHECK_RETURN(m_pSwapChain->Present(0, 0), E_FAIL);

	m_iCurrBackBuffer = (m_iCurrBackBuffer + 1) % m_iSwapChainBufferCount;

	/*____________________________________________________________________
	- GPU�� ��� ��� ����Ʈ�� ������ �� ���� ��ٸ���.
	______________________________________________________________________*/
	FAILED_CHECK_RETURN(Wait_ForGpuComplete(), E_FAIL);

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

	/*____________________________________________________________________
	 Try to create hardware device.
	______________________________________________________________________*/
	HRESULT hResult = D3D12CreateDevice(nullptr,             // default adapter
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
	/*____________________________________________________________________
		 �Ϲ������� ���̴� ���α׷��� Ư�� �ڿ���(��� ����, �ؽ�ó, ���÷�) ���� �Էµȴٰ� ����Ѵ�.
		��Ʈ ������ ���̴� ���α׷��� ����ϴ� �ڿ����� �����Ѵ�.
		���̴� ���α׷��� ���������� �ϳ��� �Լ��̰�, ���̴��� �ԷµǴ� �ڿ����� �Լ��� �Ű������鿡 �ش��ϹǷ�
		��Ʈ ������ �� �Լ� ������ �����ϴ� �����̶� �� �� �ִ�.
		______________________________________________________________________*/

		// ��Ʈ �Ű������� ���̺��̰ų�, ��Ʈ ������ �Ǵ� ��Ʈ ����̴�.
	CD3DX12_ROOT_PARAMETER RootParameter[1];

	// CBV �ϳ��� ��� ������ ���̺��� �����Ѵ�.
	CD3DX12_DESCRIPTOR_RANGE CBV_Table;
	CBV_Table.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);
	RootParameter[0].InitAsDescriptorTable(1, &CBV_Table);

	// ��Ʈ ������ ��Ʈ �Ű��������� �迭�̴�.
	CD3DX12_ROOT_SIGNATURE_DESC RootSignatureDesc(1,
		RootParameter,
		0,		// Texture�� �����Ƿ� 0.
		nullptr,	// Texture�� �����Ƿ� nullptr.
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	// ��� ���� �ϳ��� ������ ������ ������ ����Ű��, ���� �ϳ��� �̷���� ��Ʈ ������ �����Ѵ�.
	ID3DBlob* pSignatureBlob = nullptr;
	ID3DBlob* pErrorBlob = nullptr;
	FAILED_CHECK_RETURN(D3D12SerializeRootSignature(&RootSignatureDesc,
		D3D_ROOT_SIGNATURE_VERSION_1,
		&pSignatureBlob,
		&pErrorBlob), E_FAIL);
	if (nullptr != pErrorBlob)
	{
		OutputDebugStringA((char*)pErrorBlob->GetBufferPointer());
		return E_FAIL;
	}

	ThrowIfFailed(m_pGraphicDevice->CreateRootSignature(0,
		pSignatureBlob->GetBufferPointer(),
		pSignatureBlob->GetBufferSize(),
		IID_PPV_ARGS(&m_arrSig[(UINT)ROOT_SIG_TYPE::INPUT_OBJECT])));
	Engine::Safe_Release(pSignatureBlob);
	Engine::Safe_Release(pErrorBlob);

	Create_TextureRoot();

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
		HANDLE eventHandle = CreateEventEx(nullptr, false, false, EVENT_ALL_ACCESS);

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

array<const CD3DX12_STATIC_SAMPLER_DESC, 6> CGraphicDevice::GetStaticSamplers()
{
	const CD3DX12_STATIC_SAMPLER_DESC pointWrap(
		0, // shaderRegister
		D3D12_FILTER_MIN_MAG_MIP_POINT, // filter
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_WRAP); // addressW

	const CD3DX12_STATIC_SAMPLER_DESC pointClamp(
		1, // shaderRegister
		D3D12_FILTER_MIN_MAG_MIP_POINT, // filter
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP); // addressW

	const CD3DX12_STATIC_SAMPLER_DESC linearWrap(
		2, // shaderRegister
		D3D12_FILTER_MIN_MAG_MIP_LINEAR, // filter
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_WRAP); // addressW

	const CD3DX12_STATIC_SAMPLER_DESC linearClamp(
		3, // shaderRegister
		D3D12_FILTER_MIN_MAG_MIP_LINEAR, // filter
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP); // addressW

	const CD3DX12_STATIC_SAMPLER_DESC anisotropicWrap(
		4, // shaderRegister
		D3D12_FILTER_ANISOTROPIC, // filter
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressW
		0.0f,                             // mipLODBias
		8);                               // maxAnisotropy

	const CD3DX12_STATIC_SAMPLER_DESC anisotropicClamp(
		5, // shaderRegister
		D3D12_FILTER_ANISOTROPIC, // filter
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressW
		0.0f,                              // mipLODBias
		8);                                // maxAnisotropy

	return {
		pointWrap, pointClamp,
		linearWrap, linearClamp,
		anisotropicWrap, anisotropicClamp };
}


void CGraphicDevice::Free()
{
#ifdef _DEBUG
	COUT_STR("Destroy GrpahicDevice");
#endif

	for (int i = 0; i < m_iSwapChainBufferCount; i++)
		Engine::Safe_Release(m_ppSwapChainBuffer[i]);

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
}
