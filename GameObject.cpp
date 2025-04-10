#include "GameObject.h"

#include "Shader.h"
#include "Mesh.h"

#include <fstream>
#include <sstream>

using namespace std;


CGameObject::CGameObject():
    m_xmf4x4Transform { Matrix4x4::Identity() },
    m_xmf4x4World { Matrix4x4::Identity() },
    m_pstrFrameName { }
{

}

CGameObject::~CGameObject() {
    if(m_pMesh) m_pMesh->Release();

    if(m_nMaterials > 0) {
        for(int i = 0; i < m_nMaterials; i++) {
            if(m_ppMaterials[i]) m_ppMaterials[i]->Release();
        }
    }

    if(m_ppMaterials) delete[] m_ppMaterials;
}


void CGameObject::AddRef() {
    m_nReferences++;

    if(m_pSibling) m_pSibling->AddRef();
    if(m_pChild) m_pChild->AddRef();
}

void CGameObject::Release() {
    if(m_pChild) m_pChild->Release();
    if(m_pSibling) m_pSibling->Release();

    if(--m_nReferences <= 0) delete this;
}


void CGameObject::SetMesh(CMesh* pMesh) {
    if(m_pMesh) m_pMesh->Release();
    m_pMesh = pMesh;
    if(m_pMesh) m_pMesh->AddRef();
}

void CGameObject::SetShader(CShader* pShader) {
    m_nMaterials = 1;
    m_ppMaterials = new CMaterial*[m_nMaterials];
    m_ppMaterials[0] = new CMaterial();
    m_ppMaterials[0]->SetShader(pShader);
}

void CGameObject::SetShader(int nMaterial, CShader* pShader) {
    if(m_ppMaterials[nMaterial]) m_ppMaterials[nMaterial]->SetShader(pShader);
}

void CGameObject::SetMaterial(int nMaterial, CMaterial* pMaterial) {
    if(m_ppMaterials[nMaterial]) m_ppMaterials[nMaterial]->Release();
    m_ppMaterials[nMaterial] = pMaterial;
    if(m_ppMaterials[nMaterial]) m_ppMaterials[nMaterial]->AddRef();
}


void CGameObject::SetChild(CGameObject* pChild, bool bReferenceUpdate) {
    if(pChild) {
        pChild->m_pParent = this;
        if(bReferenceUpdate) pChild->AddRef();
    }
    if(m_pChild) {
        if(pChild) pChild->m_pSibling = m_pChild->m_pSibling;
        m_pChild->m_pSibling = pChild;
    }
    else {
        m_pChild = pChild;
    }
}


void CGameObject::Animate(float fTimeElapsed, XMFLOAT4X4* pxmf4x4Parent) {
    if(m_fMovingSpeed != 0.0f) Move(m_xmf3MovingDirection, m_fMovingSpeed * fTimeElapsed);

    updateBoundingBox();

    if(m_pSibling) m_pSibling->Animate(fTimeElapsed, pxmf4x4Parent);
    if(m_pChild) m_pChild->Animate(fTimeElapsed, &m_xmf4x4World);
}


void CGameObject::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera) {
    if(!is_active) return;

    OnPrepareRender();

    if(m_pMesh) {
        BoundingBox bb { m_pMesh->GetBoundingBox() };
        BoundingOrientedBox oobb { bb.Center, bb.Extents, XMFLOAT4 { 0.0f, 0.0f, 0.0f, 1.0f } };
        oobb.Transform(oobb, XMLoadFloat4x4(&m_xmf4x4World));
        if(pCamera) {
            //if(!pCamera->frustumIntersects(oobb)) return;
        }
    }

    UpdateShaderVariable(pd3dCommandList, &m_xmf4x4World);

    if(m_nMaterials > 0) {
        for(int i = 0; i < m_nMaterials; i++) {
            if(m_ppMaterials[i]) {
                if(m_ppMaterials[i]->m_pShader) m_ppMaterials[i]->m_pShader->Render(pd3dCommandList, pCamera);
                m_ppMaterials[i]->UpdateShaderVariable(pd3dCommandList);
            }

            if(m_pMesh) m_pMesh->Render(pd3dCommandList, i);
        }
    }
    if(m_pSibling) m_pSibling->Render(pd3dCommandList, pCamera);
    if(m_pChild) m_pChild->Render(pd3dCommandList, pCamera);
}


void CGameObject::CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList) {
}

void CGameObject::UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList) {
}

void CGameObject::ReleaseShaderVariables() {
}

