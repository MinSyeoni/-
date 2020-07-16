#include "AnimationControl.h"

USING(Engine)


CAniCtrl::CAniCtrl(const aiScene* pScene)
    : m_pScene(pScene)
    , m_matCamera(INIT_MATRIX)
    , m_matRootFinal(INIT_MATRIX)
    , m_matWeapon(INIT_MATRIX)
{

}

CAniCtrl::CAniCtrl(const CAniCtrl& rhs)
    :m_matRootFinal(rhs.m_matRootFinal),
    m_pScene(rhs.m_pScene),
    m_matCamera(rhs.m_matCamera),
    m_matWeapon(rhs.m_matWeapon)
{
    m_vecBoneInfo.reserve(rhs.m_vecBoneInfo.size());
    m_vecBoneInfo = rhs.m_vecBoneInfo;

    m_mapNodeHierarchy.reserve(rhs.m_mapNodeHierarchy.size());
    m_mapNodeHierarchy = rhs.m_mapNodeHierarchy;

    m_vecBoneNameMap.reserve(rhs.m_vecBoneNameMap.size());
    m_vecBoneNameMap = rhs.m_vecBoneNameMap;



    //��������
    m_vecBoneTransform = new vector<VECTOR_MATRIX>;
    m_vecBoneTransform->reserve(rhs.m_vecBoneTransform->size());
    (*m_vecBoneTransform) = *(rhs.m_vecBoneTransform);

    m_bIsClone = true;
}



HRESULT CAniCtrl::Ready_AniCtrl()
{

    m_vecBoneTransform = new vector<VECTOR_MATRIX>;
    m_vecBoneTransform->resize(m_pScene->mNumMeshes);
    m_vecBoneNameMap.resize(m_pScene->mNumMeshes);
    m_vecBoneInfo.resize(m_pScene->mNumMeshes);



        for (_uint i = 0; i < m_pScene->mNumMeshes; ++i)
        {
            _int   iNumBones = 0;
            aiMesh* pSubsetMesh = m_pScene->mMeshes[i];
            cout << m_pScene->mMeshes[i]->mNumBones << endl;
            /*__________________________________________________________________________________________________________
            - �� ����� ��Ƶ� �����̳� �ʱ�ȭ.
            ____________________________________________________________________________________________________________*/
            (*m_vecBoneTransform)[i].resize(pSubsetMesh->mNumBones);

            for (_uint j = 0; j < pSubsetMesh->mNumBones; ++j, ++iNumBones)
            {
                _uint   uiboneindex = 0;
                string   strBoneName(pSubsetMesh->mBones[j]->mName.data);
                uiboneindex = iNumBones;

                BONE_INFO tBoneInfo;
                m_vecBoneInfo[i].push_back(tBoneInfo);

                m_vecBoneNameMap[i][strBoneName] = uiboneindex;
                m_vecBoneInfo[i][uiboneindex].matboneOffset = Convert_AiToMat4(pSubsetMesh->mBones[j]->mOffsetMatrix);

            }

        }


        /*__________________________________________________________________________________________________________
        [ Hierarchy ���� ]
        - BoneNameMap�ȿ� �� Node�� Name�� ��� ����ִ��� �Ǵ�.
        - Animation Index���� �� Node�� aiNodeAnim�� ����ִ��� Ž��.
        ____________________________________________________________________________________________________________*/
        Ready_NodeHierarchy(m_pScene->mRootNode);

 
        return S_OK;
}
void CAniCtrl::Ready_NodeHierarchy(const aiNode* pNode)
{
    string strNodeName(pNode->mName.data);

    HIERARCHY_INFO* pNodeHierarchy = new HIERARCHY_INFO(pNode);

    // 1. BoneNameMap�ȿ� �� Node�� Name�� ��� ����ִ��� �Ǵ�.
    for (_uint i = 0; i < m_vecBoneNameMap.size(); ++i)
    {
        if (m_vecBoneNameMap[i].find(strNodeName) != m_vecBoneNameMap[i].end())
        {
            // �����̴� BoneNameMap�� Index ����.
            pNodeHierarchy->vecBoneMapIdx.push_back(i);
        }
    }

    // 2. Animation Index���� �� Node�� aiNodeAnim�� ����ִ��� Ž��.
    for (_uint i = 0; i < m_pScene->mNumAnimations; ++i)
    {
        const aiAnimation* pAnimation = m_pScene->mAnimations[i];
        const aiNodeAnim* pNodeAnimation = Find_NodeAnimation(pAnimation, strNodeName);

        pNodeHierarchy->mapNodeAnim.emplace(i, pNodeAnimation);
    }


    m_mapNodeHierarchy.emplace(strNodeName, pNodeHierarchy);

    /*__________________________________________________________________________________________________________
    [ ��� �ڽ� ��忡 ���� ���ȣ�� ]
    ____________________________________________________________________________________________________________*/
    for (_uint i = 0; i < pNode->mNumChildren; ++i)
        Ready_NodeHierarchy(pNode->mChildren[i]);

}

