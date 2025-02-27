#include "stdafx.h"
#include "InvenUI.h"
#include "Font.h"
#include "HPBar.h"
#include "MPBar.h"
#include "Shepard.h"
#include "Ken.h"

CInvenUI::CInvenUI(ID3D12Device* pGraphicDevice, ID3D12GraphicsCommandList* pCommandList)
	: Engine::CGameObject(pGraphicDevice, pCommandList)
{
}

CInvenUI::CInvenUI(const CInvenUI& rhs)
	: Engine::CGameObject(rhs)
	, m_bIsShow(rhs.m_bIsShow)
{
}

CInvenUI::~CInvenUI()
{
}

HRESULT CInvenUI::Ready_GameObjectPrototype()
{
	return S_OK;
}

HRESULT CInvenUI::Ready_GameObject()
{
	Add_Component();

	return S_OK;
}

HRESULT CInvenUI::LateInit_GameObject()
{
	m_pShaderCom->Set_Shader_Texture(m_pTexture->Get_Texture());	

	for (int i = 0; i < 3; ++i)
		m_pFont[i]->LateInit_GameObject();

	return S_OK;
}

_int CInvenUI::Update_GameObject(const _float& fTimeDelta)
{
	FAILED_CHECK_RETURN(Engine::CGameObject::LateInit_GameObject(), E_FAIL);

	if (m_bIsDead)
		return DEAD_OBJ;

	Engine::CGameObject::Update_GameObject(fTimeDelta);

	Set_FontText();

	if (m_bIsShow)
		for (int i = 0; i < 3; ++i)
			m_pFont[i]->Update_GameObject(fTimeDelta);

	return NO_EVENT;
}

void CInvenUI::Set_FontText()
{
	wstring strText[3];
	string strTemp[3];

	for (int i = 0; i < 3; ++i)
	{
		strTemp[i] = to_string(m_iItemNum[i]);
		strText[i].assign(strTemp[i].begin(), strTemp[i].end());
	}

	for (int i = 0; i < 3; ++i)
	{
		m_pFont[i]->Set_Pos(_vec2(WINCX * 0.93f, WINCY * (0.65f + i * 0.07f)));
		m_pFont[i]->Set_Text(strText[i].c_str());
	}
}

_int CInvenUI::LateUpdate_GameObject(const _float& fTimeDelta)
{
	NULL_CHECK_RETURN(m_pRenderer, -1);

	if (m_bIsShow)
		for (int i = 0; i < 3; ++i)
			m_pFont[i]->LateUpdate_GameObject(fTimeDelta);

	FAILED_CHECK_RETURN(m_pRenderer->Add_Renderer(CRenderer::RENDER_UI, this), -1);

	Use_ItemBandage();

	return NO_EVENT;
}

void CInvenUI::Use_ItemBandage()
{
	if (CDirectInput::Get_Instance()->KEY_DOWN(DIK_F1))
	{
		if (m_iItemNum[0] > 0)		// 붕대
		{
			list<CGameObject*>* pHpBarUIList = CObjectMgr::Get_Instance()->Get_OBJLIST(L"Layer_UI", L"HPBarUI");
			CGameObject* pPlayer = CObjectMgr::Get_Instance()->Get_GameObject(L"Layer_GameObject", L"Player");
			_float fCurHP = dynamic_cast<CPlayer*>(pPlayer)->Get_CurHP();

			m_iItemNum[0]--;

			CSoundMgr::Get_Instance()->Play_Effect(L"UseBandage.mp3");

			if (pHpBarUIList != nullptr && fCurHP < 314)
			{
				for (auto& pSrc : *pHpBarUIList)
				{
					if (CHPBar::PLAYER_HPBAER == dynamic_cast<CHPBar*>(pSrc)->Get_CurHpState())
					{
						dynamic_cast<CPlayer*>(pPlayer)->Set_CurHP(60);
						dynamic_cast<CHPBar*>(pSrc)->Set_CurHpType(1);
					}
				}
			}
		}
	}
	else if (CDirectInput::Get_Instance()->KEY_DOWN(DIK_F2))
	{
		if (m_iItemNum[1] > 0)		// 약, 동료 체력 회복
		{
			CSoundMgr::Get_Instance()->Play_Effect(L"UseMedicine.mp3");

			CGameObject* pShepard = CObjectMgr::Get_Instance()->Get_GameObject(L"Layer_GameObject", L"Shepard");
			_float fCurShepardHP = dynamic_cast<CShepard*>(pShepard)->Get_ShepardCurHP();
			CGameObject* pKen = CObjectMgr::Get_Instance()->Get_GameObject(L"Layer_GameObject", L"Ken");
			_float fCurKenHP = dynamic_cast<CKen*>(pKen)->Get_KenCurHP();

			list<CGameObject*>* pHpBarUIList = CObjectMgr::Get_Instance()->Get_OBJLIST(L"Layer_UI", L"HPBarUI");
			if (pHpBarUIList != nullptr && (fCurShepardHP < 314 || fCurKenHP < 314))
			{
				for (auto& pSrc : *pHpBarUIList)
				{
					if (CHPBar::COLLEAGUE_HPBAR == dynamic_cast<CHPBar*>(pSrc)->Get_CurHpState()
						|| CHPBar::COLLEAGUE2_HPBAR == dynamic_cast<CHPBar*>(pSrc)->Get_CurHpState())
					{
						dynamic_cast<CKen*>(pKen)->Set_KenCurHP(30);
						dynamic_cast<CShepard*>(pShepard)->Set_ShepardCurHP(30);
						dynamic_cast<CHPBar*>(pSrc)->Set_CurHpType(1);
					}
				}
			}
			m_iItemNum[1]--;
		}
	}
	else if (CDirectInput::Get_Instance()->KEY_DOWN(DIK_F3))
	{
		if (m_iItemNum[2] > 0)		// 주사기, 기력
		{		
			m_iItemNum[2]--;
			CSoundMgr::Get_Instance()->Play_Effect(L"UseSyringe.mp3");

			CGameObject* pPlayer = CObjectMgr::Get_Instance()->Get_GameObject(L"Layer_GameObject", L"Player");
			_float fCurMP = dynamic_cast<CPlayer*>(pPlayer)->Get_CurMp();

			list<CGameObject*>* pMpBarUIList = CObjectMgr::Get_Instance()->Get_OBJLIST(L"Layer_UI", L"MPBarUI");
			if (pMpBarUIList != nullptr && fCurMP < 300)
			{
				for (auto& pSrc : *pMpBarUIList)
				{
					dynamic_cast<CPlayer*>(pPlayer)->Set_CurMP(300);
					dynamic_cast<CMPBar*>(pSrc)->Set_CurMpType(1);
				}
			}
		}
	}
	else if (CDirectInput::Get_Instance()->KEY_DOWN(DIK_F4))	// 기력 치트키. 주사기 없어도 기력 회복
	{
		CGameObject* pPlayer = CObjectMgr::Get_Instance()->Get_GameObject(L"Layer_GameObject", L"Player");
		_float fCurMP = dynamic_cast<CPlayer*>(pPlayer)->Get_CurMp();

		list<CGameObject*>* pMpBarUIList = CObjectMgr::Get_Instance()->Get_OBJLIST(L"Layer_UI", L"MPBarUI");
		if (pMpBarUIList != nullptr && fCurMP < 300)
		{
			for (auto& pSrc : *pMpBarUIList)
			{
				dynamic_cast<CPlayer*>(pPlayer)->Set_CurMP(300);
				dynamic_cast<CMPBar*>(pSrc)->Set_CurMpType(1);
			}
		}
	}
}

