#pragma once
#include "Component.h"

BEGIN(Engine)

class ENGINE_DLL CShader : public CComponent
{
protected:
	explicit CShader(ID3D12Device* pGraphicDevice, ID3D12GraphicsCommandList* pCommandList);
	explicit CShader(const CShader& rhs);
	virtual ~CShader();

public:
	ID3D12PipelineState*		Get_PipelineState()						{ return m_pPipelineState; }
public:
	virtual void Begin_Shader();
	virtual void End_Shader(_uint Texnum = 0,_int Index=0);

public:
	ID3D12DescriptorHeap*			Get_CBVDescriptorHeap() { return m_pCBV_DescriptorHeap; }
	CUploadBuffer<CB_MATRIX_INFO>*	Get_UploadBuffer_MatrixInfo() { return m_pCB_MatrixInfo; }

protected:
	ID3DBlob* Compile_Shader(const wstring& wstrFilename,
							 const D3D_SHADER_MACRO* tDefines,
							 const string& strEntrypoint,
							 const string& strTarget);
protected:
	virtual HRESULT						Create_DescriptorHeaps()	PURE;
	virtual HRESULT						Create_ConstantBufferView()	PURE;
	virtual HRESULT						Create_PipelineState()		PURE;
protected:
	virtual D3D12_RASTERIZER_DESC		Create_RasterizerState()	PURE;
	virtual D3D12_BLEND_DESC			Create_BlendState()			PURE;
	virtual D3D12_DEPTH_STENCIL_DESC	Create_DepthStencilState()	PURE;
	virtual D3D12_INPUT_LAYOUT_DESC		Create_InputLayout()		PURE;

protected:


	/*____________________________________________________________________
	[ ���������� ���� ��ü(Pipeline State Object, PSO) ]
	- ���������� ���¸� ��Ÿ���� �������̽� ������.
	- PSO ������ �������� ���� �ð��� �ɸ� �� �����Ƿ�, �ʱ�ȭ �������� �����ؾ� �Ѵ�.
	- ������ ���ؼ��� PSO ���� ������ �ּ�ȭ�ؾ� �Ѵ�.
	- ���� PSO�� ����� �� �ִ� ��ü���� ��� �Բ� �׷��� �����ϴ�.
	- �׸��� ȣ�⸶�� PSO�� ���������� ���ƾ� �Ѵ�.
	______________________________________________________________________*/
	ID3D12PipelineState* m_pPipelineState = nullptr;

	ID3DBlob* m_pVS_ByteCode = nullptr;
	ID3DBlob* m_pPS_ByteCode = nullptr;

protected:
	ID3D12DescriptorHeap*			m_pCBV_DescriptorHeap = nullptr;
	CUploadBuffer<CB_MATRIX_INFO>*	m_pCB_MatrixInfo = nullptr;


public:
	virtual CComponent* Clone() PURE;
protected:
	virtual void		Free();
};

END