void CGameObject::UpdateShaderVariable(ID3D12GraphicsCommandList* pd3dCommandList, XMFLOAT4X4* pxmf4x4World) {
    XMFLOAT4X4 xmf4x4World;
    XMStoreFloat4x4(&xmf4x4World, XMMatrixTranspose(XMLoadFloat4x4(pxmf4x4World)));
    pd3dCommandList->SetGraphicsRoot32BitConstants(1, 16, &xmf4x4World, 0);
}

void CGameObject::UpdateShaderVariable(ID3D12GraphicsCommandList* pd3dCommandList, CMaterial* pMaterial) {

}


void CGameObject::ReleaseUploadBuffers() {
    if(m_pMesh) m_pMesh->ReleaseUploadBuffers();

    if(m_pSibling) m_pSibling->ReleaseUploadBuffers();
    if(m_pChild) m_pChild->ReleaseUploadBuffers();
}


void CGameObject::UpdateTransform(XMFLOAT4X4* pxmf4x4Parent) {
    m_xmf4x4World = (pxmf4x4Parent) ? Matrix4x4::Multiply(m_xmf4x4Transform, *pxmf4x4Parent) : m_xmf4x4Transform;

    if(m_pSibling) m_pSibling->UpdateTransform(pxmf4x4Parent);
    if(m_pChild) m_pChild->UpdateTransform(&m_xmf4x4World);
}


CGameObject* CGameObject::FindFrame(const char* pstrFrameName) {
    CGameObject* pFrameObject = NULL;
    if(!strncmp(m_pstrFrameName, pstrFrameName, strlen(pstrFrameName))) {
        return this;
    }

    if(m_pSibling) {
        if(pFrameObject = m_pSibling->FindFrame(pstrFrameName)) {
            return pFrameObject;
        }
    }
    if(m_pChild) {
        if(pFrameObject = m_pChild->FindFrame(pstrFrameName)) {
            return pFrameObject;
        }
    }

    return NULL;
}




XMFLOAT3 CGameObject::GetPosition() const {
    return XMFLOAT3 { m_xmf4x4World._41, m_xmf4x4World._42, m_xmf4x4World._43 };
}

XMFLOAT3 CGameObject::GetLook() const {
    XMFLOAT3 xmf3LookAt(m_xmf4x4World._31, m_xmf4x4World._32, m_xmf4x4World._33);
    xmf3LookAt = Vector3::Normalize(xmf3LookAt);
    return(xmf3LookAt);
}

XMFLOAT3 CGameObject::GetUp() const {
    XMFLOAT3 xmf3Up(m_xmf4x4World._21, m_xmf4x4World._22, m_xmf4x4World._23);
    xmf3Up = Vector3::Normalize(xmf3Up);
    return(xmf3Up);
}

XMFLOAT3 CGameObject::GetRight() const {
    XMFLOAT3 xmf3Right(m_xmf4x4World._11, m_xmf4x4World._12, m_xmf4x4World._13);
    xmf3Right = Vector3::Normalize(xmf3Right);
    return(xmf3Right);
}


void CGameObject::SetPosition(float x, float y, float z) {
    m_xmf4x4Transform._41 = x;
    m_xmf4x4Transform._42 = y;
    m_xmf4x4Transform._43 = z;

    UpdateTransform(NULL);
}

void CGameObject::SetScale(float x, float y, float z) {
    XMMATRIX mtxScale = XMMatrixScaling(x, y, z);
    m_xmf4x4Transform = Matrix4x4::Multiply(mtxScale, m_xmf4x4Transform);

    UpdateTransform(NULL);
}



void CGameObject::MoveStrafe(float fDistance) {
    XMFLOAT3 xmf3Position = GetPosition();
    XMFLOAT3 xmf3Right = GetRight();
    xmf3Position = Vector3::Add(xmf3Position, xmf3Right, fDistance);

    CGameObject::SetPosition(xmf3Position);
}

void CGameObject::MoveUp(float fDistance) {
    XMFLOAT3 xmf3Position = GetPosition();
    XMFLOAT3 xmf3Up = GetUp();
    xmf3Position = Vector3::Add(xmf3Position, xmf3Up, fDistance);

    CGameObject::SetPosition(xmf3Position);
}

void CGameObject::MoveForward(float fDistance) {
    XMFLOAT3 xmf3Position = GetPosition();
    XMFLOAT3 xmf3LookAt = GetLook();
    xmf3Position = Vector3::Add(xmf3Position, xmf3LookAt, fDistance);

    CGameObject::SetPosition(xmf3Position);
}