void CAniCtrl::Set_AnimationKey(_int AniKey)
{
    if (m_uiNewAniIndex != AniKey)
    {
        m_uiNewAniIndex = AniKey;
        m_fBlendAnimationTime = m_fAnimationTime;
        m_fBlendingTime = 1.0f;

    }

}

void CAniCtrl::Set_BlendAnimationKey(_int FirstAniKey, _int SecondAniKey)
{



    if (m_uiNewAniIndex != FirstAniKey && m_uiNewAniIndex == m_uiCurAniIndex)
    {
        m_uiNewAniIndex = FirstAniKey;
        m_fBlendAnimationTime = m_fAnimationTime;
        m_fBlendingTime = 1.0f;
        m_fAnimationTime = 0.f;

    }

    if (m_uiNewSubAniIndex != SecondAniKey && m_uiNewSubAniIndex == m_uiCurSubAniIndex)
    {

        m_uiNewSubAniIndex = SecondAniKey;
        m_fBlendAnimationTimeSub = m_fAnimationTimeSub;
        m_fBlendigTimeSub = 1.0f;
        m_fAnimationTimeSub = 0.f;
    }

}

_bool CAniCtrl::Set_IsFinish(float fTime)
{
    _float fDestTime = (_float)(fmod(m_fAnimationTime, (m_pScene->mAnimations[m_uiCurAniIndex]->mDuration)));

    if (fDestTime > (m_pScene->mAnimations[m_uiCurAniIndex]->mDuration) - fTime)
        return true;


    return false;
}

_bool CAniCtrl::Set_FindAnimation(_float fTime, _int AniKey)
{
    if (m_uiCurAniIndex != m_uiNewAniIndex)
        return false;
    if (m_uiCurAniIndex != AniKey)
        return false;

    _float fDestTime = (_float)(fmod(m_fAnimationTime, (m_pScene->mAnimations[m_uiCurAniIndex]->mDuration)));
    _float fSrcTiem = (_float)(fmod(fTime, (m_pScene->mAnimations[m_uiCurAniIndex]->mDuration)));
    if (fDestTime > fTime)
        return true;

    return false;
}

void CAniCtrl::Set_Start()
{
    m_fAnimationTime = 0.f;
    m_fAnimationTimeSub = 0.f;
}


vector<VECTOR_MATRIX> CAniCtrl::Extract_BoneBlendingTransform(_float fAniTimeFirst, _float fAniTimeSecond, _float fAngle)
{

    if (m_uiCurAniIndex >= m_pScene->mNumAnimations)
        return vector<VECTOR_MATRIX>();
    m_fAngle = fAngle;
    /*__________________________________________________________________________________________________________
    - �ִϸ��̼��� ��� �ݺ��ǵ��� fmod ������ ����.
    ____________________________________________________________________________________________________________*/
    m_fAnimationTime += fAniTimeFirst;
    m_fAnimationTimeSub += fAniTimeSecond;

    m_fAnimationTime = (_float)(fmod(m_fAnimationTime, (m_pScene->mAnimations[m_uiCurAniIndex]->mDuration)));
    m_fAnimationTimeSub = (_float)(fmod(m_fAnimationTimeSub, (m_pScene->mAnimations[m_uiCurSubAniIndex]->mDuration)));

    if (m_uiNewAniIndex != m_uiCurAniIndex)//��ü
    {
        m_fAnimationTime = 0.f;
        m_fBlendAnimationTime = (_float)(fmod(m_fBlendAnimationTime, (m_pScene->mAnimations[m_uiCurAniIndex]->mDuration)));
        m_fBlendingTime -= 0.002f * fAniTimeFirst;
    }
    if (m_fBlendingTime <= 0)//��ü
    {
        m_fBlendingTime = 0.f;
    }
    if (m_uiNewSubAniIndex != m_uiCurSubAniIndex)//��ü
    {
        m_fAnimationTimeSub = 0.f;
        m_fBlendAnimationTimeSub = (_float)(fmod(m_fBlendAnimationTimeSub, (m_pScene->mAnimations[m_uiCurSubAniIndex]->mDuration)));
        m_fBlendigTimeSub -= 0.002f * fAniTimeSecond;

    }
    if (m_fBlendigTimeSub <= 0)//��ü
    {
        m_fBlendigTimeSub = 0.f;
    }
    /*__________________________________________________________________________________________________________
    - Root Node�� ���� ����� ���ڷ� �Ѱ��ָ� ��� ȣ���� ���Ͽ� BONE_INFO�� �����͸� �����ϴ� �Լ�.
    - Read_NodeHierarchy() �Լ� ȣ���� ������ ����, ��� ������ m_vecBoneTransform�迭�� �����͸� ä��� ����.
    ____________________________________________________________________________________________________________*/
    _matrix matIdentity = INIT_MATRIX;
    if (m_uiCurAniIndex == m_uiNewAniIndex)
        Update_NodeHirearchyBlend(m_fAnimationTime, m_fAnimationTimeSub, m_pScene->mRootNode, matIdentity);
    else
        Update_NodeHirearchyBlend(m_fBlendAnimationTime, m_fBlendAnimationTimeSub, m_pScene->mRootNode, matIdentity);
    if (m_fBlendingTime <= 0)
    {
        m_uiCurAniIndex = m_uiNewAniIndex;
        m_fBlendingTime = 1.f;
        m_fAnimationTime = 0.f;

    }
    if (m_fBlendigTimeSub <= 0)
    {
        m_uiCurSubAniIndex = m_uiNewSubAniIndex;
        m_fBlendigTimeSub = 1.f;
        m_fAnimationTimeSub = 0.f;
    }


    return *m_vecBoneTransform;
}

