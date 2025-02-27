#include "stdafx.h"
#include "CardKey.h"
#include "ObjectMgr.h"
#include "DynamicCamera.h"
#include "GraphicDevice.h"
#include "ColliderMgr.h"
#include "Frustom.h"
#include "EquipUI.h"
#include "CardTagUI.h"
#include "CardKey.h"
#include "CardReader.h"
#include "PlayerStatus.h"

CCardReader::CCardReader(ID3D12Device* pGraphicDevice, ID3D12GraphicsCommandList* pCommandList)
	: Engine::CGameObject(pGraphicDevice, pCommandList)
{
}

CCardReader::CCardReader(const CCardReader& rhs)
	: Engine::CGameObject(rhs)
	, m_tMeshInfo(rhs.m_tMeshInfo)
{
}

CCardReader::~CCardReader()
{
}

HRESULT CCardReader::Ready_GameObjectPrototype()
{

	return S_OK;
}

HRESULT CCardReader::Ready_GameObject()
{
	NULL_CHECK_RETURN(m_pComponentMgr, E_FAIL);
	CGameObject::Ready_GameObject();

	FAILED_CHECK_RETURN(Add_Component(), E_FAIL);

	m_pTransCom->m_vPos = m_tMeshInfo.Pos;
	m_pTransCom->m_vScale = m_tMeshInfo.Scale;
	m_pTransCom->m_vAngle = ToDegree(m_tMeshInfo.Rotation);

	return S_OK;
}

HRESULT CCardReader::LateInit_GameObject()
{
	m_pShaderCom->Set_Shader_Texture(m_pMeshCom->Get_Texture(), m_pMeshCom->Get_NormalTexture(), m_pMeshCom->Get_SpecularTexture(), m_pMeshCom->Get_EmissiveTexture());

	return S_OK;
}

_int CCardReader::Update_GameObject(const _float& fTimeDelta)
{
	FAILED_CHECK_RETURN(Engine::CGameObject::LateInit_GameObject(), E_FAIL);

	if (m_bIsDead)
		return DEAD_OBJ;

	Engine::CGameObject::Update_GameObject(fTimeDelta);
	m_pBoxCol->Update_Collider(&m_pTransCom->m_matWorld);
	CColliderMgr::Get_Instance()->Add_Collider(CColliderMgr::OBJECT, m_pBoxCol);

	list<CGameObject*>* pEquipUIList = CObjectMgr::Get_Instance()->Get_OBJLIST(L"Layer_UI", L"EquipUI");
	for (auto& pSrc : *pEquipUIList)
		if (CEquipUI::E_CARDREADER == dynamic_cast<CEquipUI*>(pSrc)->Get_EquipType())
			m_pGameObject = dynamic_cast<CEquipUI*>(pSrc);

	if (!m_bIsReaderOn && m_bIsCollision && m_bIsEquipCard)
	{
		if (CDirectInput::Get_Instance()->KEY_DOWN(DIK_E))
		{
			m_bIsReaderOn = true;
			if (dynamic_cast<CEquipUI*>(m_pGameObject) != nullptr)
				dynamic_cast<CEquipUI*>(m_pGameObject)->Set_ShowUI(false);

			if (!m_bIsTagOnSound)
			{
				CSoundMgr::Get_Instance()->Play_Effect(L"cardTagOn.wav");
				m_bIsTagOnSound = true;
			}
			m_bIsEquipCard = false;
		}
	}

	return NO_EVENT;
}

void CCardReader::Collision_CardReader()
{
	_vec3 vShaveDir;
	for (auto& pCol : CColliderMgr::Get_Instance()->Get_ColliderList(CColliderMgr::BOX, CColliderMgr::PLAYER))
	{
		if (!m_bIsDead && CMathMgr::Get_Instance()->Collision_OBB(m_pBoxCol, pCol, &vShaveDir))
		{
			if (dynamic_cast<CEquipUI*>(m_pGameObject) != nullptr
				&& !m_bIsReaderOn)
				dynamic_cast<CEquipUI*>(m_pGameObject)->Set_ShowUI(true);
			m_bIsCollision = true;
		}
		else
		{
			if (dynamic_cast<CEquipUI*>(m_pGameObject) != nullptr)
				dynamic_cast<CEquipUI*>(m_pGameObject)->Set_ShowUI(false);
			m_bIsCollision = false;
		}
	}
}

_int CCardReader::LateUpdate_GameObject(const _float& fTimeDelta)
{
	if (!CFrustom::Get_Instance()->FrustomCulling(m_pMeshCom->Get_MeshComponent()->Get_MinPos(), m_pMeshCom->Get_MeshComponent()->Get_MaxPos(), m_pTransCom->m_matWorld))
		return NO_EVENT;

	NULL_CHECK_RETURN(m_pRenderer, -1);
	FAILED_CHECK_RETURN(m_pRenderer->Add_Renderer(CRenderer::RENDER_NONALPHA, this), -1);
	FAILED_CHECK_RETURN(m_pRenderer->Add_Renderer(CRenderer::RENDER_SHADOWDEPTH, this), -1);
	FAILED_CHECK_RETURN(m_pRenderer->Add_ColliderGroup(m_pBoxCol), -1);

	Collision_CardReader();
	if (m_bIsReaderOn)
	{
		CGameObject* pCardTagUI = CObjectMgr::Get_Instance()->Get_GameObject(L"Layer_UI", L"CardTag");
		if (nullptr != pCardTagUI)
			dynamic_cast<CCardTagUI*>(pCardTagUI)->Set_IsTagOn(true);
	}

	return NO_EVENT;
}

