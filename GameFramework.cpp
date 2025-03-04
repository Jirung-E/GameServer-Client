#include "stdafx.h"
#include "GameFramework.h"

#include <fstream>

using namespace std;


CGameFramework::CGameFramework():
    m_pdxgiFactory { nullptr },
    m_pdxgiSwapChain { nullptr },
    m_pd3dDevice { nullptr },
    
    m_pd3dCommandAllocator { nullptr },
    m_pd3dCommandQueue { nullptr },
    m_pd3dCommandList { nullptr },

    m_pd3dRtvDescriptorHeap { nullptr },
    m_nRtvDescriptorIncrementSize { 0 },

    m_pd3dDepthStencilBuffer { nullptr },
    m_pd3dDsvDescriptorHeap { nullptr },
    m_nDsvDescriptorIncrementSize { 0 },

    m_nSwapChainBufferIndex { 0 },
    
    m_hFenceEvent { nullptr },
    m_pd3dFence { nullptr },

    m_nWndClientWidth { FRAME_BUFFER_WIDTH },
    m_nWndClientHeight { FRAME_BUFFER_HEIGHT },

    m_pScene { nullptr },

    is_fullscreen { false },
    screen_width { ::GetSystemMetrics(SM_CXSCREEN) },
    screen_height { ::GetSystemMetrics(SM_CYSCREEN) }
{
    for(int i=0; i<m_nSwapChainBuffers; ++i) {
        m_ppd3dSwapChainBackBuffers[i] = nullptr;
    }

    for(int i=0; i<m_nSwapChainBuffers; ++i) {
        m_nFenceValues[i] = 0;
    }

    _tcscpy_s(m_pszFrameRate, _T("HW03 ("));
}

CGameFramework::~CGameFramework() {

}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CGameFramework::OnCreate(HINSTANCE hInstance, HWND hMainWnd) {
    m_hInstance = hInstance;
    m_hWnd = hMainWnd;

    CreateDirect3DDevice();
    CreateCommandQueueAndList();
    CreateRtvAndDsvDescriptorHeaps();
    CreateSwapChain();
    CreateDepthStencilView();

    BuildObjects();
    adaptSceneSize();

    return true;
}

void CGameFramework::OnDestroy() {
    //WaitForGpuComplete();

    ReleaseObjects();

    ::CloseHandle(m_hFenceEvent);

    for(int i=0; i<m_nSwapChainBuffers; ++i) {
        if(m_ppd3dSwapChainBackBuffers[i]) {
            m_ppd3dSwapChainBackBuffers[i]->Release();
        }
    }

    if(m_pd3dRtvDescriptorHeap) {
        m_pd3dRtvDescriptorHeap->Release();
    }
    if(m_pd3dDepthStencilBuffer) {
        m_pd3dDepthStencilBuffer->Release();
    }
    if(m_pd3dDsvDescriptorHeap) {
        m_pd3dDsvDescriptorHeap->Release();
    }

    if(m_pd3dCommandAllocator) {
        m_pd3dCommandAllocator->Release();
    }
    if(m_pd3dCommandQueue) {
        m_pd3dCommandQueue->Release();
    }
    if(m_pd3dCommandList) {
        m_pd3dCommandList->Release();
    }

    if(m_pd3dFence) {
        m_pd3dFence->Release();
    }

    m_pdxgiSwapChain->SetFullscreenState(FALSE, NULL);
    if(m_pdxgiSwapChain) {
        m_pdxgiSwapChain->Release();
    }
    if(m_pd3dDevice) {
        m_pd3dDevice->Release();
    }
    if(m_pdxgiFactory) {
        m_pdxgiFactory->Release();
    }

#if defined(_DEBUG)
    IDXGIDebug1* pdxgiDebug = NULL;
    DXGIGetDebugInterface1(0, __uuidof(IDXGIDebug1), (void**)&pdxgiDebug);
    HRESULT hResult = pdxgiDebug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_DETAIL);
    pdxgiDebug->Release();
