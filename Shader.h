#pragma once

#include "GameObject.h"
#include "Camera.h"


class CShader {
private:
    int m_nReferences = 0;

protected:
    ID3DBlob* m_pd3dVertexShaderBlob = nullptr;
    ID3DBlob* m_pd3dPixelShaderBlob = nullptr;

    int m_nPipelineStates = 0;
    ID3D12PipelineState** m_ppd3dPipelineStates = nullptr;

    D3D12_GRAPHICS_PIPELINE_STATE_DESC	m_d3dPipelineStateDesc;

public:
    CShader();
    virtual ~CShader();

public:
    void AddRef() { m_nReferences++; }
    void Release() { if(--m_nReferences <= 0) delete this; }

    virtual D3D12_INPUT_LAYOUT_DESC CreateInputLayout();
    virtual D3D12_RASTERIZER_DESC CreateRasterizerState();
    virtual D3D12_BLEND_DESC CreateBlendState();
    virtual D3D12_DEPTH_STENCIL_DESC CreateDepthStencilState();

    virtual D3D12_SHADER_BYTECODE CreateVertexShader();
    virtual D3D12_SHADER_BYTECODE CreatePixelShader();

    D3D12_SHADER_BYTECODE CompileShaderFromFile(
        const WCHAR* pszFileName, 
        LPCSTR pszShaderName,
        LPCSTR pszShaderProfile, 
        ID3DBlob** ppd3dShaderBlob
    );

    virtual void CreateShader(
        ID3D12Device* pd3dDevice,
        ID3D12GraphicsCommandList* pd3dCommandList,
        ID3D12RootSignature* pd3dGraphicsRootSignature
    );

    virtual void CreateShaderVariables(
        ID3D12Device* pd3dDevice, 
        ID3D12GraphicsCommandList* pd3dCommandList
    );
    virtual void UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList);
    virtual void ReleaseShaderVariables();
    virtual void UpdateShaderVariable(
        ID3D12GraphicsCommandList* pd3dCommandList, 
        XMFLOAT4X4* pxmf4x4World
    );
    virtual void UpdateShaderVariable(
        ID3D12GraphicsCommandList* pd3dCommandList,
        CMaterialColors* pMaterialColors
    );

    virtual void OnPrepareRender(ID3D12GraphicsCommandList* pd3dCommandList, int nPipelineState=0);
    virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera, int nPipelineState=0);
};


class CIlluminatedShader : public CShader {
public:
    CIlluminatedShader();
    virtual ~CIlluminatedShader();

public:
    virtual D3D12_INPUT_LAYOUT_DESC CreateInputLayout();

    virtual D3D12_SHADER_BYTECODE CreateVertexShader();
    virtual D3D12_SHADER_BYTECODE CreatePixelShader();

    virtual void CreateShader(
        ID3D12Device* pd3dDevice, 
        ID3D12GraphicsCommandList* pd3dCommandList, 
        ID3D12RootSignature* pd3dGraphicsRootSignature
    );

    virtual void Render(
        ID3D12GraphicsCommandList* pd3dCommandList, 
        CCamera* pCamera, 
        int nPipelineState = 0
    );
};
