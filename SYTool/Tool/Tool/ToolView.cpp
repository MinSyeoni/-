﻿
// ToolView.cpp: CToolView 클래스의 구현
//

#include "pch.h"
#include "framework.h"
// SHARED_HANDLERS는 미리 보기, 축소판 그림 및 검색 필터 처리기를 구현하는 ATL 프로젝트에서 정의할 수 있으며
// 해당 프로젝트와 문서 코드를 공유하도록 해 줍니다.
#ifndef SHARED_HANDLERS
#include "Tool.h"
#endif

#include "ToolDoc.h"
#include "ToolView.h"
#include "SphereCollider.h"
#include "BoxCollider.h"
#include "ActionCamera.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CToolView
HINSTANCE g_hInst;
HWND	  g_hWnd;


IMPLEMENT_DYNCREATE(CToolView, CView)

BEGIN_MESSAGE_MAP(CToolView, CView)
	// 표준 인쇄 명령입니다.
	ON_COMMAND(ID_FILE_PRINT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, &CView::OnFilePrintPreview)
	ON_WM_RBUTTONDOWN()
	ON_WM_RBUTTONUP()
	ON_WM_LBUTTONDOWN()
	ON_WM_MOUSEWHEEL()
	ON_WM_MOUSEMOVE()
	ON_WM_RBUTTONDOWN()
END_MESSAGE_MAP()

// CToolView 생성/소멸

CToolView::CToolView() noexcept
{
	// TODO: 여기에 생성 코드를 추가합니다.

}

CToolView::~CToolView()
{
	CObjMgr::GetInstance()->DestroyInstance();
	CToolCamera::GetInstance()->DestroyInstance();
	CActionCamera::GetInstance()->DestroyInstance();
	Engine::Safe_Release(m_pManagement);
	Engine::Safe_Release(m_pDevice);

	Engine::DestroyUtility();
	Engine::DestroyResources();
	Engine::DestroySystem();

	CPickingMgr::GetInstance()->DestroyInstance();
	Engine::CKeyMgr::GetInstance()->DestroyInstance();

	Engine::CGraphicDev::GetInstance()->DestroyInstance();
}

BOOL CToolView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: CREATESTRUCT cs를 수정하여 여기에서
	//  Window 클래스 또는 스타일을 수정합니다.

	return CView::PreCreateWindow(cs);
}

// CToolView 그리기

void CToolView::OnDraw(CDC* /*pDC*/)
{
	Render_MainApp();
	CToolDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;

	// TODO: 여기에 원시 데이터에 대한 그리기 코드를 추가합니다.

}


// CToolView 인쇄

BOOL CToolView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// 기본적인 준비
	return DoPreparePrinting(pInfo);
}

void CToolView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: 인쇄하기 전에 추가 초기화 작업을 추가합니다.
}

void CToolView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: 인쇄 후 정리 작업을 추가합니다.
}


// CToolView 진단

#ifdef _DEBUG
void CToolView::AssertValid() const
{
	CView::AssertValid();
}

void CToolView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CToolDoc* CToolView::GetDocument() const // 디버그되지 않은 버전은 인라인으로 지정됩니다.
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CToolDoc)));
	return (CToolDoc*)m_pDocument;
}
#endif //_DEBUG


// CToolView 메시지 처리기

HRESULT CToolView::Render_MainApp()
{
	_int			iExitCode = 0;

	if (nullptr == m_pGraphicDev ||
		nullptr == m_pDevice ||
		nullptr == m_pManagement)
		return E_FAIL;

	m_pDevice->SetRenderState(D3DRS_LIGHTING, FALSE);

	m_pGraphicDev->Render_Begin(D3DXCOLOR(0.7f, 0.7f, 0.7f, 1.f));

	m_pManagement->Render_Scene(m_pDevice);
	CObjMgr::GetInstance()->Render_Object();

	if (1 == m_pMapTab->m_iTexToolMode)
		CPickingMgr::GetInstance()->Draw_PickingBrush(m_pMapTab->m_fBrushRange, m_vMeshPos, true);
	else
		CPickingMgr::GetInstance()->Draw_PickingBrush(m_pMapTab->m_fBrushRange, m_vMeshPos, false);

	// 나중에 고쳐야함
	if (!m_pMapTab->m_pColliderLst.empty() && true == m_pMapTab->m_bIsColliderShow)
	{
		for (auto& pCol : m_pMapTab->m_pColliderLst)
		{
			pCol->Update_Collider(&pCol->Get_PareOriWorld());
			pCol->Render_Collider();
		}
	}

	if (m_pMyForm->m_iCurTab == 2)//Camera
	{
		m_pCameraTab->RenderCameraTool();
	}
	m_pGraphicDev->Render_End();

	return iExitCode;
}

_int CToolView::Update_MainApp(const _float& fTimeDelta)
{
	_int			iExitCode = 0;

	if (nullptr == m_pManagement)
		return -1;

	m_pManagement->Update_Scene(fTimeDelta);
	if (m_pMyForm->m_iCurTab == 2)//Camera
	{
		m_pCameraTab->UPdateCameraTool(fTimeDelta);
	}
	if (m_pMyForm->m_iCurTab == 3)
		m_pEffectTab->UpdateEffectTool(fTimeDelta);
	if (m_pMyForm->m_iCurTab == 4)
		m_pEffectTab2->UpdateEffectTool2(fTimeDelta);



	Engine::CKeyMgr::GetInstance()->KeyCheck();
	CObjMgr::GetInstance()->Update_Object(fTimeDelta);
	CToolCamera::GetInstance()->Update_Camera(fTimeDelta);
	CActionCamera::GetInstance()->UpdateActionCamera(fTimeDelta);

	return iExitCode;
}

HRESULT CToolView::Ready_MainApp()
{
	if (FAILED(Ready_Default_Setting(CGraphicDev::MODE_WIN, g_iWinCX, g_iWinCY)))
		return E_FAIL;
	return NOERROR;
}

HRESULT  CToolView::Ready_Default_Setting(CGraphicDev::WINMODE eMode,
	const _uint& iWinCX,
	const _uint& iWinCY)
{
	if (FAILED(Engine::Ready_GraphicDev(g_hWnd, eMode, iWinCX, iWinCY, &m_pGraphicDev)))
		return E_FAIL;

	m_pDevice = m_pGraphicDev->GetDevice();
	NULL_CHECK_RETURN(m_pDevice, E_FAIL);
	m_pDevice->AddRef();

	if (FAILED(Engine::Ready_Input_Device(g_hInst, g_hWnd)))
		return E_FAIL;

	return NOERROR;
}