#endif
}

void CGameFramework::CreateSwapChain() {
    RECT rcClient;
    ::GetClientRect(m_hWnd, &rcClient);
    m_nWndClientWidth = rcClient.right - rcClient.left;
    m_nWndClientHeight = rcClient.bottom - rcClient.top;

    DXGI_SWAP_CHAIN_DESC dxgiSwapChainDesc;
    ::ZeroMemory(&dxgiSwapChainDesc, sizeof(dxgiSwapChainDesc));
    dxgiSwapChainDesc.BufferCount = m_nSwapChainBuffers;
    dxgiSwapChainDesc.BufferDesc.Width = m_nWndClientWidth;
    dxgiSwapChainDesc.BufferDesc.Height = m_nWndClientHeight;
    dxgiSwapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    dxgiSwapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
    dxgiSwapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
    dxgiSwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    dxgiSwapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    dxgiSwapChainDesc.OutputWindow = m_hWnd;
    dxgiSwapChainDesc.SampleDesc.Count = (m_bMsaa4xEnable) ? 4 : 1;
    dxgiSwapChainDesc.SampleDesc.Quality = (m_bMsaa4xEnable) ? (m_nMsaa4xQualityLevels - 1) : 0;
    dxgiSwapChainDesc.Windowed = TRUE;
    dxgiSwapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    
    HRESULT hResult = m_pdxgiFactory->CreateSwapChain(m_pd3dCommandQueue,
        &dxgiSwapChainDesc, (IDXGISwapChain**)&m_pdxgiSwapChain);

    m_nSwapChainBufferIndex = m_pdxgiSwapChain->GetCurrentBackBufferIndex();
    
    hResult = m_pdxgiFactory->MakeWindowAssociation(m_hWnd, DXGI_MWA_NO_ALT_ENTER);

    //m_pdxgiFactory->MakeWindowAssociation(m_hWnd, DXGI_MWA_NO_ALT_ENTER);
    //m_nSwapChainBufferIndex = m_pdxgiSwapChain->GetCurrentBackBufferIndex();

//#ifndef _WITH_SWAPCHAIN_FULLSCREEN_STATE
    CreateRenderTargetViews();
//#endif
}

void CGameFramework::CreateRtvAndDsvDescriptorHeaps() {
    D3D12_DESCRIPTOR_HEAP_DESC d3dDescriptorHeapDesc;
    ::ZeroMemory(&d3dDescriptorHeapDesc, sizeof(D3D12_DESCRIPTOR_HEAP_DESC));

    d3dDescriptorHeapDesc.NumDescriptors = m_nSwapChainBuffers;
    d3dDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    d3dDescriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    d3dDescriptorHeapDesc.NodeMask = 0;

    HRESULT hResult = m_pd3dDevice->CreateDescriptorHeap(
        &d3dDescriptorHeapDesc, 
        __uuidof(ID3D12DescriptorHeap), 
        (void**)&m_pd3dRtvDescriptorHeap);
    m_nRtvDescriptorIncrementSize = m_pd3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

    d3dDescriptorHeapDesc.NumDescriptors = 1;
    d3dDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
    hResult = m_pd3dDevice->CreateDescriptorHeap(
        &d3dDescriptorHeapDesc, 
        __uuidof(ID3D12DescriptorHeap), 
        (void**)&m_pd3dDsvDescriptorHeap);
    m_nDsvDescriptorIncrementSize = m_pd3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
}

