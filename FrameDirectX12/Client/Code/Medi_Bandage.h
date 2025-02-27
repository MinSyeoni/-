#pragma once
#include "Include.h"
#include "GameObject.h"
#include "BoxCollider.h"
#include "DirectInput.h"

namespace Engine
{
	class CBoxCollider;
	class CStaticMesh;
	class CShader_Mesh;
}

class CDynamicCamera;

class CMedi_Bandage : public Engine::CGameObject
{
private:
	explicit CMedi_Bandage(ID3D12Device* pGraphicDevice, ID3D12GraphicsCommandList* pCommandList);
	explicit CMedi_Bandage(const CMedi_Bandage& rhs);
	virtual ~CMedi_Bandage();

public:
	HRESULT						Ready_GameObjectPrototype();
	virtual HRESULT				Ready_GameObject();
	virtual HRESULT				LateInit_GameObject();
	virtual _int				Update_GameObject(const _float& fTimeDelta);
	virtual _int				LateUpdate_GameObject(const _float& fTimeDelta);
	virtual void				Render_GameObject(const _float& fTimeDelta);
	virtual void				SetMeshInfo(MeshInfo tInfo) { m_tMeshInfo = tInfo; }

private:
	void						Check_ItemAndMouse();
	virtual HRESULT				Add_Component();
	void						Set_ConstantTable();
    void						Set_ShadowTable(CShader_Shadow* pShader);
	void						Render_ShadowDepth(CShader_Shadow* pShader);

private:
	void						ReScale_SphereCol();

private:
	Engine::CMesh*				m_pMeshCom = nullptr;
	Engine::CShader_Mesh*       m_pShaderCom = nullptr;
	Engine::CBoxCollider*		m_pBoxCollider = nullptr;
	Engine::CSphereCollider*	m_pShereCol = nullptr;

	MeshInfo					m_tMeshInfo;
	_bool						m_bIsEquip = false;
	_bool						m_bIsClick = false;
	CGameObject*				m_pGameObject = nullptr;

public:
	virtual CGameObject*		Clone_GameObject(void* prg);
	static CMedi_Bandage*		Create(ID3D12Device* pGraphicDevice,
								ID3D12GraphicsCommandList* pCommandList);

private:
	virtual void				Free();
};