void CToolView::OnInitialUpdate()
{
	CView::OnInitialUpdate();

	// TODO: 여기에 특수화된 코드를 추가 및/또는 기본 클래스를 호출합니다.
	g_hWnd = m_hWnd;
	g_hInst = AfxGetInstanceHandle();
	
	Ready_MainApp();
	Ready_Buffer_Setting();

	Engine::Create_Management(m_pDevice, &m_pManagement);
	m_pManagement->AddRef();

	Initalize_Object();

	CMainFrame* pMainFrm = dynamic_cast<CMainFrame*>(AfxGetApp()->GetMainWnd());
	CToolView* pToolView = dynamic_cast<CToolView*>(pMainFrm->m_MainSplitWnd.GetPane(0, 1));
	m_pMyForm = dynamic_cast<CMyform*>(pMainFrm->m_MainSplitWnd.GetPane(0, 0));
	m_pMapTab = m_pMyForm->m_pMapTab;
	m_pNaviTab = m_pMyForm->m_pNaviTab;
	m_pCameraTab = m_pMyForm->m_pCameraTab;
	m_pEffectTab = m_pMyForm->m_pEffectTab;
	m_pEffectTab2 = m_pMyForm->m_pEffectTab2;


}

void CToolView::Initalize_Object()
{
	CToolCamera::GetInstance()->SetGrapicDevice(m_pDevice);

	D3DXMATRIX matProj;
	D3DXMatrixPerspectiveFovLH(&matProj, D3DXToRadian(45.f), float(g_iWinCX) / float(g_iWinCY), 1.f, 1000.f);
	m_pDevice->SetTransform(D3DTS_PROJECTION, &matProj);

	// For.Timer_Default
	if (FAILED(Engine::Add_Timer(L"Timer_Default")))
		return;
	// For.Timer_60
	if (FAILED(Engine::Add_Timer(L"Timer_60")))
		return;
	// For.Timer_60
	if (FAILED(Engine::Ready_Frame(L"Frame60", 60.f)))
		return;
	// For.Font
	if (FAILED(Engine::Ready_Font(m_pDevice, L"Font_Default", L"바탕", 15, 20, FW_HEAVY), E_FAIL));
}