void CGameObject::Move(const XMFLOAT3& vDirection, float fSpeed) {
    SetPosition(
        m_xmf4x4World._41 + vDirection.x * fSpeed, 
        m_xmf4x4World._42 + vDirection.y * fSpeed,
        m_xmf4x4World._43 + vDirection.z * fSpeed
    );
}



void CGameObject::Rotate(float pitch, float yaw, float roll) {
    auto rotation = XMMatrixRotationRollPitchYaw(XMConvertToRadians(pitch), XMConvertToRadians(yaw), XMConvertToRadians(roll));
    m_xmf4x4Transform = Matrix4x4::Multiply(rotation, m_xmf4x4Transform);

    setMovingDirection(GetLook());
    UpdateTransform(NULL);
}

void CGameObject::Rotate(XMFLOAT3* pxmf3Axis, float fAngle) {
    m_xmf4x4Transform = Matrix4x4::Multiply(
        XMMatrixRotationAxis(XMLoadFloat3(pxmf3Axis), XMConvertToRadians(fAngle)), 
        m_xmf4x4Transform
    );

    UpdateTransform(NULL);
}

void CGameObject::Rotate(XMFLOAT4* pxmf4Quaternion) {
    XMMATRIX mtxRotate = XMMatrixRotationQuaternion(XMLoadFloat4(pxmf4Quaternion));
    m_xmf4x4Transform = Matrix4x4::Multiply(mtxRotate, m_xmf4x4Transform);

    UpdateTransform(NULL);
}

void CGameObject::rotateLocal(float pitch, float yaw, float roll) {
    auto rotation_quaternion = XMQuaternionRotationRollPitchYaw(XMConvertToRadians(pitch), XMConvertToRadians(yaw), XMConvertToRadians(roll));
    auto rotation_matrix = XMMatrixRotationQuaternion(rotation_quaternion);
    m_xmf4x4Transform = Matrix4x4::Multiply(rotation_matrix, m_xmf4x4Transform);
    
    setMovingDirection(GetLook());
    UpdateTransform(NULL);
}

void CGameObject::rotateLocal(XMFLOAT3* pxmf3Axis, float fAngle) {
    auto rotation_quaternion = XMQuaternionRotationAxis(XMLoadFloat3(pxmf3Axis), XMConvertToRadians(fAngle));
    auto rotation_matrix = XMMatrixRotationQuaternion(rotation_quaternion);
    m_xmf4x4Transform = Matrix4x4::Multiply(rotation_matrix, m_xmf4x4Transform);

    setMovingDirection(GetLook());
    UpdateTransform(NULL);
}




void CGameObject::updateBoundingBox() {
    m_xmOOBB_original.Transform(m_xmOOBB, XMLoadFloat4x4(&m_xmf4x4World));
    XMStoreFloat4(&m_xmOOBB.Orientation, XMQuaternionNormalize(XMLoadFloat4(&m_xmOOBB.Orientation)));
}




bool CGameObject::checkBoundingBoxIntersection(
    XMVECTOR& xmvPickRayOrigin, 
    XMVECTOR& xmvPickRayDirection, 
    float* pfNearHitDistance
) {
    return m_xmOOBB_original.Intersects(xmvPickRayOrigin, xmvPickRayDirection, *pfNearHitDistance);
}

bool CGameObject::checkBoundingBoxIntersection(const BoundingOrientedBox& other) {
    return m_xmOOBB.Intersects(other);
}

bool CGameObject::checkBoundingBoxIntersection(const CGameObject* other) {
    return m_xmOOBB.Intersects(other->m_xmOOBB);
}


void CGameObject::GenerateRayForPicking(const XMVECTOR& xmvPickPosition, const XMMATRIX& xmmtxView, XMVECTOR& xmvPickRayOrigin, XMVECTOR& xmvPickRayDirection) {
    XMMATRIX xmmtxToModel = XMMatrixInverse(NULL, XMLoadFloat4x4(&m_xmf4x4World) * xmmtxView);

    XMFLOAT3 xmf3CameraOrigin(0.0f, 0.0f, 0.0f);
    xmvPickRayOrigin = XMVector3TransformCoord(XMLoadFloat3(&xmf3CameraOrigin), xmmtxToModel);
    xmvPickRayDirection = XMVector3TransformCoord(xmvPickPosition, xmmtxToModel);
    xmvPickRayDirection = XMVector3Normalize(xmvPickRayDirection - xmvPickRayOrigin);
}

