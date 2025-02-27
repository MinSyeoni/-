#include "stdafx.h"
#include "PlayerStatus.h"
#include "DirectInput.h"
#include "GraphicDevice.h"
#include "ComponentMgr.h"
#include "ColliderMgr.h"
#include "ObjectMgr.h"
#include "Weapon.h"
#include "DynamicCamera.h"
#include "Monster.h"
#include "Aim.h"
#include "Reapear.h"
#include "ReapearTube.h"

#include "HPBar.h"
#include "MPBar.h"

CPlayerStatus::CPlayerStatus()
{
}

CPlayerStatus::~CPlayerStatus()
{
    Safe_Release(m_pBoxCollider);
    Safe_Release(m_pSphereCollider);
    Safe_Release(m_pMesh);
    Safe_Release(m_pNaviMesh);
    Safe_Release(m_pBoxCOlliderForView);
}

void CPlayerStatus::LateInit()
{
    m_pCamera = static_cast<CDynamicCamera*>(CObjectMgr::Get_Instance()->Get_GameObject(L"Layer_Camera", L"DynamicCamera"));
    m_bIsInit = true;
}

_int CPlayerStatus::UpdateState(const _float& fTimeDelta, CTransform* pTranscom)
{

    if (m_pTransCom == nullptr)
    {
        m_pTransCom = pTranscom;
        m_pNaviMesh->SetFirstNavi(m_pTransCom->m_vPos);
        //m_pTransCom->m_vPos = m_pNaviMesh->MoveOn_NaviMesh(&m_pTransCom->m_vPos, &_vec3(0.f, 0.f, 0.f), 0.f);//추가
    }
    LateInit();

    _matrix matRot;

    matRot = XMMatrixRotationY(XMConvertToRadians(-90.f));

    CutSceneCheck();
    CoverCheck();
    if (!m_bIsKeyLock)
    {
        KeyInput();
        LegKeyInput(fTimeDelta);
        Rotation(fTimeDelta);
        StatusUpdate(fTimeDelta);
        PlayerDirection(fTimeDelta);
        HPMpCheck(fTimeDelta);
        CheckAim();
        CheckSniping();
        ReloadCheck();
        WeaponChange();
        CameraShakeCheck();
    }
    else
    {
        CGameObject* pAim = CObjectMgr::Get_Instance()->Get_GameObject(L"Layer_UI", L"Aim");
        if (pAim != nullptr)
            dynamic_cast<CAim*>(pAim)->SetRender(false);
        if (m_eEquip == NONE&&m_eCurScene == NOSCENE)
        {
            m_eCurState = CPlayer::NONEIDLE;
            m_eLegState = CPlayer::NONEIDLE;
        }
        else if(m_eEquip == RIFLE && m_eCurScene == NOSCENE)
        {
            m_eCurState = CPlayer::RIFLEIDLE;
            m_eLegState = CPlayer::RIFLEIDLE;
        }


    }
    if (m_bIsShoot)
    {

        if (!m_bIsCover && m_eEquip == RIFLE)
            m_eCurState = CPlayer::RIFLEATTACK;
        else if (!m_bIsCover && m_eEquip == SNIPER)
            m_eCurState = CPlayer::SNIPERATTACK;
        else if (m_bIsCover && m_eEquip == RIFLE)
            m_eCurState = CPlayer::COVERATTACK;
        else if (m_bIsCover && m_eEquip == SNIPER)
            m_eCurState = CPlayer::COVER_ATTACKSNIPER;
    }


  
    _matrix matBone = XMMatrixInverse(nullptr, *m_matChestOffset);
    matBone = matBone * *m_matChest;

    m_pBoxCollider->Update_Collider(&m_pTransCom->m_matWorld);
    m_pBoxCOlliderForView->Update_Collider(&m_pTransCom->m_matWorld);
    m_pSphereCollider->Update_Collider(&(matBone * m_pTransCom->m_matWorld));
    CColliderMgr::Get_Instance()->Add_Collider(CColliderMgr::PLAYER, m_pBoxCollider);
    CColliderMgr::Get_Instance()->Add_Collider(CColliderMgr::PLAYER, m_pSphereCollider);

 //   CColliderMgr::Get_Instance()->Add_Collider(CColliderMgr::PLAYER, m_pBoxCOlliderForView);
 
    if (m_fSpeed >=8.f)
    {
        if(m_fSpeed==8.f)
        m_fFootTime += fTimeDelta;
        else
            m_fFootTime += fTimeDelta*3.f;
    }
    if (m_fFootTime > 1.0f)
    {
        CSoundMgr::Get_Instance()->Play_Effect(L"Step.ogg");
        m_fFootTime = 0.f;
    }


    return NO_EVENT;
}

_int CPlayerStatus::LateUpdate(const _float& fTimeDelta)
{


    CRenderer::Get_Instance()->Add_ColliderGroup(m_pSphereCollider);
    CRenderer::Get_Instance()->Add_ColliderGroup(m_pBoxCollider);
    CRenderer::Get_Instance()->Add_ColliderGroup(m_pBoxCOlliderForView);
    CRenderer::Get_Instance()->Add_NaviGroup(m_pNaviMesh);
    _vec3 vShaveDir;

    for (auto& pCol : CColliderMgr::Get_Instance()->Get_ColliderList(CColliderMgr::BOX, CColliderMgr::OBJECT))
    {


        if (CMathMgr::Get_Instance()->Collision_OBB(m_pBoxCollider, pCol, &vShaveDir))
        {
            int i = 0;
        }
    }


    AttackCheck(fTimeDelta);
    DamageByMonster(fTimeDelta);

    CollisionWithObject(fTimeDelta);

    m_ePreState = m_eCurState;
    return S_OK;
}

