#pragma once
#include "Engine_Include.h"
#include "Base.h"

BEGIN(Engine)

class ENGINE_DLL CGraphicDevice : public CBase
{
	DECLARE_SINGLETON(CGraphicDevice)

public:
	enum WINMODE { MODE_FULL, MODE_WIN };

private:
	explicit CGraphicDevice();
	virtual ~CGraphicDevice();

public:
	ID3D12Device*				Get_GraphicDevice()					{ return m_pGraphicDevice; }
	ID3D12CommandQueue*			Get_CommandQueue()					{ return m_pCommandQueue; }
	ID3D12CommandAllocator*		Get_CommandAllocator()				{ return m_pCommandAllocator; }
	ID3D12GraphicsCommandList*	Get_CommandList()					{ return m_pCommandList; }

	ID3D12GraphicsCommandList*	Get_CommandListThread() { return m_pCommandListThread; }

	const D3D12_VIEWPORT&		Get_Viewport()						{ return m_Viewport; }
	const _bool&				Get_MSAA4X_Enable()					{ return m_bIsMSAA4X_Enable; }
	const _uint&				Get_MSAA4X_QualityLevels()			{ return m_uiMSAA4X_QualityLevels; }
	const _uint&				Get_CBV_SRV_UAV_DescriptorSize()	{ return m_uiCBV_SRV_UAV_DescriptorSize; }
public:
	void						Set_PipelineState(ID3D12PipelineState* pPipelineState) { m_pPipelineState = pPipelineState; }
public:
	HRESULT Ready_GraphicDevice(HWND hWnd, HANDLE hHandle, WINMODE eMode, const _uint& iWidth, const _uint& iHeight);
	HRESULT	Render_Begin(const _rgba& vRGBA);
	HRESULT	Render_End();
private:
	HRESULT	Create_GraphicDevice(const _uint& iWidth, const _uint& iHeight);
	HRESULT	Create_CommandQueueAndList();
	HRESULT	Create_SwapChain(HWND hWnd, WINMODE eMode, const _uint& iWidth, const _uint& iHeight);
	HRESULT Create_RtvAndDsvDescriptorHeaps();
	HRESULT Create_RootSig();
	HRESULT Create_TextureRoot();
	HRESULT OnResize(const _uint& iWidth, const _uint& iHeight);
public:
	ID3D12RootSignature* GetLootSig(_uint eType) { return m_arrSig[eType]; }

private:
	ID3D12RootSignature*	m_arrSig[(UINT)ROOT_SIG_TYPE::ENDSIG];

public:
	HRESULT	Wait_ForGpuComplete();	// CPU �� GPU����ȭ �Լ�
	HRESULT Begin_ResetCmdList();
	HRESULT End_ResetCmdList();

	HRESULT Begin_ResetCmdListThread();
	HRESULT End_ResetCmdListThread();
private:
	void	Log_Adapters();
	void	Log_AdapterOutputs(IDXGIAdapter* adapter);
	void	Log_OutputDisplayModes(IDXGIOutput* output, DXGI_FORMAT format);

private:
	array<const CD3DX12_STATIC_SAMPLER_DESC, 6> GetStaticSamplers();


private:
	/*____________________________________________________________________
	[ Factory / SwapChain / Device ]
	IDXGIFactory	: IDXGISwapChain �������̽� ������ ���÷��� �����(�׷��� ī��) ���ſ� ����.
	IDXGISwapChain	: �ַ� ���÷��̸� �����ϱ� ���� �ʿ�.
	ID3D12Device	: �ַ� ���ҽ��� �����ϱ� ���� �ʿ�.
	______________________________________________________________________*/
	IDXGIFactory4*		m_pFactory				 = nullptr;
	IDXGISwapChain*		m_pSwapChain			 = nullptr;
	ID3D12Device*		m_pGraphicDevice		 = nullptr;

	/*____________________________________________________________________
	[ CPU/GPU ����ȭ ]
	ID3D12Fence				: GPU�� CPU�� ����ȭ�� ���� �������� ���̴� ��ǥ �������̽�.
	UINT64 m_nFenceValue	: Fence�� ��. �ð����� Ư�� Fence������ �ĺ��ϴ� ����.
							�ĸ���� ���� ������ �潺 ���� �����ϱ� ���� m_uiSwapChainBuffers���� ��ŭ ����.
	______________________________________________________________________*/
	ID3D12Fence*	m_pFence		 = nullptr;
	UINT64			m_uiCurrentFence = 0;