bool CGameObject::PickObjectByRayIntersection(const XMVECTOR& xmvPickPosition, const XMMATRIX& xmmtxView, float* pfHitDistance) {
    XMVECTOR xmvPickRayOrigin, xmvPickRayDirection;
    GenerateRayForPicking(xmvPickPosition, xmmtxView, xmvPickRayOrigin, xmvPickRayDirection);
    return checkBoundingBoxIntersection(xmvPickRayOrigin, xmvPickRayDirection, pfHitDistance);
}


void CGameObject::setBoundingBox(const XMFLOAT3& xmCenter, const XMFLOAT3& xmExtents, const XMFLOAT4& xmOrientation) {
    m_xmOOBB_original = BoundingOrientedBox { xmCenter, xmExtents, xmOrientation };
    updateBoundingBox();
}

void CGameObject::setBoundingBox(const XMFLOAT3& xmExtents) {
    m_xmOOBB_original = BoundingOrientedBox { XMFLOAT3 { 0.0f, 0.0f, 0.0f }, xmExtents, XMFLOAT4 { 0.0f, 0.0f, 0.0f, 1.0f } };
    updateBoundingBox();
}








static int ReadIntegerFromFile(FILE* pInFile) {
    int nValue = 0;
    UINT nReads = (UINT)::fread(&nValue, sizeof(int), 1, pInFile);
    return(nValue);
}

static float ReadFloatFromFile(FILE* pInFile) {
    float fValue = 0;
    UINT nReads = (UINT)::fread(&fValue, sizeof(float), 1, pInFile);
    return(fValue);
}

static BYTE ReadStringFromFile(FILE* pInFile, char* pstrToken, size_t bufferSize) {
    BYTE nStrLength = 0;
    UINT nReads = 0;
    nReads = (UINT)::fread(&nStrLength, sizeof(BYTE), 1, pInFile);
    if(nStrLength >= bufferSize) {
        nStrLength = static_cast<BYTE>(bufferSize - 1);
    }
    nReads = (UINT)::fread(pstrToken, sizeof(char), nStrLength, pInFile);
    pstrToken[nStrLength] = '\0';

    return nStrLength;
}




//#define _WITH_DEBUG_FRAME_HIERARCHY


MATERIALSLOADINFO* CGameObject::LoadMaterialsInfoFromFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, FILE* pInFile) {
    char pstrToken[64] = { '\0' };
    UINT nReads = 0;

    int nMaterial = 0;

    MATERIALSLOADINFO* pMaterialsInfo = new MATERIALSLOADINFO;

    pMaterialsInfo->m_nMaterials = ::ReadIntegerFromFile(pInFile);
    pMaterialsInfo->m_pMaterials = new MATERIALLOADINFO[pMaterialsInfo->m_nMaterials];

    for(; ; ) {
        ::ReadStringFromFile(pInFile, pstrToken, sizeof(pstrToken));

        if(!strcmp(pstrToken, "<Material>:")) {
            nMaterial = ::ReadIntegerFromFile(pInFile);
        }
        else if(!strcmp(pstrToken, "<AlbedoColor>:")) {
            nReads = (UINT)::fread(&(pMaterialsInfo->m_pMaterials[nMaterial].m_xmf4AlbedoColor), sizeof(float), 4, pInFile);
        }
        else if(!strcmp(pstrToken, "<EmissiveColor>:")) {
            nReads = (UINT)::fread(&(pMaterialsInfo->m_pMaterials[nMaterial].m_xmf4EmissiveColor), sizeof(float), 4, pInFile);
        }
        else if(!strcmp(pstrToken, "<SpecularColor>:")) {
            nReads = (UINT)::fread(&(pMaterialsInfo->m_pMaterials[nMaterial].m_xmf4SpecularColor), sizeof(float), 4, pInFile);
        }
        else if(!strcmp(pstrToken, "<Glossiness>:")) {
            nReads = (UINT)::fread(&(pMaterialsInfo->m_pMaterials[nMaterial].m_fGlossiness), sizeof(float), 1, pInFile);
        }
        else if(!strcmp(pstrToken, "<Smoothness>:")) {
            nReads = (UINT)::fread(&(pMaterialsInfo->m_pMaterials[nMaterial].m_fSmoothness), sizeof(float), 1, pInFile);
        }
        else if(!strcmp(pstrToken, "<Metallic>:")) {
            nReads = (UINT)::fread(&(pMaterialsInfo->m_pMaterials[nMaterial].m_fSpecularHighlight), sizeof(float), 1, pInFile);
        }
        else if(!strcmp(pstrToken, "<SpecularHighlight>:")) {
            nReads = (UINT)::fread(&(pMaterialsInfo->m_pMaterials[nMaterial].m_fMetallic), sizeof(float), 1, pInFile);
        }
        else if(!strcmp(pstrToken, "<GlossyReflection>:")) {
            nReads = (UINT)::fread(&(pMaterialsInfo->m_pMaterials[nMaterial].m_fGlossyReflection), sizeof(float), 1, pInFile);
        }
        else if(!strcmp(pstrToken, "</Materials>")) {
            break;
        }
    }
    return(pMaterialsInfo);
}