void CGameFramework::CreateDirect3DDevice() {
    HRESULT hResult;

    UINT dxgiFactoryFlags = 0;

#if defined(_DEBUG)
    ID3D12Debug* pd3dDebugController = NULL;
    hResult = D3D12GetDebugInterface(__uuidof(ID3D12Debug), (void**)&pd3dDebugController);
    if(pd3dDebugController) {
        pd3dDebugController->EnableDebugLayer();
        pd3dDebugController->Release();
    }
    dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
#endif

    hResult = ::CreateDXGIFactory2(dxgiFactoryFlags, __uuidof(IDXGIFactory4), (void**)&m_pdxgiFactory);

    IDXGIAdapter1* pd3dAdapter = NULL;

    for(UINT i=0; DXGI_ERROR_NOT_FOUND != m_pdxgiFactory->EnumAdapters1(i, &pd3dAdapter); ++i) {
        DXGI_ADAPTER_DESC1 dxgiAdapterDesc;
        pd3dAdapter->GetDesc1(&dxgiAdapterDesc);
        if(dxgiAdapterDesc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) {
            continue;
        }
        if(SUCCEEDED(D3D12CreateDevice(pd3dAdapter, D3D_FEATURE_LEVEL_12_0, __uuidof(ID3D12Device), (void**)&m_pd3dDevice))) {
            break;
        }
    }

    if(!pd3dAdapter) {
        m_pdxgiFactory->EnumWarpAdapter(__uuidof(IDXGIFactory4), (void**)&pd3dAdapter);
        hResult = D3D12CreateDevice(pd3dAdapter, D3D_FEATURE_LEVEL_12_0, __uuidof(ID3D12Device), (void**)&m_pd3dDevice);
    }

    D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS d3dMsaaQualityLevels;
    d3dMsaaQualityLevels.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    d3dMsaaQualityLevels.SampleCount = 4;
    d3dMsaaQualityLevels.Flags = D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE;
    d3dMsaaQualityLevels.NumQualityLevels = 0;
    hResult = m_pd3dDevice->CheckFeatureSupport(
        D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS, 
        &d3dMsaaQualityLevels, 
        sizeof(D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS));
    m_nMsaa4xQualityLevels = d3dMsaaQualityLevels.NumQualityLevels;

    m_bMsaa4xEnable = (m_nMsaa4xQualityLevels > 1) ? true : false;

    hResult = m_pd3dDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, __uuidof(ID3D12Fence), (void**)&m_pd3dFence);
    
    for(UINT i = 0; i < m_nSwapChainBuffers; i++) {
        m_nFenceValues[i] = 0;
    }

    m_hFenceEvent = ::CreateEvent(NULL, FALSE, FALSE, NULL);

    if(pd3dAdapter) {
        pd3dAdapter->Release();
    }
}

void CGameFramework::CreateCommandQueueAndList() {
    D3D12_COMMAND_QUEUE_DESC d3dCommandQueueDesc;
    ::ZeroMemory(&d3dCommandQueueDesc, sizeof(D3D12_COMMAND_QUEUE_DESC));
    d3dCommandQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    d3dCommandQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
    HRESULT hResult = m_pd3dDevice->CreateCommandQueue(
        &d3dCommandQueueDesc, 
        __uuidof(ID3D12CommandQueue), 
        (void**)&m_pd3dCommandQueue);

    hResult = m_pd3dDevice->CreateCommandAllocator(
        D3D12_COMMAND_LIST_TYPE_DIRECT, 
        __uuidof(ID3D12CommandAllocator), 
        (void**)&m_pd3dCommandAllocator);

    hResult = m_pd3dDevice->CreateCommandList(
        0, D3D12_COMMAND_LIST_TYPE_DIRECT, 
        m_pd3dCommandAllocator, NULL,
        __uuidof(ID3D12GraphicsCommandList),
        (void**)&m_pd3dCommandList);

    hResult = m_pd3dCommandList->Close();
}

