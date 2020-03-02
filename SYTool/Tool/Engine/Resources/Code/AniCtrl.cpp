#include "AniCtrl.h"

USING(Engine)

Engine::CAniCtrl::CAniCtrl(LPD3DXANIMATIONCONTROLLER pAniCtrl)
	: m_pAniCtrl(pAniCtrl)
	, m_iCurrentTrack(0)
	, m_iNewTrack(1)
	, m_fAccTime(0.f)
	, m_iOldAniIndex(99)
	, m_dPeriod(0.0)
{
	m_pAniCtrl->AddRef();
}

Engine::CAniCtrl::CAniCtrl(const CAniCtrl& rhs)
	: m_iCurrentTrack(rhs.m_iCurrentTrack)
	, m_iNewTrack(rhs.m_iNewTrack)
	, m_fAccTime(rhs.m_fAccTime)
	, m_iOldAniIndex(rhs.m_iOldAniIndex)
	, m_dPeriod(0.0)
{
	// �ִϸ��̼��� ������ �Ǿ �ȵǱ� ������ �Լ��� ���� ������ �Ѵ�.
	rhs.m_pAniCtrl->CloneAnimationController(rhs.m_pAniCtrl->GetMaxNumAnimationOutputs(),  // ���� �� ���� ��ü���� �����ǰ� �ִ� �ִϸ��̼��� ����
											rhs.m_pAniCtrl->GetMaxNumAnimationSets(),      // ���� ������ �ִϸ��̼��� �ִ� ����(1���ڿ� 2������ ���� ����)
											rhs.m_pAniCtrl->GetMaxNumTracks(),			   // ��� ������ �ִ� Ʈ���� ����(��κ� Ʈ���� �� �� �����, ���� ���ƺ��� �� ���� ���)
											rhs.m_pAniCtrl->GetMaxNumEvents(), // �޽��� ���ϰ� �ִ� ���� �̺�Ʈ, �޽��� ��Ư�� ȿ��(���� �츮�� ������� ���Ѵ�)
											&m_pAniCtrl);			   
	// Ʈ�� ���� �����ϰ��� �ϴ� �ϳ��� ������ �ø���, Ʈ���� ����϶�� ����ϴ� �ý���.
	// �ϳ��� ������ ������ �ִϸ��̼� set�̶� ���Ѵ�. �������� ��

}

Engine::CAniCtrl::~CAniCtrl(void)
{

}

HRESULT Engine::CAniCtrl::Ready_AniCtrl(void)
{
	return S_OK;
}