void CPlayerStatus::SetMesh(CMesh* pMesh)
{
    m_pMesh = pMesh;
    m_pMesh->AddRef();

    m_pBoxCollider = static_cast<Engine::CBoxCollider*>(CComponentMgr::Get_Instance()->Clone_Collider(L"Prototype_BoxCol", COMPONENTID::ID_STATIC, CCollider::COL_BOX, false, nullptr, _vec3(0.f, 6.f, 0.f), _vec3(0.f, 0.f, 0.f), 0.f, _vec3(100.f, 150.f, 100.f), nullptr));
    m_pBoxCOlliderForView = static_cast<Engine::CBoxCollider*>(CComponentMgr::Get_Instance()->Clone_Collider(L"Prototype_BoxCol", COMPONENTID::ID_STATIC, CCollider::COL_BOX, false, nullptr, _vec3(0.f, 10.f, 0.f), _vec3(0.f, 0.f, 0.f), 0.f, _vec3(60.f, 70.f, 60.f), nullptr));


    m_pSphereCollider = static_cast<Engine::CSphereCollider*>(CComponentMgr::Get_Instance()->Clone_Collider(L"Prototype_SphereCol", COMPONENTID::ID_STATIC, CCollider::COL_SPHERE, false, nullptr, _vec3(0.f, 0.f, 0.f), _vec3(0.f, 0.f, 0.f), 60.f/*여기반지름*/, _vec3(1.f, 1.f, 1.f), nullptr));


    m_pNaviMesh = static_cast<Engine::CNaviMesh*>(CComponentMgr::Get_Instance()->Clone_Component(L"Mesh_Navi", ID_STATIC));

    m_matChest = m_pMesh->Find_BoneMatrix("Chest");
    m_matChestOffset = m_pMesh->Find_BoneOffset("Chest");
}

void CPlayerStatus::KeyInput()
{
    if (m_eCurState == CPlayer::RIFLEDRAW || m_eCurState == CPlayer::RIFLEHOLSTER || m_eCurState == CPlayer::SNIPERDRAW || m_eCurState == CPlayer::SNIPERHOSTER 
        || m_eCurState == CPlayer::SNIPERRELOAD || m_eCurState == CPlayer::SNIPERATTACK ||m_eCurState == CPlayer::COVER_SNIPER_HOLSTER 
        ||m_eCurState == CPlayer::COVER_SNIPER_DRAW || m_eCurState == CPlayer::COVER_RELOAD||m_eCurState==CPlayer::COVER_RIFLE_DRAW||m_eCurState==CPlayer::COVER_RILFE_HOSTER||m_eCurState==CPlayer::COVER_ATTACKSNIPER)//무기드는중 리턴 
        return;

    if (m_pCamera == nullptr)
        return;
    if (m_eEquip == RIFLE)
    {
        if (!m_bIsShoot)
        {
            m_eCurState = CPlayer::RIFLEIDLE;
            if (m_bIsCover)
                m_eCurState = CPlayer::COVERIDLE;
        }
        else
        {
            m_eCurState = CPlayer::RIFLEATTACK;
            if (m_bIsCover)
                m_eCurState = CPlayer::COVERATTACK;
        }
    }
    else if (m_eEquip == SNIPER)
    {
        if (!m_bIsShoot)
        {

            m_eCurState = CPlayer::RIFLEIDLE;
            if (m_bIsCover)
                m_eCurState = CPlayer::COVERIDLE;
        }
        else
        {
            if (m_pCamera->Get_ZoomOut())
            {
                m_eCurState = CPlayer::SNIPERATTACK;
                if (m_bIsCover)
                    m_eCurState = CPlayer::COVER_ATTACKSNIPER;
            }
        }


    }
    else
    {
        m_pCamera->Set_ZoomInOut(false);
        m_eCurState = CPlayer::NONEIDLE;
        m_eLegState = CPlayer::NONEIDLE;

    }
    if ( KEY_PRESSING(DIKEYBOARD_R)&&  m_eEquip!= NONE && BulletCountCheck())
    {
        m_eCurState = CPlayer::SNIPERRELOAD;
        CSoundMgr::Get_Instance()->Play_Effect(L"WeaponReload.ogg");
        if(!m_bIsCover)
            m_eCurState = CPlayer::COVER_RELOAD;

        m_pCamera->Set_ZoomInOut(false);
    }




    if (MOUSE_PRESSING(MOUSEBUTTON::DIM_LB) && m_eEquip!=NONE &&ShootingBulletCheck())
    {
        if (m_eEquip == RIFLE )
        {
            m_eCurState = CPlayer::RIFLEATTACK;
            m_pCamera->Set_CameraShakeType(CDynamicCamera::RIFLE);
            if (m_bIsCover)
                m_eCurState = CPlayer::COVERATTACK;

            m_bIsShoot = true;
        }
        else if (m_eEquip == SNIPER )
        {
            if (m_pCamera->Get_ZoomOut())
            {
                m_pCamera->Set_CameraShakeType(CDynamicCamera::SNIPER);
                m_eCurState = CPlayer::SNIPERATTACK;

                if (m_bIsCover)
                    m_eCurState = CPlayer::COVER_ATTACKSNIPER;
                m_bIsShoot = true;

            }
        }



    }
    else
        m_bIsShoot = false;




    if ( WeaponStateCheck(RIFLE)&&   KEY_DOWN(DIKEYBOARD_2) && (m_eEquip == NONE ||m_eEquip == SNIPER))
    {
        if (!m_bIsShoot && m_eEquip == NONE)
        {
            m_eCurState = CPlayer::RIFLEDRAW;
            if (m_bIsCover)
                m_eCurState = CPlayer::COVER_RIFLE_DRAW;
            CSoundMgr::Get_Instance()->Play_Effect(L"Reload.ogg");
            m_pCamera->Set_ZoomInOut(false);
        }
        else if (!m_bIsShoot && m_eEquip == SNIPER) 
        {

            m_eCurState = CPlayer::SNIPERHOSTER;

            if (m_bIsCover)
                m_eCurState = CPlayer::COVER_SNIPER_HOLSTER;
            CSoundMgr::Get_Instance()->Play_Effect(L"WeaponReload.ogg");
            m_bIsSwapingWeapon = true;
            m_eSwapEquip = RIFLE;
            m_pCamera->Set_ZoomInOut(false);
        }


    }
    if (WeaponStateCheck(SNIPER)&& KEY_DOWN(DIKEYBOARD_3) && (m_eEquip == NONE || m_eEquip == RIFLE))
    {
        if (!m_bIsShoot &&m_eEquip == NONE)
        {

            m_eCurState = CPlayer::SNIPERDRAW;

            if (m_bIsCover)
                m_eCurState = CPlayer::COVER_SNIPER_DRAW;
            m_pCamera->Set_ZoomInOut(false);
            CSoundMgr::Get_Instance()->Play_Effect(L"WeaponReload.ogg");
        }
        else if (!m_bIsShoot && m_eEquip == RIFLE)
        {
            m_eCurState = CPlayer::RIFLEHOLSTER;
            m_bIsSwapingWeapon = true;
            m_eSwapEquip = SNIPER;
            if (m_bIsCover)
                m_eCurState = CPlayer::COVER_RILFE_HOSTER;
            CSoundMgr::Get_Instance()->Play_Effect(L"WeaponReload.ogg");
            m_pCamera->Set_ZoomInOut(false);
        }
    }





    if (KEY_PRESSING(DIKEYBOARD_G)&& !m_bIsCover)
    {
        if (m_eEquip == RIFLE)
        {
            
            m_pCamera->Set_ZoomInOut(false);
            if (!m_bIsShoot)
            {
                m_eCurState = CPlayer::RIFLEHOLSTER;
            }

        }
        else if (m_eEquip == SNIPER)
        {

            m_pCamera->Set_ZoomInOut(false);
            if (!m_bIsShoot)
            {
                m_eCurState = CPlayer::SNIPERHOSTER;
            }



        }




    }


    if (MOUSE_KEYDOWN(MOUSEBUTTON::DIM_RB))
    {
        if (m_eEquip == RIFLE)
        {
           
            if (!m_bIsCover)
            {
                _bool IsZoom = m_pCamera->Get_ZoomOut();
                if (IsZoom == false)
                    m_pCamera->Set_ZoomInOut(true);
                else
                    m_pCamera->Set_ZoomInOut(false);
            }
          
        }
        else if (m_eEquip == SNIPER&&!m_bIsShoot)
        {

            _bool IsZoom = m_pCamera->Get_ZoomOut();
            if (IsZoom == false)
            {
                m_bIsZoom = true;
                m_pCamera->Set_ZoomInOut(true, 20.f);
            }
            else
            {
                m_bIsZoom = false;
                m_pCamera->Set_ZoomInOut(false);

            }
        }
    }

}

