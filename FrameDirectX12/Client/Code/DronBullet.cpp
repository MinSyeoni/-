#include "stdafx.h"
#include "DronBullet.h"
#include "ObjectMgr.h"
#include "GraphicDevice.h"
#include "ColliderMgr.h"
#include "Frustom.h"

#include "Player.h"
#include "Effect.h"
#include "LightMgr.h"
CDronBullet::CDronBullet(ID3D12Device* pGraphicDevice, ID3D12GraphicsCommandList* pCommandList)
	: Engine::CGameObject(pGraphicDevice, pCommandList)
{
}

CDronBullet::CDronBullet(const CDronBullet& rhs)
	: Engine::CGameObject(rhs)
	, m_tMeshInfo(rhs.m_tMeshInfo)
{
}

CDronBullet::~CDronBullet()
{
}

HRESULT CDronBullet::Ready_GameObjectPrototype()
{

	return S_OK;
}

HRESULT CDronBullet::Ready_GameObject()
{
	NULL_CHECK_RETURN(m_pComponentMgr, E_FAIL);
	CGameObject::Ready_GameObject();

	FAILED_CHECK_RETURN(Add_Component(), E_FAIL);

	m_pTransCom->m_vPos = m_tMeshInfo.Pos;
	m_pTransCom->m_vAngle = ToDegree(m_tMeshInfo.Rotation);

	m_tagLight.m_eType = LIGHTTYPE::D3DLIGHT_POINT;
	m_tagLight.m_vDiffuse = _vec4{ 0.6f,0.0f,0.0f,1.0f };
	m_tagLight.m_vAmbient = _vec4{ 0.4f,0.4f,0.4f,1.0f };
	m_tagLight.m_vSpecular = _vec4{ 0.4f,0.4f,0.4f,1.0f };
	m_tagLight.m_vDirection = _vec4{ 1.0f,1.0f,-1.f,1.0f };
	m_tagLight.m_vPosition = _vec4{ 300.f,10.f,300.f,0.f };
	m_tagLight.m_fRange = 30.f;

	m_tagLight.m_vPosition.x = m_pTransCom->m_vPos.x;
	m_tagLight.m_vPosition.y = m_pTransCom->m_vPos.y;
	m_tagLight.m_vPosition.z = m_pTransCom->m_vPos.z;

	if (FAILED(CLight_Manager::Get_Instance()->Add_Light(m_pGraphicDevice, m_pCommandList, &m_tagLight)))
		return E_FAIL;

	m_uiLightIndex = CLight_Manager::Get_Instance()->Get_LightIndex();
	CLight_Manager::Get_Instance()->Set_LightOnOff(m_uiLightIndex, true);



	return S_OK;
}

HRESULT CDronBullet::LateInit_GameObject()
{
	//m_pShaderCom->Set_Shader_Texture(m_pMeshCom->Get_Texture(), m_pMeshCom->Get_NormalTexture(), m_pMeshCom->Get_SpecularTexture(), m_pMeshCom->Get_EmissiveTexture());
	m_pDronEffect = dynamic_cast<CEffect*>(CObjectMgr::Get_Instance()->Get_NewGameObject(L"Prototype_Effect_DronBullet", L"NONE", &m_pTransCom->m_vPos));
	return S_OK;
}

_int CDronBullet::Update_GameObject(const _float& fTimeDelta)
{
	FAILED_CHECK_RETURN(Engine::CGameObject::LateInit_GameObject(), E_FAIL);

	if (m_bIsDead)
	{
		m_pDronEffect->Dead_GameObject();
		m_pDronEffect->SetPos(m_pTransCom->m_vPos);
		m_pDronEffect->Update_GameObject(fTimeDelta);
		CLight_Manager::Get_Instance()->Set_LightOnOff(m_uiLightIndex, false);
		return DEAD_OBJ;

	}
	Engine::CGameObject::Update_GameObject(fTimeDelta);
	m_pBoxCollider->Update_Collider(&m_pTransCom->m_matWorld);
	CColliderMgr::Get_Instance()->Add_Collider(CColliderMgr::COMBAT, m_pBoxCollider);

	CGameObject* pPlayer = CObjectMgr::Get_Instance()->Get_GameObject(L"Layer_GameObject", L"Player");
	if (pPlayer == nullptr)
		return E_FAIL;
	
	m_vPlayerPos = pPlayer->Get_Transform()->m_vPos;
	m_vPlayerPos.y = 13.f;
	_vec3 vChaseDir = m_vPlayerPos - m_pTransCom->m_vPos;
	vChaseDir.Normalize();
	m_pTransCom->m_vPos = m_pTransCom->m_vPos + vChaseDir * 50.f * fTimeDelta;
	if (m_pDronEffect != nullptr)
	{
		m_pDronEffect->SetPos(m_pTransCom->m_vPos);
		m_pDronEffect->Update_GameObject(fTimeDelta);
	}
	_vec4 vLightPos; 
	memcpy(&vLightPos, &m_pTransCom->m_vPos, sizeof(_vec3));
	vLightPos.w = 1.f;

	m_tagLight.m_vPosition = vLightPos;
	CLight_Manager::Get_Instance()->Set_LightInfo(m_uiLightIndex, m_tagLight);


	return NO_EVENT;
}