void CInvenUI::Render_GameObject(const _float& fTimeDelta)
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

HRESULT CInvenUI::Add_Component()
{
	NULL_CHECK_RETURN(m_pComponentMgr, E_FAIL);

	// Buffer
	m_pBufferCom = static_cast<Engine::CRcTex*>(m_pComponentMgr->Clone_Component(L"Prototype_RcTex", COMPONENTID::ID_STATIC));
	NULL_CHECK_RETURN(m_pBufferCom, E_FAIL);
	m_mapComponent[ID_STATIC].emplace(L"Com_Buffer", m_pBufferCom);

	// Shader
	m_pShaderCom = static_cast<Engine::CShader_UI*>(m_pComponentMgr->Clone_Component(L"Prototype_Shader_UI", COMPONENTID::ID_STATIC));
	NULL_CHECK_RETURN(m_pShaderCom, E_FAIL);
	m_mapComponent[ID_STATIC].emplace(L"Com_Shader", m_pShaderCom);

	// TransCom 
	m_pTransCom = static_cast<CTransform*>(m_pComponentMgr->Clone_Component(L"Prototype_Transform", COMPONENTID::ID_DYNAMIC));
	if (nullptr != m_pTransCom)
		m_mapComponent[ID_DYNAMIC].emplace(L"Com_Transform", m_pTransCom);

	// Font
	for (int i = 0; i < 3; ++i)
	{
		m_pFont[i] = static_cast<CFont*>(CObjectMgr::Get_Instance()->Get_NewGameObject(L"Prototype_Font_NetmarbleLight", L"fuck", nullptr));
		m_pFont[i]->Ready_GameObjectClone(L"", _vec2{ 0.f, 0.f }, D2D1::ColorF::White);
	}

	//Texture
	m_pTexture = static_cast<Engine::CTexture*>(m_pComponentMgr->Clone_Component(L"Prototype_Texture_MedicineIcon", COMPONENTID::ID_STATIC));
	NULL_CHECK_RETURN(m_pTexture, E_FAIL);
	m_mapComponent[ID_STATIC].emplace(L"Com_Texture", m_pTexture);

	return S_OK;
}

void CInvenUI::Set_ConstantTable()
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

CGameObject* CInvenUI::Clone_GameObject(void* pArg)
{
	CGameObject* pInstance = new CInvenUI(*this);

	if (FAILED(pInstance->Ready_GameObject()))
		return nullptr;

	return pInstance;
}

CInvenUI* CInvenUI::Create(ID3D12Device* pGraphicDevice, ID3D12GraphicsCommandList* pCommandList)
{
	CInvenUI* pInstance = new CInvenUI(pGraphicDevice, pCommandList);

	if (FAILED(pInstance->Ready_GameObjectPrototype()))
		Engine::Safe_Release(pInstance);

	return pInstance;
}

void CInvenUI::Free()
{
	CGameObject::Free();

	for (int i = 0; i < 3; ++i)
		Safe_Release(m_pFont[i]);
}