void CPlayerStatus::LegKeyInput(const _float& fTimeDelt)
{
    if (KEY_PRESSING(DIKEYBOARD_W))
    {
        if (m_bIsCover)
            m_bIsCover = false;

        if (m_eEquip == RIFLE || m_eEquip == SNIPER)
        {
            m_pCamera->Set_ZoomInOut(false);

            m_eLegState = CPlayer::RIFLEWALKNORTH;
            m_fSpeed = 12.f;

            if (KEY_PRESSING(DIKEYBOARD_LSHIFT) && m_uiMp > 0)
            {
                // 기력 추가함 
                m_uiMp -= 10.f*fTimeDelt;

                m_eLegState = CPlayer::RUNNORTH;
                m_fSpeed = 25.f;
            }


        }
        else if (m_eEquip == NONE&&m_eCurState!=CPlayer::RIFLEDRAW&&m_eCurState!=CPlayer::SNIPERDRAW)
        {
            m_pCamera->Set_ZoomInOut(false);
            if (!m_bIsShoot)
                m_eCurState = CPlayer::NONEWALK;

            m_eLegState = CPlayer::NONEWALK;
            m_fSpeed = 12.f;
        }
    }
    else if (KEY_PRESSING(DIKEYBOARD_S)&&m_eEquip!=NONE)
    {

        if (m_bIsCover)
            m_bIsCover = false;

        m_pCamera->Set_ZoomInOut(false);

        m_eLegState = CPlayer::RIFLEWALKSOUTH;
        m_fSpeed = 12.f;

        if (KEY_PRESSING(DIKEYBOARD_LSHIFT) && m_uiMp > 0)
        {
            m_uiMp -= 10.f * fTimeDelt;
            m_eLegState = CPlayer::RUNSOUTH;
            m_fSpeed = 25.f;
        }
    }
    else if (KEY_PRESSING(DIKEYBOARD_A) && m_eEquip != NONE)
    {
        if (m_bIsCover)
            m_bIsCover = false;

        m_pCamera->Set_ZoomInOut(false);

        m_eLegState = CPlayer::RIFLEWALKWEST;
        m_fSpeed = 8.f;

        if (KEY_PRESSING(DIKEYBOARD_LSHIFT) && m_uiMp > 0)
        {
            m_uiMp -= 12.f * fTimeDelt;
            m_eLegState = CPlayer::RUNWEST;
            m_fSpeed = 25.f;
        }
    }
    else if (KEY_PRESSING(DIKEYBOARD_D) && m_eEquip != NONE)
    {
        if (m_bIsCover)
            m_bIsCover = false;

        m_pCamera->Set_ZoomInOut(false);

        m_eLegState = CPlayer::RIFLEWALKEAST;
        m_fSpeed = 8.f;

        if (KEY_PRESSING(DIKEYBOARD_LSHIFT) && m_uiMp > 0)
        {
            m_uiMp -= 12.f * fTimeDelt;
            m_eLegState = CPlayer::RUNEAST;
            m_fSpeed = 25.f;
        }
    }
    else
    {
        if (m_eEquip == NONE)
        m_eLegState = CPlayer::NONEIDLE;
        else
            m_eLegState = CPlayer::RIFLEIDLE;

        m_fSpeed = 0.f;

    }
    if (m_bIsCover == true)
      {
            m_eLegState = m_eCurState;
       
      }

}