_int CDronBullet::LateUpdate_GameObject(const _float& fTimeDelta)
{

	if (!m_bIsLateInit)
		return NO_EVENT;

	NULL_CHECK_RETURN(m_pRenderer, -1);
	if(m_pDronEffect!=nullptr)
	m_pDronEffect->LateUpdate_GameObject(fTimeDelta);

	FAILED_CHECK_RETURN(m_pRenderer->Add_ColliderGroup(m_pBoxCollider), -1);

	_vec3 vShaveDir;
	for (auto& pCol : CColliderMgr::Get_Instance()->Get_ColliderList(CColliderMgr::BOX, CColliderMgr::PLAYER))
	{
		if (!m_bIsDead && CMathMgr::Get_Instance()->Collision_OBB(pCol, m_pBoxCollider, &vShaveDir))
		{
			CGameObject* pPlayer = CObjectMgr::Get_Instance()->Get_GameObject(L"Layer_GameObject", L"Player");
			if (pPlayer == nullptr)
				return E_FAIL;
			dynamic_cast<CPlayer*>(pPlayer)->Set_FlameDamage(10);
			m_bIsDead = true;
		}
	}

	for (auto& pCol : CColliderMgr::Get_Instance()->Get_ColliderList(CColliderMgr::BOX, CColliderMgr::PLAYERVIEW))
	{
		if (!m_bIsDead && CMathMgr::Get_Instance()->Collision_OBB(pCol, m_pBoxCollider, &vShaveDir))
		{
			m_bIsDead = true;
		}
	}


	for (auto& pCol : CColliderMgr::Get_Instance()->Get_ColliderList(CColliderMgr::BOX, CColliderMgr::PASSBOX))
	{
		if (!m_bIsDead && CMathMgr::Get_Instance()->Collision_OBB(pCol, m_pBoxCollider, &vShaveDir))
		{
			m_bIsDead = true;
		}
	}


	return NO_EVENT;
}

void CDronBullet::Render_GameObject(const _float& fTimeDelta)
{
	if (m_bIsDead)
		return;

}



void CDronBullet::Render_ShadowDepth(CShader_Shadow* pShader)
{

}

HRESULT CDronBullet::Add_Component()
{
	NULL_CHECK_RETURN(m_pComponentMgr, E_FAIL);

	// Box
	m_pBoxCollider = static_cast<Engine::CBoxCollider*>(m_pComponentMgr->Clone_Collider(L"Prototype_BoxCol", COMPONENTID::ID_STATIC, CCollider::COL_BOX, false, nullptr, _vec3(0.f, 0.f, 0.f), _vec3(0.f, 0.f, 0.f), 0.f, _vec3(1.f, 1.f, 1.f), this));
	NULL_CHECK_RETURN(m_pBoxCollider, E_FAIL);
	m_mapComponent[ID_STATIC].emplace(L"Com_BoxCol", m_pBoxCollider);

	return S_OK;
}

void CDronBullet::Set_ConstantTable()
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

	//m_pShaderCom->Get_UploadBuffer_MatrixInfo()->CopyData(0, tCB_MatrixInfo);
}

void CDronBullet::Set_ShadowTable(CShader_Shadow* pShader)
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

CGameObject* CDronBullet::Clone_GameObject(void* prg)
{
	CGameObject* pInstance = new CDronBullet(*this);

	MeshInfo tMeshInfo = *reinterpret_cast<MeshInfo*>(prg);
	static_cast<CDronBullet*>(pInstance)->m_tMeshInfo = tMeshInfo;

	if (FAILED(pInstance->Ready_GameObject()))
		return nullptr;

	return pInstance;
}

CDronBullet* CDronBullet::Create(ID3D12Device* pGraphicDevice, ID3D12GraphicsCommandList* pCommandList)
{
	CDronBullet* pInstance = new CDronBullet(pGraphicDevice, pCommandList);

	if (FAILED(pInstance->Ready_GameObjectPrototype()))
		Engine::Safe_Release(pInstance);

	return pInstance;
}

void CDronBullet::Free()
{
	Safe_Release(m_pDronEffect);
	CGameObject::Free();
}
