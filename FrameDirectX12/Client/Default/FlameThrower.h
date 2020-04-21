#pragma once

#include "Include.h"
#include "Transform.h"

namespace Engine
{
	class CTransform;
}

class CFlameThrower
{
public:
	enum FLAMESTATE { FLAME_IDLE, BC_Start/*���ӽ��*/, CB_Enter/*�ѹ߽��*/, CB_Exit/*��� ���� ����?*/, CB_FireLoop/*ȭ�����*/,
					CB_FireRecoil/*�����𸣰���*/, CB_FireStart/*�����𸣰���*/, CB_Idle/*������*/, CB_Twitch/*�θ����θ���*/, CB_WalkDown/*?��������*/,
					CB_WalkEast/*��*/, CB_WalkNorth/*��*/, CB_WalkSouth/*��*/, CB_WalkUp/*��*/, CB_WalkWest/*??*/ };

public:
	CFlameThrower();
	virtual ~CFlameThrower();

public:
	void					Initialized();
	HRESULT					Late_Initialized();
	_int					Update_FlameThrower(const _float& fTimeDelta, CTransform* pTransform);
	_int					LateUpdate_FlameThrower(const _float& fTimeDelta, CTransform* pTransform);
	void					Release();

public:
	const FLAMESTATE&		Get_CurState() { return m_eCurState; }
	const FLAMESTATE&		Get_PreState() { return m_ePreState; }
	void					Set_CurState(FLAMESTATE eState) { m_eCurState = eState; }
	void					Set_PreState(FLAMESTATE eState) { m_ePreState = eState; }
	
	void					Set_Transform(CTransform* pTransform) { m_pTransCom = pTransform; m_pTransCom->AddRef(); };

private:
	FLAMESTATE				m_eCurState;
	FLAMESTATE				m_ePreState;

	CTransform*				m_pTransCom = nullptr;

	float					m_fTime = 0.f;
	float					m_fSpineAngle = 0.f;
	float					m_fAniTime = 0.f;

	_int					m_iRandState = 0;
};