void CPlayerStatus::WeaponChange()
{
    if (m_eCurState == CPlayer::RIFLEDRAW || m_eCurState ==CPlayer::COVER_RIFLE_DRAW)
    {

        list<CGameObject*>* pList = CObjectMgr::Get_Instance()->Get_OBJLIST(L"Layer_GameObject", L"Weapon");

        for (auto& pSrc : *pList)
        {
            if (static_cast<CWeapon*>(pSrc)->Get_WeaponType() == CWeapon::RIFLE && static_cast<CWeapon*>(pSrc)->Get_WeaponState() == CWeapon::BAG)
            {
                if (m_eCurState ==CPlayer::RIFLEDRAW  &&m_pMesh->Set_FindAnimation(1800.f, (int)CPlayer::RIFLEDRAW))
                {
                    static_cast<CWeapon*>(pSrc)->SetWeaponState(CWeapon::EQUIP);
                    
                }
                if (m_eCurState == CPlayer::COVER_RIFLE_DRAW && m_pMesh->Set_FindAnimation(1800.f, (int)CPlayer::COVER_RIFLE_DRAW))
                {
                    static_cast<CWeapon*>(pSrc)->SetWeaponState(CWeapon::EQUIP);

                }

            }
        }

        if (m_eCurState == CPlayer::RIFLEDRAW  && m_pMesh->Set_FindAnimation(5500.f, (int)CPlayer::RIFLEDRAW))
        {
            m_eCurState = CPlayer::RIFLEIDLE;
            m_eEquip = RIFLE;
        }
        if (m_eCurState == CPlayer::COVER_RIFLE_DRAW && m_pMesh->Set_FindAnimation(5500.f, (int)CPlayer::COVER_RIFLE_DRAW))
        {

            m_eCurState = CPlayer::COVERIDLE;
            m_eEquip = RIFLE;
        }


    }

    if (m_eCurState == CPlayer::SNIPERDRAW || m_eCurState == CPlayer::COVER_SNIPER_DRAW)
    {

        list<CGameObject*>* pList = CObjectMgr::Get_Instance()->Get_OBJLIST(L"Layer_GameObject", L"Weapon");

        for (auto& pSrc : *pList)
        {
            if (static_cast<CWeapon*>(pSrc)->Get_WeaponType() == CWeapon::SNIPER && static_cast<CWeapon*>(pSrc)->Get_WeaponState() == CWeapon::BAG)
            {
                if (m_eCurState==CPlayer::SNIPERDRAW  && m_pMesh->Set_FindAnimation(1800.f, (int)CPlayer::SNIPERDRAW))
                {
                    static_cast<CWeapon*>(pSrc)->SetWeaponState(CWeapon::EQUIP);
                }

                if (m_eCurState == CPlayer::COVER_SNIPER_DRAW && m_pMesh->Set_FindAnimation(1800.f, (int)CPlayer::COVER_SNIPER_DRAW))
                {
                    static_cast<CWeapon*>(pSrc)->SetWeaponState(CWeapon::EQUIP);
                }
            }
        }

        if (m_eCurState==CPlayer::SNIPERDRAW &&  m_pMesh->Set_FindAnimation(5500.f, (int)CPlayer::SNIPERDRAW))
        {
            m_eCurState = CPlayer::RIFLEIDLE;

            m_eEquip = SNIPER;
        }
        if (m_eCurState == CPlayer::COVER_SNIPER_DRAW && m_pMesh->Set_FindAnimation(5500.f, (int)CPlayer::COVER_SNIPER_DRAW))
        {
            m_eCurState = CPlayer::COVERIDLE;

            m_eEquip = SNIPER;
        }
    }









    if (m_eCurState == CPlayer::RIFLEHOLSTER || m_eCurState == CPlayer::COVER_RILFE_HOSTER)
    {
        list<CGameObject*>* pList = CObjectMgr::Get_Instance()->Get_OBJLIST(L"Layer_GameObject", L"Weapon");

        for (auto& pSrc : *pList)
        {
            if (static_cast<CWeapon*>(pSrc)->Get_WeaponType() == CWeapon::RIFLE && static_cast<CWeapon*>(pSrc)->Get_WeaponState() == CWeapon::EQUIP)
            {
                if (m_eCurState==CPlayer::RIFLEHOLSTER &&m_pMesh->Set_FindAnimation(5000.f, (int)CPlayer::RIFLEHOLSTER))
                {
                    static_cast<CWeapon*>(pSrc)->SetWeaponState(CWeapon::BAG);
                }
                if (m_eCurState == CPlayer::COVER_RILFE_HOSTER && m_pMesh->Set_FindAnimation(5000.f, (int)CPlayer::COVER_RILFE_HOSTER))
                {
                    static_cast<CWeapon*>(pSrc)->SetWeaponState(CWeapon::BAG);
                }

            }
        }

        if (m_eCurState==CPlayer::RIFLEHOLSTER && m_pMesh->Set_FindAnimation(5000.f, (_int)CPlayer::RIFLEHOLSTER))
        {
            m_eCurState = CPlayer::NONEIDLE;
            m_eEquip = NONE;
            if (m_bIsSwapingWeapon && m_eSwapEquip == SNIPER)
            {
                m_eCurState = CPlayer::SNIPERDRAW;
                m_bIsSwapingWeapon = false;
                m_eEquip = SNIPER;
                m_eSwapEquip = NONE;
            }
        }
        if (m_eCurState == CPlayer::COVER_RILFE_HOSTER && m_pMesh->Set_FindAnimation(5000.f, (_int)CPlayer::COVER_RILFE_HOSTER))
        {
            m_eCurState = CPlayer::NONEIDLE;
            m_eEquip = NONE;
            if (m_bIsSwapingWeapon && m_eSwapEquip == SNIPER)
            {
                m_eCurState = CPlayer::COVER_SNIPER_DRAW;
                m_bIsSwapingWeapon = false;
                m_eEquip = SNIPER;
                m_eSwapEquip = NONE;
            }
        }
    }

    if (m_eCurState == CPlayer::SNIPERHOSTER || m_eCurState == CPlayer::COVER_SNIPER_HOLSTER)
    {
        list<CGameObject*>* pList = CObjectMgr::Get_Instance()->Get_OBJLIST(L"Layer_GameObject", L"Weapon");

        for (auto& pSrc : *pList)
        {
            if (static_cast<CWeapon*>(pSrc)->Get_WeaponType() == CWeapon::SNIPER && static_cast<CWeapon*>(pSrc)->Get_WeaponState() == CWeapon::EQUIP)
            {
                if (m_eCurState==CPlayer::SNIPERHOSTER &&  m_pMesh->Set_FindAnimation(5000.f, (int)CPlayer::SNIPERHOSTER))
                {
                    static_cast<CWeapon*>(pSrc)->SetWeaponState(CWeapon::BAG);
                }

                if (m_eCurState == CPlayer::COVER_SNIPER_HOLSTER && m_pMesh->Set_FindAnimation(5000.f, (int)CPlayer::COVER_SNIPER_HOLSTER))
                {
                    static_cast<CWeapon*>(pSrc)->SetWeaponState(CWeapon::BAG);
                }
            }
        }

        if (m_eCurState ==CPlayer::SNIPERHOSTER &&  m_pMesh->Set_FindAnimation(5000.f, (_int)CPlayer::SNIPERHOSTER))
        {
            m_eCurState = CPlayer::NONEIDLE;
            m_eEquip = NONE;

            if (m_bIsSwapingWeapon && m_eSwapEquip == RIFLE)
            {
                m_eCurState = CPlayer::RIFLEDRAW;
                m_eEquip = RIFLE;
                m_bIsSwapingWeapon = false;
                m_eSwapEquip = NONE;

            }
        }
        if (m_eCurState == CPlayer::COVER_SNIPER_HOLSTER && m_pMesh->Set_FindAnimation(5000.f, (_int)CPlayer::COVER_SNIPER_HOLSTER))
        {
            m_eCurState = CPlayer::NONEIDLE;
            m_eEquip = NONE;

            if (m_bIsSwapingWeapon && m_eSwapEquip == RIFLE)
            {
                m_eCurState = CPlayer::COVER_RIFLE_DRAW;
                m_eEquip = RIFLE;
                m_bIsSwapingWeapon = false;
                m_eSwapEquip = NONE;

            }
        }
    }


}

