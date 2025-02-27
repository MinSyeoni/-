#include "stdafx.h"
#include "MousePoint.h"
#include "DirectInput.h"

CMousePoint::CMousePoint(ID3D12Device* pGraphicDevice, ID3D12GraphicsCommandList* pCommandList)
	: Engine::CGameObject(pGraphicDevice, pCommandList)
{
}

CMousePoint::CMousePoint(const CMousePoint& rhs)
	: Engine::CGameObject(rhs)
{
}

CMousePoint::~CMousePoint()
{
}

HRESULT CMousePoint::Ready_GameObjectPrototype()
{
	return S_OK;
}

HRESULT CMousePoint::Ready_GameObject()
{
	Add_Component();

	return S_OK;
}

HRESULT CMousePoint::LateInit_GameObject()
{
	m_pShaderCom->Set_Shader_Texture(m_pTexture->Get_Texture());

	m_pTransCom->m_vScale = _vec3{ 0.02f, 0.03f, 0.02f };
	m_pTransCom->m_vPos.x = _float(2.f / WINCX * WINCX / 2) - 1.f;
	m_pTransCom->m_vPos.y = _float(-2.f / WINCY * WINCY / 2) + 1.f;

	return S_OK;
}

_int CMousePoint::Update_GameObject(const _float& fTimeDelta)
{
	FAILED_CHECK_RETURN(Engine::CGameObject::LateInit_GameObject(), E_FAIL);

	if (m_bIsDead)
		return DEAD_OBJ;

	GetCursorPos(&m_pt);
	ScreenToClient(g_hWnd, &m_pt);

	Engine::CGameObject::Update_GameObject(fTimeDelta);

	return NO_EVENT;
}

_int CMousePoint::LateUpdate_GameObject(const _float& fTimeDelta)
{
	NULL_CHECK_RETURN(m_pRenderer, -1);

	FAILED_CHECK_RETURN(m_pRenderer->Add_Renderer(CRenderer::RENDER_UI, this), -1);

	_float X = (((2.0f * m_pt.x) / WINCX) - 0.98f);
	_float Y = (((-2.0f * m_pt.y) / WINCY) + 0.93f);
	m_pTransCom->m_vPos.x = X;
	m_pTransCom->m_vPos.y = Y;

	return NO_EVENT;
}

void CMousePoint::Render_GameObject(const _float& fTimeDelta)
{
	if (!m_bIsShow)
		return;

	Set_ConstantTable();
	m_pShaderCom->Begin_Shader();
	m_pBufferCom->Begin_Buffer();
	m_pShaderCom->End_Shader();
	m_pBufferCom->End_Buffer();
	m_pBufferCom->Render_Buffer();
}

HRESULT CMousePoint::Add_Component()
{
	NULL_CHECK_RETURN(m_pComponentMgr, E_FAIL);

	// Buffer
	m_pBufferCom = static_cast<Engine::CRcTex*>(m_pComponentMgr->Clone_Component(L"Prototype_RcTex", COMPONENTID::ID_STATIC));
	NULL_CHECK_RETURN(m_pBufferCom, E_FAIL);
	m_mapComponent[ID_STATIC].emplace(L"Com_Buffer", m_pBufferCom);

	wstring wstrComponent = L"Com_Shader";
	// Shader
	m_pShaderCom = static_cast<Engine::CShader_UI*>(m_pComponentMgr->Clone_Component(L"Prototype_Shader_UI", COMPONENTID::ID_STATIC));
	NULL_CHECK_RETURN(m_pShaderCom, E_FAIL);
	m_mapComponent[ID_STATIC].emplace(wstrComponent, m_pShaderCom);

	// Texture 
	m_pTexture = static_cast<Engine::CTexture*>(m_pComponentMgr->Clone_Component(L"Prototype_Texture_MousePoint", COMPONENTID::ID_STATIC));
	NULL_CHECK_RETURN(m_pTexture, E_FAIL);
	m_mapComponent[ID_STATIC].emplace(L"Com_Texture", m_pTexture);

	// TransCom 
	m_pTransCom = static_cast<CTransform*>(m_pComponentMgr->Clone_Component(L"Prototype_Transform", COMPONENTID::ID_DYNAMIC));
	if (nullptr != m_pTransCom) 
		m_mapComponent[ID_DYNAMIC].emplace(L"Com_Transform", m_pTransCom);

	return S_OK;
}

void CMousePoint::Set_ConstantTable()
{
	_matrix matView = INIT_MATRIX;
	_matrix matProj = INIT_MATRIX;

	CB_MATRIX_INFO	tCB_MatrixInfo;
	ZeroMemory(&tCB_MatrixInfo, sizeof(CB_MATRIX_INFO));

	_matrix matWVP = m_pTransCom->m_matWorld * matView * matProj;
	XMStoreFloat4x4(&tCB_MatrixInfo.matWVP, XMMatrixTranspose(matWVP));
	XMStoreFloat4x4(&tCB_MatrixInfo.matWorld, XMMatrixTranspose(m_pTransCom->m_matWorld));
	XMStoreFloat4x4(&tCB_MatrixInfo.matView, XMMatrixTranspose(matView));
	XMStoreFloat4x4(&tCB_MatrixInfo.matProj, XMMatrixTranspose(matProj));

	m_pShaderCom->Get_UploadBuffer_MatrixInfo()->CopyData(0, tCB_MatrixInfo);
}

CGameObject* CMousePoint::Clone_GameObject(void* pArg)
{
	CGameObject* pInstance = new CMousePoint(*this);

	if (FAILED(pInstance->Ready_GameObject()))
		return nullptr;

	return pInstance;
}

CMousePoint* CMousePoint::Create(ID3D12Device* pGraphicDevice, ID3D12GraphicsCommandList* pCommandList)
{
	CMousePoint* pInstance = new CMousePoint(pGraphicDevice, pCommandList);

	if (FAILED(pInstance->Ready_GameObjectPrototype()))
		Engine::Safe_Release(pInstance);

	return pInstance;
}

void CMousePoint::Free()
{
	CGameObject::Free();
}
