#pragma once

#include "Include.h"
#include "Transform.h"
#include "NaviMesh.h"
#include "BoxCollider.h"
#include "Astar.h"

namespace Engine
{
	class CTransform;
	class CMesh;
	class CBoxCollider;
	class CNaviMesh;
	class CAstar;
}

class CZombi
{
public:
	enum ZOMBISTATE { ZOM_BasePose/*������X*/, ZOM_CB_Active/*������ õõ�� �������鼭 ����*/, ZOM_CB_CombatActive/*���� �ؿ��� ���ö����*/, ZOM_CB_CombatActive_Ceiling/*�ϴÿ��� ������ ����*/,
					  ZOM_CB_Idle/*���ߺξ�?X*/, ZOM_CB_IdlePose/*��������X*/, ZOM_DG_GetUpBack/*�����ִ� �Ͼ��*/, ZOM_DG_GetUpFront/*������ִ� �Ͼ��*/, ZOM_EX_IdleOffset/*������*/,
					  ZOM_EX_IdlePose/*��������X*/, ZOM_EX_Run/*�޸���*/, ZOM_EX_WalkSlow/*�ȱ�*/, ZOM_BC_Dead/*����, �� Ʀ*/, ZOM_Base_Pose2/*������X*/,
					  ZOM_LEFT_ATK/*���� ������*/, ZOM_Base_Pose3/*������X*/, ZOM_RIGHT_ATK/*������ ������*/};

public:
	CZombi();
	virtual ~CZombi();

public:
	void					Initialized();
	HRESULT					Late_Initialized();
	_int					Update_Zombi(const _float& fTimeDelta, CTransform* pTransform, CMesh* m_pMeshCom);
	_int					LateUpdate_Zombi(const _float& fTimeDelta, CTransform* pTransform, CMesh* m_pMeshCom);
	void					Animation_Test(const _float& fTimeDelta, CMesh* m_pMeshCom);
	void					Attak_Player(Engine::CMesh* m_pMeshCom, CZombi::ZOMBISTATE eState);
	void					Release();

public:
	const ZOMBISTATE&		Get_CurState() { return m_eCurState; }
	const ZOMBISTATE&		Get_PreState() { return m_ePreState; }
	void					Set_CurState(ZOMBISTATE eState) { m_eCurState = eState; }
	void					Set_PreState(ZOMBISTATE eState) { m_ePreState = eState; }
	
	void					Set_Transform(CTransform* pTransform) { m_pTransCom = pTransform; m_pTransCom->AddRef(); }
	void					Set_NaviMesh(CNaviMesh* pNavimesh) { m_pNaviMesh = pNavimesh; m_pNaviMesh->AddRef(); }
	void					Set_Astar(CAstar* pAstar) { m_pAstarCom = pAstar; m_pAstarCom->AddRef(); }

public:		// ��ȣ�ۿ� 
	const _bool&			Get_IsDeadZombi() const { return m_bIsZombiState[1]; }
	const _bool&			Get_IsHit() const { return m_bIsZombiState[2]; }
	const _bool&			Get_IsAtkPlayer() const { return m_bIsZombiState[3]; }
	void					Set_IsHit(_bool bIsHit) { m_bIsZombiState[2] = bIsHit; }
	const _float&			Get_CurHp() { return m_fCurHp; }	
	void					Set_HitDamage(_float fDamage) { m_fHitDamage = fDamage; }
	const _float&			Get_AtkDamage() { return m_fAtkDamage; }
	void					Set_InitAni(_uint iAni) { m_iInitAni = iAni; }
	void					Set_InitDrawID(_uint iID) { m_iDrawID = iID; }
	_uint					Get_DrawID() { return m_iDrawID; }

private:
	void					MoveByAstar(const _float& fTimeDelta);
	void					Update_ZombiHP();
	void					Chase_Player(const _float& fTimeDelta);
	_bool					Check_PlayerRange(_float fRange);

private:
	ZOMBISTATE				m_eCurState;
	ZOMBISTATE				m_ePreState;

	CTransform*				m_pTransCom = nullptr;
	CNaviMesh*				m_pNaviMesh = nullptr;
	CAstar*					m_pAstarCom = nullptr;
	Engine::CMesh*			m_pMeshCom = nullptr;

private:
//	_float					m_fTime = 0.f;
	_float					m_fAniTime = 0.f;
	_float					m_fAniDelay = 0.f;

	_vec3					m_vPlayerPos = _vec3(0.f, 0.f, 0.f);
	_vec3					m_vChaseDir = _vec3(0.f, 0.f, 0.f);

	_bool					m_bIsZombiState[4] = {false};	// 0=m_bIsTurn, 1=m_bIsDead, 2=m_bIsHit, 3=m_bIsATK;

	_float					m_fHitDamage = 0.f; // ������
	_float					m_fAtkDamage = 0.f; // ������
	_float					m_fSpeed = 0.f;
	_float					m_fCurHp = 0.f;
	_float					m_fMaxHp = 0.f;

	_uint					m_iInitAni;
	_uint					m_iDrawID;

	//�������߰�����
	_bool m_bIsDeadSound = false;
};