vector<VECTOR_MATRIX> CAniCtrl::Extract_BoneTransform(_float fAnimationTime)
{


    /*__________________________________________________________________________________________________________
    [ Extract_BoneTransform ]
    - Ư�� �ð�(fAnimationTime)�� Ư�� �ִϸ��̼� �ε���(uiAnimationIdx)�� �Ѱ��ָ�
    �� Bone�� ���¸� Matrix �������� �����ϴ� �Լ�.
    - �� �����Ӹ��� ȣ���.
    ____________________________________________________________________________________________________________*/
    if (m_uiCurAniIndex >= m_pScene->mNumAnimations)
        return vector<VECTOR_MATRIX>();

    /*__________________________________________________________________________________________________________
    - �ִϸ��̼��� ��� �ݺ��ǵ��� fmod ������ ����.
    ____________________________________________________________________________________________________________*/
    m_fAnimationTime += fAnimationTime;

    m_fAnimationTime = (_float)(fmod(m_fAnimationTime, (m_pScene->mAnimations[m_uiCurAniIndex]->mDuration)));

    if (m_uiNewAniIndex != m_uiCurAniIndex)
    {
        m_fAnimationTime = m_fBlendAnimationTime;
        m_fAnimationTime = (_float)(fmod(m_fAnimationTime, (m_pScene->mAnimations[m_uiCurAniIndex]->mDuration)));
        m_fBlendingTime -= 0.002f * fAnimationTime;
    }
    if (m_fBlendingTime <= 0)
    {
        m_fBlendingTime = 0.f;
    }
    /*__________________________________________________________________________________________________________
    - Root Node�� ���� ����� ���ڷ� �Ѱ��ָ� ��� ȣ���� ���Ͽ� BONE_INFO�� �����͸� �����ϴ� �Լ�.
    - Read_NodeHierarchy() �Լ� ȣ���� ������ ����, ��� ������ m_vecBoneTransform�迭�� �����͸� ä��� ����.
    ____________________________________________________________________________________________________________*/
    _matrix matIdentity = INIT_MATRIX;
    Update_NodeHierarchy(m_fAnimationTime, m_pScene->mRootNode, matIdentity);

    if (m_fBlendingTime <= 0)
    {
        m_fBlendingTime = 0.f;
        m_uiCurAniIndex = m_uiNewAniIndex;
        m_fBlendingTime = 1.f;
        m_fAnimationTime = 0.f;

    }

    return *m_vecBoneTransform;
}

