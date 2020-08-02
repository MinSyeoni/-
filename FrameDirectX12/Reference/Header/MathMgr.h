#pragma once
#include "Engine_Include.h"
#include "Base.h"

BEGIN(Engine)
class CCollider;

typedef struct tagOBB
{
	_vec3			vPoint[8];		// ť�긦 �̷�� ����
	_vec3			vCenter;		// ť���� ���� ����
	_vec3			vProjAxis[3];	// ť���� ���Ϳ��� �� ������ ������� ���� ����
	_vec3			vAxis[3];		// ť���� �� ��� �����ϴ� ������ �� ����

}OBB;

typedef struct tagRay
{
	_vec3 vOri;
	_vec3 vDir;
}RAY;

class ENGINE_DLL CMathMgr : public CBase
{
	DECLARE_SINGLETON(CMathMgr)

private:
	explicit CMathMgr();
	virtual ~CMathMgr();
public:
	_bool	Collision_AABB(CCollider* pDstCollider, CCollider * pSrcCollider, _vec3* vDir);
	_bool	Collision_OBB(CCollider* pDstCollider, CCollider * pSrcCollider, _vec3* vDir);
	_bool	Collision_Spere(CCollider* pDstCollider, CCollider * pSrcCollider, _vec3* vDir);
	_int    Collision_Ray(CCollider* pDstCollider);

	_bool   Collision_SpereWithMousePoint(CCollider* pDstCollider, HWND hwnd,float* fDist);
	_bool   Collision_BoxWithMousePoint(CCollider* pDstCollider,HWND hwnd,float* fDist);

	_bool   Collision_BoXWithCamera(CCollider* pDstCollider, float* fDist);
	_bool   Collision_SphereWithCamera(CCollider* pDstCollider, float* fDist);


public:
	void	Set_Point(OBB* pObb, const _vec3* pMin, const _vec3* pMax);
	void	Set_Axis(OBB* pObb);

	RAY m_tRay;
private:
	virtual void Free();
};

END
