#include "stdafx.h"
#include "ReapearTube.h"
#include "ObjectMgr.h"
#include "DynamicCamera.h"
#include "GraphicDevice.h"
#include "ColliderMgr.h"
#include "Frustom.h"
#include "Reapear.h"
CReapearTube::CReapearTube(ID3D12Device* pGraphicDevice, ID3D12GraphicsCommandList* pCommandList)
	: Engine::CGameObject(pGraphicDevice, pCommandList)
{
}

CReapearTube::CReapearTube(const CReapearTube& rhs)
	: Engine::CGameObject(rhs)
{

}

CReapearTube::~CReapearTube()
{
}

HRESULT CReapearTube::Ready_GameObjectPrototype()
{
#ifdef _DEBUG
	COUT_STR("Ready Prototype RectObject");
#endif


	return S_OK;
}

HRESULT CReapearTube::Ready_GameObject(int iType)
{

	NULL_CHECK_RETURN(m_pComponentMgr, E_FAIL);
	CGameObject::Ready_GameObject();

	FAILED_CHECK_RETURN(Add_Component(), E_FAIL);
	// Buffer
	m_pMeshCom = static_cast<Engine::CMesh*>(m_pComponentMgr->Clone_Component(L"Mesh_Fluid", COMPONENTID::ID_STATIC));
	NULL_CHECK_RETURN(m_pMeshCom, E_FAIL);
	m_mapComponent[ID_STATIC].emplace(L"Com_Mesh", m_pMeshCom);
#ifdef _DEBUG
	COUT_STR("Success Static - Clone Mesh");
#endif

	m_pBoxCom = static_cast<Engine::CBoxCollider*>(m_pComponentMgr->Clone_Collider(L"Prototype_BoxCol", COMPONENTID::ID_STATIC, CCollider::COL_BOX, true, m_pMeshCom, _vec3(0.f, 0.f, 0.f), _vec3(0.f, 0.f, 0.f), 0.f, _vec3(300.f, 300.f, 300.f), this));
	NULL_CHECK_RETURN(m_pBoxCom, E_FAIL);
	m_mapComponent[ID_STATIC].emplace(L"Com_BoxCol", m_pBoxCom);

	m_pTransCom->m_vScale = _vec3(1.f, 1.f, 1.f);

	m_eTubePos = (TUBEPOS)iType;

	CGameObject* pReapear = CObjectMgr::Get_Instance()->Get_GameObject(L"Layer_GameObject", L"Reapear");

	CMesh* pMesh = dynamic_cast<CMesh*>(pReapear->Get_Component(L"Com_Mesh", ID_STATIC));

	if (m_eTubePos == LEFT)
     {m_pBoneMatrix = pMesh->Get_AnimationComponent()->Find_BoneMatrix("LeftWrist");
	m_pBoneMatrixOffset = pMesh->Get_AnimationComponent()->Find_BoneOffset("LeftWrist");
	}
	else
	{
		m_pBoneMatrix = pMesh->Get_AnimationComponent()->Find_BoneMatrix("RightWrist");
		m_pBoneMatrixOffset = pMesh->Get_AnimationComponent()->Find_BoneOffset("RightWrist");
	}
	m_pReapear = pReapear->Get_Transform();



	return S_OK;
}

HRESULT CReapearTube::LateInit_GameObject()
{
#ifdef _DEBUG
	COUT_STR("LateInit RectObject");
#endif

	/*____________________________________________________________________
	Get GameObject - DynamicCamera
	______________________________________________________________________*/


#ifdef _DEBUG
	COUT_STR("Success Get DynamicCamera");
#endif

	m_pShaderCom->Set_Shader_Texture(m_pMeshCom->Get_Texture(), m_pMeshCom->Get_NormalTexture(), m_pMeshCom->Get_SpecularTexture(), m_pMeshCom->Get_EmissiveTexture());

	return S_OK;
}

_int CReapearTube::Update_GameObject(const _float& fTimeDelta)
{
	FAILED_CHECK_RETURN(Engine::CGameObject::LateInit_GameObject(), E_FAIL);

	if (m_bIsDead)
		return DEAD_OBJ;
	if (m_bIsDrop)
		m_fAccTime += fTimeDelta * 500.f;

	if (m_fAccTime > 800.f)
		m_bIsDead = true;

	/*____________________________________________________________________
	TransCom - Update WorldMatrix.
	______________________________________________________________________*/
	Engine::CGameObject::Update_GameObject(fTimeDelta);

	_matrix matblend;
	_matrix Offset = XMMatrixInverse(nullptr, *m_pBoneMatrixOffset);
	_matrix matScale = XMMatrixScaling(1.f, 1.f, 1.f);
	matblend = Offset * *m_pBoneMatrix;
	m_pTransCom->m_matWorld =  matblend * m_pReapear->m_matWorld;
	m_pTransCom->m_matWorld._42 -= m_fAccTime;
	m_pBoxCom->Update_Collider(&m_pTransCom->m_matWorld);

	CColliderMgr::Get_Instance()->Add_Collider(CColliderMgr::BOSS, m_pBoxCom);
	return NO_EVENT;
}

