#pragma once

#include <vector>

#include "stdafx.h"
#include "Camera.h"
#include "GameObject.h"
//#include "Airplane.h"

#include "Shader.h"
#include "Player.h"

#include "Light.h"


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

    ID3D12RootSignature* m_pd3dGraphicsRootSignature = nullptr;

public:
    XMFLOAT4 bg_color { };

public:
    Scene();
    virtual ~Scene();

public:
    virtual bool OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
    virtual bool OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
    virtual bool ProcessInput(UCHAR* pKeysBuffer);

    ID3D12RootSignature* CreateGraphicsRootSignature(ID3D12Device* pd3dDevice);
    ID3D12RootSignature* GetGraphicsRootSignature();

    virtual void CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
    virtual void UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList);
    virtual void ReleaseShaderVariables();

    virtual void ReleaseUploadBuffers();

    virtual void BuildDefaultLightsAndMaterials();
    virtual void BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
    virtual void ReleaseObjects();
    virtual void AnimateObjects(float fTimeElapsed);

    virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList);

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

    std::vector<FlyingObject*> enemies;
    int enemy_count = 10;

    float player_speed = 100.0f;
    float player_rotation_speed = 45.0f;
    LIGHT* player_searchlight = nullptr;
    vector<LIGHT*> enemy_lights;

    bool mouse_pressed = false;

    POINT old_cursor_pos { };

    HeightMap* terrain_height_map = nullptr;
    CGameObject* terrain = nullptr;

public:
	GameScene();
	virtual ~GameScene();

public:
    virtual void CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);

    virtual void ReleaseUploadBuffers() {
        Scene::ReleaseUploadBuffers();

        if(m_pPlayer) m_pPlayer->ReleaseUploadBuffers();
    }

    virtual void BuildDefaultLightsAndMaterials();
    virtual void BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);

    virtual void AnimateObjects(float fTimeElapsed);

    virtual bool OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
    virtual bool OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);

    virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList);

protected:
    void checkEnemyByBulletCollisions();

    void movePlayer(float fTimeElapsed);

    void updateCamera();

    void buildTerrain(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
};