void CGameFramework::CreateRenderTargetViews() {
    D3D12_CPU_DESCRIPTOR_HANDLE d3dRtvCPUDescriptorHandle = m_pd3dRtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
    for(UINT i=0; i<m_nSwapChainBuffers; ++i) {
        m_pdxgiSwapChain->GetBuffer(i, __uuidof(ID3D12Resource), (void**)&m_ppd3dSwapChainBackBuffers[i]);
        m_pd3dDevice->CreateRenderTargetView(m_ppd3dSwapChainBackBuffers[i], NULL, d3dRtvCPUDescriptorHandle);
        d3dRtvCPUDescriptorHandle.ptr += m_nRtvDescriptorIncrementSize;
    }
}

void CGameFramework::CreateDepthStencilView() {
    D3D12_RESOURCE_DESC d3dResourceDesc;
    d3dResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    d3dResourceDesc.Alignment = 0;
    d3dResourceDesc.Width = m_nWndClientWidth;
    d3dResourceDesc.Height = m_nWndClientHeight;
    d3dResourceDesc.DepthOrArraySize = 1;
    d3dResourceDesc.MipLevels = 1;
    d3dResourceDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    d3dResourceDesc.SampleDesc.Count = (m_bMsaa4xEnable) ? 4 : 1;
    d3dResourceDesc.SampleDesc.Quality = (m_bMsaa4xEnable) ? (m_nMsaa4xQualityLevels - 1) : 0;
    d3dResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    d3dResourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

    D3D12_HEAP_PROPERTIES d3dHeapProperties;
    ::ZeroMemory(&d3dHeapProperties, sizeof(D3D12_HEAP_PROPERTIES));
    d3dHeapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;
    d3dHeapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    d3dHeapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
    d3dHeapProperties.CreationNodeMask = 1;
    d3dHeapProperties.VisibleNodeMask = 1;

    D3D12_CLEAR_VALUE d3dClearValue;
    d3dClearValue.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    d3dClearValue.DepthStencil.Depth = 1.0f;
    d3dClearValue.DepthStencil.Stencil = 0;

    m_pd3dDevice->CreateCommittedResource(
        &d3dHeapProperties, D3D12_HEAP_FLAG_NONE,
        &d3dResourceDesc, D3D12_RESOURCE_STATE_DEPTH_WRITE, &d3dClearValue,
        __uuidof(ID3D12Resource), (void**)&m_pd3dDepthStencilBuffer);

    D3D12_DEPTH_STENCIL_VIEW_DESC d3dDepthStencilViewDesc;
    ::ZeroMemory(&d3dDepthStencilViewDesc, sizeof(D3D12_DEPTH_STENCIL_VIEW_DESC));
    d3dDepthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    d3dDepthStencilViewDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
    d3dDepthStencilViewDesc.Flags = D3D12_DSV_FLAG_NONE;

    D3D12_CPU_DESCRIPTOR_HANDLE d3dDsvCPUDescriptorHandle =
        m_pd3dDsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
    m_pd3dDevice->CreateDepthStencilView(
        m_pd3dDepthStencilBuffer,
        &d3dDepthStencilViewDesc, 
        d3dDsvCPUDescriptorHandle);
}