void Engine::CAniCtrl::Set_AnimationSet(const _uint& iIndex)
{
	if (m_iOldAniIndex == iIndex)
		return;

	m_iNewTrack = m_iCurrentTrack == 0 ? 1 : 0;

	LPD3DXANIMATIONSET		pAS = NULL;
	// �ε����� �ش��ϴ� �ִϸ��̼��� ���� ��ü �־��ִ� �Լ�
	m_pAniCtrl->GetAnimationSet(iIndex, &pAS); 
	// m_pAniCtrl->GetAnimationSetByName(); ���ڿ��� �̿��Ͽ� �ִϸ��̼� ������ ������ �Լ�
	
	
	// �ð� ���� ��ȯ(���� �ִϸ��̼� Ʈ���� �����ǰ� ��ġ�ϴ� �� == ���� �ִϸ��̼� ���� ��ü ��� �ð�) 
	m_dPeriod = pAS->GetPeriod();
	
	
	// Ʈ�� ���� ����ϰ����ϴ� �ִϸ��̼� ���� �����ϴ� �Լ�
	m_pAniCtrl->SetTrackAnimationSet(m_iNewTrack, pAS);

	// ������� �ʴ� �ʰ� �ִ� �̺�Ʈ �������� �ƿ� �����Ѵ�.(���� : �̺�Ʈ ���� ������ ��Ȥ ������ ���� �ʴ� ��� �߻�)
	m_pAniCtrl->UnkeyAllTrackEvents(m_iCurrentTrack);
	m_pAniCtrl->UnkeyAllTrackEvents(m_iNewTrack);

	// Ű ������ : �ִϸ��̼� ��� ���� Ư�� ������ �ǹ�, ���� ��� ���� ��� �߿� �Լ��� �ɸ��� ������ ������ �ϳ��� Ű �������̶� ���� �� �ִ�.
	// �ִϸ��̼� �޽��� ��� ��� Ű �������� ������ ���� ����(���� ���Ƽ�)
	// �� �� ���۸� Ű ���������� ���ּܵ� ���� �� ������ ���� �ڿ������� �ִϸ��̼��� �����ȴ�.

	// ���� �� �Լ��� ���� ����Ǿ����� ����� �������� �Լ� (3���� : �������� Ʈ������ �ִϸ��̼��� ������ ���ΰ�)
	// �������� ��� �Ǽ��� double���� ���
	m_pAniCtrl->KeyTrackEnable(m_iCurrentTrack, FALSE, m_fAccTime + 0.25);

	// �ִϸ��̼� ��� �ӵ�(���� �ڵ� �ٰŷδ� ���� �ӵ��� �����ϴ� �Լ�, �ӵ��� ��� ���� 1)
	m_pAniCtrl->KeyTrackSpeed(m_iCurrentTrack, 1.f, m_fAccTime, 0.25, D3DXTRANSITION_LINEAR);

	// �ռ� �ÿ� � Ʈ���� �ִ� Ű �����ӿ� ������ ������ ������ �����ϴ� �Լ�(2���� : ����ġ ��, ���� ���� 1)
	m_pAniCtrl->KeyTrackWeight(m_iCurrentTrack, 0.1f, m_fAccTime, 0.25, D3DXTRANSITION_LINEAR);


	m_pAniCtrl->SetTrackEnable(m_iNewTrack, TRUE);
	m_pAniCtrl->KeyTrackSpeed(m_iNewTrack, 1.f, m_fAccTime, 0.25, D3DXTRANSITION_LINEAR);
	m_pAniCtrl->KeyTrackWeight(m_iNewTrack, 0.9f, m_fAccTime, 0.25, D3DXTRANSITION_LINEAR);

	m_pAniCtrl->ResetTime();
	m_fAccTime = 0.f;

	// Ʈ���� Ȱ��ȭ �ϴ� �Լ�(�ִϸ��̼� ���� ����ϴ� �Լ��� �ƴϴ�. )
	m_pAniCtrl->SetTrackPosition(m_iNewTrack, 0.0);

	m_iOldAniIndex = iIndex;

	m_iCurrentTrack = m_iNewTrack;


	//pAS->GetPeriod(); // �ð� ���� ��ȯ�ϴ� �Լ�(���� �ִϸ��̼� ���� Ʈ���� �����ǰ� ��ġ�ϴ� ��)
	//m_pAniCtrl->GetTrackDesc(); // ������ Ʈ�� ������ ��ȯ�ϴ� �Լ�

}

void Engine::CAniCtrl::Play_Animation(const _float& fTimeDelta)
{
	// �ִϸ��̼� ���� ����ϴ� �Լ�
	m_pAniCtrl->AdvanceTime(fTimeDelta, NULL); // 2���� : �ִϸ��̼� ���ۿ� ���� ���峪 ����Ʈ�� ����ϴ� ��ü �ּ�(fmodó�� ���� ���� ����ϴ� ���� ����. ����� ������ ���ϰ� ������ �ڵ��� ���߸� Ŀ����)
	m_fAccTime += fTimeDelta;
}

CAniCtrl* Engine::CAniCtrl::Create(LPD3DXANIMATIONCONTROLLER pAniCtrl)
{
	CAniCtrl*			pInstance = new CAniCtrl(pAniCtrl);

	if (FAILED(pInstance->Ready_AniCtrl()))
		Safe_Release(pInstance);

	return pInstance;
}

CAniCtrl* Engine::CAniCtrl::Create(const CAniCtrl& rhs)
{
	CAniCtrl*	pInstance = new CAniCtrl(rhs);

	if (FAILED(pInstance->Ready_AniCtrl()))
		Safe_Release(pInstance);

	return pInstance;
}

void Engine::CAniCtrl::Free(void)
{
	Safe_Release(m_pAniCtrl);
}

_bool Engine::CAniCtrl::Is_AnimationSetEnd(void)
{
	D3DXTRACK_DESC			TrackInfo;
	ZeroMemory(&TrackInfo, sizeof(D3DXTRACK_DESC));

	// ���� Ʈ���� ������ ������ �ִ� �Լ�

	m_pAniCtrl->GetTrackDesc(m_iCurrentTrack, &TrackInfo);

	if (TrackInfo.Position >= m_dPeriod - 0.1)
		return true;

	return false;
}

