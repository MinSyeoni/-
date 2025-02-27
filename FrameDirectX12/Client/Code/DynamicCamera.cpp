#include "stdafx.h"
#include "DynamicCamera.h"
#include "DirectInput.h"
#include "GraphicDevice.h"
#include "ObjectMgr.h"
#include "PlayerArm.h"
#include "PlayerLeg.h"
CDynamicCamera::CDynamicCamera(ID3D12Device* pGraphicDevice, ID3D12GraphicsCommandList* pCommandList)
    : Engine::CCamera(pGraphicDevice, pCommandList)
{
}

CDynamicCamera::CDynamicCamera(const CDynamicCamera& rhs)
    : Engine::CCamera(rhs)
    , m_fViewZ(rhs.m_fViewZ)
    , m_vDir(rhs.m_vDir)
{
}


CDynamicCamera::~CDynamicCamera()
{
}

HRESULT CDynamicCamera::Ready_GameObjectPrototype(const CAMERA_INFO& tCameraInfo,
    const PROJ_INFO& tProjInfo,
    const ORTHO_INFO& tOrthoInfo)
{
#ifdef _DEBUG
    COUT_STR("Ready Prototype DynamicCamera");
#endif

    FAILED_CHECK_RETURN(Engine::CCamera::Ready_GameObject(tCameraInfo, tProjInfo, tOrthoInfo), E_FAIL);

    return S_OK;
}

HRESULT CDynamicCamera::Ready_GameObject()
{
#ifdef _DEBUG
    COUT_STR("Ready Clone DynamicCamera");
#endif

    return S_OK;
}

HRESULT CDynamicCamera::LateInit_GameObject()
{
#ifdef _DEBUG
    COUT_STR("LateInit DynamicCamera");
#endif
    if (nullptr != CObjectMgr::Get_Instance()->Get_GameObject(L"Layer_GameObject", L"Player"))
    {
        m_pPlayer = static_cast<CPlayer*>(CObjectMgr::Get_Instance()->Get_GameObject(L"Layer_GameObject", L"Player"));
        m_pPlayerMatrix = &m_pPlayer->Get_Transform()->m_matWorld;
        m_pPlayerArm = static_cast<CPlayerArm*>(m_pPlayer->Get_PlayerArm());


        m_pmatCamera = static_cast<CMesh*>(m_pPlayerArm->Get_Component(L"Com_Mesh", COMPONENTID::ID_STATIC))->Get_AnimationComponent()->Get_CameraMatrix();

        
        //   m_pmatLegCamera= static_cast<CMesh*>(m_pPlayerLeg->Get_Component(L"Com_Mesh", COMPONENTID::ID_STATIC))->Get_AnimationComponent()->Get_CameraMatrix();
    }
    return S_OK;
}

_int CDynamicCamera::Update_GameObject(const _float& fTimeDelta)
{

    if (m_eShakeType == RIFLE)
    {
        m_fCameraShakeX += fTimeDelta * 30.f;
        m_fCameraShakeY += fTimeDelta * 13.f;
        m_fRadiusX = 0.015f;
        m_fRadiusY = 0.02f;
    }
    else if (m_eShakeType == RIFLERUN)
    {
        m_fCameraShakeX += fTimeDelta * 40.f;
        m_fCameraShakeY += fTimeDelta * 3.f;
        m_fRadiusX = 0.03f;
        m_fRadiusY = 0.035f;


    }
    else if (m_eShakeType == RIFLEWALK)
    {
        m_fCameraShakeX += fTimeDelta * 35.f;
        m_fCameraShakeY += fTimeDelta * 18.f;
        m_fRadiusX = 0.025f;
        m_fRadiusY = 0.03f;


    }
    else if (m_eShakeType == SNIPER)
    {

        m_fCameraShakeY += fTimeDelta * 5.f;
        m_fCameraShakeX = 0.f;
        m_fRadiusY = 2.f;

        if (3.14/2 <(m_fCameraShakeY))
        {
            m_fCameraShakeX = 0.f;
            m_fCameraShakeY = 0.f;
            m_fRadiusX = 0.f;
            m_fRadiusY = 0.f;
            m_eShakeType = NONE;

        }
    }
    else
    {
        m_fCameraShakeX = 0.f;
        m_fCameraShakeY = 0.f;
        m_fRadiusX = 0.f;
        m_fRadiusY = 0.f;

    }
    FAILED_CHECK_RETURN(Engine::CGameObject::LateInit_GameObject(), E_FAIL);

    /*____________________________________________________________________
    View ��� Update.
    ______________________________________________________________________*/

    if (m_bIsZoom == true)
    {
        m_fZoom -= fTimeDelta * 500.f;
        if (m_fMaxZoom > m_fZoom)
            m_fZoom = m_fMaxZoom;
        Set_ProjForV(m_fFov);
    }
    else
    {
        m_fZoom += fTimeDelta * 500.f;
        if (m_fMinZoom < m_fZoom)
            m_fZoom = m_fMinZoom;
        Set_ProjForV(65.f);
    }


    MouseInput();


    Engine::CGameObject::Update_GameObject(fTimeDelta);
    Engine::CCamera::Update_GameObject(fTimeDelta);
    _matrix BoneMatrix = (*m_pmatCamera);
    _matrix RotY = XMMatrixRotationX(XMConvertToRadians(-90));
    _matrix CameraMatrix = BoneMatrix * RotY **m_pPlayerMatrix;
    CameraMatrix = XMMatrixInverse(0, CameraMatrix);
    CGraphicDevice::Get_Instance()->SetViewMatrix(m_tCameraInfo.matView);
    CGraphicDevice::Get_Instance()->SetProjMatrix(m_tProjInfo.matProj);

    if (m_bIsDead)
        return DEAD_OBJ;

    return NO_EVENT;

}

