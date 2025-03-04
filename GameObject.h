#pragma once

#include "Mesh.h"
#include "Camera.h"
#include "Material.h"


class CShader;


class CGameObject {
private:
    int m_nReferences = 0;

protected:
    XMFLOAT4X4 m_xmf4x4World;
    XMFLOAT4X4 m_xmf4x4Transform;

    CGameObject* m_pParent = nullptr;
    CGameObject* m_pChild = nullptr;
    CGameObject* m_pSibling = nullptr;

    char m_pstrFrameName[64];

    CMesh* m_pMesh = nullptr;

    int m_nMaterials = 0;
    CMaterial** m_ppMaterials = nullptr;

    BoundingOrientedBox m_xmOOBB { };
    BoundingOrientedBox m_xmOOBB_original { };

    XMFLOAT3 m_xmf3MovingDirection = XMFLOAT3 { 0.0f, 0.0f, 1.0f };
    float m_fMovingSpeed = 0.0f;

public:
    bool rotate_local = false;

    bool is_active = true;

public:
    CGameObject();
    virtual ~CGameObject();

public:
    void AddRef();
    void Release();

    void SetMesh(CMesh* pMesh);
    void SetShader(CShader* pShader);
    void SetShader(int nMaterial, CShader* pShader);
    void SetMaterial(int nMaterial, CMaterial* pMaterial);

    void SetChild(CGameObject* pChild, bool bReferenceUpdate = false);

    virtual void BuildMaterials(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList) { }
    
    virtual void OnInitialize() { }

    virtual void Animate(float fTimeElapsed, XMFLOAT4X4* pxmf4x4Parent = nullptr);

    virtual void OnPrepareRender() { }
    virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera = nullptr);

    virtual void CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
    virtual void UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList);
    virtual void ReleaseShaderVariables();

    virtual void UpdateShaderVariable(ID3D12GraphicsCommandList* pd3dCommandList, XMFLOAT4X4* pxmf4x4World);
    virtual void UpdateShaderVariable(ID3D12GraphicsCommandList* pd3dCommandList, CMaterial* pMaterial);

    virtual void ReleaseUploadBuffers();

    CGameObject* GetParent() { return(m_pParent); }

    void UpdateTransform(XMFLOAT4X4* pxmf4x4Parent=nullptr);

    CGameObject* FindFrame(const char* pstrFrameName);
    UINT GetMeshType() { return((m_pMesh) ? m_pMesh->GetType() : 0); }



    XMFLOAT3 GetPosition() const;
    XMFLOAT3 GetLook() const;
    XMFLOAT3 GetUp() const;
    XMFLOAT3 GetRight() const;

    void SetPosition(float x, float y, float z);
    void SetPosition(const XMFLOAT3& xmf3Position) { SetPosition(xmf3Position.x, xmf3Position.y, xmf3Position.z); }
    void SetScale(float x, float y, float z);

    void MoveStrafe(float fDistance = 1.0f);
    void MoveUp(float fDistance = 1.0f);
    void MoveForward(float fDistance = 1.0f);
    void Move(const XMFLOAT3& vDirection, float fSpeed);

    void Rotate(float pitch=10.0f, float yaw=10.0f, float roll=10.0f);
    void Rotate(XMFLOAT3* pxmf3Axis, float fAngle);
    void Rotate(XMFLOAT4* pxmf4Quaternion);

    void rotateLocal(float pitch, float yaw, float roll);
    void rotateLocal(XMFLOAT3* pxmf3Axis, float fAngle);

    void setWorldTransform(const XMFLOAT4X4& xmmtxWorld) { m_xmf4x4World = xmmtxWorld; }
    XMFLOAT4X4 getWorldTransform() { return m_xmf4x4World; }

    void setTransform(const XMFLOAT4X4& xmmtxTransform) { m_xmf4x4Transform = xmmtxTransform; }
    XMFLOAT4X4 getTransform() { return m_xmf4x4Transform; }

    //void Move(XMFLOAT3& vDirection, float fSpeed);

    void setMovingDirection(const XMFLOAT3& xmf3MovingDirection) { m_xmf3MovingDirection = Vector3::Normalize(xmf3MovingDirection); }
    void setMovingSpeed(float fSpeed) { m_fMovingSpeed = fSpeed; }
    float getMovingSpeed() { return m_fMovingSpeed; }
    XMFLOAT3 getMovingDirection() { return m_xmf3MovingDirection; }


    bool checkBoundingBoxIntersection(
        XMVECTOR& xmvPickRayOrigin,
        XMVECTOR& xmvPickRayDirection,
        float* pfNearHitDistance
    );
    bool checkBoundingBoxIntersection(const BoundingOrientedBox& other);
    bool checkBoundingBoxIntersection(const CGameObject* other);

    void GenerateRayForPicking(
        const XMVECTOR& xmvPickPosition, 
        const XMMATRIX& xmmtxView, 
        XMVECTOR& xmvPickRayOrigin, 
        XMVECTOR& xmvPickRayDirection
    );
    bool PickObjectByRayIntersection(
        const XMVECTOR& xmPickPosition, 
        const XMMATRIX& xmmtxView, 
        float* pfHitDistance
    );

    void setBoundingBox(const XMFLOAT3& xmCenter, const XMFLOAT3& xmExtents, const XMFLOAT4& xmOrientation={ 0.0f, 0.0f, 0.0f, 1.0f });
    void setBoundingBox(const XMFLOAT3& xmExtents);

    BoundingOrientedBox getBoundingBox() { return m_xmOOBB; }