CMeshLoadInfo* CGameObject::LoadMeshInfoFromFile(FILE* pInFile, float size) {
    char pstrToken[64] = { '\0' };
    UINT nReads = 0;

    int nPositions = 0, nColors = 0, nNormals = 0, nIndices = 0, nSubMeshes = 0, nSubIndices = 0;

    CMeshLoadInfo* pMeshInfo = new CMeshLoadInfo;

    pMeshInfo->m_nVertices = ::ReadIntegerFromFile(pInFile);
    ::ReadStringFromFile(pInFile, pMeshInfo->m_pstrMeshName, sizeof(pstrToken));

    for(; ; ) {
        ::ReadStringFromFile(pInFile, pstrToken, sizeof(pstrToken));

        if(!strcmp(pstrToken, "<Bounds>:")) {
            nReads = (UINT)::fread(&(pMeshInfo->m_xmf3AABBCenter), sizeof(XMFLOAT3), 1, pInFile);
            nReads = (UINT)::fread(&(pMeshInfo->m_xmf3AABBExtents), sizeof(XMFLOAT3), 1, pInFile);
            pMeshInfo->m_xmf3AABBCenter = Vector3::ScalarProduct(pMeshInfo->m_xmf3AABBCenter, size, false);
            pMeshInfo->m_xmf3AABBExtents = Vector3::ScalarProduct(pMeshInfo->m_xmf3AABBExtents, size, false);
        }
        else if(!strcmp(pstrToken, "<Positions>:")) {
            nPositions = ::ReadIntegerFromFile(pInFile);
            if(nPositions > 0) {
                pMeshInfo->m_nType |= VERTEXT_POSITION;
                pMeshInfo->m_pxmf3Positions = new XMFLOAT3[nPositions];
                nReads = (UINT)::fread(pMeshInfo->m_pxmf3Positions, sizeof(XMFLOAT3), nPositions, pInFile);
                for(int i = 0; i < nPositions; i++) {
                    pMeshInfo->m_pxmf3Positions[i] = Vector3::ScalarProduct(pMeshInfo->m_pxmf3Positions[i], size, false);
                }
            }
        }
        else if(!strcmp(pstrToken, "<Colors>:")) {
            nColors = ::ReadIntegerFromFile(pInFile);
            if(nColors > 0) {
                pMeshInfo->m_nType |= VERTEXT_COLOR;
                pMeshInfo->m_pxmf4Colors = new XMFLOAT4[nColors];
                nReads = (UINT)::fread(pMeshInfo->m_pxmf4Colors, sizeof(XMFLOAT4), nColors, pInFile);
            }
        }
        else if(!strcmp(pstrToken, "<Normals>:")) {
            nNormals = ::ReadIntegerFromFile(pInFile);
            if(nNormals > 0) {
                pMeshInfo->m_nType |= VERTEXT_NORMAL;
                pMeshInfo->m_pxmf3Normals = new XMFLOAT3[nNormals];
                nReads = (UINT)::fread(pMeshInfo->m_pxmf3Normals, sizeof(XMFLOAT3), nNormals, pInFile);
            }
        }
        else if(!strcmp(pstrToken, "<Indices>:")) {
            nIndices = ::ReadIntegerFromFile(pInFile);
            if(nIndices > 0) {
                pMeshInfo->m_pnIndices = new UINT[nIndices];
                nReads = (UINT)::fread(pMeshInfo->m_pnIndices, sizeof(int), nIndices, pInFile);
            }
        }
        else if(!strcmp(pstrToken, "<SubMeshes>:")) {
            pMeshInfo->m_nSubMeshes = ::ReadIntegerFromFile(pInFile);
            if(pMeshInfo->m_nSubMeshes > 0) {
                pMeshInfo->m_pnSubSetIndices = new int[pMeshInfo->m_nSubMeshes];
                pMeshInfo->m_ppnSubSetIndices = new UINT*[pMeshInfo->m_nSubMeshes];
                for(int i = 0; i < pMeshInfo->m_nSubMeshes; i++) {
                    pMeshInfo->m_ppnSubSetIndices[i] = NULL;
                    ::ReadStringFromFile(pInFile, pstrToken, sizeof(pstrToken));
                    if(!strcmp(pstrToken, "<SubMesh>:")) {
                        int nIndex = ::ReadIntegerFromFile(pInFile);
                        pMeshInfo->m_pnSubSetIndices[i] = ::ReadIntegerFromFile(pInFile);
                        if(pMeshInfo->m_pnSubSetIndices[i] > 0) {
                            pMeshInfo->m_ppnSubSetIndices[i] = new UINT[pMeshInfo->m_pnSubSetIndices[i]];
                            nReads = (UINT)::fread(pMeshInfo->m_ppnSubSetIndices[i], sizeof(UINT), pMeshInfo->m_pnSubSetIndices[i], pInFile);
                        }

                    }
                }
            }
        }
        else if(!strcmp(pstrToken, "</Mesh>")) {
            break;
        }
    }
    return(pMeshInfo);
}