void CGameFramework::ChangeSwapChainState() {
    WaitForGpuComplete();

    is_fullscreen = !is_fullscreen;
    m_pdxgiSwapChain->SetFullscreenState(is_fullscreen, NULL);

    int width = m_nWndClientWidth;
    int height = m_nWndClientHeight;
    if(is_fullscreen) {
        width = screen_width;
        height = screen_height;
    }
    m_pScene->setSceneSize(width, height);

    for(int i=0; i<m_nSwapChainBuffers; i++) {
        if(m_ppd3dSwapChainBackBuffers[i]) {
            m_ppd3dSwapChainBackBuffers[i]->Release();
        }
    }

    DXGI_MODE_DESC dxgiTargetParameters;
    dxgiTargetParameters.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    dxgiTargetParameters.Width = width;
    dxgiTargetParameters.Height = height;
    dxgiTargetParameters.RefreshRate.Numerator = 60;
    dxgiTargetParameters.RefreshRate.Denominator = 1;
    dxgiTargetParameters.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
    dxgiTargetParameters.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
    m_pdxgiSwapChain->ResizeTarget(&dxgiTargetParameters);

    for(int i = 0; i < m_nSwapChainBuffers; i++) {
        if(m_ppd3dSwapChainBackBuffers[i]) m_ppd3dSwapChainBackBuffers[i]->Release();
    }

    DXGI_SWAP_CHAIN_DESC dxgiSwapChainDesc;
    m_pdxgiSwapChain->GetDesc(&dxgiSwapChainDesc);
    m_pdxgiSwapChain->ResizeBuffers(
        m_nSwapChainBuffers, width, height,
        dxgiSwapChainDesc.BufferDesc.Format, dxgiSwapChainDesc.Flags);

    m_nSwapChainBufferIndex = m_pdxgiSwapChain->GetCurrentBackBufferIndex();

    CreateRenderTargetViews();
    CreateDepthStencilView();
}


void CGameFramework::BuildObjects() {
    m_pd3dCommandList->Reset(m_pd3dCommandAllocator, NULL);

    //m_pScene = new LobyScene { };
    m_pScene = new GameScene { };
    m_pScene->BuildObjects(m_pd3dDevice, m_pd3dCommandList);

    m_pd3dCommandList->Close();
    ID3D12CommandList* ppd3dCommandLists[] = { m_pd3dCommandList };
    m_pd3dCommandQueue->ExecuteCommandLists(1, ppd3dCommandLists);

    WaitForGpuComplete();

    if(m_pScene) m_pScene->ReleaseUploadBuffers();

    m_GameTimer.Reset();
}

void CGameFramework::ReleaseObjects() {
    if(m_pScene) {
        m_pScene->ReleaseObjects();
        delete m_pScene;
    }
}

void CGameFramework::AnimateObjects() {
    float fTimeElapsed = m_GameTimer.GetTimeElapsed();

    if(m_pScene) {
        m_pScene->AnimateObjects(fTimeElapsed);
    }
}

void CGameFramework::FrameAdvance() {
    m_GameTimer.Tick(0.0f);

    ProcessInput();

    AnimateObjects();

    HRESULT hResult = m_pd3dCommandAllocator->Reset();
    hResult = m_pd3dCommandList->Reset(m_pd3dCommandAllocator, NULL);

    D3D12_RESOURCE_BARRIER d3dResourceBarrier;
    ::ZeroMemory(&d3dResourceBarrier, sizeof(D3D12_RESOURCE_BARRIER));
    d3dResourceBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    d3dResourceBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    d3dResourceBarrier.Transition.pResource = m_ppd3dSwapChainBackBuffers[m_nSwapChainBufferIndex];
    d3dResourceBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
    d3dResourceBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
    d3dResourceBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    m_pd3dCommandList->ResourceBarrier(1, &d3dResourceBarrier);

    D3D12_CPU_DESCRIPTOR_HANDLE d3dRtvCPUDescriptorHandle =
        m_pd3dRtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
    d3dRtvCPUDescriptorHandle.ptr += (m_nSwapChainBufferIndex * m_nRtvDescriptorIncrementSize);

    m_pd3dCommandList->ClearRenderTargetView(
        d3dRtvCPUDescriptorHandle, (const FLOAT*)&m_pScene->bg_color, 0, NULL);

    D3D12_CPU_DESCRIPTOR_HANDLE d3dDsvCPUDescriptorHandle =
        m_pd3dDsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();

    m_pd3dCommandList->ClearDepthStencilView(
        d3dDsvCPUDescriptorHandle,
        D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 
        1.0f, 0, 0, NULL);

    m_pd3dCommandList->OMSetRenderTargets(
        1, &d3dRtvCPUDescriptorHandle, TRUE,
        &d3dDsvCPUDescriptorHandle);

    if(m_pScene) {
        m_pScene->Render(m_pd3dCommandList);
    }

    d3dResourceBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
    d3dResourceBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
    d3dResourceBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    m_pd3dCommandList->ResourceBarrier(1, &d3dResourceBarrier);

    hResult = m_pd3dCommandList->Close();

    ID3D12CommandList* ppd3dCommandLists[] = { m_pd3dCommandList };
    m_pd3dCommandQueue->ExecuteCommandLists(1, ppd3dCommandLists);

    WaitForGpuComplete();

    m_pdxgiSwapChain->Present(0, 0);

    MoveToNextFrame();

    m_GameTimer.GetFrameRate(m_pszFrameRate + 6, 20);

    ::SetWindowText(m_hWnd, m_pszFrameRate);
}