bool CPlayerStatus::WeaponStateCheck(EQUIPTYPE eType)
{

    list<CGameObject*>* pList = CObjectMgr::Get_Instance()->Get_OBJLIST(L"Layer_GameObject", L"Weapon");

    if (pList == nullptr)
        return FALSE;

    for (auto& pSrc : *pList)
    {
        if (CWeapon::BAG == dynamic_cast<CWeapon*>(pSrc)->Get_WeaponState())
        {
            if (eType == RIFLE && dynamic_cast<CWeapon*>(pSrc)->Get_WeaponType() == CWeapon::RIFLE)
            {
                return true;

            }
            if (eType == SNIPER && dynamic_cast<CWeapon*>(pSrc)->Get_WeaponType() == CWeapon::SNIPER)
            {

                return true;

            }

        }

    }

    return false;

}


     


void CPlayerStatus::CollisionWithObject(const _float& fTimeDelta)
{

    _vec3 vShaveDir;
    for (auto& pCol : CColliderMgr::Get_Instance()->Get_ColliderList(CColliderMgr::BOX, CColliderMgr::OBJECT))
    {
        if (CMathMgr::Get_Instance()->Collision_OBB(m_pBoxCollider, pCol, &vShaveDir))
        {
            m_pTransCom->m_vPos += vShaveDir;
            
            m_pTransCom->m_matWorld._41 += vShaveDir.x;
            m_pTransCom->m_matWorld._42 += vShaveDir.y;
            m_pTransCom->m_matWorld._43 += vShaveDir.z;
        }

    }

    // 통로 박스용 충돌 하나 만들게 
    for (auto& pCol : CColliderMgr::Get_Instance()->Get_ColliderList(CColliderMgr::BOX, CColliderMgr::PASSBOX))
    {
        if (CMathMgr::Get_Instance()->Collision_OBB(m_pBoxCollider, pCol, &vShaveDir))
        {
            m_pTransCom->m_vPos += vShaveDir;

            m_pTransCom->m_matWorld._41 += vShaveDir.x;
            m_pTransCom->m_matWorld._42 += vShaveDir.y;
            m_pTransCom->m_matWorld._43 += vShaveDir.z;
        }
    }
}

void CPlayerStatus::CutSceneCheck()
{

    if (m_eCurScene == FIRSTSCENE)
    {

        m_fSpine = 0.f;
        m_pTransCom->m_vPos = _vec3(300.f, 0.f, 480.f);
        m_pTransCom->m_vAngle = _vec3{ 0.f,0.f,0.f };
        m_bIsKeyLock = true;


        if (m_eCurState == CPlayer::HEADSTART)
        {
            if (m_pMesh->Set_IsAniFinsh(400.f))
                m_eCurState = CPlayer::HEADIDLE;

        }
        if (m_eCurState == CPlayer::HEADEXIT)
        {

            if (m_pMesh->Set_IsAniFinsh(400.f))
                m_eCurState = CPlayer::NONEIDLE;

        }
    }


}

