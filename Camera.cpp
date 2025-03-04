#include "Camera.h"


CCamera::CCamera() :
    m_xmf4x4View { Matrix4x4::Identity() },
    m_xmf4x4Projection { Matrix4x4::Identity() },
    m_d3dViewport { 0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT, 0.0f, 1.0f },
    m_d3dScissorRect { 0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT } {

}

CCamera::~CCamera() {

}


void CCamera::CreateShaderVariables(
    ID3D12Device* pd3dDevice,
    ID3D12GraphicsCommandList* pd3dCommandList
) {
    UINT ncbElementBytes = ((sizeof(VS_CB_CAMERA_INFO) + 255) & ~255); //256ÀÇ ¹è¼ö
    m_pd3dcbCamera = ::CreateBufferResource(
        pd3dDevice, pd3dCommandList, 
        nullptr, ncbElementBytes, 
        D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, 
        nullptr
    );

    m_pd3dcbCamera->Map(0, nullptr, (void**)&m_pcbMappedCamera);
}

void CCamera::ReleaseShaderVariables() {
    if(m_pd3dcbCamera) {
        m_pd3dcbCamera->Unmap(0, NULL);
        m_pd3dcbCamera->Release();
    }
}

void CCamera::UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList) {
    XMFLOAT4X4 xmf4x4View;
    XMStoreFloat4x4(&xmf4x4View, XMMatrixTranspose(XMLoadFloat4x4(&m_xmf4x4View)));
    ::memcpy(&m_pcbMappedCamera->m_xmf4x4View, &xmf4x4View, sizeof(XMFLOAT4X4));

    XMFLOAT4X4 xmf4x4Projection;
    XMStoreFloat4x4(&xmf4x4Projection, XMMatrixTranspose(XMLoadFloat4x4(&m_xmf4x4Projection)));
    ::memcpy(&m_pcbMappedCamera->m_xmf4x4Projection, &xmf4x4Projection, sizeof(XMFLOAT4X4));

    ::memcpy(&m_pcbMappedCamera->m_xmf3Position, &m_xmf3Position, sizeof(XMFLOAT3));

    D3D12_GPU_VIRTUAL_ADDRESS d3dGpuVirtualAddress = m_pd3dcbCamera->GetGPUVirtualAddress();
    pd3dCommandList->SetGraphicsRootConstantBufferView(0, d3dGpuVirtualAddress);
}


void CCamera::GenerateViewMatrix(XMFLOAT3 xmf3Position, XMFLOAT3 xmf3LookAt, XMFLOAT3 xmf3Up) {
    m_xmf4x4View = Matrix4x4::LookAtLH(xmf3Position, xmf3LookAt, xmf3Up);
}


void CCamera::GenerateProjectionMatrix(
    float fNearPlaneDistance,
    float fFarPlaneDistance,
    float fAspectRatio,
    float fFOVAngle
) {
    aspect_ratio = fAspectRatio;
    fov_angle = fFOVAngle;

    m_xmf4x4Projection = Matrix4x4::PerspectiveFovLH(
        XMConvertToRadians(fFOVAngle),
        fAspectRatio,
        fNearPlaneDistance,
        fFarPlaneDistance
    );
}

void CCamera::SetViewport(
    int xTopLeft, int yTopLeft,
    int nWidth, int nHeight,
    float fMinZ, float fMaxZ
) {
    m_d3dViewport.TopLeftX = float(xTopLeft);
    m_d3dViewport.TopLeftY = float(yTopLeft);
    m_d3dViewport.Width = float(nWidth);
    m_d3dViewport.Height = float(nHeight);
    m_d3dViewport.MinDepth = fMinZ;
    m_d3dViewport.MaxDepth = fMaxZ;
}

void CCamera::SetScissorRect(LONG xLeft, LONG yTop, LONG xRight, LONG yBottom) {
    m_d3dScissorRect.left = xLeft;
    m_d3dScissorRect.top = yTop;
    m_d3dScissorRect.right = xRight;
    m_d3dScissorRect.bottom = yBottom;
}

void CCamera::SetViewportsAndScissorRects(ID3D12GraphicsCommandList* pd3dCommandList) {
    pd3dCommandList->RSSetViewports(1, &m_d3dViewport);
    pd3dCommandList->RSSetScissorRects(1, &m_d3dScissorRect);
}

XMFLOAT4X4 CCamera::getViewMatrix() const {
    return m_xmf4x4View;
}

XMFLOAT4X4 CCamera::getProjectionMatrix() const {
    return m_xmf4x4Projection;
}

D3D12_VIEWPORT CCamera::getViewport() const {
    return m_d3dViewport;
}

D3D12_RECT CCamera::getScissorRect() const {
    return m_d3dScissorRect;
}


float CCamera::getFovAngle() {
    return fov_angle;
}

float CCamera::getAspectRatio() {
    return aspect_ratio;
}


void CCamera::rotateHorizontal(float degrees) {
    horizontal_angle += degrees;
}

void CCamera::rotateVertical(float degrees) {
    vertical_angle += degrees;
    if(vertical_angle > +horizontal_max_angle) vertical_angle = +horizontal_max_angle;
    if(vertical_angle < -horizontal_max_angle) vertical_angle = -horizontal_max_angle;
}

void CCamera::setHorizontalMaxAngle(float angle) {
    horizontal_max_angle = angle;
    if(horizontal_max_angle > 89.0f) horizontal_max_angle = 89.0f;
}

void CCamera::setHorizontalMinAngle(float angle) {
    horizontal_min_angle = angle;
    if(horizontal_min_angle < -89.0f) horizontal_min_angle = -89.0f;
}
