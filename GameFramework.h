#pragma once

#include "stdafx.h"

#include "Timer.h"
#include "Scene.h"
#include "Player.h"
#include "Camera.h"


class CGameFramework {
private:
    HINSTANCE m_hInstance;
    HWND m_hWnd;

    int m_nWndClientWidth;
    int m_nWndClientHeight;

    IDXGIFactory4* m_pdxgiFactory;
    IDXGISwapChain3* m_pdxgiSwapChain;
    ID3D12Device* m_pd3dDevice;

    bool m_bMsaa4xEnable = false;

    UINT m_nMsaa4xQualityLevels = 0;

    static const UINT m_nSwapChainBuffers = 2;
    UINT m_nSwapChainBufferIndex;

    ID3D12Resource* m_ppd3dSwapChainBackBuffers[m_nSwapChainBuffers];
    ID3D12DescriptorHeap* m_pd3dRtvDescriptorHeap;
    UINT m_nRtvDescriptorIncrementSize;

    ID3D12Resource* m_pd3dDepthStencilBuffer;
    ID3D12DescriptorHeap* m_pd3dDsvDescriptorHeap;
    UINT m_nDsvDescriptorIncrementSize;

    ID3D12CommandQueue* m_pd3dCommandQueue;
    ID3D12CommandAllocator* m_pd3dCommandAllocator;
    ID3D12GraphicsCommandList* m_pd3dCommandList;

    ID3D12Fence* m_pd3dFence;
    UINT64 m_nFenceValues[m_nSwapChainBuffers];
    HANDLE m_hFenceEvent;
    
    CGameTimer m_GameTimer;

    _TCHAR m_pszFrameRate[70];

    POINT m_ptOldCursorPos;

    Scene* m_pScene;

    bool is_fullscreen;
    int screen_width;
    int screen_height;

#if defined(_DEBUG)
    ID3D12Debug* m_pd3dDebugController;
#endif


public:
    CGameFramework();
    ~CGameFramework();

public:
    bool OnCreate(HINSTANCE hInstance, HWND hMainWnd);
    void OnDestroy();

    void CreateSwapChain();
    void CreateRtvAndDsvDescriptorHeaps();
    void CreateDirect3DDevice();
    void CreateCommandQueueAndList();

    void CreateRenderTargetViews();
    void CreateDepthStencilView();

    void ChangeSwapChainState();

    void BuildObjects();
    void ReleaseObjects();
    void AnimateObjects();

    void FrameAdvance();
    void MoveToNextFrame();
    void WaitForGpuComplete();

    void ProcessInput();
    void OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
    void OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
    LRESULT CALLBACK OnProcessingWindowMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);

    void adaptSceneSize();
};