void CPlayerStatus::ShootingCheck()
{
    ShootingBossCheck();
    CGameObject* pMonster = nullptr;

    float fMinDistToMonster = 99999.f;
    float fDist = 0;
    bool Collision = false;

    bool bIsBossCollision = false;
    for (auto& pCol : CColliderMgr::Get_Instance()->Get_ColliderList(CColliderMgr::SPHERE, CColliderMgr::MONSTER))
    {
         Collision = CMathMgr::Get_Instance()->Collision_SphereWithCamera(pCol,&fDist);
        if ((fDist < fMinDistToMonster) && Collision)
        {
            fMinDistToMonster = fDist;
            pMonster = pCol->Get_Owner();
          
        }

    }
    for (auto& pCol : CColliderMgr::Get_Instance()->Get_ColliderList(CColliderMgr::SPHERE, CColliderMgr::BOSS))
    {

        Collision = CMathMgr::Get_Instance()->Collision_SphereWithCamera(pCol, &fDist);
        if ((fDist < fMinDistToMonster) && Collision)
        {
            fMinDistToMonster = fDist;
            pMonster = pCol->Get_Owner();
            bIsBossCollision = true;

        }

    }


  /*  CGameObject* pObject = nullptr;
    float fMinDistToObject = 999.f;

    fDist = 0.f;
    for (auto& pCol : CColliderMgr::Get_Instance()->Get_ColliderList(CColliderMgr::BOX, CColliderMgr::BULLETDECAL))
    {
        Collision = CMathMgr::Get_Instance()->Collision_BoXWithCamera(pCol, &fDist);
        if ((fDist < fMinDistToObject) && Collision)
        {
            fMinDistToObject = fDist;
            pObject = pCol->Get_Owner();

        }

    }*/

    if (pMonster == nullptr)
        return;

 /*   if (fMinDistToObject < fMinDistToMonster)
        return;*/
    if (!bIsBossCollision)
    {
        CMonster::MONKIND  eMonKind = CMonster::NONAME;
        eMonKind = dynamic_cast<CMonster*>(pMonster)->Get_MONKIND();

        float AttackDamage = 30;

        if (m_eEquip == SNIPER)
            AttackDamage = 110;

        if (eMonKind == CMonster::ZOMBI)
        {
            CZombi* pZombi = dynamic_cast<CMonster*>(pMonster)->Get_Zombi();
            pZombi->Set_HitDamage(AttackDamage);
            pZombi->Set_IsHit(true);

        }

        if (eMonKind == CMonster::DRON)
        {
            CDron* pDron = dynamic_cast<CMonster*>(pMonster)->Get_Dron();
            pDron->Set_HitDamage(AttackDamage);
            pDron->Set_IsHit(true);

        }
    }
    else
    {
        int iDamage = 5.f;
        if (m_eEquip == SNIPER)
            iDamage = 20.f;
        static_cast<CReapear*>(pMonster)->SetDamage(iDamage);
    }



    }

void CPlayerStatus::ShootingBossCheck()
{
    float fDist= 999 ;

    _bool Collision;
    CGameObject* pGameObject = nullptr;
    for (auto& pCol : CColliderMgr::Get_Instance()->Get_ColliderList(CColliderMgr::BOX, CColliderMgr::BOSS))
    {
        Collision = CMathMgr::Get_Instance()->Collision_BoXWithCamera(pCol, &fDist);
        if (Collision)
        {
       
            pGameObject = pCol->Get_Owner();

        }

    }
    if (pGameObject == nullptr)
        return;


    static_cast<CReapearTube*>(pGameObject)->SetDamage();



}








void CPlayerStatus::StatusUpdate(const _float& fTimeDelta)
{

    if (m_pTransCom == nullptr)
        return;

    _vec3 vRight;
    memcpy(&vRight, &m_pTransCom->m_matWorld._11, sizeof(_vec3));
    vRight.Normalize();
    switch (m_eLegState)
    {
    case CPlayer::RIFLEWALKNORTH:
    {
        m_pTransCom->m_vPos = m_pNaviMesh->MoveOn_NaviMesh(&m_pTransCom->m_vPos, &m_pTransCom->m_vDir, m_fSpeed * fTimeDelta);
        break;
    }
    case CPlayer::RIFLEWALKSOUTH:
    {
        m_pTransCom->m_vPos = m_pNaviMesh->MoveOn_NaviMesh(&m_pTransCom->m_vPos, &_vec3(m_pTransCom->m_vDir * -1), m_fSpeed * fTimeDelta);
        break;
    }
    case CPlayer::RIFLEWALKEAST:
    {
        m_pTransCom->m_vPos = m_pNaviMesh->MoveOn_NaviMesh(&m_pTransCom->m_vPos, &_vec3(vRight), m_fSpeed * fTimeDelta);
        break;
    }
    case CPlayer::RIFLEWALKWEST:
    {
        m_pTransCom->m_vPos = m_pNaviMesh->MoveOn_NaviMesh(&m_pTransCom->m_vPos, &_vec3(vRight * -1.f), m_fSpeed * fTimeDelta);
        break;
    }
    case CPlayer::NONEWALK:
    {
        m_pTransCom->m_vPos = m_pNaviMesh->MoveOn_NaviMesh(&m_pTransCom->m_vPos, &m_pTransCom->m_vDir, m_fSpeed * fTimeDelta);
        break;

    }
    case CPlayer :: RUNNORTH:
    {
        m_pTransCom->m_vPos = m_pNaviMesh->MoveOn_NaviMesh(&m_pTransCom->m_vPos, &m_pTransCom->m_vDir, m_fSpeed * fTimeDelta);
        break;
    }
    case CPlayer::RUNSOUTH:
    {
        m_pTransCom->m_vPos = m_pNaviMesh->MoveOn_NaviMesh(&m_pTransCom->m_vPos, &_vec3(m_pTransCom->m_vDir * -1), m_fSpeed * fTimeDelta);
        break;
    }
    case CPlayer::RUNWEST:        
    {
        m_pTransCom->m_vPos = m_pNaviMesh->MoveOn_NaviMesh(&m_pTransCom->m_vPos, &_vec3(vRight * -1.f), m_fSpeed * fTimeDelta);
        break;
    }
    case CPlayer::RUNEAST:
    {
        m_pTransCom->m_vPos = m_pNaviMesh->MoveOn_NaviMesh(&m_pTransCom->m_vPos, &_vec3(vRight), m_fSpeed * fTimeDelta);
        break;
    }
    default:
        break;
    }



}