void CAniCtrl::Update_NodeHierarchy(_float fAnimationTime,
    const aiNode* pNode,
    const _matrix& matParentTransform)
{

    /*__________________________________________________________________________________________________________
    [ Update_NodeHierarchy ]
    - Bone�� Transform�� ���ϴ� �Լ�.
    - ���������� ���ȣ���Ͽ� BONE_INFO�迭�� matFinalTransform������ �����͸� ä���.
    - ���ȣ���� ���� �ڽ� ���� �������鼭, ���� ��Ī�� boneTransformation�� ����.
    ____________________________________________________________________________________________________________*/
    string strNodeName(pNode->mName.data);

    auto            iter_find = m_mapNodeHierarchy.find(strNodeName);
    const aiNodeAnim* pNodeAnimation = iter_find->second->mapNodeAnim[m_uiCurAniIndex];
    const aiNodeAnim* pNewNodeAnimation = iter_find->second->mapNodeAnim[m_uiNewAniIndex];

    _matrix            matNodeTransform = Convert_AiToMat4(pNode->mTransformation);

    /*__________________________________________________________________________________________________________
    - �ִϸ��̼� ������ �ִ� ����� ���.
    ____________________________________________________________________________________________________________*/


    if (pNodeAnimation)
    {
        /*__________________________________________________________________________________________________________
        - �־��� KeyFrame�� ������ AnimationTime������ �̿��� Interpolation(����)�� �ϰ� ���� ����.
        ____________________________________________________________________________________________________________*/
        // Scale
        const aiVector3D& vScale = Calc_InterPolatedValue_From_Key(fAnimationTime, pNodeAnimation->mNumScalingKeys, pNodeAnimation->mScalingKeys, pNewNodeAnimation->mNumScalingKeys, pNewNodeAnimation->mScalingKeys);

        // Rotation
        const aiQuaternion& vRotate = Calc_InterPolatedValue_From_Key(fAnimationTime, pNodeAnimation->mNumRotationKeys, pNodeAnimation->mRotationKeys, pNewNodeAnimation->mNumRotationKeys, pNewNodeAnimation->mRotationKeys);


        // Trans
        const aiVector3D& vTrans = Calc_InterPolatedValue_From_Key(fAnimationTime, pNodeAnimation->mNumPositionKeys, pNodeAnimation->mPositionKeys, pNewNodeAnimation->mNumPositionKeys, pNewNodeAnimation->mPositionKeys);

        // Scale * Rotation * Trans
        _matrix   matScale = XMMatrixScaling(vScale.x, vScale.y, vScale.z);
        _matrix   matRotate = Convert_AiToMat3(vRotate.GetMatrix());
        _matrix   matTrans = XMMatrixTranslation(vTrans.x, vTrans.y, vTrans.z);

        /*__________________________________________________________________________________________________________
        - ������ vector�� quaternion�� matrix�� ��ȯ�ǰ�, �̵�/ȸ��/ũ�� ��ȯ�� ���� NodeTransform(Bone Transform)�� �ϼ�.
        - �� NodeTransform�� Bone Space���� ���ǵǾ��� �������� Parent Bone Space���� ���ǵǵ��� �ϴ� ��ȯ�Ӱ� ���ÿ�,
        Parent Bone Space���� ���ǵ� �����鿡�� ���ϴ� ��ȯ�̴�.

        - ���� ��� ����� ��ǥ�迡�� Ư�� �����鿡 ��ȯ�� ���ؼ� ���Ͽ� ��ġ�ϵ��� ����� ��.
        ____________________________________________________________________________________________________________*/
        matNodeTransform = matScale * matRotate * matTrans;
    }

    /*__________________________________________________________________________________________________________
    [ ���� Transformation�� �̿��ؼ� Bone Transformation�� �����ϰ�, �ڽ� Node�� �����ϴ� �κ� ]

    - matGlobalTransform   : Bone Space���� ���ǵǾ��� �������� Model Space���� ���ǵ����� ��.
    - matParentTransform   : �θ� Bone Space���� ���ǵǾ��� �������� Model Space���� ���ǵǵ��� ��.
    - matNodeTransform      : Bone Space���� ���ǵǾ��� �������� �θ� Bone Space���� ���ǵǵ��� ��.
                  Ȥ�� �θ� bone Space�� �������� �� ������ ��ȯ.
    ____________________________________________________________________________________________________________*/
    _matrix matGlobalTransform = matNodeTransform * matParentTransform;
    /*__________________________________________________________________________________________________________
    - Bone�� �ִ� ��忡 ���ؼ��� Bone Transform�� ����.
    - m_vecBoneNameMap�� map<string, _uint>Ÿ������, bone�� �̸��� index�� ����.
    - mapBone�� ���캸�� ������, ��� Bone�� Node������, ��� Node�� Bone�� �ƴϱ� ����.
    ____________________________________________________________________________________________________________*/
    for (_uint& iIdx : iter_find->second->vecBoneMapIdx)
    {
        _uint uiBoneIdx = m_vecBoneNameMap[iIdx][strNodeName];
        {
            /*__________________________________________________________________________________________________________
            - matBoneOffset�� Model Space���� ���ǵǾ��� �������� Bone Space���� ���ǵǵ��� ����� ��.
            - matGlobalTransform = matNodeTransform * matParentTransform -> ParentTransform�� ���� �ٽ� Model Space���� ����.
            - GlobalTransform�� ���� �����ϴ� ������ �ڽ� ��忡 ParentTransform���� �����ֱ� ����.
            ____________________________________________________________________________________________________________*/
            m_vecBoneInfo[iIdx][uiBoneIdx].matfinalTransform = m_vecBoneInfo[iIdx][uiBoneIdx].matboneOffset
                * matGlobalTransform
                * Convert_AiToMat4(m_pScene->mRootNode->mTransformation);

            (*m_vecBoneTransform)[iIdx][uiBoneIdx] = m_vecBoneInfo[iIdx][uiBoneIdx].matfinalTransform;
        }

    }

    /*__________________________________________________________________________________________________________
    [ ��� �ڽ� ��忡 ���� ���ȣ�� ]
    ____________________________________________________________________________________________________________*/
    for (_uint i = 0; i < pNode->mNumChildren; ++i)
        Update_NodeHierarchy(fAnimationTime, pNode->mChildren[i], matGlobalTransform);

}

