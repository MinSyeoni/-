#pragma once
#include "Engine_Include.h"
#include "Base.h"
#include "Target.h"
#include "LightTarget.h"
#include "Shader_Blend.h"
#include "RcTex.h"
#include "Shader_Shadow.h"
#include "ShadowDepthTarget.h"
#include "Shader_Shadow.h"
#include "DownSampleTarget.h"
#include "Shader_DownSample.h"
BEGIN(Engine)

class CGameObject;
class CComponentMgr;

class ENGINE_DLL CRenderer : public CBase
{
	DECLARE_SINGLETON(CRenderer)

public:
	enum RENDERGROUP 
	{
		RENDER_SHADOWDEPTH,
		RENDER_PRIORITY,
		RENDER_NONALPHA,
		RENDER_ALPHA,
		RENDER_UI,
		RENDER_FONT,
		RENDER_END
	};

private:
	explicit CRenderer();
	virtual ~CRenderer();

public:
	HRESULT	Ready_Renderer(ID3D12Device* pGraphicDevice, ID3D12GraphicsCommandList* pCommandList);
	HRESULT	Add_Renderer(const RENDERGROUP& eRenderID, CGameObject* pGameObject);
	void	Render_Renderer(const _float& fTimeDelta);
private:
	HRESULT Render_ShadowDepth();
	HRESULT Render_LightAcc();
	HRESULT	Render_Priority(const _float& fTimeDelta);
	HRESULT	Render_NonAlpha(const _float& fTimeDelta);
	HRESULT	Render_Alpha(const _float& fTimeDelta);
	HRESULT	Render_UI(const _float& fTimeDelta);
	HRESULT	Render_Font(const _float& fTimeDelta);
	HRESULT Render_Blend();
	HRESULT Render_PostPoressing();
	HRESULT Render_DownSampleing();
public:
	void	Clear_RenderGroup();

private:
	HRESULT	Ready_ShaderPrototype();

private:
	ID3D12Device*				m_pGraphicDevice	= nullptr;
	ID3D12GraphicsCommandList*	m_pCommandList		= nullptr;
	CComponentMgr*				m_pComponentMgr		= nullptr;

	list<CGameObject*>			m_RenderList[RENDER_END];
public:
	CShadowDepthTarget* Get_ShadowDepthTarget() { return m_ShadowDepthTarget; };


private: //����Ÿ�ٰ���
	CTarget*     m_DifferdTarget=nullptr;
	CLightTarget* m_LightTarget = nullptr;
	CShadowDepthTarget* m_ShadowDepthTarget = nullptr;
	CDownSampleTarget* m_DownSampleTarget = nullptr;

	_bool m_blsShowTarget = false;

private://��������
	CRcTex* m_pBlendBuffer;
	CShader_Blend* m_pBlendShader;
	_bool m_blsBlendInit=false;
private://�׸��ڰ���
	CShader_Shadow* m_pShadowShader;
private://�ٿ���ø�����
	CRcTex* m_pDownSampleBuffer;
	CShader_DownSample* m_pDownSampleShader;
	_bool m_bIsDownSampleInit = false;

private:
	virtual void		Free();
};

END