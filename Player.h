#pragma once

#define DIR_FORWARD				0x01
#define DIR_BACKWARD			0x02
#define DIR_LEFT				0x04
#define DIR_RIGHT				0x08
#define DIR_UP					0x10
#define DIR_DOWN				0x20

#include "GameObject.h"
#include "Camera.h"




class CPlayer : public CGameObject {
protected:
    XMFLOAT3 m_xmf3Position;

public:
    CPlayer();
    virtual ~CPlayer();

public:
    XMFLOAT3 GetPosition() { return m_xmf3Position; }
    void SetPosition(const XMFLOAT3& xmf3Position);

    virtual void OnPrepareRender();
};








class ChessPlayer : public CPlayer {
public:
    ChessPlayer(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature);
    virtual ~ChessPlayer();

public:
    virtual void OnPrepareRender();
};


