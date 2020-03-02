#ifndef DynamicMesh_h__
#define DynamicMesh_h__

#include "Mesh.h"
#include "HierarchyLoader.h"
#include "AniCtrl.h"

BEGIN(Engine)

class ENGINE_DLL CDynamicMesh : public CMesh 
{
private:
	explicit CDynamicMesh(LPDIRECT3DDEVICE9 pGraphicDev);
	explicit CDynamicMesh(const CDynamicMesh& rhs);
	virtual ~CDynamicMesh(void);

public:
	const D3DXFRAME_DERIVED*		Get_FrameByName(const char* pFrameName);
	_bool Is_AnimationSetEnd(void);	

public:
	HRESULT		Ready_Mesh(const _tchar* pFilePath, const _tchar* pFileName);
	void		Render_Mesh(void);
	void		Render_Mesh(LPD3DXEFFECT pEffect);

	void	Set_AnimationSet(const _uint& iIndex);
	void	Play_Animation(const _float& fTimeDelta);

private:
	void	Update_FrameMatrix(D3DXFRAME_DERIVED* pFrame, const _matrix* pParentMatrix);
	void	SetUp_FrameMatrixPointer(D3DXFRAME_DERIVED* pFrame);

private:
	D3DXFRAME*				m_pRootFrame;
	CHierarchyLoader*		m_pLoader;
	CAniCtrl*				m_pAniCtrl;
	list<D3DXMESHCONTAINER_DERIVED*>		m_MeshContainerList;


public:
	static CDynamicMesh*	Create(LPDIRECT3DDEVICE9 pGraphicDev, const _tchar* pFilePath, const _tchar* pFileName);
	virtual CResources*		Clone(void);
	virtual void			Free(void);
};



END
#endif // DynamicMesh_h__


//typedef struct _D3DXFRAME
//{
//	LPSTR                   Name;			// ���� �̸�(x���Ͽ��� �о�� ���� �̸�)
//	D3DXMATRIX              TransformationMatrix; // ���� �ε� ���¿����� ���� ���� ���� ���
//
//	LPD3DXMESHCONTAINER     pMeshContainer;	//	�޽��� �����ϴ� �κ� ���� �Ǵ� �޽�
//
//	struct _D3DXFRAME       *pFrameSibling;			// ���� ���� �ּҸ� �����ϱ� ���� ������
//	struct _D3DXFRAME       *pFrameFirstChild;		// �ڽ� ���� �ּҸ� �����ϱ� ���� ������
//} D3DXFRAME, *LPD3DXFRAME;
//
//typedef struct _D3DXMESHCONTAINER
//{
//	LPSTR                   Name;				// �̸� : ���� ���Ǿ� ���� ����		
//
//	D3DXMESHDATA            MeshData;			// �������� �޽� �İ�ü ���� �� ������ �����صδ� ����ü
//
//	LPD3DXMATERIAL          pMaterials;			// �޽��� ���� ���� ������ �����ϱ� ���� �İ�ü
//	LPD3DXEFFECTINSTANCE    pEffects;			// ������� ����
//	DWORD                   NumMaterials;		// ������ ����(������� ����)
//	DWORD                  *pAdjacency;			// �̿��ϴ� ����(ù �ּҷ� ��� ������ ��ȸ�ϱ� ���� �����ͷ� �Ҵ���)
//
//	LPD3DXSKININFO          pSkinInfo;			// �о�鿩�� �޽��� ������ �������� ��Ű�׿� �ʿ��� ���� ��� �Լ��� ����
//
//	struct _D3DXMESHCONTAINER *pNextMeshContainer;
//} D3DXMESHCONTAINER, *LPD3DXMESHCONTAINER;
//
//typedef struct _D3DXMESHDATA
//{
//	D3DXMESHDATATYPE Type;
//
//	// current mesh data interface
//	union
//	{
//		LPD3DXMESH              pMesh;
//		LPD3DXPMESH             pPMesh;
//		LPD3DXPATCHMESH         pPatchMesh;
//	};
//} D3DXMESHDATA, *LPD3DXMESHDATA;