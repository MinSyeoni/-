#pragma once
#include "Include.h"
#include "GameObject.h"


class CWeapon : public CGameObject
{

public:
	enum WEAPONSTATE { DROP, EQUIP, BAG };

	enum WEAPONTYPE { PISTOL, RIFLE, SNIPER,NpcRifle };
protected:
	explicit CWeapon(ID3D12Device* pGraphicDevice, ID3D12GraphicsCommandList* pCommandList);
	explicit CWeapon(const CWeapon& rhs);
	virtual ~CWeapon();

public:
	virtual HRESULT AddComponent();

	 virtual void CreateShootEffect();
	virtual void	Render_GameObject(const _float& fTimeDelta);
	virtual void    Render_ShadowDepth(CShader_Shadow* pShader);
	virtual void    Render_LimLight(CShader_LimLight* pShader);

	void SetWeaponState(WEAPONSTATE eState) { m_eWeaponState = eState; };
	WEAPONTYPE Get_WeaponType() { return m_eWeaponType; };
	WEAPONSTATE Get_WeaponState(){return m_eWeaponState;};
	void      Reload();
	_int      GetMaxBulletCount() { return m_iMaxBullet; }
	_int      GetCurBulletCount() { return m_iCurBullet; }
	_int      Get_WeaponBulletCount() { return m_iCurBullet; };
	_int      Get_weaponMaxBulletCount() { return m_iMaxBullet; };
private:
	void			Set_ConstantTable();
	void            Set_ShadowTable(CShader_Shadow* pShader);
	void            Set_LimTable(CShader_LimLight* pShader);
protected:
	Engine::CMesh*				m_pMeshCom = nullptr;
	Engine::CShader_Mesh*       m_pShaderCom = nullptr;

protected:
	vector<vector<_matrix>> m_vecBoneMatirx;   

	WEAPONSTATE m_eWeaponState = DROP;

	WEAPONTYPE m_eWeaponType = RIFLE;

	_int m_iCurBullet;
	_int m_iMaxBullet;
	_int m_iFullBullet=0;

protected:
	virtual void			Free();


};