CGameObject* CGameObject::LoadFrameHierarchyFromFile(
    ID3D12Device* pd3dDevice, 
    ID3D12GraphicsCommandList* pd3dCommandList, 
    ID3D12RootSignature* pd3dGraphicsRootSignature, 
    FILE* pInFile, 
    float size
) {
    char pstrToken[64] = { '\0' };
    UINT nReads = 0;

    int nFrame = 0;

    CGameObject* pGameObject = new CGameObject { };

    for(; ; ) {
        ::ReadStringFromFile(pInFile, pstrToken, sizeof(pstrToken));
        if(!strcmp(pstrToken, "<Frame>:")) {
            nFrame = ::ReadIntegerFromFile(pInFile);
            ::ReadStringFromFile(pInFile, pGameObject->m_pstrFrameName, sizeof(pstrToken));
        }
        else if(!strcmp(pstrToken, "<Transform>:")) {           // 여기는 읽은 값을 안쓰는듯?
            XMFLOAT3 xmf3Position, xmf3Rotation, xmf3Scale;
            XMFLOAT4 xmf4Rotation;
            nReads = (UINT)::fread(&xmf3Position, sizeof(float), 3, pInFile);
            nReads = (UINT)::fread(&xmf3Rotation, sizeof(float), 3, pInFile); //Euler Angle
            nReads = (UINT)::fread(&xmf3Scale, sizeof(float), 3, pInFile);
            nReads = (UINT)::fread(&xmf4Rotation, sizeof(float), 4, pInFile); //Quaternion
        }
        else if(!strcmp(pstrToken, "<TransformMatrix>:")) {
            nReads = (UINT)::fread(&pGameObject->m_xmf4x4Transform, sizeof(float), 16, pInFile);
            pGameObject->m_xmf4x4Transform._41 *= size;
            pGameObject->m_xmf4x4Transform._42 *= size;
            pGameObject->m_xmf4x4Transform._43 *= size;
        }
        else if(!strcmp(pstrToken, "<Mesh>:")) {
            CMeshLoadInfo* pMeshInfo = pGameObject->LoadMeshInfoFromFile(pInFile, size);
            pGameObject->setBoundingBox(pMeshInfo->m_xmf3AABBCenter, pMeshInfo->m_xmf3AABBExtents);
            if(pMeshInfo) {
                CMesh* pMesh = NULL;
                if(pMeshInfo->m_nType & VERTEXT_NORMAL) {
                    pMesh = new CMeshIlluminatedFromFile(pd3dDevice, pd3dCommandList, pMeshInfo);
                }
                if(pMesh) pGameObject->SetMesh(pMesh);
                delete pMeshInfo;
            }
        }
        else if(!strcmp(pstrToken, "<Materials>:")) {
            MATERIALSLOADINFO* pMaterialsInfo = pGameObject->LoadMaterialsInfoFromFile(pd3dDevice, pd3dCommandList, pInFile);
            if(pMaterialsInfo && (pMaterialsInfo->m_nMaterials > 0)) {
                pGameObject->m_nMaterials = pMaterialsInfo->m_nMaterials;
                pGameObject->m_ppMaterials = new CMaterial*[pMaterialsInfo->m_nMaterials];

                for(int i = 0; i < pMaterialsInfo->m_nMaterials; i++) {
                    pGameObject->m_ppMaterials[i] = NULL;

                    CMaterial* pMaterial = new CMaterial();

                    CMaterialColors* pMaterialColors = new CMaterialColors(&pMaterialsInfo->m_pMaterials[i]);
                    pMaterial->SetMaterialColors(pMaterialColors);

                    if(pGameObject->GetMeshType() & VERTEXT_NORMAL) pMaterial->SetIlluminatedShader();

                    pGameObject->SetMaterial(i, pMaterial);
                }
            }
        }
        else if(!strcmp(pstrToken, "<Children>:")) {
            int nChilds = ::ReadIntegerFromFile(pInFile);
            if(nChilds > 0) {
                for(int i = 0; i < nChilds; i++) {
                    CGameObject* pChild = CGameObject::LoadFrameHierarchyFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, pInFile, size);
                    if(pChild) pGameObject->SetChild(pChild);
#ifdef _WITH_DEBUG_RUNTIME_FRAME_HIERARCHY
                    TCHAR pstrDebug[256] = { 0 };
                    _stprintf_s(pstrDebug, 256, _T("(Child Frame: %p) (Parent Frame: %p)\n"), pChild, pGameObject);
                    OutputDebugString(pstrDebug);
#endif
                }
            }
        }
        else if(!strcmp(pstrToken, "</Frame>")) {
            break;
        }
    }
    return(pGameObject);
}


