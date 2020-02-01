#include "VIBuffer.h"
#include "GraphicDevice.h"

USING(Engine)

CVIBuffer::CVIBuffer(ID3D12Device* pGraphicDevice, ID3D12GraphicsCommandList* pCommandList)
	: CComponent(pGraphicDevice, pCommandList)
{
	ZeroMemory(&m_tSubMeshGeometry, sizeof(SUBMESH_GEOMETRY));
}

CVIBuffer::CVIBuffer(const CVIBuffer & rhs)
	: CComponent(rhs)
	, m_pVB_CPU(rhs.m_pVB_CPU)
	, m_pIB_CPU(rhs.m_pIB_CPU)
	, m_pVB_GPU(rhs.m_pVB_GPU)
	, m_pIB_GPU(rhs.m_pIB_GPU)
	, m_pVB_Uploader(rhs.m_pVB_Uploader)
	, m_pIB_Uploader(rhs.m_pIB_Uploader)
	, m_uiVertexByteStride(rhs.m_uiVertexByteStride)
	, m_uiVB_ByteSize(rhs.m_uiVB_ByteSize)
	, m_uiIB_ByteSize(rhs.m_uiIB_ByteSize)
	, m_IndexFormat(rhs.m_IndexFormat)
	, m_tSubMeshGeometry(rhs.m_tSubMeshGeometry)
	, m_PrimitiveTopology(rhs.m_PrimitiveTopology)
{
	if (m_pVB_GPU != nullptr)
		m_pVB_GPU->AddRef();

	if (m_pIB_GPU != nullptr)
		m_pIB_GPU->AddRef();

	if (m_pVB_Uploader != nullptr)
		m_pVB_Uploader->AddRef();

	if (m_pIB_Uploader != nullptr)
		m_pIB_Uploader->AddRef();

	if (m_pIB_CPU != nullptr)
		m_pIB_CPU->AddRef();


	if (m_pVB_CPU != nullptr)
		m_pVB_CPU->AddRef();
}


CVIBuffer::~CVIBuffer()
{
}

D3D12_VERTEX_BUFFER_VIEW CVIBuffer::Get_VertexBufferView() const
{
	D3D12_VERTEX_BUFFER_VIEW VertexBufferView;
	VertexBufferView.BufferLocation	= m_pVB_GPU->GetGPUVirtualAddress();
	VertexBufferView.StrideInBytes	= m_uiVertexByteStride;
	VertexBufferView.SizeInBytes	= m_uiVB_ByteSize;

	return VertexBufferView;
}

D3D12_INDEX_BUFFER_VIEW CVIBuffer::Get_IndexBufferView() const
{
	D3D12_INDEX_BUFFER_VIEW IndexBufferView;
	IndexBufferView.BufferLocation	= m_pIB_GPU->GetGPUVirtualAddress();
	IndexBufferView.Format			= m_IndexFormat;
	IndexBufferView.SizeInBytes		= m_uiIB_ByteSize;

	return IndexBufferView;
}

HRESULT CVIBuffer::Ready_Buffer()
{
	NULL_CHECK_RETURN(m_pGraphicDevice, E_FAIL);
	NULL_CHECK_RETURN(m_pCommandList, E_FAIL);

	return S_OK;
}

void CVIBuffer::Begin_Buffer()
{
	// �޽��� ���� ���� ��� �ε��� ���� �並 �����Ѵ�.
	m_pCommandList->IASetVertexBuffers(0, 						 // ���� ����. (�Է� ������ �� 16��)
									   1, 						 // �Է� ���Ե鿡 ���� ���� ���� ����.
									   &Get_VertexBufferView()); // ���� ���� ���� ù ���Ҹ� ����Ű�� ������.
	m_pCommandList->IASetIndexBuffer(&Get_IndexBufferView());

	// �޽��� ������Ƽ�� ������ �����Ѵ�.
	m_pCommandList->IASetPrimitiveTopology(m_PrimitiveTopology);
}

void CVIBuffer::Render_Buffer()
{
	/*____________________________________________________________________
	�������� ������������ �Է� ������ �ܰ�� ���޵ȴ�.
	______________________________________________________________________*/
	m_pCommandList->DrawIndexedInstanced(m_tSubMeshGeometry.uiIndexCount,	// �׸��⿡ ����� �ε������� ����. (�ν��Ͻ� ��)
										 1,									// �׸� �ν��Ͻ� ����.
										 0,									// �ε��� ������ ù index
										 0, 								// �׸��� ȣ�⿡ ���̴� �ε����鿡 ���� ���� ��.
										 0);								// �ν��Ͻ� 
}


