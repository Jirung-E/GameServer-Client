//-----------------------------------------------------------------------------
// File: CPlayer.cpp
//-----------------------------------------------------------------------------

#include "stdafx.h"
#include "Player.h"
#include "Shader.h"



CPlayer::CPlayer() : TankObject { } {
    m_xmf3Position = XMFLOAT3(0.0f, 0.0f, 0.0f);
    m_xmf3Right = XMFLOAT3(1.0f, 0.0f, 0.0f);
    m_xmf3Up = XMFLOAT3(0.0f, 1.0f, 0.0f);
    m_xmf3Look = XMFLOAT3(0.0f, 0.0f, 1.0f);

    m_xmf3Velocity = XMFLOAT3(0.0f, 0.0f, 0.0f);
    m_xmf3Gravity = XMFLOAT3(0.0f, 0.0f, 0.0f);
    m_fMaxVelocityXZ = 0.0f;
    m_fMaxVelocityY = 0.0f;
    m_fFriction = 0.0f;

    m_fPitch = 0.0f;
    m_fRoll = 0.0f;
    m_fYaw = 0.0f;
}

CPlayer::~CPlayer() {
    ReleaseShaderVariables();
}


void CPlayer::SetPosition(const XMFLOAT3& xmf3Position) {
    Move(XMFLOAT3(xmf3Position.x - m_xmf3Position.x, xmf3Position.y - m_xmf3Position.y, xmf3Position.z - m_xmf3Position.z), false);
}


void CPlayer::Move(DWORD dwDirection, float fDistance, bool bVelocity) {
    if(m_bBlowingUp) return;

    if(dwDirection) {
        XMFLOAT3 xmf3Shift { 0.0f, 0.0f, 0.0f };

        if(dwDirection & DIR_FORWARD)   xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Look, fDistance);
        if(dwDirection & DIR_BACKWARD)  xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Look, -fDistance);
        if(dwDirection & DIR_RIGHT)     xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Right, fDistance);
        if(dwDirection & DIR_LEFT)      xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Right, -fDistance);
        if(dwDirection & DIR_UP)        xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Up, fDistance);
        if(dwDirection & DIR_DOWN)      xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Up, -fDistance);

        Move(xmf3Shift, bVelocity);
    }
}

void CPlayer::Move(const XMFLOAT3& xmf3Shift, bool bVelocity) {
    if(m_bBlowingUp) return;

    if(bVelocity) {
        if(xmf3Shift.x != 0.0f || xmf3Shift.y != 0.0f || xmf3Shift.z != 0.0f) {
            m_xmf3Velocity = Vector3::Add(m_xmf3Velocity, xmf3Shift);
        }
        m_xmf3Velocity = Vector3::Add(m_xmf3Velocity, xmf3Shift);
    }
    else {
        m_xmf3Position = Vector3::Add(m_xmf3Position, xmf3Shift);
    }
}


void CPlayer::Rotate(float x, float y, float z) {
    if(m_bBlowingUp) return;

    if(x != 0.0f) {
        m_fPitch += x;
        if(m_fPitch > +89.0f) { x -= (m_fPitch - 89.0f); m_fPitch = +89.0f; }
        if(m_fPitch < -89.0f) { x -= (m_fPitch + 89.0f); m_fPitch = -89.0f; }
    }
    if(y != 0.0f) {
        m_fYaw += y;
        if(m_fYaw > 360.0f) m_fYaw -= 360.0f;
        if(m_fYaw < 0.0f) m_fYaw += 360.0f;
    }
    if(z != 0.0f) {
        m_fRoll += z;
        if(m_fRoll > +20.0f) { z -= (m_fRoll - 20.0f); m_fRoll = +20.0f; }
        if(m_fRoll < -20.0f) { z -= (m_fRoll + 20.0f); m_fRoll = -20.0f; }
    }

    if(y != 0.0f) {
        XMMATRIX xmmtxRotate = XMMatrixRotationAxis(XMLoadFloat3(&m_xmf3Up), XMConvertToRadians(y));
        m_xmf3Look = Vector3::TransformNormal(m_xmf3Look, xmmtxRotate);
        m_xmf3Right = Vector3::TransformNormal(m_xmf3Right, xmmtxRotate);
    }

    m_xmf3Look = Vector3::Normalize(m_xmf3Look);
    m_xmf3Right = Vector3::CrossProduct(m_xmf3Up, m_xmf3Look, true);
    m_xmf3Up = Vector3::CrossProduct(m_xmf3Look, m_xmf3Right, true);
}

void CPlayer::Rotate(XMFLOAT3* pxmf3Axis, float fAngle) {
    auto mtx = XMMatrixRotationAxis(XMLoadFloat3(pxmf3Axis), XMConvertToRadians(fAngle));

    m_xmf3Look = Vector3::TransformNormal(m_xmf3Look, mtx);
    m_xmf3Right = Vector3::TransformNormal(m_xmf3Right, mtx);
    m_xmf3Up = Vector3::TransformNormal(m_xmf3Up, mtx);
}


void CPlayer::rotateY(float degree) {
    if(m_bBlowingUp) return;

    m_fYaw += degree;
    if(m_fYaw > 360.0f) m_fYaw -= 360.0f;
    if(m_fYaw < 0.0f) m_fYaw += 360.0f;

    XMMATRIX xmmtxRotate = XMMatrixRotationAxis(XMLoadFloat3(&m_xmf3Up), XMConvertToRadians(degree));
    m_xmf3Look = Vector3::TransformNormal(m_xmf3Look, xmmtxRotate);
    m_xmf3Right = Vector3::TransformNormal(m_xmf3Right, xmmtxRotate);

    m_xmf3Look = Vector3::Normalize(m_xmf3Look);
    m_xmf3Right = Vector3::CrossProduct(m_xmf3Up, m_xmf3Look, true);
    m_xmf3Up = Vector3::CrossProduct(m_xmf3Look, m_xmf3Right, true);
}


