﻿#pragma once


// CMapTab 대화 상자
#include "Terrain.h"
#include "MainFrm.h"
#include "ToolView.h"
#include "StaticObject.h"

BEGIN(Engine)
class CComponent;
class CToolCollider;
class CSphereCollider;
class CBoxCollider;
END

class CToolView;
class CMapTab : public CDialogEx
{
	DECLARE_DYNAMIC(CMapTab)

public:
	CMapTab(CWnd* pParent = nullptr);   // 표준 생성자입니다.
	virtual ~CMapTab();

// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG1 };
#endif

protected:
	virtual void			DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

	DECLARE_MESSAGE_MAP()

public:
	afx_msg void			OnBnClickedWireMode();
	afx_msg void			OnBnClickedSolidMode();
	afx_msg void			OnBnClickedCreateTerrain();
	afx_msg void			OnBnClickedSetTexture();
	afx_msg void			OnBnClickedTextureMode();
	afx_msg void			OnBnClickedHeightMode();
	afx_msg void			OnBnClickedSplattingMode();

	afx_msg void			OnBnClickedStaticWireMode();
	afx_msg void			OnBnClickedStaticSolidMode();
	afx_msg void			OnBnClickedStaticCreateMode();
	afx_msg void			OnBnClickedStaticModyMode();
	afx_msg void			OnBnClickedStaticDeleteMode();
	afx_msg void			OnBnClickedStaticCreate();
	afx_msg void			OnBnClickedStaticDelete();
	afx_msg void			OnBnClickedSaveStaticObj();
	afx_msg void			OnBnClickedLoadStaticObj();

	afx_msg void			OnBnClickedCheck_ColliderMode();
	afx_msg void			OnBnClickedColliderSphereMode();
	afx_msg void			OnBnClickedColliderBoxMode();
	afx_msg	void			OnBnClickedColliderShow();
	afx_msg	void			OnBnClickedSetOn_Mesh();

	afx_msg BOOL			OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	void ModifyBrush(RECT rc[11], CPoint& pt, short zDelta);
	virtual BOOL			OnInitDialog();

	void					Initialize_String();
	afx_msg void			OnBnClickedAddHeight();
	afx_msg void			OnBnClickedBoxBrush();
	afx_msg void			OnBnClickedCircleBrush();

public:
	float					m_fPosX;
	float					m_fPosY;
	float					m_fPosZ;
	float					m_fBrushRange;
	float					m_fBrushHeight;
	float					m_fTerrianPosX;
	float					m_fTerrianPosY;
	float					m_fTerrainPosZ;

	float					m_fScaleX;
	float					m_fScaleY;
	float					m_fScaleZ;
	float					m_fRotX;
	float					m_fRotY;
	float					m_fRotZ;

	_vec3					m_vMeshPos = { 0.f,0.f,0.f };
	_vec3					m_vTerrainPos = { 0.f,0.f,0.f };
	_vec3					m_vMeshRot = { 0.f,0.f,0.f };
	_vec3					m_vMeshScale = { 1.f, 1.f, 1.f };

	int						m_iCntX;
	int						m_iCntZ;
	int						m_iInterval;

	_uint					m_iTexToolMode = 0;	// 0-TEX, 1-HEIGHT, 2-SPLATTING
	_uint					m_iObjToolMode = 0;	// 0-CREATE, 1-MODY, 2-DEL
	_uint					m_iColliderState = 0; // 0-SPHERE, 1-BOX
	_uint					m_iBrushShape = 0; //0-Box, 1-Circle 2 -다듬기

	_bool					m_bIsPickingStaticObj = false;
	_bool					m_bIsPickingCollider = false;
	_bool					m_bIsColliderMode = false;
	_bool					m_bIsColliderShow = false;
	_bool					m_bIsSetOnMesh = false;

	CStaticObject*			m_pPickStaticObj = nullptr;
	CToolCollider*			m_pPickCollider = nullptr;

	_bool					m_bIsBrushMode[2] = { false };	// 0. AddHeight, 1. Splatting

	// 테스트
	CSphereCollider*			m_pSphereCol = nullptr;
	CBoxCollider*				m_pBoxCol = nullptr;
	list<CToolCollider*>		m_pColliderLst;
	_matrix						m_matColliderWorld;

private:

	void					Load_ResourceList(CString strPath, CListBox* pListBox, CString pDir);
	void					Change_TerrainTexture();
	void					Change_HeightMapTexture();
	void					ModifyStaticObj(RECT  rc[11], CPoint& pt, short zDelta);

private:
	Engine::CComponent*		m_pComponent = nullptr;

private:
	_int					m_iIdx = 0;

	CListBox				m_TexListBox;
	CListBox				m_HeightTexLst;
	CString					m_strTexList;
	CListBox				m_StaticObjLst;

	CMainFrame*				m_pMainFrm = nullptr;
	CToolView*				m_pToolView = nullptr;

	// object
	CEdit					m_EditPosX;
	CEdit					m_EditPosY;
	CEdit					m_EditPosZ;
	CEdit					m_EditScaleX;
	CEdit					m_EditScaleY;
	CEdit					m_EditScaleZ;
	CEdit					m_EditRotX;
	CEdit					m_EditRotY;
	CEdit					m_EditRotZ;

	MESHDATA*				m_pMeshData = nullptr;
	CString					m_strPath = L"";
	VTXTEX*					m_pTerrainVtx = nullptr;

	// Collider
	CButton					m_BnColliderMode;
	COLLIDER*				m_pColliderData = nullptr;
	CButton					m_BnShowCollider;
	CButton					m_BnSetOn;

public:
	CComboBox				m_CbColliderID;

	afx_msg void			OnBnClickedSaveTerrain();
	CEdit					m_EditRange;
	CEdit					m_EditHeight;
	afx_msg void OnBnClickedGradually();
	afx_msg void OnBnSaveForClient();
};