void CGameFramework::MoveToNextFrame() {
    m_nSwapChainBufferIndex = m_pdxgiSwapChain->GetCurrentBackBufferIndex();

    UINT64 nFenceValue = ++m_nFenceValues[m_nSwapChainBufferIndex];
    HRESULT hResult = m_pd3dCommandQueue->Signal(m_pd3dFence, nFenceValue);

    if(m_pd3dFence->GetCompletedValue() < nFenceValue) {
        hResult = m_pd3dFence->SetEventOnCompletion(nFenceValue, m_hFenceEvent);
        ::WaitForSingleObject(m_hFenceEvent, INFINITE);
    }
}

void CGameFramework::WaitForGpuComplete() {
    const UINT64 nFenceValue = ++m_nFenceValues[m_nSwapChainBufferIndex];
    HRESULT hResult = m_pd3dCommandQueue->Signal(m_pd3dFence, nFenceValue);

    if(m_pd3dFence->GetCompletedValue() < nFenceValue) {
        hResult = m_pd3dFence->SetEventOnCompletion(nFenceValue, m_hFenceEvent);
        ::WaitForSingleObject(m_hFenceEvent, INFINITE);
    }
}


void CGameFramework::ProcessInput() {
    static UCHAR pKeysBuffer[256];
    if(GetKeyboardState(pKeysBuffer)) {
        m_pScene->ProcessInput(pKeysBuffer);
    }
}

void CGameFramework::OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam) {
}

void CGameFramework::OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam) {
}

LRESULT CGameFramework::OnProcessingWindowMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam) {
    switch(nMessageID) {
        case WM_ACTIVATE: {
            if(LOWORD(wParam) == WA_INACTIVE)
                m_GameTimer.Stop();
            else
                m_GameTimer.Start();
            break;
        }
        case WM_SIZE: {
            m_nWndClientWidth = LOWORD(lParam);
            m_nWndClientHeight = HIWORD(lParam);
            break;
        }

        case WM_LBUTTONDOWN:
        case WM_LBUTTONUP:
        case WM_RBUTTONDOWN:
        case WM_RBUTTONUP:
        case WM_MOUSEMOVE: {
            m_pScene->OnProcessingMouseMessage(hWnd, nMessageID, wParam, lParam);
            break;
        }

        case WM_KEYDOWN:
            switch(wParam) {
                case VK_F9:
                    ChangeSwapChainState();
                    break;
            }
        case WM_KEYUP: {
            m_pScene->OnProcessingKeyboardMessage(hWnd, nMessageID, wParam, lParam);
            break;
        }

        case QUIT_GAME_SCENE: {
            ::PostQuitMessage(0);
            break;
        }

        case WM_DESTROY:
            PostQuitMessage(0);
            break;
        default:
            return DefWindowProc(hWnd, nMessageID, wParam, lParam);
    }

    return 0;
}


void CGameFramework::adaptSceneSize() {
    int width = m_nWndClientWidth;
    int height = m_nWndClientHeight;
    if(is_fullscreen) {
        width = screen_width;
        height = screen_height;
    }
    m_pScene->setSceneSize(width, height);
}