void CToolView::Ready_Buffer_Setting()
{
	if (FAILED(Engine::Reserve_ContainerSize(RESOURCE_END), E_FAIL));

	if (FAILED(Engine::Ready_Buffer(m_pDevice,
		RESOURCE_STATIC,
		L"Buffer_RcTex",
		Engine::BUFFER_RCTEX,
		nullptr)))
		return;

	if (FAILED(Engine::Ready_Buffer(m_pDevice,
		RESOURCE_STATIC,
		L"Buffer_TerrainTex",
		Engine::BUFFER_TERRAINTEX,
		nullptr,
		768,
		768,
		1)))
		return;

	if (FAILED(Engine::Ready_Mesh(m_pDevice, RESOURCE_STAGE, L"ammocase.X", Engine::TYPE_STATIC, L"../Resources/StaticMesh/", L"ammocase.x"), E_FAIL))
		return;
	if (FAILED(Engine::Ready_Mesh(m_pDevice, RESOURCE_STAGE, L"barricade.X", Engine::TYPE_STATIC, L"../Resources/StaticMesh/", L"barricade.x"), E_FAIL))
		return;
	if (FAILED(Engine::Ready_Mesh(m_pDevice, RESOURCE_STAGE, L"Board.X", Engine::TYPE_STATIC, L"../Resources/StaticMesh/", L"Board.x"), E_FAIL))
		return;
	if (FAILED(Engine::Ready_Mesh(m_pDevice, RESOURCE_STAGE, L"Book1.X", Engine::TYPE_STATIC, L"../Resources/StaticMesh/", L"Book1.x"), E_FAIL))
		return;
	if (FAILED(Engine::Ready_Mesh(m_pDevice, RESOURCE_STAGE, L"Book2.X", Engine::TYPE_STATIC, L"../Resources/StaticMesh/", L"Book2.x"), E_FAIL))
		return;
	if (FAILED(Engine::Ready_Mesh(m_pDevice, RESOURCE_STAGE, L"Book3.X", Engine::TYPE_STATIC, L"../Resources/StaticMesh/", L"Book3.x"), E_FAIL))
		return;
	if (FAILED(Engine::Ready_Mesh(m_pDevice, RESOURCE_STAGE, L"Book4.X", Engine::TYPE_STATIC, L"../Resources/StaticMesh/", L"Book4.x"), E_FAIL))
		return;
	if (FAILED(Engine::Ready_Mesh(m_pDevice, RESOURCE_STAGE, L"Book5.X", Engine::TYPE_STATIC, L"../Resources/StaticMesh/", L"Book5.x"), E_FAIL))
		return;
	if (FAILED(Engine::Ready_Mesh(m_pDevice, RESOURCE_STAGE, L"Book6.X", Engine::TYPE_STATIC, L"../Resources/StaticMesh/", L"Book6.x"), E_FAIL))
		return;
	if (FAILED(Engine::Ready_Mesh(m_pDevice, RESOURCE_STAGE, L"Book7.X", Engine::TYPE_STATIC, L"../Resources/StaticMesh/", L"Book7.x"), E_FAIL))
		return;
	if (FAILED(Engine::Ready_Mesh(m_pDevice, RESOURCE_STAGE, L"booksSet.X", Engine::TYPE_STATIC, L"../Resources/StaticMesh/", L"booksSet.x"), E_FAIL))
		return;
	if (FAILED(Engine::Ready_Mesh(m_pDevice, RESOURCE_STAGE, L"bottle01.X", Engine::TYPE_STATIC, L"../Resources/StaticMesh/", L"bottle01.x"), E_FAIL))
		return;
	if (FAILED(Engine::Ready_Mesh(m_pDevice, RESOURCE_STAGE, L"Box.X", Engine::TYPE_STATIC, L"../Resources/StaticMesh/", L"Box.x"), E_FAIL))
		return;
	if (FAILED(Engine::Ready_Mesh(m_pDevice, RESOURCE_STAGE, L"chairoffice.X", Engine::TYPE_STATIC, L"../Resources/StaticMesh/", L"chairoffice.x"), E_FAIL))
		return;
	if (FAILED(Engine::Ready_Mesh(m_pDevice, RESOURCE_STAGE, L"Computer1.X", Engine::TYPE_STATIC, L"../Resources/StaticMesh/", L"Computer1.x"), E_FAIL))
		return;
	if (FAILED(Engine::Ready_Mesh(m_pDevice, RESOURCE_STAGE, L"Computer2.X", Engine::TYPE_STATIC, L"../Resources/StaticMesh/", L"Computer2.x"), E_FAIL))
		return;
	if (FAILED(Engine::Ready_Mesh(m_pDevice, RESOURCE_STAGE, L"containerBlue.X", Engine::TYPE_STATIC, L"../Resources/StaticMesh/", L"containerBlue.x"), E_FAIL))
		return;
	if (FAILED(Engine::Ready_Mesh(m_pDevice, RESOURCE_STAGE, L"containerGray.X", Engine::TYPE_STATIC, L"../Resources/StaticMesh/", L"containerGray.x"), E_FAIL))
		return;
	if (FAILED(Engine::Ready_Mesh(m_pDevice, RESOURCE_STAGE, L"containerGreen.X", Engine::TYPE_STATIC, L"../Resources/StaticMesh/", L"containerGreen.x"), E_FAIL))
		return;
	if (FAILED(Engine::Ready_Mesh(m_pDevice, RESOURCE_STAGE, L"containerRed.X", Engine::TYPE_STATIC, L"../Resources/StaticMesh/", L"containerRed.x"), E_FAIL))
		return;
	if (FAILED(Engine::Ready_Mesh(m_pDevice, RESOURCE_STAGE, L"datapad.X", Engine::TYPE_STATIC, L"../Resources/StaticMesh/", L"datapad.x"), E_FAIL))
		return;
	if (FAILED(Engine::Ready_Mesh(m_pDevice, RESOURCE_STAGE, L"Desk3.X", Engine::TYPE_STATIC, L"../Resources/StaticMesh/", L"Desk3.x"), E_FAIL))
		return;
	if (FAILED(Engine::Ready_Mesh(m_pDevice, RESOURCE_STAGE, L"deskLamp.X", Engine::TYPE_STATIC, L"../Resources/StaticMesh/", L"deskLamp.x"), E_FAIL))
		return;
	if (FAILED(Engine::Ready_Mesh(m_pDevice, RESOURCE_STAGE, L"dumpster.X", Engine::TYPE_STATIC, L"../Resources/StaticMesh/", L"dumpster.x"), E_FAIL))
		return;
	if (FAILED(Engine::Ready_Mesh(m_pDevice, RESOURCE_STAGE, L"junk.X", Engine::TYPE_STATIC, L"../Resources/StaticMesh/", L"junk.x"), E_FAIL))
		return;
	if (FAILED(Engine::Ready_Mesh(m_pDevice, RESOURCE_STAGE, L"Lathe.X", Engine::TYPE_STATIC, L"../Resources/StaticMesh/", L"Lathe.x"), E_FAIL))
		return;
	if (FAILED(Engine::Ready_Mesh(m_pDevice, RESOURCE_STAGE, L"map1.X", Engine::TYPE_STATIC, L"../Resources/StaticMesh/", L"map1.x"), E_FAIL))
		return;
	if (FAILED(Engine::Ready_Mesh(m_pDevice, RESOURCE_STAGE, L"map1_roof.X", Engine::TYPE_STATIC, L"../Resources/StaticMesh/", L"map1_roof.x"), E_FAIL))
		return;
	if (FAILED(Engine::Ready_Mesh(m_pDevice, RESOURCE_STAGE, L"medCrate.X", Engine::TYPE_STATIC, L"../Resources/StaticMesh/", L"medCrate.x"), E_FAIL))
		return;
	if (FAILED(Engine::Ready_Mesh(m_pDevice, RESOURCE_STAGE, L"minimap.X", Engine::TYPE_STATIC, L"../Resources/StaticMesh/", L"minimap.x"), E_FAIL))
		return;
	if (FAILED(Engine::Ready_Mesh(m_pDevice, RESOURCE_STAGE, L"paper01.X", Engine::TYPE_STATIC, L"../Resources/StaticMesh/", L"paper01.x"), E_FAIL))
		return;
	if (FAILED(Engine::Ready_Mesh(m_pDevice, RESOURCE_STAGE, L"paper02.X", Engine::TYPE_STATIC, L"../Resources/StaticMesh/", L"paper02.x"), E_FAIL))
		return;
	if (FAILED(Engine::Ready_Mesh(m_pDevice, RESOURCE_STAGE, L"planter.X", Engine::TYPE_STATIC, L"../Resources/StaticMesh/", L"planter.x"), E_FAIL))
		return;
	if (FAILED(Engine::Ready_Mesh(m_pDevice, RESOURCE_STAGE, L"Siren.X", Engine::TYPE_STATIC, L"../Resources/StaticMesh/", L"Siren.x"), E_FAIL))
		return;
	if (FAILED(Engine::Ready_Mesh(m_pDevice, RESOURCE_STAGE, L"Solar.X", Engine::TYPE_STATIC, L"../Resources/StaticMesh/", L"Solar.x"), E_FAIL))
		return;
	if (FAILED(Engine::Ready_Mesh(m_pDevice, RESOURCE_STAGE, L"Missile.X", Engine::TYPE_STATIC, L"../Resources/StaticMesh/", L"Missile.x"), E_FAIL))
		return;
	if (FAILED(Engine::Ready_Mesh(m_pDevice, RESOURCE_STAGE, L"door1.X", Engine::TYPE_STATIC, L"../Resources/StaticMesh/", L"door1.x"), E_FAIL))
		return;
	if (FAILED(Engine::Ready_Mesh(m_pDevice, RESOURCE_STAGE, L"door2.X", Engine::TYPE_STATIC, L"../Resources/StaticMesh/", L"door2.x"), E_FAIL))
		return;
	if (FAILED(Engine::Ready_Mesh(m_pDevice, RESOURCE_STAGE, L"card.X", Engine::TYPE_STATIC, L"../Resources/StaticMesh/", L"card.x"), E_FAIL))
		return;
	if (FAILED(Engine::Ready_Mesh(m_pDevice, RESOURCE_STAGE, L"SquareCliffL.X", Engine::TYPE_STATIC, L"../Resources/StaticMesh/", L"SquareCliffL.x"), E_FAIL))
		return;
	if (FAILED(Engine::Ready_Mesh(m_pDevice, RESOURCE_STAGE, L"CornerCliffS.X", Engine::TYPE_STATIC, L"../Resources/StaticMesh/", L"CornerCliffS.x"), E_FAIL))
		return;
	if (FAILED(Engine::Ready_Mesh(m_pDevice, RESOURCE_STAGE, L"CornerCliffL.X", Engine::TYPE_STATIC, L"../Resources/StaticMesh/", L"CornerCliffL.x"), E_FAIL))
		return;
	if (FAILED(Engine::Ready_Mesh(m_pDevice, RESOURCE_STAGE, L"RoundCliffS.X", Engine::TYPE_STATIC, L"../Resources/StaticMesh/", L"RoundCliffS.x"), E_FAIL))
		return;
	if (FAILED(Engine::Ready_Mesh(m_pDevice, RESOURCE_STAGE, L"RoundCliffL.X", Engine::TYPE_STATIC, L"../Resources/StaticMesh/", L"RoundCliffL.x"), E_FAIL))
		return;
	if (FAILED(Engine::Ready_Mesh(m_pDevice, RESOURCE_STAGE, L"apollo.X", Engine::TYPE_STATIC, L"../Resources/StaticMesh/", L"apollo.x"), E_FAIL))
		return;
	if (FAILED(Engine::Ready_Mesh(m_pDevice, RESOURCE_STAGE, L"passage_top.X", Engine::TYPE_STATIC, L"../Resources/StaticMesh/", L"passage_top.x"), E_FAIL))
		return;
	if (FAILED(Engine::Ready_Mesh(m_pDevice, RESOURCE_STAGE, L"passage_side.X", Engine::TYPE_STATIC, L"../Resources/StaticMesh/", L"passage_side.x"), E_FAIL))
		return;
	if (FAILED(Engine::Ready_Mesh(m_pDevice, RESOURCE_STAGE, L"passage_bottom.X", Engine::TYPE_STATIC, L"../Resources/StaticMesh/", L"passage_bottom.x"), E_FAIL))
		return;
	if (FAILED(Engine::Ready_Mesh(m_pDevice, RESOURCE_STAGE, L"passage_test.X", Engine::TYPE_STATIC, L"../Resources/StaticMesh/", L"passage_test.x"), E_FAIL))
		return;
	if (FAILED(Engine::Ready_Mesh(m_pDevice, RESOURCE_STAGE, L"minibase.X", Engine::TYPE_STATIC, L"../Resources/StaticMesh/", L"minibase.x"), E_FAIL))
		return;
	if (FAILED(Engine::Ready_Mesh(m_pDevice, RESOURCE_STAGE, L"point_top.X", Engine::TYPE_STATIC, L"../Resources/StaticMesh/", L"point_top.x"), E_FAIL))
		return;
	if (FAILED(Engine::Ready_Mesh(m_pDevice, RESOURCE_STAGE, L"point_bottom.X", Engine::TYPE_STATIC, L"../Resources/StaticMesh/", L"point_bottom.x"), E_FAIL))
		return;
	if (FAILED(Engine::Ready_Mesh(m_pDevice, RESOURCE_STAGE, L"passage_point.X", Engine::TYPE_STATIC, L"../Resources/StaticMesh/", L"passage_point.x"), E_FAIL))
		return;
	if (FAILED(Engine::Ready_Mesh(m_pDevice, RESOURCE_STAGE, L"antenna.X", Engine::TYPE_STATIC, L"../Resources/StaticMesh/", L"antenna.x"), E_FAIL))
		return;
	if (FAILED(Engine::Ready_Mesh(m_pDevice, RESOURCE_STAGE, L"medikit_syringe.X", Engine::TYPE_STATIC, L"../Resources/StaticMesh/", L"medikit_syringe.x"), E_FAIL))
		return;
	if (FAILED(Engine::Ready_Mesh(m_pDevice, RESOURCE_STAGE, L"medikit_vaccine.X", Engine::TYPE_STATIC, L"../Resources/StaticMesh/", L"medikit_vaccine.x"), E_FAIL))
		return;
	if (FAILED(Engine::Ready_Mesh(m_pDevice, RESOURCE_STAGE, L"medikit_bandage.X", Engine::TYPE_STATIC, L"../Resources/StaticMesh/", L"medikit_bandage.x"), E_FAIL))
		return;
	if (FAILED(Engine::Ready_Mesh(m_pDevice, RESOURCE_STAGE, L"medikit.X", Engine::TYPE_STATIC, L"../Resources/StaticMesh/", L"medikit.x"), E_FAIL))
		return;
	if (FAILED(Engine::Ready_Mesh(m_pDevice, RESOURCE_STAGE, L"box0_1.X", Engine::TYPE_STATIC, L"../Resources/StaticMesh/", L"box0_1.x"), E_FAIL))
		return;
	if (FAILED(Engine::Ready_Mesh(m_pDevice, RESOURCE_STAGE, L"box0_2.X", Engine::TYPE_STATIC, L"../Resources/StaticMesh/", L"box0_2.x"), E_FAIL))
		return;
	if (FAILED(Engine::Ready_Mesh(m_pDevice, RESOURCE_STAGE, L"box0_3.X", Engine::TYPE_STATIC, L"../Resources/StaticMesh/", L"box0_3.x"), E_FAIL))
		return;
	if (FAILED(Engine::Ready_Mesh(m_pDevice, RESOURCE_STAGE, L"box0_4.X", Engine::TYPE_STATIC, L"../Resources/StaticMesh/", L"box0_4.x"), E_FAIL))
		return;
	if (FAILED(Engine::Ready_Mesh(m_pDevice, RESOURCE_STAGE, L"box0_5.X", Engine::TYPE_STATIC, L"../Resources/StaticMesh/", L"box0_5.x"), E_FAIL))
		return;
	if (FAILED(Engine::Ready_Mesh(m_pDevice, RESOURCE_STAGE, L"box0_6.X", Engine::TYPE_STATIC, L"../Resources/StaticMesh/", L"box0_6.x"), E_FAIL))
		return;
	if (FAILED(Engine::Ready_Mesh(m_pDevice, RESOURCE_STAGE, L"box0_7.X", Engine::TYPE_STATIC, L"../Resources/StaticMesh/", L"box0_7.x"), E_FAIL))
		return;
	if (FAILED(Engine::Ready_Mesh(m_pDevice, RESOURCE_STAGE, L"box0_8.X", Engine::TYPE_STATIC, L"../Resources/StaticMesh/", L"box0_8.x"), E_FAIL))
		return;
	if (FAILED(Engine::Ready_Mesh(m_pDevice, RESOURCE_STAGE, L"cardreader.X", Engine::TYPE_STATIC, L"../Resources/StaticMesh/", L"cardreader.x"), E_FAIL))
		return;
	if (FAILED(Engine::Ready_Mesh(m_pDevice, RESOURCE_STAGE, L"passage_box1.X", Engine::TYPE_STATIC, L"../Resources/StaticMesh/", L"passage_box1.x"), E_FAIL))
		return;
	if (FAILED(Engine::Ready_Mesh(m_pDevice, RESOURCE_STAGE, L"passage_box2.X", Engine::TYPE_STATIC, L"../Resources/StaticMesh/", L"passage_box2.x"), E_FAIL))
		return;
	if (FAILED(Engine::Ready_Mesh(m_pDevice, RESOURCE_STAGE, L"reaper_ground.X", Engine::TYPE_STATIC, L"../Resources/StaticMesh/", L"reaper_ground.x"), E_FAIL))
		return;

	///// 테스트 /////
	if (FAILED(Engine::Ready_Mesh(m_pDevice, RESOURCE_STAGE, L"test.X", Engine::TYPE_STATIC, L"../Resources/StaticMesh/", L"test.x"), E_FAIL))
		return;

	//if (FAILED(Engine::Ready_Mesh(m_pDevice,
	//	RESOURCE_STAGE,
	//	L"Mesh_Navigation",
	//	Engine::TYPE_NAVI,
	//	NULL,
	//	NULL)));

	if (FAILED((Engine::Ready_Buffer(m_pDevice,
		RESOURCE_STATIC,
		L"Buffer_CubeTex",
		Engine::BUFFER_CUBETEX,
		nullptr))))
		return;

	if (FAILED(Engine::Ready_Texture(m_pDevice,
		RESOURCE_STAGE,
		L"Texture_Terrain",
		Engine::TEX_NORMAL,
		L"../Resources/Texture/Terrain/Terrain%d.png", 2)))
		return;

	if (FAILED(Engine::Ready_Texture(m_pDevice,
		RESOURCE_STAGE,
		L"Texture_Default",
		Engine::TEX_NORMAL,
		L"../Resources/Texture/Default/Default%d.png", 1)))
		return;

	if (FAILED(Engine::Ready_Texture(m_pDevice,
		RESOURCE_STAGE,
		L"Texture_AlphaTest",
		Engine::TEX_NORMAL,
		L"../Resources/Texture/AlphaTest/AlphaTest%d.png", 1)))
		return;

	if (FAILED(Engine::Ready_Texture(m_pDevice,
		RESOURCE_STAGE,
		L"Texture_AlphaBlend",
		Engine::TEX_NORMAL,
		L"../Resources/Texture/AlphaBlend/AlphaBlend%d.tga", 9)))
		return;
}

