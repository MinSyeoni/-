#pragma once

#include "Base.h"
#include "Target.h"

BEGIN(Engine)

class ENGINE_DLL CTarget_Manager final : public CBase
{
	DECLARE_SINGLETON(CTarget_Manager)

private:
	explicit CTarget_Manager();
	virtual ~CTarget_Manager() = default;

public:
	HRESULT Add_Target(ID3D12Device* pGraphicDevice, ID3D12GraphicsCommandList* pCommandList, const _tchar* pTargetTag, const _uint& iWidth, const _uint& iHeight, DXGI_FORMAT format, _vec4 ClearColor);
	HRESULT Add_MRT(const _tchar* pMRTTag, const _tchar* pTargetTag);
	
public:
	HRESULT Begin_MRT(const _tchar* pMRTTag);
	HRESULT End_MRT(const _tchar* pMRTTag);


private:
	map<const _tchar*, CTarget*>				m_mapTarget;
	typedef map<const _tchar*, CTarget*>		MAPTARGET;

private: // MRT ���� ����: ���ÿ� ��ġ�� �µǾ���ϴ� Ÿ�ٵ��� �׷�ȭ�Ѵ�.
	map<const _tchar*, list<CTarget*>>			m_mapMRT;
	typedef map<const _tchar*, list<CTarget*>>	MAPMRT;

private:
	CTarget* Find_Target(const _tchar* pTargetTag);
	list<CTarget*>* Find_MRT(const _tchar* pMRTTag);

public:
	virtual void Free();

};

END