_int CReapearTube::LateUpdate_GameObject(const _float& fTimeDelta)
{
	NULL_CHECK_RETURN(m_pRenderer, -1);

	/*____________________________________________________________________
	[ Renderer - Add Render Group ]
	______________________________________________________________________*/
	FAILED_CHECK_RETURN(m_pRenderer->Add_Renderer(CRenderer::RENDER_NONALPHA, this), -1);

	/*____________________________________________________________________
	[ Set PipelineState ]
	______________________________________________________________________*/
	FAILED_CHECK_RETURN(m_pRenderer->Add_ColliderGroup(m_pBoxCom), -1);

	return NO_EVENT;
}

void CReapearTube::Render_GameObject(const _float& fTimeDelta)
{
	Set_ConstantTable();

	m_pShaderCom->Begin_Shader();

	m_pMeshCom->Render_Mesh(m_pShaderCom);

}

void CReapearTube::Render_ShadowDepth(CShader_Shadow* pShader)
{
	Set_ShadowTable(pShader);
	m_pMeshCom->Render_ShadowMesh(pShader);
	pShader->Set_ShadowFinish();


}

void CReapearTube::SetDamage()
{
	if (m_eTubePos == LEFT)
	{
		CGameObject* pReapear = CObjectMgr::Get_Instance()->Get_GameObject(L"Layer_GameObject", L"Reapear");

		dynamic_cast<CReapear*>(pReapear)->SetLeftLock();
		m_bIsDrop = true;
	}
	else
	{

		CGameObject* pReapear = CObjectMgr::Get_Instance()->Get_GameObject(L"Layer_GameObject", L"Reapear");

		dynamic_cast<CReapear*>(pReapear)->SetRightLock();
		m_bIsDrop = true;
	}


}

HRESULT CReapearTube::Add_Component()
{
	NULL_CHECK_RETURN(m_pComponentMgr, E_FAIL);


	// Shader
	m_pShaderCom = static_cast<Engine::CShader_Mesh*>(m_pComponentMgr->Clone_Component(L"Prototype_Shader_Mesh", COMPONENTID::ID_STATIC));
	NULL_CHECK_RETURN(m_pShaderCom, E_FAIL);
	m_mapComponent[ID_STATIC].emplace(L"Com_Shader", m_pShaderCom);
#ifdef _DEBUG
	COUT_STR("Success RectObject - Clone ShaderCom");
#endif

	return S_OK;
}

void CReapearTube::Set_ConstantTable()
{
	_matrix matView = INIT_MATRIX;
	_matrix matProj = INIT_MATRIX;

	CB_MATRIX_INFO	tCB_MatrixInfo;
	ZeroMemory(&tCB_MatrixInfo, sizeof(CB_MATRIX_INFO));


	matView = CGraphicDevice::Get_Instance()->GetViewMatrix();
	matProj = CGraphicDevice::Get_Instance()->GetProjMatrix();


	_matrix matWVP = m_pTransCom->m_matWorld * matView * matProj;
	XMStoreFloat4x4(&tCB_MatrixInfo.matWVP, XMMatrixTranspose(matWVP));
	XMStoreFloat4x4(&tCB_MatrixInfo.matWorld, XMMatrixTranspose(m_pTransCom->m_matWorld));
	XMStoreFloat4x4(&tCB_MatrixInfo.matView, XMMatrixTranspose(matView));
	XMStoreFloat4x4(&tCB_MatrixInfo.matProj, XMMatrixTranspose(matProj));

	m_pShaderCom->Get_UploadBuffer_MatrixInfo()->CopyData(0, tCB_MatrixInfo);
}

void CReapearTube::Set_ShadowTable(CShader_Shadow* pShader)
{

	_matrix matRotY = XMMatrixRotationY(XMConvertToRadians(-90));

	_matrix matView = INIT_MATRIX;
	_matrix matProj = INIT_MATRIX;

	CB_SHADOW_INFO	tCB_MatrixInfo;

	ZeroMemory(&tCB_MatrixInfo, sizeof(CB_SHADOW_INFO));

	matView = CFrustom::Get_Instance()->Get_LightView();
	matProj = CFrustom::Get_Instance()->Get_LightProj();
	XMStoreFloat4x4(&tCB_MatrixInfo.matWorld, XMMatrixTranspose(matRotY * m_pTransCom->m_matWorld));
	XMStoreFloat4x4(&tCB_MatrixInfo.matView, XMMatrixTranspose(matView));
	XMStoreFloat4x4(&tCB_MatrixInfo.matProj, XMMatrixTranspose(matProj));
	tCB_MatrixInfo.blsMesh = true;


	_int offset = pShader->Get_CBMeshCount();
	pShader->Get_UploadBuffer_ShadowInfo()->CopyData(offset, tCB_MatrixInfo);




}

CGameObject* CReapearTube::Clone_GameObject(void* prg)
{
	CGameObject* pInstance = new CReapearTube(*this);

	int iType = *reinterpret_cast<_int*>(prg);

	if (FAILED(static_cast<CReapearTube*>( pInstance)->Ready_GameObject(iType)))
		return nullptr;

	return pInstance;
}

CReapearTube* CReapearTube::Create(ID3D12Device* pGraphicDevice, ID3D12GraphicsCommandList* pCommandList)
{
	CReapearTube* pInstance = new CReapearTube(pGraphicDevice, pCommandList);

	if (FAILED(pInstance->Ready_GameObjectPrototype()))
		Engine::Safe_Release(pInstance);

	return pInstance;
}

void CReapearTube::Free()
{

	CGameObject::Free();


}