void CToolView::OnLButtonDown(UINT nFlags, CPoint point)
{
	// TODO: 여기에 메시지 처리기 코드를 추가 및/또는 기본값을 호출합니다.
	if (0 == m_pMyForm->m_iCurTab)
	{
		m_pMapTab->UpdateData(TRUE);	// MapTab
		if (0 == m_pMapTab->m_iObjToolMode)	// 생성 모드
		{
			bool retflag;
			Picking_TerrainOnStaticObject(retflag);
			Picking_MeshOnStaticObject(retflag);
			if (retflag) return;
		}
		else // 수정, 삭제 시 메쉬 클릭
		{
			if (true == m_pMapTab->m_bIsColliderMode)
			{
				bool retflag;
				Picking_MouseOnCollider(retflag);
				if (retflag) return;
			}
			else
			{
				bool retflag;
				Picking_MouseOnStaticObject(retflag);
				if (retflag) return;
			}
		}
		m_pMapTab->UpdateData(FALSE);
	}
	else if (1 == m_pMyForm->m_iCurTab)
	{
		m_pNaviTab->UpdateData(TRUE);	// NaviTab
		if (0 == m_pNaviTab->m_iCurNaviMode)	// 생성 모드
		{
			bool retflag;
			Get_TerrainInfo();

			_float fDistTemp = 10000000.f;
			_float fFixDist = 0.f;
			list<Engine::CGameObject*> pObjLst = CObjMgr::GetInstance()->GetGameObjectLst(CObjMgr::OBJ_OBJECT);
			auto& iter_front = pObjLst.begin();

			auto iter = pObjLst.begin();
			for (; iter != pObjLst.end(); ++iter)
			{
				Engine::CTransform* pTransCom = dynamic_cast<CStaticObject*>(*iter)->Get_StaticTranscom();

				if (CPickingMgr::GetInstance()->IsCheckStaticObjgectMesh(
					dynamic_cast<CStaticObject*>(*iter),
					*pTransCom->Get_WorldMatrix(),
					&fFixDist,
					&m_vMeshPos))
				{
					if (nullptr == dynamic_cast<CStaticObject*>(*iter))
					{
						AfxMessageBox(L"메쉬를 클릭하세요.");
						continue;
					}

					if (fFixDist <= fDistTemp)
					{
						fDistTemp = fFixDist;
						iter_front = iter;
					}
				}
			}
			Create_NaviPointCell(retflag);
			if (retflag) return;
		}
		else if (1 == m_pNaviTab->m_iCurNaviMode)	// 수정 모드
		{
			bool retflag;
			Modify_NaviPointCell(retflag);
			if (retflag) return;
		}
		m_pNaviTab->UpdateData(FALSE);
	}
	else if (2 == m_pMyForm->m_iCurTab)
	{
		CheckCameraTabButton();

	}
	else if (3 == m_pMyForm->m_iCurTab || m_pMyForm->m_iCurTab == 4)
	{
		bool bIsChange = false;

		CToolEffect* pEffectSelect = nullptr;

		for (auto& pSrc : CObjMgr::GetInstance()->GetGameObjectLst(CObjMgr::OBJ_EFFECT))
		{

			CToolEffect* pEffect = dynamic_cast<CToolEffect*>(pSrc);
			CToolCollider* pToolCollider = pEffect->Get_Collider();

			if (pToolCollider == nullptr)
				continue;



			if (true == CPickingMgr::GetInstance()->IsCheckColiderMesh((pToolCollider)->Get_ColliderMesh(), pToolCollider->Get_WorldMat()))
			{
				pEffect->m_bIsCheck = true;
				m_pEffectTab->m_pEffectData = pEffect;
				m_pEffectTab2->m_pEffectData = pEffect;
			
			
				bIsChange = true;
				pEffectSelect = pEffect;
				break;
			}

		}
		if (pEffectSelect == nullptr)
			return;

		for (auto& pSrc : CObjMgr::GetInstance()->GetGameObjectLst(CObjMgr::OBJ_EFFECT))
		{
			CToolEffect* pEffect = dynamic_cast<CToolEffect*>(pSrc);
			if (pEffectSelect != pEffect)
				pEffect->SetCheck(false);

		}
		m_pEffectTab->InitEffect();
		m_pEffectTab2->FindEffectData();
	}
	CView::OnLButtonDown(nFlags, point);
}