CGameObject* CGameObject::LoadGeometryFromFile(
    ID3D12Device* pd3dDevice,
    ID3D12GraphicsCommandList* pd3dCommandList,
    ID3D12RootSignature* pd3dGraphicsRootSignature,
    const char* pstrFileName,
    float size
) {
    FILE* pInFile = NULL;
    ::fopen_s(&pInFile, pstrFileName, "rb");
    if(!pInFile) {
        ::MessageBox(NULL, _T("Failed to open file"), _T("Error"), MB_OK);
        return nullptr;
    }
    ::rewind(pInFile);

    CGameObject* pGameObject = nullptr;
    char pstrToken[64] = { '\0' };

    for(; ; ) {
        ::ReadStringFromFile(pInFile, pstrToken, sizeof(pstrToken));

        if(!strcmp(pstrToken, "<Hierarchy>:")) {
            pGameObject = CGameObject::LoadFrameHierarchyFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, pInFile, size);
        }
        else if(!strcmp(pstrToken, "</Hierarchy>")) {
            break;
        }
    }

#ifdef _WITH_DEBUG_FRAME_HIERARCHY
    TCHAR pstrDebug[256] = { 0 };
    _stprintf_s(pstrDebug, 256, _T("Frame Hierarchy\n"));
    OutputDebugString(pstrDebug);

    CGameObject::PrintFrameInfo(pGameObject, NULL);
#endif

    return(pGameObject);
}