void CAniCtrl::Update_NodeHirearchyBlend(_float fAnimationTime, _float fAnimationTimeSub, const aiNode* pNode, const _matrix& matParentTransform)
{

    /*__________________________________________________________________________________________________________
    [ Update_NodeHierarchy ]
    - Bone�� Transform�� ���ϴ� �Լ�.
    - ���������� ���ȣ���Ͽ� BONE_INFO�迭�� matFinalTransform������ �����͸� ä���.
    - ���ȣ���� ���� �ڽ� ���� �������鼭, ���� ��Ī�� boneTransformation�� ����.
    ____________________________________________________________________________________________________________*/
    string strNodeName(pNode->mName.data);
    auto            iter_find = m_mapNodeHierarchy.find(strNodeName);

    if (strNodeName == "LowerBack")
        m_bIsFisrtAni = true;
    if (strNodeName == "Pelvis")
        m_bIsFisrtAni = false;

    const  aiNodeAnim* pNodeAnimation;
    const  aiNodeAnim* pNewNodeAnimation;
    _float AniTime = 0.f;
    if (m_bIsFisrtAni == true)
    {
        pNodeAnimation = iter_find->second->mapNodeAnim[m_uiCurAniIndex];
        pNewNodeAnimation = iter_find->second->mapNodeAnim[m_uiNewAniIndex];
        AniTime = fAnimationTime;
    }
    else
    {
        pNodeAnimation = iter_find->second->mapNodeAnim[m_uiCurSubAniIndex];
        pNewNodeAnimation = iter_find->second->mapNodeAnim[m_uiNewSubAniIndex];
        AniTime = fAnimationTimeSub;
    }


    _matrix            matNodeTransform = Convert_AiToMat4(pNode->mTransformation);

    /*__________________________________________________________________________________________________________
    - �ִϸ��̼� ������ �ִ� ����� ���.
    ____________________________________________________________________________________________________________*/

    if (strNodeName == "Camera")
    {
        const aiVector3D& vScale = Calc_InterPolatedValue_From_Key(AniTime, pNodeAnimation->mNumScalingKeys, pNodeAnimation->mScalingKeys, pNewNodeAnimation->mNumScalingKeys, pNewNodeAnimation->mScalingKeys);

        // Rotation
        const aiQuaternion& vRotate = Calc_InterPolatedValue_From_Key(AniTime, pNodeAnimation->mNumRotationKeys, pNodeAnimation->mRotationKeys, pNewNodeAnimation->mNumRotationKeys, pNewNodeAnimation->mRotationKeys);

        // Trans
        const aiVector3D& vTrans = Calc_InterPolatedValue_From_Key(AniTime, pNodeAnimation->mNumPositionKeys, pNodeAnimation->mPositionKeys, pNewNodeAnimation->mNumPositionKeys, pNewNodeAnimation->mPositionKeys);

        // Scale * Rotation * Trans
        _matrix   matScale = XMMatrixScaling(vScale.x, vScale.y, vScale.z);
        _matrix   matRotate = Convert_AiToMat3(vRotate.GetMatrix());
        _matrix   matTrans = XMMatrixTranslation(vTrans.x, vTrans.y, vTrans.z);

        _matrix matGlobalTransform = matNodeTransform * matParentTransform;

        m_matCamera = matGlobalTransform * Convert_AiToMat4(m_pScene->mRootNode->mTransformation);
    }


    if (pNodeAnimation)
    {
        /*__________________________________________________________________________________________________________
        - �־��� KeyFrame�� ������ AnimationTime������ �̿��� Interpolation(����)�� �ϰ� ���� ����.
        ____________________________________________________________________________________________________________*/
        // Scale
        const aiVector3D& vScale = Calc_InterPolatedValue_From_Key(AniTime, pNodeAnimation->mNumScalingKeys, pNodeAnimation->mScalingKeys, pNewNodeAnimation->mNumScalingKeys, pNewNodeAnimation->mScalingKeys);

        // Rotation
        const aiQuaternion& vRotate = Calc_InterPolatedValue_From_Key(AniTime, pNodeAnimation->mNumRotationKeys, pNodeAnimation->mRotationKeys, pNewNodeAnimation->mNumRotationKeys, pNewNodeAnimation->mRotationKeys);


        // Trans
        const aiVector3D& vTrans = Calc_InterPolatedValue_From_Key(AniTime, pNodeAnimation->mNumPositionKeys, pNodeAnimation->mPositionKeys, pNewNodeAnimation->mNumPositionKeys, pNewNodeAnimation->mPositionKeys);

        // Scale * Rotation * Trans
        _matrix   matScale = XMMatrixScaling(vScale.x, vScale.y, vScale.z);
        _matrix   matRotate = Convert_AiToMat3(vRotate.GetMatrix());
        _matrix   matTrans = XMMatrixTranslation(vTrans.x, vTrans.y, vTrans.z);

        if (strNodeName == "LowerBack")
        {
            _matrix            matRotationX = XMMatrixRotationX(XMConvertToRadians(m_fAngle));

            matRotate = matRotationX * matRotate;
        }

        /*__________________________________________________________________________________________________________
        - ������ vector�� quaternion�� matrix�� ��ȯ�ǰ�, �̵�/ȸ��/ũ�� ��ȯ�� ���� NodeTransform(Bone Transform)�� �ϼ�.
        - �� NodeTransform�� Bone Space���� ���ǵǾ��� �������� Parent Bone Space���� ���ǵǵ��� �ϴ� ��ȯ�Ӱ� ���ÿ�,
        Parent Bone Space���� ���ǵ� �����鿡�� ���ϴ� ��ȯ�̴�.

        - ���� ��� ����� ��ǥ�迡�� Ư�� �����鿡 ��ȯ�� ���ؼ� ���Ͽ� ��ġ�ϵ��� ����� ��.
        ____________________________________________________________________________________________________________*/
        matNodeTransform = matScale * matRotate * matTrans;
    }

    /*__________________________________________________________________________________________________________
    [ ���� Transformation�� �̿��ؼ� Bone Transformation�� �����ϰ�, �ڽ� Node�� �����ϴ� �κ� ]

    - matGlobalTransform   : Bone Space���� ���ǵǾ��� �������� Model Space���� ���ǵ����� ��.
    - matParentTransform   : �θ� Bone Space���� ���ǵǾ��� �������� Model Space���� ���ǵǵ��� ��.
    - matNodeTransform      : Bone Space���� ���ǵǾ��� �������� �θ� Bone Space���� ���ǵǵ��� ��.
                  Ȥ�� �θ� bone Space�� �������� �� ������ ��ȯ.
    ____________________________________________________________________________________________________________*/
    _matrix matGlobalTransform = matNodeTransform * matParentTransform;

    if (strNodeName == "root")
    {
        m_matRootFinal = matGlobalTransform * Convert_AiToMat4(m_pScene->mRootNode->mTransformation);
    }

    if (strNodeName == "Prop01")
    {
        m_matWeapon = matGlobalTransform * Convert_AiToMat4(m_pScene->mRootNode->mTransformation);
    }
    /*__________________________________________________________________________________________________________
    - Bone�� �ִ� ��忡 ���ؼ��� Bone Transform�� ����.
    - m_vecBoneNameMap�� map<string, _uint>Ÿ������, bone�� �̸��� index�� ����.
    - mapBone�� ���캸�� ������, ��� Bone�� Node������, ��� Node�� Bone�� �ƴϱ� ����.
    ____________________________________________________________________________________________________________*/
    for (_uint& iIdx : iter_find->second->vecBoneMapIdx)
    {
        _uint uiBoneIdx = m_vecBoneNameMap[iIdx][strNodeName];
        {
            /*__________________________________________________________________________________________________________
            - matBoneOffset�� Model Space���� ���ǵǾ��� �������� Bone Space���� ���ǵǵ��� ����� ��.
            - matGlobalTransform = matNodeTransform * matParentTransform -> ParentTransform�� ���� �ٽ� Model Space���� ����.
            - GlobalTransform�� ���� �����ϴ� ������ �ڽ� ��忡 ParentTransform���� �����ֱ� ����.
            ____________________________________________________________________________________________________________*/
            m_vecBoneInfo[iIdx][uiBoneIdx].matfinalTransform = m_vecBoneInfo[iIdx][uiBoneIdx].matboneOffset
                * matGlobalTransform
                * Convert_AiToMat4(m_pScene->mRootNode->mTransformation);

            (*m_vecBoneTransform)[iIdx][uiBoneIdx] = m_vecBoneInfo[iIdx][uiBoneIdx].matfinalTransform;
        }

    }

    /*__________________________________________________________________________________________________________
    [ ��� �ڽ� ��忡 ���� ���ȣ�� ]
    ____________________________________________________________________________________________________________*/
    for (_uint i = 0; i < pNode->mNumChildren; ++i)
        Update_NodeHirearchyBlend(fAnimationTime, fAnimationTimeSub, pNode->mChildren[i], matGlobalTransform);
}

