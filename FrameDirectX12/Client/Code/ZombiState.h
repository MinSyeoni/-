#pragma once

#include "Include.h"
#include "Transform.h"
#include "NaviMesh.h"

namespace Engine
{
	class CTransform;
	class CMesh;
	class CBoxCollider;
	class CNaviMesh;
}

class CZombiState
{
public:
	enum ZOMBISTATE { ZOM_BasePose/*������X*/, ZOM_CB_Active/*������ õõ�� �������鼭 ����*/, ZOM_CB_CombatActive/*���� �ؿ��� ���ö����*/, ZOM_CB_CombatActive_Ceiling/*�ϴÿ��� ������ ����*/,
					  ZOM_CB_Idle/*���ߺξ�?X*/, ZOM_CB_IdlePose/*��������X*/, ZOM_DG_GetUpBack/*�����ִ� �Ͼ��*/, ZOM_DG_GetUpFront/*������ִ� �Ͼ��*/, ZOM_EX_IdleOffset/*������*/,
					  ZOM_EX_IdlePose/*��������X*/, ZOM_EX_Run/*�޸���*/, ZOM_EX_WalkSlow/*�ȱ�*/, ZOM_BC_Dead/*����, �� Ʀ*/, ZOM_Base_Pose2/*������X*/,
					  ZOM_LEFT_ATK/*���� ������*/, ZOM_Base_Pose3/*������X*/, ZOM_RIGHT_ATK/*������ ������*/};

public:
	CZombiState();
	virtual ~CZombiState();

public:
	void					Initialized();
	HRESULT					Late_Initialized();
	_int					Update_Zombi(const _float& fTimeDelta, CTransform* pTransform, CMesh* m_pMeshCom);
	_int					LateUpdate_Zombi(const _float& fTimeDelta, CTransform* pTransform, CMesh* m_pMeshCom);
	void					Animation_Test(const _float& fTimeDelta, CMesh* m_pMeshCom);
	void					Attak_Player(Engine::CMesh* m_pMeshCom, CZombiState::ZOMBISTATE eState);
	void					Release();

public:
	const ZOMBISTATE&		Get_CurState() { return m_eCurState; }
	const ZOMBISTATE&		Get_PreState() { return m_ePreState; }
	void					Set_CurState(ZOMBISTATE eState) { m_eCurState = eState; }
	void					Set_PreState(ZOMBISTATE eState) { m_ePreState = eState; }
	
	void					Set_Transform(CTransform* pTransform) { m_pTransCom = pTransform; m_pTransCom->AddRef(); }
	void					Set_NaviMesh(CNaviMesh* pNavimesh) { m_pNaviMesh = pNavimesh; m_pNaviMesh->AddRef(); }

	const _bool&			Get_IsDeadZombi() const { return m_bIsDead; }

private:
	void					Chase_Player(const _float& fTimeDelta);
	_bool					Check_PlayerRange(_float fRange);

private:
	ZOMBISTATE				m_eCurState;
	ZOMBISTATE				m_ePreState;

	CTransform*				m_pTransCom = nullptr;
	CNaviMesh*				m_pNaviMesh = nullptr;
	Engine::CMesh*			m_pMeshCom = nullptr;

private:
	_float					m_fTime = 0.f;
	_float					m_fAniTime = 0.f;
	_float					m_fSpeed = 0.f;
	_float					m_fAniDelay = 0.f;

	_vec3					m_vPlayerPos = _vec3(0.f, 0.f, 0.f);

	_bool					m_bIsTurn = false;
	_bool					m_bIsDead = false;
	_vec3					m_vChaseDir = _vec3(0.f, 0.f, 0.f);
};

