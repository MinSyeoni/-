#pragma once
#include "Shader.h"
#include "Texture.h"
BEGIN(Engine)

class ENGINE_DLL CShader_DefaultTex : public CShader
{
private:
	explicit CShader_DefaultTex(ID3D12Device* pGraphicDevice, ID3D12GraphicsCommandList* pCommandList);
	explicit CShader_DefaultTex(const CShader_DefaultTex& rhs);
	virtual ~CShader_DefaultTex();



public:
	HRESULT			Ready_Shader();
	virtual void	Begin_Shader();
	virtual void	End_Shader(_uint Texnum = 0);

public:
	void Set_Shader_Texture(vector< ComPtr<ID3D12Resource>> pVecTexture);
private:
	// CShader��(��) ���� ��ӵ�
	virtual HRESULT						Create_DescriptorHeaps() override;
	virtual HRESULT						Create_ConstantBufferView()	override;
	virtual HRESULT						Create_PipelineState() override;
private:
	virtual D3D12_RASTERIZER_DESC		Create_RasterizerState() override;
	virtual D3D12_BLEND_DESC			Create_BlendState() override;
	virtual D3D12_DEPTH_STENCIL_DESC	Create_DepthStencilState() override;
	virtual D3D12_INPUT_LAYOUT_DESC		Create_InputLayout() override;


public:
	virtual CComponent *		Clone() override;
	static CShader_DefaultTex* Create(ID3D12Device* pGraphicDevice,
		ID3D12GraphicsCommandList* pCommandList);
private:
	virtual void				Free();
};

END