void CToolView::Modify_NaviPointCell(bool& retflag)
{
	retflag = true;
	if (m_pNaviTab->m_pPointLst.empty() || m_pNaviTab->m_pCellLst.empty())
		retflag = false;

	int iTmp = 0;
	for (auto& pPoint : m_pNaviTab->m_pPointLst)
	{
		if (true == CPickingMgr::GetInstance()->IsCheckSphereCollider(pPoint->m_pSphereCol))
		{
			pPoint->Set_CheckPoint(true); 
			m_pNaviTab->m_pPointTmp = pPoint;
		}
		if (false == CPickingMgr::GetInstance()->IsCheckSphereCollider(pPoint->m_pSphereCol))
			pPoint->Set_CheckPoint(false);
		iTmp++;
	}

	for (auto& pPoint : m_pNaviTab->m_pPointLst)
	{
		if (nullptr == m_pNaviTab->m_pPointTmp)
			return;

		if(pPoint == m_pNaviTab->m_pPointTmp)
			m_pNaviTab->Get_NaviPointPos();
	}

	retflag = false;
}

void CToolView::Create_NaviPointCell(bool& retflag)
{
	retflag = true;

	CString	strIndex = L"";

	if (!m_pNaviTab->m_pPointLst.empty())	// 겹치게 찍었을 때
	{
		for (auto& pPoint : m_pNaviTab->m_pPointLst)
		{
			if (true == CPickingMgr::GetInstance()->IsCheckSphereCollider(pPoint->m_pSphereCol))
			{
				m_vMeshPos = pPoint->m_pSphereCol->Get_Pos();
				break;
			}
		}
	}

	m_pNaviTab->m_vNaviPos = m_vMeshPos;
	m_pNaviTab->m_fPosX = m_vMeshPos.x;
	m_pNaviTab->m_fPosY = m_vMeshPos.y;
	m_pNaviTab->m_fPosZ = m_vMeshPos.z;

	Check_ClockDirection(retflag);

	if (3 == m_iIdxCnt)
	{
		m_pNaviTab->m_pToolCell = CToolCell::Create(m_pGraphicDev->GetDevice(),
			m_pNaviTab->m_pToolPoint[0], m_pNaviTab->m_pToolPoint[1], m_pNaviTab->m_pToolPoint[2]);
		CObjMgr::GetInstance()->AddObject(m_pNaviTab->m_pToolCell, CObjMgr::OBJ_CELL);
		m_pNaviTab->m_pCellLst.push_back(m_pNaviTab->m_pToolCell);
		CObjMgr::GetInstance()->m_ObjLst[CObjMgr::OBJ_POINT].clear();

		strIndex.Format(L"%d번 Cell", m_pNaviTab->m_iCellCnt);
		m_pNaviTab->m_NaviList.InsertString(m_pNaviTab->m_iCellCnt, strIndex);
		m_pNaviTab->m_iCellCnt++;

		m_iIdxCnt = 0;
	}
	
	retflag = false;
}