void CPlayerStatus::Rotation(const _float& fTimeDelta)
{

    //m_pTransCom->Chase_Target(m_vecTargetPos, fTimeDelta*0.05f);
}

void CPlayerStatus::PlayerDirection(const _float& fTimeDelta)
{

    _long dwMouseMove;


    if (dwMouseMove = CDirectInput::Get_Instance()->Get_DIMouseMove(CDirectInput::DIMM_X) )
    {

        _vec3 vUp = _vec3{ 0.f,1.f,0.f };


        m_pTransCom->m_vAngle.y += (_float)dwMouseMove * fTimeDelta *2.5f;
        m_pTransCom->m_vDir = _vec3{ m_pTransCom->m_matWorld._31,m_pTransCom->m_matWorld._32,m_pTransCom->m_matWorld._33 };
        m_pTransCom->m_vDir.y = 0.f;
        m_pTransCom->m_vDir.Normalize();
        m_vecTargetPos = m_pTransCom->m_vPos + (m_pTransCom->m_vDir) * 30.f;

        if (m_bIsCover)
        {
            if (m_fCoverAngleY + 30.f < m_pTransCom->m_vAngle.y)
                m_pTransCom->m_vAngle.y = m_fCoverAngleY + 30.f;
            if(m_fCoverAngleY-30.f>m_pTransCom->m_vAngle.y)
                m_pTransCom->m_vAngle.y = m_fCoverAngleY - 30.f;


        }

    }
    if (dwMouseMove = CDirectInput::Get_Instance()->Get_DIMouseMove(CDirectInput::DIMM_Y))
    {
        m_fSpine -= dwMouseMove * fTimeDelta;

        if (m_fSpine > 30.f)
            m_fSpine = 30.f;
        if (m_fSpine < -30.f)
            m_fSpine = -30.f;
    }




}

void CPlayerStatus::AttackCheck(const _float& fTimeDelta)
{
    if (m_eCurState == CPlayer::RIFLEATTACK)
    {


        m_fShootingTime += fTimeDelta;

        if (m_fShootingTime > 0.3f)
        {
            list<CGameObject*>* pList = CObjectMgr::Get_Instance()->Get_OBJLIST(L"Layer_GameObject", L"Weapon");

            for (auto& pSrc : *pList)
            {
                if (static_cast<CWeapon*>(pSrc)->Get_WeaponType() == CWeapon::RIFLE && static_cast<CWeapon*>(pSrc)->Get_WeaponState() == CWeapon::EQUIP)
                {
                    static_cast<CWeapon*>(pSrc)->CreateShootEffect();
                }
            }


            ShootingCheck();
            m_fShootingTime = 0.f;
        }

    }
    if (m_eCurState == CPlayer::SNIPERATTACK)
    {


        if (m_eCurState != m_ePreState)
        {
            list<CGameObject*>* pList = CObjectMgr::Get_Instance()->Get_OBJLIST(L"Layer_GameObject", L"Weapon");

            for (auto& pSrc : *pList)
            {
                if (static_cast<CWeapon*>(pSrc)->Get_WeaponType() == CWeapon::SNIPER && static_cast<CWeapon*>(pSrc)->Get_WeaponState() == CWeapon::EQUIP)
                {

                    CSoundMgr::Get_Instance()->Play_Effect(L"Sniper.wav");
                    static_cast<CWeapon*>(pSrc)->CreateShootEffect();
                }
            }

            ShootingCheck();
            m_fShootingTime = 0.f;
        }

    }




    if (m_bIsShoot == false)
        m_fShootingTime = 0.f;



}

void CPlayerStatus::DamageByMonster(const _float& fTimeDelta)
{
    if (m_bIshit == true)
    {
        m_fhitCool += fTimeDelta;

        if (m_fhitCool > 1.f)
        {
            m_fhitCool = 0.f;
            m_bIshit = false;
        }

    }

    if (m_bIshit)
        return;


    for (auto& pCol : CColliderMgr::Get_Instance()->Get_ColliderList(CColliderMgr::SPHERE, CColliderMgr::MONSTER))
    {
        _vec3 vDir;
        if (CMathMgr::Get_Instance()->Collision_Spere(pCol, m_pSphereCollider, &vDir))
        {

            CGameObject* pMonster = pCol->Get_Owner();

            if (pMonster == nullptr)
                return;
            CMonster::MONKIND  eMonKind = CMonster::NONAME;
            eMonKind = dynamic_cast<CMonster*>(pMonster)->Get_MONKIND();

            if (eMonKind == CMonster::ZOMBI)
            {
                CZombi* pZombi = dynamic_cast<CMonster*>(pMonster)->Get_Zombi();
                if (pZombi->Get_IsAtkPlayer())
                {
                    CObjectMgr::Get_Instance()->Add_GameObject(L"Layer_UI", L"Prototype_DamageBlood", L"Damage", nullptr);
                    m_bIshit = true;
                    m_uiHp -= 11;

                }

            }

        }


    }


}

void CPlayerStatus::CheckAim()
{
    CGameObject* pAim = CObjectMgr::Get_Instance()->Get_GameObject(L"Layer_UI", L"Aim");
    if (pAim == nullptr)
        return;
  

    if (m_eEquip == RIFLE)
    {
        dynamic_cast<CAim*>(pAim)->SetRender(true);

    }
    else if (m_eEquip == SNIPER)
    {
        if (m_pCamera->Get_ZoomOut() == true)
            dynamic_cast<CAim*>(pAim)->SetRender(true, 1);
        else
            dynamic_cast<CAim*>(pAim)->SetRender(false,2);


    }
    else
        dynamic_cast<CAim*>(pAim)->SetRender(false,2);

}

void CPlayerStatus::ReloadCheck()
{
    if (m_eCurState == CPlayer::SNIPERRELOAD)
    {
        if (m_pMesh->Set_IsAniFinsh()&&m_eCurState==m_ePreState)
        {
            m_eCurState = CPlayer::RIFLEIDLE;

        }
    }

    if (m_eCurState == CPlayer::COVER_RELOAD && m_eCurState == m_ePreState)
    {

        if (m_pMesh->Set_IsAniFinsh())
        {
            m_eCurState = CPlayer::COVERIDLE;

        }

    }
 



}