aiNodeAnim* CAniCtrl::Find_NodeAnimation(const aiAnimation* pAnimation,
    const string strNodeName)
{
    for (_uint i = 0; i < pAnimation->mNumChannels; ++i)
    {
        if (strNodeName == pAnimation->mChannels[i]->mNodeName.data)
            return pAnimation->mChannels[i];
    }

    return nullptr;
}


/*__________________________________________________________________________________________________________
- Key Frame�̶�, �ִϸ��̼��� ǥ���ϴ� Frame�� �� �� ���� �߷��� ���� Frame�� ���Ѵ�.

- Animation�� ��� Frame���� ����⿡�� �޸� ���� ���ϰ�, �ð��� �����ɸ��� ������,
Key Frame�� ����� �ڿ������� �������� ���� �� ������ ������ �����ؼ� ����.

- Translation�� Scale�� vector ���·� �Ǿ��ְ�, ������ ���ϴ� Rotation�� quaternion ���·� �Ǿ��ִ�.
- vector�� quaternion�� ���� ����� �ٸ��� ������, �Լ��� �����ε��Ͽ� ����.
____________________________________________________________________________________________________________*/
aiVector3D CAniCtrl::Calc_InterPolatedValue_From_Key(const _float& fAnimationTime,
    const _uint& uiNumKeys,
    const aiVectorKey* pVectorKey,
    const _uint& uiNewNumKeys,
    const aiVectorKey* pNewVectorKey)
{
    aiVector3D ret;
    aiVector3D Preret;

    _uint uiKeyindex = Find_KeyIndex(fAnimationTime, uiNumKeys, pVectorKey);
    _uint uiNextKeyIndex = uiKeyindex + 1;


    /*__________________________________________________________________________________________________________
    [ Key Frame ���̸� �����Ͽ� Ư�� �ð��� Node�� Transformation�� ���ϴ� �κ� ]
    ____________________________________________________________________________________________________________*/
    // assert(uiNextKeyIndex < uiNumKeys);

    _float fTimeDelta = (_float)(pVectorKey[uiNextKeyIndex].mTime - pVectorKey[uiKeyindex].mTime);
    _float fFactor = (fAnimationTime - (_float)pVectorKey[uiKeyindex].mTime) / fTimeDelta;

    // assert(fFactor >= 0.0f && fFactor <= 1.0f);

    const aiVector3D& StartValue = pVectorKey[uiKeyindex].mValue;
    const aiVector3D& EndValue = pVectorKey[uiNextKeyIndex].mValue;

    ret.x = StartValue.x + (EndValue.x - StartValue.x) * fFactor;
    ret.y = StartValue.y + (EndValue.y - StartValue.y) * fFactor;
    ret.z = StartValue.z + (EndValue.z - StartValue.z) * fFactor;


    if (((m_uiCurAniIndex != m_uiNewAniIndex) && m_bIsFisrtAni) || ((m_uiCurSubAniIndex != m_uiNewSubAniIndex) && !m_bIsFisrtAni))
    {

        uiKeyindex = Find_KeyIndex(0, uiNewNumKeys, pNewVectorKey);
        uiNextKeyIndex = uiKeyindex + 1;


        _float fTimeDelta = (_float)(pNewVectorKey[uiNextKeyIndex].mTime - pNewVectorKey[uiKeyindex].mTime);
        _float fFactor = (0 - (_float)pNewVectorKey[uiKeyindex].mTime) / fTimeDelta;

        // assert(fFactor >= 0.0f && fFactor <= 1.0f);

        const aiVector3D& StartValue = pNewVectorKey[uiKeyindex].mValue;
        const aiVector3D& EndValue = pNewVectorKey[uiNextKeyIndex].mValue;

        Preret.x = StartValue.x + (EndValue.x - StartValue.x) * fFactor;
        Preret.y = StartValue.y + (EndValue.y - StartValue.y) * fFactor;
        Preret.z = StartValue.z + (EndValue.z - StartValue.z) * fFactor;

        if (m_bIsFisrtAni == true)
            ret = ret * m_fBlendingTime + Preret * (1.f - m_fBlendingTime);
        else
            ret = ret * m_fBlendigTimeSub + Preret * (1.f - m_fBlendigTimeSub);
    }


    return ret;
}