void CPlayer::Update(float fTimeElapsed) {
    if(m_bBlowingUp) return;

    m_xmf3Velocity = Vector3::Add(m_xmf3Velocity, m_xmf3Gravity);
    float fLength = sqrtf(m_xmf3Velocity.x * m_xmf3Velocity.x + m_xmf3Velocity.z * m_xmf3Velocity.z);
    float fMaxVelocityXZ = m_fMaxVelocityXZ;
    if(fLength > m_fMaxVelocityXZ) {
        m_xmf3Velocity.x *= (fMaxVelocityXZ / fLength);
        m_xmf3Velocity.z *= (fMaxVelocityXZ / fLength);
    }
    float fMaxVelocityY = m_fMaxVelocityY;
    fLength = sqrtf(m_xmf3Velocity.y * m_xmf3Velocity.y);
    if(fLength > m_fMaxVelocityY) {
        m_xmf3Velocity.y *= (fMaxVelocityY / fLength);
    }

    XMFLOAT3 xmf3Velocity = Vector3::ScalarProduct(m_xmf3Velocity, fTimeElapsed, false);
    Move(xmf3Velocity, false);

    fLength = Vector3::Length(m_xmf3Velocity);
    float fDeceleration = (m_fFriction * fTimeElapsed);
    if(fDeceleration > fLength) {
        fDeceleration = fLength;
    }
    m_xmf3Velocity = Vector3::Add(m_xmf3Velocity, Vector3::ScalarProduct(m_xmf3Velocity, -fDeceleration, true));
}


void CPlayer::rotateLookDirection(float dx, float dy) {
    if(m_bBlowingUp) return;

        look_direction_pitch += dx;
        if(look_direction_pitch > look_direction_pitch_max) { 
            dx -= (look_direction_pitch - look_direction_pitch_max);
            look_direction_pitch = look_direction_pitch_max;
        }
        if(look_direction_pitch < look_direction_pitch_min) {
            dx -= (look_direction_pitch - look_direction_pitch_min);
            look_direction_pitch = look_direction_pitch_min;
        }

        look_direction_yaw += dy;
        if(look_direction_yaw > 360.0f) look_direction_yaw -= 360.0f;
        if(look_direction_yaw < 0.0f)   look_direction_yaw += 360.0f;

    //XMMATRIX xmmtxRotate = XMMatrixRotationRollPitchYaw(XMConvertToRadians(dx), XMConvertToRadians(dy), 0.0f);
    //look_direction = Vector3::TransformNormal(look_direction, xmmtxRotate);
    //look_direction = Vector3::Normalize(look_direction);

    look_direction = Vector3::TransformNormal(
        { 0.0f, 0.0f, 1.0f }, 
        XMMatrixRotationRollPitchYaw(
            XMConvertToRadians(look_direction_pitch), 
            XMConvertToRadians(look_direction_yaw), 
            0.0f
        )
    );
}


void CPlayer::OnPrepareRender() {
    m_xmf4x4Transform._11 = m_xmf3Right.x; m_xmf4x4Transform._12 = m_xmf3Right.y; m_xmf4x4Transform._13 = m_xmf3Right.z;
    m_xmf4x4Transform._21 = m_xmf3Up.x; m_xmf4x4Transform._22 = m_xmf3Up.y; m_xmf4x4Transform._23 = m_xmf3Up.z;
    m_xmf4x4Transform._31 = m_xmf3Look.x; m_xmf4x4Transform._32 = m_xmf3Look.y; m_xmf4x4Transform._33 = m_xmf3Look.z;
    m_xmf4x4Transform._41 = m_xmf3Position.x; m_xmf4x4Transform._42 = m_xmf3Position.y; m_xmf4x4Transform._43 = m_xmf3Position.z;

    UpdateTransform(NULL);
}






///////////////////////////////////////////////////////////////////////////////////////////////////////////////////








CTankPlayer::CTankPlayer(
    ID3D12Device* pd3dDevice, 
    ID3D12GraphicsCommandList* pd3dCommandList, 
    ID3D12RootSignature* pd3dGraphicsRootSignature
): 
    CPlayer { }
{
    m_fMaxVelocityXZ = 5.0f;
    m_fMaxVelocityY = 0.0f;
    m_fFriction = 5.0f;
    
    CGameObject* tank = CGameObject::LoadGeometryFromFile(
        pd3dDevice, pd3dCommandList, 
        pd3dGraphicsRootSignature, 
        "Model/M26.bin"
    );
    tank->SetPosition(0.0f, 0.0f, 0.0f);
    tank->rotateLocal(0.0f, 180.0f, 0.0f);

    SetChild(tank);

    OnInitialize();

    CreateShaderVariables(pd3dDevice, pd3dCommandList);
}

CTankPlayer::~CTankPlayer() {
}


void CTankPlayer::OnPrepareRender() {
    CPlayer::OnPrepareRender();
}


void CTankPlayer::Animate(float fTimeElapsed, XMFLOAT4X4* pxmf4x4Parent) {
    CPlayer::Update(fTimeElapsed);

    CPlayer::Animate(fTimeElapsed);
}