ID3D12Resource * CVIBuffer::Create_DefaultBuffer(const void * InitData, 
												 UINT64 uiByteSize,
												 ID3D12Resource *& pUploadBuffer)
{
	Engine::CGraphicDevice::Get_Instance()->Begin_ResetCmdList();

	ID3D12Resource* pDefaultBuffer = nullptr;

	/*____________________________________________________________________
	���� �⺻ ���� �ڿ��� �����Ѵ�.
	______________________________________________________________________*/
	ThrowIfFailed(m_pGraphicDevice->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
															D3D12_HEAP_FLAG_NONE,
															&CD3DX12_RESOURCE_DESC::Buffer(uiByteSize),
															D3D12_RESOURCE_STATE_COMMON,
															nullptr,
															IID_PPV_ARGS(&pDefaultBuffer)));
 
	/*____________________________________________________________________
	CPU �޸��� �ڷḦ �⺻ ���ۿ� �����Ϸ���, �ӽ� ���ε� ���� ������ �Ѵ�.
	______________________________________________________________________*/
	ThrowIfFailed(m_pGraphicDevice->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
															D3D12_HEAP_FLAG_NONE,
															&CD3DX12_RESOURCE_DESC::Buffer(uiByteSize),
															D3D12_RESOURCE_STATE_GENERIC_READ,
															nullptr,
															IID_PPV_ARGS(&pUploadBuffer)));

	/*____________________________________________________________________
	�⺻ ���ۿ� ������ �ڷḦ �����Ѵ�.
	______________________________________________________________________*/
	D3D12_SUBRESOURCE_DATA subResourceData = {};
	subResourceData.pData		= InitData;
	subResourceData.RowPitch	= uiByteSize;
	subResourceData.SlicePitch	= subResourceData.RowPitch;

	/*____________________________________________________________________
	�⺻ ���� �ڿ������� �ڷ� ���縦 ��û�Ѵ�.
	�跫������ �����ڸ�, ���� �Լ� UpdateSubresources�� CPU �޸𸮸� �ӽ� ���ε� ���� �����ϰ�,
	ID3D12CommandList::CopySubresourceRegion�� �̿��ؼ� �ӽ� ���ε� ���� �ڷḦ mBuffer�� �����Ѵ�.
	______________________________________________________________________*/
	m_pCommandList->ResourceBarrier(1, 
									&CD3DX12_RESOURCE_BARRIER::Transition(pDefaultBuffer,
																		  D3D12_RESOURCE_STATE_COMMON, 
																		  D3D12_RESOURCE_STATE_COPY_DEST));
	UpdateSubresources<1>(m_pCommandList, pDefaultBuffer, pUploadBuffer, 0, 0, 1, &subResourceData);

	m_pCommandList->ResourceBarrier(1, 
		&CD3DX12_RESOURCE_BARRIER::Transition(pDefaultBuffer,
											  D3D12_RESOURCE_STATE_COPY_DEST, 
											  D3D12_RESOURCE_STATE_GENERIC_READ));

	/*____________________________________________________________________
	[ ���� ]
	���� �Լ� ȣ�� ���Ŀ���, UploadBuffer�� ��� �����ؾ� �Ѵ�.
	������ ���縦 �����ϴ� ��� ����� ���� ������� �ʾұ� �����̴�.
	���簡 �Ϸ�Ǿ����� Ȯ������ �Ŀ� ȣ���ڰ� UploadBuffer�� �����ϸ� �ȴ�.
	______________________________________________________________________*/
	CGraphicDevice::Get_Instance()->Wait_ForGpuComplete();
	Engine::CGraphicDevice::Get_Instance()->End_ResetCmdList();

	return pDefaultBuffer;
}

void CVIBuffer::Free()
{
	CComponent::Free();

	Engine::Safe_Release(m_pVB_CPU);
	Engine::Safe_Release(m_pIB_CPU);
	Engine::Safe_Release(m_pVB_GPU);
	Engine::Safe_Release(m_pIB_GPU);
	Engine::Safe_Release(m_pVB_Uploader);
	Engine::Safe_Release(m_pIB_Uploader);
}