void CToolView::Check_ClockDirection(bool& retflag)
{
	if (CObjMgr::GetInstance()->GetGameObject(CObjMgr::OBJ_CELL) != nullptr)
	{
		int i = 0;
		for (auto& pSRC : CObjMgr::GetInstance()->m_ObjLst[CObjMgr::OBJ_CELL])
		{
			CToolPoint* pToolPoint1 = nullptr;
			CToolPoint* pToolPoint2 = nullptr;

			int i = 0;
			if (CObjMgr::GetInstance()->m_ObjLst[CObjMgr::OBJ_POINT].size() == 2)
			{
				for (auto& pPoint : CObjMgr::GetInstance()->m_ObjLst[CObjMgr::OBJ_POINT])
				{
					if (i == 0)
						pToolPoint1 = dynamic_cast<CToolPoint*>(pPoint);
					else
						pToolPoint2 = dynamic_cast<CToolPoint*>(pPoint);
					i++;
				}
				_vec3 PointAtoB = 
					pToolPoint2->m_pTransCom->m_vInfo[Engine::INFO_POS]
					- pToolPoint1->m_pTransCom->m_vInfo[Engine::INFO_POS];
				_vec3 PointAtoC = m_vMeshPos - pToolPoint1->m_pTransCom->m_vInfo[Engine::INFO_POS];
				_vec3 Cross;
				D3DXVec3Cross(&Cross, &PointAtoB, &PointAtoC);

				if (Cross.y < 0)
				{
					AfxMessageBox(L"반시계 방향! 시계 방향으로 찍을 것");
					return;
				}
			}
			m_pNaviTab->m_pToolPoint[m_iIdxCnt] = CToolPoint::Create(m_pGraphicDev->GetDevice(), m_vMeshPos);
			CObjMgr::GetInstance()->AddObject(m_pNaviTab->m_pToolPoint[m_iIdxCnt], CObjMgr::OBJ_POINT);
			m_pNaviTab->m_pPointLst.push_back(m_pNaviTab->m_pToolPoint[m_iIdxCnt]);
			m_iIdxCnt++;
			return;
		}
		m_pNaviTab->m_pToolPoint[m_iIdxCnt] = CToolPoint::Create(m_pGraphicDev->GetDevice(), m_vMeshPos);
		CObjMgr::GetInstance()->AddObject(m_pNaviTab->m_pToolPoint[m_iIdxCnt], CObjMgr::OBJ_POINT);
		m_pNaviTab->m_pPointLst.push_back(m_pNaviTab->m_pToolPoint[m_iIdxCnt]);
		m_iIdxCnt++;
		return;
	}
	else
	{
		if (CObjMgr::GetInstance()->m_ObjLst[CObjMgr::OBJ_POINT].size() == 2)
		{
			CToolPoint* pToolPoint1 = nullptr;
			CToolPoint* pToolPoint2 = nullptr;
			int i = 0;

			for (auto& pPoint : CObjMgr::GetInstance()->m_ObjLst[CObjMgr::OBJ_POINT])
			{
				if (i == 0)
					pToolPoint1 = dynamic_cast<CToolPoint*>(pPoint);
				else
					pToolPoint2 = dynamic_cast<CToolPoint*>(pPoint);
				i++;
			}

			_vec3 PointAtoB = pToolPoint2->m_pTransCom->m_vInfo[Engine::INFO_POS]
				- pToolPoint1->m_pTransCom->m_vInfo[Engine::INFO_POS];
			_vec3 PointAtoC = m_vMeshPos - pToolPoint1->m_pTransCom->m_vInfo[Engine::INFO_POS];
			_vec3 Cross;
			D3DXVec3Cross(&Cross, &PointAtoB, &PointAtoC);

			if (Cross.y < 0)
			{
				AfxMessageBox(L"반시계 방향! 시계 방향으로 찍을 것");
				return;
			}
		}
		m_pNaviTab->m_pToolPoint[m_iIdxCnt] = CToolPoint::Create(m_pGraphicDev->GetDevice(), m_vMeshPos);
		CObjMgr::GetInstance()->AddObject(m_pNaviTab->m_pToolPoint[m_iIdxCnt], CObjMgr::OBJ_POINT);
		m_pNaviTab->m_pPointLst.push_back(m_pNaviTab->m_pToolPoint[m_iIdxCnt]);
		m_iIdxCnt++;
		return;
	}
}

