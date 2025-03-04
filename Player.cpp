//-----------------------------------------------------------------------------
// File: CPlayer.cpp
//-----------------------------------------------------------------------------

#include "stdafx.h"
#include "Player.h"
#include "Shader.h"



CPlayer::CPlayer() : CGameObject { } {
    m_xmf3Position = XMFLOAT3(0.0f, 0.0f, 0.0f);
}

CPlayer::~CPlayer() {
    ReleaseShaderVariables();
}


void CPlayer::SetPosition(const XMFLOAT3& xmf3Position) {
    m_xmf3Position = xmf3Position;
}


void CPlayer::OnPrepareRender() {
    m_xmf4x4Transform._41 = m_xmf3Position.x; 
    m_xmf4x4Transform._42 = m_xmf3Position.y; 
    m_xmf4x4Transform._43 = m_xmf3Position.z;

    UpdateTransform(NULL);
}






///////////////////////////////////////////////////////////////////////////////////////////////////////////////////








ChessPlayer::ChessPlayer(
    ID3D12Device* pd3dDevice, 
    ID3D12GraphicsCommandList* pd3dCommandList, 
    ID3D12RootSignature* pd3dGraphicsRootSignature
): 
    CPlayer { }
{
    CGameObject* piece = CGameObject::LoadFromObjFile(
        pd3dDevice, pd3dCommandList, 
        pd3dGraphicsRootSignature, 
        "Model/king.obj",
        0.5f
    );
    piece->SetPosition(0.0f, 0.0f, 0.0f);

    SetChild(piece);

    OnInitialize();

    CreateShaderVariables(pd3dDevice, pd3dCommandList);
}

ChessPlayer::~ChessPlayer() {
}


void ChessPlayer::OnPrepareRender() {
    CPlayer::OnPrepareRender();
}