CGameObject* CGameObject::LoadFromObjFile(
    ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList,
    ID3D12RootSignature* pd3dGraphicsRootSignature,
    const char* pstrFileName, float size
) {
    CGameObject* pGameObject = new CGameObject { };

    {
        MATERIALLOADINFO material_info { };
        pGameObject->m_nMaterials = 1;
        pGameObject->m_ppMaterials = new CMaterial*[1];
        pGameObject->m_ppMaterials[0] = NULL;

        CMaterial* pMaterial = new CMaterial { };
        CMaterialColors* pMaterialColors = new CMaterialColors { &material_info };
        pMaterial->SetMaterialColors(pMaterialColors);
        pMaterial->SetIlluminatedShader();

        pGameObject->SetMaterial(0, pMaterial);
    }

    CMeshLoadInfo* pMeshInfo = new CMeshLoadInfo { };
    pMeshInfo->m_nType |= VERTEXT_POSITION;
    pMeshInfo->m_nType |= VERTEXT_NORMAL;
    //pMeshInfo->m_nType |= VERTEXT_COLOR;

    vector<XMFLOAT3> vertices;
    vector<UINT> indices;

    {
        ifstream obj { pstrFileName };
        if(obj.is_open()) {
            string line;
            while(getline(obj, line)) {
                if(line[0] == 'v') {
                    istringstream iss { line };
                    char c;
                    float x, y, z;
                    iss >> c >> x >> y >> z;
                    vertices.push_back({ x * size, y * size, -z * size });
                }
                else if(line[0] == 'f') {
                    istringstream iss { line };
                    char c;
                    UINT i1, i2, i3;
                    iss >> c >> i1 >> i2 >> i3;
                    indices.push_back(i3 - 1);
                    indices.push_back(i2 - 1);
                    indices.push_back(i1 - 1);
                }
            }
        }
    }

    // 프리미티브별로 노말을 하나씩 가져야 하는데, 이게 지금 안됨.
    // 그래서 프리미티브별로 정점을 가지도록 정점을 복사해야함
    pMeshInfo->m_nVertices = static_cast<int>(indices.size());
    pMeshInfo->m_pxmf3Positions = new XMFLOAT3[pMeshInfo->m_nVertices];

    pMeshInfo->m_nIndices = pMeshInfo->m_nVertices;
    pMeshInfo->m_pnIndices = new UINT[pMeshInfo->m_nIndices];

    pMeshInfo->m_pxmf3Normals = new XMFLOAT3[pMeshInfo->m_nVertices];

    int fragment_count = pMeshInfo->m_nVertices / 3;
    for(int i=0; i<fragment_count; ++i) {
        XMFLOAT3 v1 = vertices[indices[i*3+0]];
        XMFLOAT3 v2 = vertices[indices[i*3+1]];
        XMFLOAT3 v3 = vertices[indices[i*3+2]];

        XMFLOAT3 e1 = Vector3::Subtract(v2, v1);
        XMFLOAT3 e2 = Vector3::Subtract(v3, v1);
        XMFLOAT3 normal = Vector3::CrossProduct(e1, e2);
        normal = Vector3::Normalize(normal);

        pMeshInfo->m_pxmf3Positions[i*3+0] = v1;
        pMeshInfo->m_pxmf3Positions[i*3+1] = v2;
        pMeshInfo->m_pxmf3Positions[i*3+2] = v3;

        pMeshInfo->m_pxmf3Normals[i*3+0] = normal;
        pMeshInfo->m_pxmf3Normals[i*3+1] = normal;
        pMeshInfo->m_pxmf3Normals[i*3+2] = normal;

        pMeshInfo->m_pnIndices[i*3+0] = i*3+0;
        pMeshInfo->m_pnIndices[i*3+1] = i*3+1;
        pMeshInfo->m_pnIndices[i*3+2] = i*3+2;
    }

    CMesh* mesh = new CMeshIlluminatedFromFile { pd3dDevice, pd3dCommandList, pMeshInfo };
    pGameObject->SetMesh(mesh);
    delete pMeshInfo;

    return pGameObject;
}


void CGameObject::PrintFrameInfo(CGameObject* pGameObject, CGameObject* pParent) {
    TCHAR pstrDebug[256] = { 0 };
    _stprintf_s(pstrDebug, 256, _T("(Frame: %p) (Parent: %p)\n"), pGameObject, pParent);
    OutputDebugString(pstrDebug);

    if(pGameObject->m_pSibling) CGameObject::PrintFrameInfo(pGameObject->m_pSibling, pParent);
    if(pGameObject->m_pChild) CGameObject::PrintFrameInfo(pGameObject->m_pChild, pGameObject);
}





















////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////






















CRotatingObject::CRotatingObject(): 
    CGameObject { },
    m_xmf3RotationAxis { 0.0f, 1.0f, 0.0f }, 
    m_fRotationSpeed { 90.0f }
{

}


void CRotatingObject::Animate(float fTimeElapsed, XMFLOAT4X4* pxmf4x4Parent) {
    if(rotate_local) {
        rotateLocal(&m_xmf3RotationAxis, m_fRotationSpeed * fTimeElapsed);
    }
    else {
        Rotate(&m_xmf3RotationAxis, m_fRotationSpeed * fTimeElapsed);
    }
    CGameObject::Animate(fTimeElapsed, pxmf4x4Parent);
}







CubeObject::CubeObject() : CGameObject { } {
    MATERIALLOADINFO material_info { };
    m_nMaterials = 1;
    m_ppMaterials = new CMaterial*[1];
    m_ppMaterials[0] = NULL;

    CMaterial* pMaterial = new CMaterial { };
    CMaterialColors* pMaterialColors = new CMaterialColors { &material_info };
    pMaterial->SetMaterialColors(pMaterialColors);
    pMaterial->SetIlluminatedShader();

    SetMaterial(0, pMaterial);
}