protected:
    void updateBoundingBox();






public:
    static MATERIALSLOADINFO* LoadMaterialsInfoFromFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, FILE* pInFile);
    static CMeshLoadInfo* LoadMeshInfoFromFile(FILE* pInFile, float size=1.0f);

    static CGameObject* LoadFrameHierarchyFromFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, FILE* pInFile, float size=1.0f);
    static CGameObject* LoadGeometryFromFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, const char* pstrFileName, float size=1.0f);
    static CGameObject* LoadFromObjFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, const char* pstrFileName, float size=1.0f);

    static void PrintFrameInfo(CGameObject* pGameObject, CGameObject* pParent);
};

















////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////














class CRotatingObject : public CGameObject {
protected:
    XMFLOAT3 m_xmf3RotationAxis;
    float m_fRotationSpeed;

public:
    CRotatingObject();

public:
    void SetRotationSpeed(float fRotationSpeed) { m_fRotationSpeed = fRotationSpeed; }
    void SetRotationAxis(const XMFLOAT3& xmf3RotationAxis) { m_xmf3RotationAxis = xmf3RotationAxis; }
    float getRotationSpeed() { return m_fRotationSpeed; }
    XMFLOAT3 getRotationAxis() { return m_xmf3RotationAxis; }

    virtual void Animate(float fTimeElapsed, XMFLOAT4X4* pxmf4x4Parent=NULL);
};

















#define EXPLOSION_DEBRISES 240


class CExplosiveObject : public CRotatingObject {
protected:
    XMFLOAT4X4 m_pxmf4x4Transforms[EXPLOSION_DEBRISES] { };

    float m_fElapsedTimes = 0.0f;
    float m_fDuration = 2.0f;
    float m_fExplosionSpeed = 10.0f;
    float m_fExplosionRotation = 720.0f;

public:
    bool m_bBlowingUp = false;

    static CMesh* m_pExplosionMesh;
    static XMFLOAT3	m_pxmf3SphereVectors[EXPLOSION_DEBRISES];

public:
    CExplosiveObject();

public:
    virtual void Animate(float fTimeElapsed, XMFLOAT4X4* pxmf4x4Parent=nullptr);
    virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera = nullptr);

    static void PrepareExplosion(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, CMesh* debrises_mesh);
};
















class CubeObject : public CRotatingObject {
public:
    CubeObject();
};












class BulletObject : public CubeObject {
protected:
    float limit_distance = 1000.0f;
    float moved_distance = 0.0f;

public:
    BulletObject(float speed = 100.0f);

public:
    virtual void Animate(float fTimeElapsed, XMFLOAT4X4* pxmf4x4Parent = NULL);

    void Reset();
};














class LauncherObject : public CExplosiveObject {
protected:
    vector<BulletObject*> bullets;
    float fire_delay = 0.25f;
    float last_fire_elapsed = 0.0f;

public:
    XMFLOAT3 fire_offset = XMFLOAT3 { 0.0f, -1.0f, 0.0f };

public:
    virtual void fire();

    void setBulletMesh(CMesh* pMesh);
    void setBulletBoundingBox(const XMFLOAT3& xmCenter, const XMFLOAT3& xmExtents, const XMFLOAT4& xmOrientation = { 0.0f, 0.0f, 0.0f, 1.0f });

    bool checkBulletCollision(LauncherObject* other);

    virtual void Animate(float fTimeElapsed, XMFLOAT4X4* pxmf4x4Parent = NULL);
    virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera = NULL);

protected:
    BulletObject* getAvailableBullet();
};





class FlyingObject : public LauncherObject {
public:
    FlyingObject();
    virtual ~FlyingObject();
};














class HellicopterObject : public FlyingObject {
protected:
    CGameObject* m_pMainRotorFrame = NULL;
    CGameObject* m_pTailRotorFrame = NULL;

public:
    HellicopterObject();
    virtual ~HellicopterObject();

public:
    virtual void OnInitialize();
    virtual void Animate(float fTimeElapsed, XMFLOAT4X4* pxmf4x4Parent = NULL);
};












class TankObject : public LauncherObject {
protected:
    CGameObject* m_pTurretFrame = NULL;
    CGameObject* m_pCannonFrame = NULL;
    CGameObject* m_pGunFrame = NULL;

    float cannon_angle = 0.0f;

public:
    const float cannon_angle_up_limit = 50.0f;
    const float cannon_angle_down_limit = -10.0f;
    const float cannon_rotation_speed = 90.0f;

    const float turret_rotation_speed = 90.0f;

public:
    TankObject();
    virtual ~TankObject();

public:
    XMFLOAT3 getCannonLook() const;
    XMFLOAT3 getCannonPosition() const;
    float getCannonAngle() const;
    
    XMFLOAT3 getTurretPosition() const;

    void rotateCannon(float dt, float degree);
    void rotateTurret(float dt, float degree);

    virtual void fire();

    virtual void OnInitialize();
};