aiQuaternion CAniCtrl::Calc_InterPolatedValue_From_Key(const _float& fAnimationTime,
    const _uint& uiNumKeys,
    const aiQuatKey* pQuatKey, const _uint& uiNewNumKeys,
    const aiQuatKey* pNewQuatKey)
{

    aiQuaternion ret;
    aiQuaternion ret2;
    aiQuaternion ret3;

    _uint uiKeyIndex = Find_KeyIndex(fAnimationTime, uiNumKeys, pQuatKey);
    _uint uiNextKeyIndex = uiKeyIndex + 1;

    // assert(uiNextKeyIndex < uiNumKeys);

    _float fTimeDelta = (_float)(pQuatKey[uiNextKeyIndex].mTime - pQuatKey[uiKeyIndex].mTime);
    _float fFactor = (fAnimationTime - (_float)pQuatKey[uiKeyIndex].mTime) / fTimeDelta;

    // assert(fFactor >= 0.0f && fFactor <= 1.0f);

    const aiQuaternion& StartValue = pQuatKey[uiKeyIndex].mValue;
    const aiQuaternion& EndValue = pQuatKey[uiNextKeyIndex].mValue;

    aiQuaternion::Interpolate(ret, StartValue, EndValue, fFactor);

    ret = ret.Normalize();

    if (((m_uiCurAniIndex != m_uiNewAniIndex) && m_bIsFisrtAni) || ((m_uiCurSubAniIndex != m_uiNewSubAniIndex) && !m_bIsFisrtAni))
    {
        _uint uiKeyIndex = Find_KeyIndex(0, uiNewNumKeys, pNewQuatKey);
        _uint uiNextKeyIndex = uiKeyIndex + 1;

        // assert(uiNextKeyIndex < uiNumKeys);

        _float fTimeDelta = (_float)(pNewQuatKey[uiNextKeyIndex].mTime - pNewQuatKey[uiKeyIndex].mTime);
        _float fFactor = (0 - (_float)pNewQuatKey[uiKeyIndex].mTime) / fTimeDelta;

        // assert(fFactor >= 0.0f && fFactor <= 1.0f);

        const aiQuaternion& StartValue = pNewQuatKey[uiKeyIndex].mValue;
        const aiQuaternion& EndValue = pNewQuatKey[uiNextKeyIndex].mValue;

        aiQuaternion::Interpolate(ret2, StartValue, EndValue, fFactor);

        ret2 = ret2.Normalize();

        if (m_bIsFisrtAni == true)
            aiQuaternion::Interpolate(ret, ret2, ret, m_fBlendingTime);
        else
            aiQuaternion::Interpolate(ret, ret2, ret, m_fBlendigTimeSub);


        ret = ret.Normalize();
    }




    return ret;
}

