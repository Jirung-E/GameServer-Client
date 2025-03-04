#pragma once

#define DIR_FORWARD				0x01
#define DIR_BACKWARD			0x02
#define DIR_LEFT				0x04
#define DIR_RIGHT				0x08
#define DIR_UP					0x10
#define DIR_DOWN				0x20

#include "GameObject.h"
#include "Camera.h"




class CPlayer : public TankObject {
protected:
    XMFLOAT3					m_xmf3Position;
    XMFLOAT3					m_xmf3Right;
    XMFLOAT3					m_xmf3Up;
    XMFLOAT3					m_xmf3Look;

    float           			m_fPitch;
    float           			m_fYaw;
    float           			m_fRoll;

    XMFLOAT3					m_xmf3Velocity;
    XMFLOAT3     				m_xmf3Gravity;
    float           			m_fMaxVelocityXZ;
    float           			m_fMaxVelocityY;
    float           			m_fFriction;

    XMFLOAT3 look_direction = XMFLOAT3 { 0.0f, 0.0f, 1.0f };
    float look_direction_yaw = 0.0f;
    float look_direction_pitch = 0.0f;
    float look_direction_pitch_max = -cannon_angle_down_limit;
    float look_direction_pitch_min = -cannon_angle_up_limit;

public:
    CPlayer();
    virtual ~CPlayer();

public:
    XMFLOAT3 GetPosition() { return m_xmf3Position; }
    XMFLOAT3 GetLookVector() { return m_xmf3Look; }
    XMFLOAT3 GetUpVector() { return m_xmf3Up; }
    XMFLOAT3 GetRightVector() { return m_xmf3Right; }

    void SetFriction(float fFriction) { m_fFriction = fFriction; }
    void SetGravity(const XMFLOAT3& xmf3Gravity) { m_xmf3Gravity = xmf3Gravity; }
    void SetMaxVelocityXZ(float fMaxVelocity) { m_fMaxVelocityXZ = fMaxVelocity; }
    void SetMaxVelocityY(float fMaxVelocity) { m_fMaxVelocityY = fMaxVelocity; }
    void SetVelocity(const XMFLOAT3& xmf3Velocity) { m_xmf3Velocity = xmf3Velocity; }
    void SetPosition(const XMFLOAT3& xmf3Position);

    const XMFLOAT3& GetVelocity() const { return(m_xmf3Velocity); }
    float GetYaw() const { return(m_fYaw); }
    float GetPitch() const { return(m_fPitch); }
    float GetRoll() const { return(m_fRoll); }

    void Move(DWORD dwDirection, float fDistance, bool bVelocity = false);
    void Move(const XMFLOAT3& xmf3Shift, bool bVelocity = false);

    void Rotate(float x, float y, float z);
    void Rotate(XMFLOAT3* pxmf3Axis, float fAngle);

    void rotateY(float degree);

    void Update(float fTimeElapsed);

    XMFLOAT3 getLookDirection() const {
        return look_direction;
    }
    float getLookDirectionYaw() const {
        return look_direction_yaw;
    }
    float getLookDirectionPitch() const {
        return look_direction_pitch;
    }

    void rotateLookDirection(float dx, float dy);

    virtual void OnPrepareRender();
};








class CTankPlayer : public CPlayer {
public:
    CTankPlayer(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature);
    virtual ~CTankPlayer();

public:
    virtual void OnPrepareRender();
    virtual void Animate(float fTimeElapsed, XMFLOAT4X4* pxmf4x4Parent=nullptr);
};