void CToolView::Get_TerrainInfo()
{
	if (nullptr == CObjMgr::GetInstance()->GetGameObject(CObjMgr::OBJ_TERRAIN))
	{
		MessageBox(L"피킹 할 Terrain 없다 만들어라");
		return;
	}
	Engine::CGameObject* pObj = CObjMgr::GetInstance()->GetGameObject(CObjMgr::OBJ_TERRAIN);
	Engine::CTerrainTex* pBufferCom = dynamic_cast<Engine::CTerrainTex*>(pObj->Get_Component(L"Com_Buffer", Engine::ID_STATIC));
	Engine::CTransform* pTransCom = dynamic_cast<CTerrain*>(pObj)->Get_TransCom();

	CPickingMgr::GetInstance()->SetTerrainSize(m_pMapTab->m_iCntX, m_pMapTab->m_iCntZ);

	//Engine::CTransform* pTransCom = dynamic_cast<Engine::CTransform*>(pObj->Get_Component(L"Com_Transform", Engine::ID_DYNAMIC));
	CPickingMgr::GetInstance()->PickingTerrain(&m_vMeshPos, pBufferCom->m_pVtxTexOrigin, pTransCom->Get_WorldMatrix());
	m_vMeshPos.y = CPickingMgr::GetInstance()->Compute_HeightOnTerrain(&m_vMeshPos, pBufferCom->Get_VtxPos(), m_pMapTab->m_iCntX, m_pMapTab->m_iCntZ);
	if (m_vMeshPos.y <= -431602080.)		///////////// ???
		m_vMeshPos.y = 0.f;
}
void CToolView::Picking_MouseOnCollider(bool& retflag)
{
	retflag = true;
	if (m_pMapTab->m_pColliderLst.empty())
		return;

	for (auto& iter = m_pMapTab->m_pColliderLst.begin(); iter != m_pMapTab->m_pColliderLst.end();)
	{
		if (true == CPickingMgr::GetInstance()->IsCheckColiderMesh((*iter)->Get_ColliderMesh(), (*iter)->Get_WorldMat())
			|| true == CPickingMgr::GetInstance()->IsCheckSphereCollider((*iter)))
		{
			(*iter)->Set_ColType(CToolCollider::COL_TRUE);
			m_pMapTab->m_CbColliderID.SetCurSel((*iter)->Get_ColID());

			m_pMapTab->m_fPosX = (*iter)->Get_WorldMat()._41;
			m_pMapTab->m_fPosY = (*iter)->Get_WorldMat()._42;
			m_pMapTab->m_fPosZ = (*iter)->Get_WorldMat()._43;
			m_pMapTab->m_vMeshPos = { m_pMapTab->m_fPosX, m_pMapTab->m_fPosY, m_pMapTab->m_fPosZ };

			m_pMapTab->m_fRotX = (*iter)->Get_Angle().x;
			m_pMapTab->m_fRotY = (*iter)->Get_Angle().y;
			m_pMapTab->m_fRotZ = (*iter)->Get_Angle().z;
			m_pMapTab->m_vMeshRot = { m_pMapTab->m_fRotX, m_pMapTab->m_fRotY, m_pMapTab->m_fRotZ };

			if (CPickingMgr::GetInstance()->IsCheckSphereCollider((*iter)))
			{
				m_pMapTab->m_fScaleX = (*iter)->Get_WorldMat()._11;
				m_pMapTab->m_fScaleY = (*iter)->Get_WorldMat()._22;
				m_pMapTab->m_fScaleZ = (*iter)->Get_WorldMat()._33;
			}
			else
			{
				m_pMapTab->m_fScaleX = (*iter)->Get_Scale().x;
				m_pMapTab->m_fScaleY = (*iter)->Get_Scale().y;
				m_pMapTab->m_fScaleZ = (*iter)->Get_Scale().z;
			}
			m_pMapTab->m_vMeshScale = { m_pMapTab->m_fScaleX, m_pMapTab->m_fScaleY, m_pMapTab->m_fScaleZ };

			m_pMapTab->m_pPickCollider = (*iter);
			m_pMapTab->m_bIsPickingCollider = true;
		//	break;
		}
		else
		{
			(*iter)->Set_ColType(CToolCollider::COL_FALSE);
			m_pMapTab->m_bIsPickingCollider = false;
		}
		iter++;
	}
	retflag = false;
}
void CToolView::Picking_MeshOnStaticObject(bool& retflag)
{
	retflag = true;
	if (nullptr == CObjMgr::GetInstance()->GetGameObject(CObjMgr::OBJ_OBJECT))
		retflag = false;

	_float fDistTemp = 10000000.f;
	_float fFixDist = 0.f;
	list<Engine::CGameObject*> pObjLst = CObjMgr::GetInstance()->GetGameObjectLst(CObjMgr::OBJ_OBJECT);
	auto& iter_front = pObjLst.begin();

	auto iter = pObjLst.begin();
	for (; iter != pObjLst.end(); ++iter)
	{
		Engine::CTransform* pTransCom = dynamic_cast<CStaticObject*>(*iter)->Get_StaticTranscom();

		if (CPickingMgr::GetInstance()->IsCheckStaticObjgectMesh(
			dynamic_cast<CStaticObject*>(*iter),
			*pTransCom->Get_WorldMatrix(),
			&fFixDist,
			&m_vMeshPos))
		{
			if (nullptr == dynamic_cast<CStaticObject*>(*iter))
			{
				AfxMessageBox(L"메쉬를 클릭하세요.");
				continue;
			}

			if (fFixDist <= fDistTemp)
			{
				fDistTemp = fFixDist;
				iter_front = iter;

				if (true == m_pMapTab->m_bIsColliderMode)
				{
					dynamic_cast<CStaticObject*>(*iter_front)->Get_StaticTranscom()->Get_Info(Engine::INFO_POS, &m_vMeshPos);
					m_pMapTab->m_pPickStaticObj = dynamic_cast<CStaticObject*>(*iter_front);
				}

				m_pMapTab->m_vMeshPos = m_vMeshPos;
				m_pMapTab->m_fPosX = m_vMeshPos.x;
				m_pMapTab->m_fPosY = m_vMeshPos.y;
				m_pMapTab->m_fPosZ = m_vMeshPos.z;
			}
		}
	}
	retflag = false;
}
void CToolView::Picking_MouseOnStaticObject(bool& retflag)
{
	retflag = true;
	if (nullptr == CObjMgr::GetInstance()->GetGameObject(CObjMgr::OBJ_OBJECT))
	{
		MessageBox(L"수정할 Object 없다 만들어라");
		return;
	}

	_float fDistTemp = 10000000.f;
	_float fFixDist = 0.f;

	list<Engine::CGameObject*> pObjLst = CObjMgr::GetInstance()->GetGameObjectLst(CObjMgr::OBJ_OBJECT);
	auto& iter_front = pObjLst.begin();

	auto iter = pObjLst.begin();
	for (; iter != pObjLst.end(); ++iter)
	{
		Engine::CTransform* pTransCom = dynamic_cast<CStaticObject*>(*iter)->Get_StaticTranscom();

		if (CPickingMgr::GetInstance()->IsCheckStaticObjgectMesh(
			dynamic_cast<CStaticObject*>(*iter),
			*pTransCom->Get_WorldMatrix(),
			&fFixDist,
			&m_vMeshPos))
		{
			if (nullptr == dynamic_cast<CStaticObject*>(*iter))
			{
				AfxMessageBox(L"메쉬를 클릭하세요.");
				continue;
			}

			if (fFixDist <= fDistTemp)
			{
				fDistTemp = fFixDist;
				iter_front = iter;
			}
			else
				m_pMapTab->m_bIsPickingStaticObj = false;
		}
	}

	if (iter_front != pObjLst.end())
	{
		dynamic_cast<CStaticObject*>(*iter_front)->Get_StaticTranscom()->Get_Info(Engine::INFO_POS, &m_vMeshPos);
		m_vMeshScale = dynamic_cast<CStaticObject*>(*iter_front)->Get_StaticTranscom()->m_vScale;
		m_vMeshRot = dynamic_cast<CStaticObject*>(*iter_front)->Get_StaticTranscom()->m_vAngle;

		m_pMapTab->m_vMeshPos = m_vMeshPos;
		m_pMapTab->m_fPosX = m_vMeshPos.x;
		m_pMapTab->m_fPosY = m_vMeshPos.y;
		m_pMapTab->m_fPosZ = m_vMeshPos.z;

		m_pMapTab->m_vMeshScale = m_vMeshScale;
		m_pMapTab->m_fScaleX = m_vMeshScale.x;
		m_pMapTab->m_fScaleY = m_vMeshScale.y;
		m_pMapTab->m_fScaleZ = m_vMeshScale.z;

		m_pMapTab->m_vMeshRot = m_vMeshRot;
		m_pMapTab->m_fRotX = m_vMeshRot.x;
		m_pMapTab->m_fRotY = m_vMeshRot.y;
		m_pMapTab->m_fRotZ = m_vMeshRot.z;

		m_pMapTab->m_pPickStaticObj = dynamic_cast<CStaticObject*>(*iter_front);
		m_pMapTab->m_bIsPickingStaticObj = true;
	}

	retflag = false;
}
void CToolView::Picking_TerrainOnStaticObject(bool& retflag)
{
	retflag = true;
	Get_TerrainInfo();

	m_pMapTab->m_vMeshPos = m_vMeshPos;
	m_pMapTab->m_fPosX = m_vMeshPos.x;
	m_pMapTab->m_fPosY = m_vMeshPos.y;
	m_pMapTab->m_fPosZ = m_vMeshPos.z;

	retflag = false;
}