_matrix* CAniCtrl::Find_BoneMatrix(string strBoneName)
{
    /*__________________________________________________________________________________________________________
    [ Key�� Ž�� ]
    - vector<MAP_BONENAME> m_vecBoneNameMap�� ��ȸ�ϸ鼭 strBoneName Key���� �ִ��� Ž��.
    ____________________________________________________________________________________________________________*/
    for (_uint i = 0; i < m_vecBoneNameMap.size(); ++i)
    {
        auto iter_find = m_vecBoneNameMap[i].find(strBoneName);

        if (iter_find == m_vecBoneNameMap[i].end())
            continue;

        /*_____________________________________________________
        _____________________________________________________
        [ Ž�� ���� ]
        - Bone�� m_vecBoneInfo���� FinalTransform ��ȯ.
        ____________________________________________________________________________________________________________*/
        return &(m_vecBoneInfo[i][iter_find->second].matfinalTransform);


    }

    return nullptr;
}

_matrix* CAniCtrl::Find_BoneOffset(string strBoneName)
{
    for (_uint i = 0; i < m_vecBoneNameMap.size(); ++i)
    {
        auto iter_find = m_vecBoneNameMap[i].find(strBoneName);

        if (iter_find == m_vecBoneNameMap[i].end())
            continue;

        /*_____________________________________________________
        _____________________________________________________
        [ Ž�� ���� ]
        - Bone�� m_vecBoneInfo���� FinalTransform ��ȯ.
        ____________________________________________________________________________________________________________*/
        return &(m_vecBoneInfo[i][iter_find->second].matboneOffset);
    }
}

_uint CAniCtrl::Find_KeyIndex(const _float& fAnimationTime,
    const _uint& uiNumKey,
    const aiVectorKey* pVectorKey)
{
    // assert(uiNumKey > 0);

    for (_uint i = 0; i < uiNumKey - 1; ++i)
    {
        if (fAnimationTime < (_float)pVectorKey[i + 1].mTime)
            return i;
    }

    // assert(0);

    return S_OK;
}

_uint CAniCtrl::Find_KeyIndex(const _float& fAnimationTime,
    const _uint& uiNumKey,
    const aiQuatKey* pQuatKey)
{
    // assert(uiNumKey > 0);

    for (_uint i = 0; i < uiNumKey - 1; ++i)
    {
        if (fAnimationTime < (_float)pQuatKey[i + 1].mTime)
            return i;
    }

    // assert(0);

    return S_OK;
}

_matrix CAniCtrl::Convert_AiToMat4(const aiMatrix4x4& m)
{
    _matrix matResult;

    matResult.m[0][0] = m[0][0];   matResult.m[0][1] = m[1][0];   matResult.m[0][2] = m[2][0];   matResult.m[0][3] = m[3][0];
    matResult.m[1][0] = m[0][1];   matResult.m[1][1] = m[1][1];   matResult.m[1][2] = m[2][1];   matResult.m[1][3] = m[3][1];
    matResult.m[2][0] = m[0][2];   matResult.m[2][1] = m[1][2];   matResult.m[2][2] = m[2][2];   matResult.m[2][3] = m[3][2];
    matResult.m[3][0] = m[0][3];   matResult.m[3][1] = m[1][3];   matResult.m[3][2] = m[2][3];   matResult.m[3][3] = m[3][3];

    return matResult;
}

_matrix CAniCtrl::Convert_AiToMat3(const aiMatrix3x3& m)
{
    _matrix matResult;

    matResult.m[0][0] = m[0][0];   matResult.m[0][1] = m[1][0];   matResult.m[0][2] = m[2][0];   matResult.m[0][3] = 0;
    matResult.m[1][0] = m[0][1];   matResult.m[1][1] = m[1][1];   matResult.m[1][2] = m[2][1];   matResult.m[1][3] = 0;
    matResult.m[2][0] = m[0][2];   matResult.m[2][1] = m[1][2];   matResult.m[2][2] = m[2][2];   matResult.m[2][3] = 0;
    matResult.m[3][0] = 0;         matResult.m[3][1] = 0;         matResult.m[3][2] = 0;         matResult.m[3][3] = 1.0f;

    return matResult;
}

CAniCtrl* CAniCtrl::Create(const aiScene* pScece)
{
    CAniCtrl* pInstance = new CAniCtrl(pScece);

    if (FAILED(pInstance->Ready_AniCtrl()))
        Engine::Safe_Release(pInstance);

    return pInstance;
}

void CAniCtrl::Free()
{


    if (!m_bIsClone)
    {
        for (auto& MyPair : m_mapNodeHierarchy)
        {
            Safe_Delete(MyPair.second);
        }
        m_mapNodeHierarchy.clear();

        m_pScene = nullptr;
    }

    Safe_Delete(m_vecBoneTransform);

    m_mapNodeHierarchy.clear();
}