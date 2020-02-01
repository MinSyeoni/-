#pragma once
#include "Engine_Typedef.h"
#include "Engine_Macro.h"

/*____________________________________________________________________
[ UploadBuffer ]
______________________________________________________________________*/
template<typename T>
class CUploadBuffer
{
public:
	CUploadBuffer(ID3D12Device* pGraphicDevice, _uint uiElementCount, _bool bIsConstantBuffer)
		: m_pGraphicDevice(pGraphicDevice)
		, m_bIsConstantBuffer(bIsConstantBuffer)
		, m_uiElementCount(uiElementCount)
	{
		m_uiElementByteSize = sizeof(T);

		/*____________________________________________________________________
		��� ���� ������ ũ��� �ݵ�� 256����Ʈ�� ����̾�� �Ѵ�.
		�̴� �ϵ��� m * 256����Ʈ �����¿��� �����ϴ� n * 256����Ʈ ������
		��� �ڷḸ �� �� �ֱ� �����̴�.

		typedef struct D3D12_CONSTANT_BUFFER_VIEW_DESC 
		{
			UINT64 OffsetInBytes;			// 256�� ���.
			UINT   SizeInBytes;				// 256�� ���.
		} D3D12_CONSTANT_BUFFER_VIEW_DESC;
		______________________________________________________________________*/
		if (m_bIsConstantBuffer)
			m_uiElementByteSize = (sizeof(T) + 255) & ~255;

		ThrowIfFailed(m_pGraphicDevice->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
																D3D12_HEAP_FLAG_NONE,
																&CD3DX12_RESOURCE_DESC::Buffer(m_uiElementByteSize*uiElementCount),
																D3D12_RESOURCE_STATE_GENERIC_READ,
																nullptr,
																IID_PPV_ARGS(&m_pUploadBuffer)));

		ThrowIfFailed(m_pUploadBuffer->Map(0, nullptr, reinterpret_cast<void**>(&m_pMappedData)));

		/*____________________________________________________________________
		�ڿ��� �� ����ϱ� ������ ������ ������ �ʿ䰡 ����.
		�׷���, �ڿ��� GPU�� ����ϴ� �߿��� CPU���� �ڿ��� �������� �ʾƾ� �Ѵ�.
		(����, �ݵ�� ����ȭ ����� ����ؾ� �Ѵ�.)
		______________________________________________________________________*/
	}
	CUploadBuffer& operator=(const CUploadBuffer& rhs) = delete;
	~CUploadBuffer()
	{
		Free();
	}

	ID3D12Resource* Resource() const
	{
		return m_pUploadBuffer;
	}

	void CopyData(_int iElementIndex, const T& data)
	{
		memcpy(&m_pMappedData[iElementIndex * m_uiElementByteSize], &data, sizeof(T));
		// m_pUploadBuffer->Map(0, nullptr, reinterpret_cast<void**>(&m_pMappedData));
	}

	void ReleaseData()
	{
		//if (m_pUploadBuffer != nullptr)
		//	m_pUploadBuffer->Unmap(0, nullptr);
	}

	void Free()
	{
		if (m_pUploadBuffer != nullptr)
			m_pUploadBuffer->Unmap(0, nullptr);

		m_pMappedData = nullptr;
	}

	void Set_IsClone() { m_bIsClone = true; }

private:
	ID3D12Device*	m_pGraphicDevice	= nullptr;
	ID3D12Resource* m_pUploadBuffer		= nullptr;

	_uint			m_uiElementCount	= 0;
	BYTE*			m_pMappedData		= nullptr;
	_uint			m_uiElementByteSize = 0;
	_bool			m_bIsConstantBuffer = false;

	_bool			m_bIsClone			= false;
};
