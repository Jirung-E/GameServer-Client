#pragma once

#include "stdafx.h"

#define ASPECT_RATIO (float(FRAME_BUFFER_WIDTH) / float(FRAME_BUFFER_HEIGHT))


struct VS_CB_CAMERA_INFO {
    XMFLOAT4X4 m_xmf4x4View;
    XMFLOAT4X4 m_xmf4x4Projection;
    XMFLOAT3 m_xmf3Position;
};

class CCamera {
protected:
    XMFLOAT3 m_xmf3Position { 0.0f, 0.0f, 0.0f };

    XMFLOAT4X4 m_xmf4x4View;
    XMFLOAT4X4 m_xmf4x4Projection;

    D3D12_VIEWPORT m_d3dViewport;
    D3D12_RECT m_d3dScissorRect;

    float fov_angle = 90.0f;
    float aspect_ratio = ASPECT_RATIO;
    
    ID3D12Resource* m_pd3dcbCamera = nullptr;
    VS_CB_CAMERA_INFO* m_pcbMappedCamera = nullptr;

    float horizontal_angle = 0.0f;
    float vertical_angle = 0.0f;

    float horizontal_max_angle = 89.0f;
    float horizontal_min_angle = -89.0f;
    
public:
    enum Mode {
        FirstPerson,
        ThirdPerson
    };

protected:
    Mode mode;

public:
    CCamera();
    virtual ~CCamera();

public:
    virtual void CreateShaderVariables(
        ID3D12Device* pd3dDevice,
        ID3D12GraphicsCommandList* pd3dCommandList
    );
    virtual void ReleaseShaderVariables();
    virtual void UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList);

    void GenerateViewMatrix(XMFLOAT3 xmf3Position, XMFLOAT3 xmf3LookAt, XMFLOAT3 xmf3Up);
    void GenerateProjectionMatrix(
        float fNearPlaneDistance,
        float fFarPlaneDistance,
        float fAspectRatio,
        float fFOVAngle
    );

    void SetViewport(
        int xTopLeft, int yTopLeft,
        int nWidth, int nHeight,
        float fMinZ = 0.0f, float fMaxZ = 1.0f
    );
    void SetScissorRect(LONG xLeft, LONG yTop, LONG xRight, LONG yBottom);

    virtual void SetViewportsAndScissorRects(ID3D12GraphicsCommandList* pd3dCommandList);

    XMFLOAT4X4 getViewMatrix() const;
    XMFLOAT4X4 getProjectionMatrix() const;
    D3D12_VIEWPORT getViewport() const;
    D3D12_RECT getScissorRect() const;

    float getFovAngle();
    float getAspectRatio();

    void rotateHorizontal(float degrees);
    void rotateVertical(float degrees);

    void setHorizontalMaxAngle(float angle);
    void setHorizontalMinAngle(float angle);
};