void CToolView::Picking_Terrain(bool& retflag)
{
	retflag = true;

	Get_TerrainInfo();
	m_pMapTab->m_vTerrainPos = m_vMeshPos;
	m_pMapTab->m_fTerrianPosX = m_vMeshPos.x;
	m_pMapTab->m_fTerrianPosY = m_vMeshPos.y;
	m_pMapTab->m_fTerrainPosZ = m_vMeshPos.z;

	retflag = false;
}

void CToolView::CheckCameraTabButton()
{
	if (m_pCameraTab->m_AddAtButton.GetCheck() || m_pCameraTab->m_AddEyeButton.GetCheck())
	{
		Get_TerrainInfo();
		m_pCameraTab->UpdateData(TRUE);
		m_pCameraTab->m_PosX = m_vMeshPos.x;
		m_pCameraTab->m_PosY = m_vMeshPos.y;
		m_pCameraTab->m_PosZ = m_vMeshPos.z;
		m_pCameraTab->UpdateData(FALSE);

	}
}


BOOL CToolView::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	// TODO: 여기에 메시지 처리기 코드를 추가 및/또는 기본값을 호출합니다.
	if ((m_pCameraTab->m_AddAtButton.GetCheck() || m_pCameraTab->m_AddEyeButton.GetCheck()) && 2 == m_pMyForm->m_iCurTab)
	{
		m_pCameraTab->UpdateData(TRUE);
		m_pCameraTab->m_PosY += zDelta * 0.001f;
		m_pCameraTab->UpdateData(FALSE);
	}

	return CView::OnMouseWheel(nFlags, zDelta, pt);
}

void CToolView::OnMouseMove(UINT nFlags, CPoint point)
{
	// TODO: 여기에 메시지 처리기 코드를 추가 및/또는 기본값을 호출합니다.
	m_pMapTab->UpdateData(TRUE);

	if (nullptr == CObjMgr::GetInstance()->GetGameObject(CObjMgr::OBJ_TERRAIN))
		return;

	Engine::CGameObject* pObj = CObjMgr::GetInstance()->GetGameObject(CObjMgr::OBJ_TERRAIN);
	Engine::CTerrainTex* pBufferCom = dynamic_cast<Engine::CTerrainTex*>(pObj->Get_Component(L"Com_Buffer", Engine::ID_STATIC));
	Engine::CComponent* pComponent = pObj->Get_Component(L"Com_Buffer", Engine::ID_STATIC);

	bool retflag;
	Picking_Terrain(retflag);
	if (retflag) return;

	if (1 == m_pMapTab->m_iTexToolMode)	// HeightMode
	{
		DWORD dwIdx = 0;
		if ((nFlags & MK_LBUTTON) == MK_LBUTTON)
		{
			CPickingMgr::GetInstance()->PickTerrainIndex(&dwIdx, pBufferCom->m_pVtxTexOrigin);
			dynamic_cast<Engine::CTerrainTex*>(pComponent)->Set_TerrainHeight(m_pMapTab->m_fBrushRange, m_pMapTab->m_fBrushHeight, m_vMeshPos, m_pMapTab->m_iBrushShape);
		}
	}
	else if (2 == m_pMapTab->m_iTexToolMode)	// SplattingMode
		return;

	m_pMapTab->UpdateData(FALSE);
	CView::OnMouseMove(nFlags, point);
}



void CToolView::OnRButtonDown(UINT nFlags, CPoint point)
{
	// TODO: 여기에 메시지 처리기 코드를 추가 및/또는 기본값을 호출합니다.
	if (3 == m_pMyForm->m_iCurTab || 4 == m_pMyForm->m_iCurTab)
	{
		for (auto& pSrc : CObjMgr::GetInstance()->GetGameObjectLst(CObjMgr::OBJ_EFFECT))
		{

			dynamic_cast<CToolEffect*>(pSrc)->m_bIsCheck = false;

		}


	}
	m_pEffectTab->m_pEffectData = nullptr;
	m_pEffectTab2->m_pEffectData = nullptr;
	CView::OnRButtonDown(nFlags, point);
}