bool CPlayerStatus::BulletCountCheck()
{


    list<CGameObject*>* pList = CObjectMgr::Get_Instance()->Get_OBJLIST(L"Layer_GameObject", L"Weapon");

    if (pList == nullptr)
        return FALSE;

    for (auto& pSrc : *pList)
    {
        if (CWeapon::EQUIP == dynamic_cast<CWeapon*>(pSrc)->Get_WeaponState())
        {

            if (dynamic_cast<CWeapon*>(pSrc)->GetMaxBulletCount() < 0)
                return false;
            dynamic_cast<CWeapon*>(pSrc)->Reload();
            return true;
     
        }

    }
    return true;
}

bool CPlayerStatus::ShootingBulletCheck()
{



    list<CGameObject*>* pList = CObjectMgr::Get_Instance()->Get_OBJLIST(L"Layer_GameObject", L"Weapon");

    if (pList == nullptr)
        return FALSE;

    for (auto& pSrc : *pList)
    {
        if (CWeapon::EQUIP == dynamic_cast<CWeapon*>(pSrc)->Get_WeaponState())
        {

            if (dynamic_cast<CWeapon*>(pSrc)->GetCurBulletCount() <= 0)
                return false;
           
            return true;
        }

    }
    return true;

}

void CPlayerStatus::CheckSniping()
{

    if (m_eCurState == CPlayer::SNIPERATTACK && m_pMesh->Set_IsAniFinsh())
    {

        m_eCurState = CPlayer::RIFLEIDLE;
        m_bIsShoot = false;


        if (m_bIsZoom)
            m_pCamera->Set_ZoomInOut(true, 20.f);
    }
    if (m_eCurState == CPlayer::COVER_ATTACKSNIPER && m_pMesh->Set_FindAnimation(2200.f,(int)CPlayer::COVER_ATTACKSNIPER))
    {

        m_eCurState = CPlayer::COVERIDLE;
        m_bIsShoot = false;

        m_bIsZoom = false;
      
    }
}

void CPlayerStatus::CoverCheck()
{
    if (KEY_DOWN(DIK_U))
    {
        m_bIsCover = true;
        m_vCoverPoint = m_pTransCom->m_vPos;
        m_fCoverAngleY = m_pTransCom->m_vAngle.y;
    }
    if (KEY_DOWN(DIK_I))
        m_bIsCover = false;

}

void CPlayerStatus::CameraShakeCheck()
{

    if (m_eCurState == CPlayer::RIFLEATTACK ||m_eCurState ==CPlayer::COVERATTACK)
    {
        if(m_eLegState ==CPlayer::RIFLEIDLE)
        m_pCamera->Set_CameraShakeType(CDynamicCamera::RIFLE);
        else if(m_eLegState == CPlayer::RUNEAST || m_eLegState == CPlayer::RUNWEST|| m_eLegState == CPlayer::RUNNORTH|| m_eLegState == CPlayer::RUNSOUTH)
            m_pCamera->Set_CameraShakeType(CDynamicCamera::RIFLERUN);
        else
            m_pCamera->Set_CameraShakeType(CDynamicCamera::RIFLEWALK);

    }
    else if ( (m_eCurState==CPlayer::COVER_ATTACKSNIPER ||m_eCurState == CPlayer::SNIPERATTACK)&& m_pCamera->Get_CameraShakeType()==CDynamicCamera::SNIPER)
    {
     
        return;

    }
    else if ((m_eCurState == CPlayer::COVER_ATTACKSNIPER || m_eCurState == CPlayer::SNIPERATTACK) && m_pCamera->Get_CameraShakeType() == CDynamicCamera::NONE)
    {

        m_pCamera->Set_ZoomInOut(false);
    }
    else
        m_pCamera->Set_CameraShakeType(CDynamicCamera::NONE);



}

void CPlayerStatus::CoverPositionCheck()
{
    if (m_bIsCover  && m_bIsShoot == true)
    {
        _vec3 vRight;
        memcpy(&vRight, &m_pTransCom->m_matWorld._31, sizeof(_vec3));
        vRight.Normalize();

        m_pTransCom->m_vPos = m_vCoverPoint + (vRight * 5.f);


    }


}

void CPlayerStatus::WeaponSwap()
{
}

void CPlayerStatus::HPMpCheck(const _float& fTimeDelta) // BAR 체크 약간 수정
{
    list<CGameObject*>* pHpBarUIList = CObjectMgr::Get_Instance()->Get_OBJLIST(L"Layer_UI", L"HPBarUI");
    list<CGameObject*>* pMpBarUIList = CObjectMgr::Get_Instance()->Get_OBJLIST(L"Layer_UI", L"MPBarUI");
    if (pHpBarUIList != nullptr && m_bIshit)
    {
        for (auto& pSrc : *pHpBarUIList)
            if (CHPBar::PLAYER_HPBAER == dynamic_cast<CHPBar*>(pSrc)->Get_CurHpState())
                dynamic_cast<CHPBar*>(pSrc)->Set_CurHpType(0);
    }

    if (pMpBarUIList != nullptr && KEY_PRESSING(DIKEYBOARD_LSHIFT))
    {
        for (auto& pSrc : *pMpBarUIList)
            dynamic_cast<CMPBar*>(pSrc)->Set_CurMpType(0);
    }
}

void CPlayerStatus::Test()
{
    for (auto& pCol : CColliderMgr::Get_Instance()->Get_ColliderList(CColliderMgr::BOX, CColliderMgr::NPC))
    {
        float fDir;
        if (CMathMgr::Get_Instance()->Collision_ViewAngle(m_pTransCom->m_vPos,m_pTransCom->m_vDir, pCol, 80.f,&fDir))
        {
            int i = 0;
        }

    }


}