_int CDynamicCamera::LateUpdate_GameObject(const _float& fTimeDelta)
{
    return NO_EVENT;
}

void CDynamicCamera::Render_GameObject(const _float& fTimeDelta)
{
}

void CDynamicCamera::MouseInput()
{





    if (m_pmatCamera != nullptr)
    {

        _matrix RotY = XMMatrixRotationY(XMConvertToRadians(-90));


        _matrix BoneMatrix = (*m_pmatCamera);
        _matrix CameraMatrix = BoneMatrix * *m_pPlayerMatrix;


        _vec3 CameraRight;
        _vec3 CameraUp;
        _vec3 CameraLook;
        _vec3 CameraPos;

        memcpy(&CameraRight, &CameraMatrix._11, sizeof(_vec3));//����
        memcpy(&CameraUp, &CameraMatrix._21, sizeof(_vec3));
        memcpy(&CameraLook, &CameraMatrix._31, sizeof(_vec3));//����
        memcpy(&CameraPos, &CameraMatrix._41, sizeof(_vec3));



        _float fSpine = m_pPlayer->Get_SpineAngle();

        float fCameraShakeX = sin(m_fCameraShakeX)* m_fRadiusX;
        float fCameraShakeY = cos(m_fCameraShakeY) * m_fRadiusY;

        m_tCameraInfo.vEye = CameraPos + (CameraLook * (70.f - fSpine)) + CameraRight * m_fZoom - CameraUp * 50.f;
        m_tCameraInfo.vAt = CameraPos + (CameraLook * (70.f + fSpine)) - CameraUp * 50.f;

        m_tCameraInfo.vAt.x += fCameraShakeX;
        m_tCameraInfo.vAt.y += fCameraShakeY;


        if (m_tCameraInfo.vEye.x == 0 && m_tCameraInfo.vEye.z == 0)
            m_tCameraInfo.vEye.x = 3;


        m_tCameraInfo.vUp = _vec3(0.f, 1.0f, 0.f);
    }
}

void CDynamicCamera::Set_ZoomInOut(_bool ZoomIn, float fov)
{


    if (m_fZoom >= 150.f && ZoomIn == true && m_bIsZoom == false)
        m_bIsZoom = true;

    if (m_fZoom <= 50.f && ZoomIn == false && m_bIsZoom == true)
        m_bIsZoom = false;

    m_fFov = fov;
}



CGameObject* CDynamicCamera::Clone_GameObject(void* pArg)
{
    CGameObject* pInstance = new CDynamicCamera(*this);

    if (FAILED(pInstance->Ready_GameObject()))
        return nullptr;

    return pInstance;
}

CDynamicCamera* CDynamicCamera::Create(ID3D12Device* pGraphicDevice,
    ID3D12GraphicsCommandList* pCommandList,
    const CAMERA_INFO& tCameraInfo,
    const PROJ_INFO& tProjInfo,
    const ORTHO_INFO& tOrthoInfo)
{
    CDynamicCamera* pInstnace = new CDynamicCamera(pGraphicDevice, pCommandList);

    if (FAILED(pInstnace->Ready_GameObjectPrototype(tCameraInfo, tProjInfo, tOrthoInfo)))
        Engine::Safe_Release(pInstnace);

    return pInstnace;
}

void CDynamicCamera::Free()
{
    CCamera::Free();
}