	/*____________________________________________________________________
	[ ��� ��⿭�� ��� ��� ]
	ID3D12CommandQueue			: ��� ��⿭�� ��ǥ�ϴ� �������̽�.
	ID3D12CommandAllocator		: ��� �Ҵ���. ��� ��Ͽ� �߰��� ��ɵ��� �� �Ҵ����� �޸𸮿� ����ȴ�.
								  ExecuteCommandLists�� ��� ����� �����ϸ�, ��� ��⿭�� �� �Ҵ��ڿ� ��� ��ɵ��� �����Ѵ�.
	ID3D12GraphicsCommandList	: ���� �׷��� �۾��� ���� ��� ����� ��ǥ�ϴ� �������̽�.
	______________________________________________________________________*/
	ID3D12CommandQueue*			m_pCommandQueue		= nullptr;
	ID3D12CommandAllocator*		m_pCommandAllocator	= nullptr;
	ID3D12GraphicsCommandList*	m_pCommandList		= nullptr;


	ID3D12CommandQueue*			m_pCommandQueueThread = nullptr;
	ID3D12CommandAllocator*		m_pCommandAllocatorThread = nullptr;
	ID3D12GraphicsCommandList*	m_pCommandListThread = nullptr;

	/*____________________________________________________________________
	[ ������ ��(Descriptor Heap) ]
	- �׸��� ����� �����ϱ� ����, ���� �ش� �׸��� ȣ���� ������ �ڿ����� ������ ���������ο� ����� �Ѵ�.
	- ������ ���������ο� ���̴� ���� �ش� �ڿ��� �����ϴ� ������(Descriptor) ��ü�̴�.
	- �����ڴ� �ڿ��� GPU���� �������ִ� �淮�� �ڷᱸ����� �� �� �ִ�.
	- GPU�� �ڿ� �����ڸ� ���ؼ� �ڿ��� ���� �ڷῡ �����ϸ�, �� �ڷḦ ����ϴ� �� �ʿ��� ���� ���� �����ڷκ��� ��´�.
	- �����ڴ� �ڿ� �ڷḦ �����ϴ� ������ �Ӹ� �ƴ϶�, �ڿ��� GPU�� �����ϴ� ����. 
	  (�ڿ��� ������������ ��� �ܰ迡 ����� �ϴ����� ������.)
	
	1. CBV(��� ����), SRV(���̴� �ڿ�), UAV(���� ���� ����)�� ����.
	2. ǥ�� ����� �����ڴ� �ؽ�ó ���뿡 ���̴� ǥ�� �����(smapler) �ڿ��� ����.
	3. RTV �����ڴ� ���� ���(Render Target)�ڿ��� ����.
	4. DSV �����ڴ� ����, ���ٽ�(Depth/Stencil)�ڿ��� ����.

	- �����ڵ��� ���� ���α׷��� �ʱ�ȭ �������� �����ؾ� �Ѵ�.
	______________________________________________________________________*/
	static const _int		m_iSwapChainBufferCount = 2;
	_int					m_iCurrBackBuffer		= 0;

	ID3D12Resource*			m_ppSwapChainBuffer[m_iSwapChainBufferCount];
	ID3D12Resource*			m_pDepthStencilBuffer = nullptr;

	ID3D12DescriptorHeap*	m_pRTV_Heap = nullptr;
	ID3D12DescriptorHeap*	m_pDSV_Heap = nullptr;

	_uint					m_uiRTV_DescriptorSize			= 0;
	_uint					m_uiDSV_DescriptorSize			= 0;
	_uint					m_uiCBV_SRV_UAV_DescriptorSize	= 0;

	ID3D12PipelineState*	m_pPipelineState				= nullptr;		// �׷��Ƚ� ���������� ���� ��ü�� ���� �������̽� ������.

	/*____________________________________________________________________
	[ �ڿ� ���� ���� ]
	ResourceBarrier�� ����̹����� ���ҽ��� ����� �޸𸮿� ���� ���� �׼����� 
	����ȭ�ؾ� �ϴ� ��Ȳ�� �׷��� ����̹��� �˸���.
	______________________________________________________________________*/
	D3D12_RESOURCE_BARRIER	m_ResourceBarrier;
	DXGI_PRESENT_PARAMETERS	m_PresentParameters;

	/*____________________________________________________________________
	[ ����Ʈ�� ���� �簢�� ]
	- ���� �簢���� Ư�� �ȼ����� �ø��ϴ� �뵵�� ���δ�.
	- �ĸ� ���۸� �������� ���� ���簢���� ����, �����ϸ�, 
	  ������ �� ���� ���簢���� �ٱ��� �ִ� �ȼ����� �ĸ� ���ۿ� �����Ϳ� ���� �ʴ´�.
	______________________________________________________________________*/
	D3D12_VIEWPORT	m_Viewport;		// ����Ʈ.
	D3D12_RECT		m_ScissorRect;	// ���� �簢��.

	/*____________________________________________________________________
	4X MSAA ǰ�� ���� ���� ����.
	______________________________________________________________________*/
	_bool m_bIsMSAA4X_Enable		= false;
	_uint m_uiMSAA4X_QualityLevels	= 0;

private:
	virtual void Free();
};

END