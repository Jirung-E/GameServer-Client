#pragma once

#include <vector>
#include <unordered_map>

#include "stdafx.h"
#include "Camera.h"
#include "GameObject.h"
#include "Shader.h"
#include "Player.h"
#include "Light.h"
#include "Network.h"
#include "Console.h"


class Scene {
protected:
    std::vector<CGameObject*> m_pObjects;

    CCamera* camera = nullptr;

    LIGHT* m_pLights = nullptr;
    int m_nLights = 0;

    XMFLOAT4 m_xmf4GlobalAmbient;

    ID3D12Resource* m_pd3dcbLights = nullptr;
    LIGHTS* m_pcbMappedLights = nullptr;

    int scene_width;
    int scene_height;

    float near_plane_distance = 0.1f;
    float far_plane_distance = 10.0f;
    float fov = 90.0f;

    float m_fElapsedTime = 0.0f;

    ID3D12Device* m_pd3dDevice = nullptr;
    ID3D12GraphicsCommandList* m_pd3dCommandList = nullptr;
    ID3D12RootSignature* m_pd3dGraphicsRootSignature = nullptr;

public:
    XMFLOAT4 bg_color { };

public:
    Scene(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
    virtual ~Scene();

public:
    virtual bool OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
    virtual bool OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
    virtual bool ProcessInput(UCHAR* pKeysBuffer);

    ID3D12RootSignature* CreateGraphicsRootSignature();
    ID3D12RootSignature* GetGraphicsRootSignature();

    virtual void CreateShaderVariables();
    virtual void UpdateShaderVariables();
    virtual void ReleaseShaderVariables();

    virtual void ReleaseUploadBuffers();

    virtual void BuildDefaultLightsAndMaterials();
    virtual void BuildObjects();
    virtual void ReleaseObjects();
    virtual void AnimateObjects(float fTimeElapsed);

    virtual void Render();

    void setSceneSize(int width, int height);
    int getSceneWidth();
    int getSceneHeight();

protected:
    virtual void adaptCamera();
};


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

#define QUIT_GAME_SCENE (WM_APP+WM_QUIT)


class GameScene : public Scene {
private:
    CPlayer* m_pPlayer = nullptr;
    float player_speed = 100.0f;
    float player_rotation_speed = 45.0f;
    CMesh* player_mesh = nullptr;
    
    CMaterial* black_material = nullptr;
    CMaterial* white_material = nullptr;
    CMaterial* gray_material = nullptr;

    bool mouse_pressed = false;

    POINT old_cursor_pos { };

    Console console;
    WsaGuard wsa_guard;
    TcpConnection tcp_connection;
    long long client_id;
    std::string name;
    PacketParser packet_parser;
    std::unordered_map<long long, CPlayer*> players;

public:
	GameScene(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
	virtual ~GameScene();

public:
    virtual void CreateShaderVariables();

    virtual void ReleaseUploadBuffers();

    virtual void BuildDefaultLightsAndMaterials();
    virtual void BuildObjects();
    virtual void ReleaseObjects();
    virtual void AnimateObjects(float fTimeElapsed);

    virtual bool OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
    virtual bool OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);

    virtual void Render();

protected:
    CPlayer* addPlayer(long long id, const float x, const float z);
    void movePlayer(float fTimeElapsed);

    void updateCamera();

    void connectToServer();
    void login();
    void move(char direction);
    void recvFromServer();
    void processPacket(Packet& packet);
};