void CCardReader::Render_GameObject(const _float& fTimeDelta)
{
	Set_ConstantTable();

	m_pShaderCom->Begin_Shader();

	m_pMeshCom->Render_Mesh(m_pShaderCom);
}

void CCardReader::Render_ShadowDepth(CShader_Shadow* pShader)
{
	Set_ShadowTable(pShader);
	m_pMeshCom->Render_ShadowMesh(pShader);
	pShader->Set_ShadowFinish();
}

HRESULT CCardReader::Add_Component()
{
	NULL_CHECK_RETURN(m_pComponentMgr, E_FAIL);

	// Shader
	m_pShaderCom = static_cast<Engine::CShader_Mesh*>(m_pComponentMgr->Clone_Component(L"Prototype_Shader_Mesh", COMPONENTID::ID_STATIC));
	NULL_CHECK_RETURN(m_pShaderCom, E_FAIL);
	m_mapComponent[ID_STATIC].emplace(L"Com_Shader", m_pShaderCom);

	// Buffer
	m_pMeshCom = static_cast<Engine::CMesh*>(m_pComponentMgr->Clone_Component(m_tMeshInfo.MeshTag.c_str(), COMPONENTID::ID_STATIC));
	NULL_CHECK_RETURN(m_pMeshCom, E_FAIL);
	m_mapComponent[ID_STATIC].emplace(L"Com_Mesh", m_pMeshCom);

	// Box
	m_pBoxCol = static_cast<Engine::CBoxCollider*>(m_pComponentMgr->Clone_Collider(L"Prototype_BoxCol", COMPONENTID::ID_STATIC, CCollider::COL_BOX, true, m_pMeshCom, _vec3(0.f, 0.f, 0.f), _vec3(0.f, 0.f, 0.f), 0.f, _vec3(300.f, 300.f, 300.f), this));
	NULL_CHECK_RETURN(m_pBoxCol, E_FAIL);
	m_mapComponent[ID_STATIC].emplace(L"Com_BoxCol", m_pBoxCol);

	return S_OK;
}

void CCardReader::Set_ConstantTable()
{
	_matrix matView = INIT_MATRIX;
	_matrix matProj = INIT_MATRIX;

	CB_MATRIX_INFO	tCB_MatrixInfo;
	ZeroMemory(&tCB_MatrixInfo, sizeof(CB_MATRIX_INFO));

	matView = CGraphicDevice::Get_Instance()->GetViewMatrix();
	matProj = CGraphicDevice::Get_Instance()->GetProjMatrix();

	_matrix matWVP = m_pTransCom->m_matWorld * matView * matProj;

	XMStoreFloat4x4(&tCB_MatrixInfo.matWorld, XMMatrixTranspose(m_pTransCom->m_matWorld));
	XMStoreFloat4x4(&tCB_MatrixInfo.matView, XMMatrixTranspose(matView));
	XMStoreFloat4x4(&tCB_MatrixInfo.matProj, XMMatrixTranspose(matProj));
	XMStoreFloat4x4(&tCB_MatrixInfo.matWVP, XMMatrixTranspose(matWVP));

	m_pShaderCom->Get_UploadBuffer_MatrixInfo()->CopyData(0, tCB_MatrixInfo);
}

void CCardReader::Set_ShadowTable(CShader_Shadow* pShader)
{
	_matrix matView = INIT_MATRIX;
	_matrix matProj = INIT_MATRIX;

	CB_SHADOW_INFO	tCB_MatrixInfo;

	ZeroMemory(&tCB_MatrixInfo, sizeof(CB_SHADOW_INFO));

	matView = CFrustom::Get_Instance()->Get_LightView();
	matProj = CFrustom::Get_Instance()->Get_LightProj();
	XMStoreFloat4x4(&tCB_MatrixInfo.matWorld, XMMatrixTranspose(m_pTransCom->m_matWorld));
	XMStoreFloat4x4(&tCB_MatrixInfo.matView, XMMatrixTranspose(matView));
	XMStoreFloat4x4(&tCB_MatrixInfo.matProj, XMMatrixTranspose(matProj));
	tCB_MatrixInfo.blsMesh = true;

	_int offset = pShader->Get_CBMeshCount();
	pShader->Get_UploadBuffer_ShadowInfo()->CopyData(offset, tCB_MatrixInfo);
}

CGameObject* CCardReader::Clone_GameObject(void* prg)
{
	CGameObject* pInstance = new CCardReader(*this);

	MeshInfo tMeshInfo = *reinterpret_cast<MeshInfo*>(prg);
	static_cast<CCardReader*>(pInstance)->m_tMeshInfo = tMeshInfo;

	if (FAILED(pInstance->Ready_GameObject()))
		return nullptr;

	return pInstance;
}

CCardReader* CCardReader::Create(ID3D12Device* pGraphicDevice, ID3D12GraphicsCommandList* pCommandList)
{
	CCardReader* pInstance = new CCardReader(pGraphicDevice, pCommandList);

	if (FAILED(pInstance->Ready_GameObjectPrototype()))
		Engine::Safe_Release(pInstance);

	return pInstance;
}

void CCardReader::Free()
{
	CGameObject::Free();
}
