#include "Shader.h"


CShader::CShader() {

}

CShader::~CShader() {
    ReleaseShaderVariables();

    if(m_ppd3dPipelineStates) {
        for(int i=0; i<m_nPipelineStates; i++) {
            if(m_ppd3dPipelineStates[i]) {
                m_ppd3dPipelineStates[i]->Release();
            }
        }
        delete[] m_ppd3dPipelineStates;
    }
}


D3D12_INPUT_LAYOUT_DESC CShader::CreateInputLayout() {
    D3D12_INPUT_LAYOUT_DESC d3dInputLayoutDesc;
    d3dInputLayoutDesc.pInputElementDescs = NULL;
    d3dInputLayoutDesc.NumElements = 0;

    return d3dInputLayoutDesc;
}

D3D12_RASTERIZER_DESC CShader::CreateRasterizerState() {
    D3D12_RASTERIZER_DESC d3dRasterizerDesc;
    ::ZeroMemory(&d3dRasterizerDesc, sizeof(D3D12_RASTERIZER_DESC));
    d3dRasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;
    d3dRasterizerDesc.CullMode = D3D12_CULL_MODE_BACK;
    d3dRasterizerDesc.FrontCounterClockwise = FALSE;
    d3dRasterizerDesc.DepthBias = 0;
    d3dRasterizerDesc.DepthBiasClamp = 0.0f;
    d3dRasterizerDesc.SlopeScaledDepthBias = 0.0f;
    d3dRasterizerDesc.DepthClipEnable = TRUE;
    d3dRasterizerDesc.MultisampleEnable = FALSE;
    d3dRasterizerDesc.AntialiasedLineEnable = FALSE;
    d3dRasterizerDesc.ForcedSampleCount = 0;
    d3dRasterizerDesc.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;
    return d3dRasterizerDesc;
}
 
D3D12_BLEND_DESC CShader::CreateBlendState() {
    D3D12_BLEND_DESC d3dBlendDesc;
    ::ZeroMemory(&d3dBlendDesc, sizeof(D3D12_BLEND_DESC));

    d3dBlendDesc.AlphaToCoverageEnable = FALSE;
    d3dBlendDesc.IndependentBlendEnable = FALSE;
    d3dBlendDesc.RenderTarget[0].BlendEnable = true;
    d3dBlendDesc.RenderTarget[0].LogicOpEnable = FALSE;
    d3dBlendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
    d3dBlendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
    d3dBlendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
    d3dBlendDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_SRC_ALPHA;
    d3dBlendDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_INV_SRC_ALPHA;
    d3dBlendDesc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
    d3dBlendDesc.RenderTarget[0].LogicOp = D3D12_LOGIC_OP_NOOP;
    d3dBlendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

    return d3dBlendDesc;
}

D3D12_DEPTH_STENCIL_DESC CShader::CreateDepthStencilState() {
    D3D12_DEPTH_STENCIL_DESC d3dDepthStencilDesc;
    ::ZeroMemory(&d3dDepthStencilDesc, sizeof(D3D12_DEPTH_STENCIL_DESC));

    d3dDepthStencilDesc.DepthEnable = TRUE;
    d3dDepthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
    d3dDepthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
    d3dDepthStencilDesc.StencilEnable = FALSE;
    d3dDepthStencilDesc.StencilReadMask = 0x00;
    d3dDepthStencilDesc.StencilWriteMask = 0x00;
    d3dDepthStencilDesc.FrontFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
    d3dDepthStencilDesc.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
    d3dDepthStencilDesc.FrontFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
    d3dDepthStencilDesc.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_NEVER;
    d3dDepthStencilDesc.BackFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
    d3dDepthStencilDesc.BackFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
    d3dDepthStencilDesc.BackFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
    d3dDepthStencilDesc.BackFace.StencilFunc = D3D12_COMPARISON_FUNC_NEVER;

    return d3dDepthStencilDesc;
}


D3D12_SHADER_BYTECODE CShader::CreateVertexShader() {
    D3D12_SHADER_BYTECODE d3dShaderByteCode;
    d3dShaderByteCode.BytecodeLength = 0;
    d3dShaderByteCode.pShaderBytecode = NULL;

    return d3dShaderByteCode;
}

D3D12_SHADER_BYTECODE CShader::CreatePixelShader() {
    D3D12_SHADER_BYTECODE d3dShaderByteCode;
    d3dShaderByteCode.BytecodeLength = 0;
    d3dShaderByteCode.pShaderBytecode = NULL;

    return d3dShaderByteCode;
}


D3D12_SHADER_BYTECODE CShader::CompileShaderFromFile(
    const WCHAR* pszFileName, 
    LPCSTR pszShaderName, 
    LPCSTR pszShaderProfile, 
    ID3DBlob** ppd3dShaderBlob
) {
    UINT nCompileFlags = 0;
#if defined(_DEBUG)
    nCompileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

    ID3DBlob* pd3dErrorBlob = NULL;
    HRESULT hResult = ::D3DCompileFromFile(
        pszFileName, NULL, D3D_COMPILE_STANDARD_FILE_INCLUDE, 
        pszShaderName, pszShaderProfile, 
        nCompileFlags, 0, ppd3dShaderBlob, &pd3dErrorBlob
    );
    char* pErrorString = NULL;
    if(pd3dErrorBlob) pErrorString = (char*)pd3dErrorBlob->GetBufferPointer();
    
    D3D12_SHADER_BYTECODE d3dShaderByteCode;
    d3dShaderByteCode.BytecodeLength = (*ppd3dShaderBlob)->GetBufferSize();
    d3dShaderByteCode.pShaderBytecode = (*ppd3dShaderBlob)->GetBufferPointer();

    return d3dShaderByteCode;
}


void CShader::CreateShader(
    ID3D12Device* pd3dDevice,
    ID3D12GraphicsCommandList* pd3dCommandList,
    ID3D12RootSignature* pd3dGraphicsRootSignature
) {
    ::ZeroMemory(&m_d3dPipelineStateDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
    m_d3dPipelineStateDesc.pRootSignature = pd3dGraphicsRootSignature;
    m_d3dPipelineStateDesc.VS = CreateVertexShader();
    m_d3dPipelineStateDesc.PS = CreatePixelShader();
    m_d3dPipelineStateDesc.RasterizerState = CreateRasterizerState();
    m_d3dPipelineStateDesc.BlendState = CreateBlendState();
    m_d3dPipelineStateDesc.DepthStencilState = CreateDepthStencilState();
    m_d3dPipelineStateDesc.InputLayout = CreateInputLayout();
    m_d3dPipelineStateDesc.SampleMask = UINT_MAX;
    m_d3dPipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    m_d3dPipelineStateDesc.NumRenderTargets = 1;
    m_d3dPipelineStateDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
    m_d3dPipelineStateDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
    m_d3dPipelineStateDesc.SampleDesc.Count = 1;
    m_d3dPipelineStateDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;

    HRESULT hResult = pd3dDevice->CreateGraphicsPipelineState(
        &m_d3dPipelineStateDesc,
        __uuidof(ID3D12PipelineState),
        (void**)&m_ppd3dPipelineStates[0]
    );
}


void CShader::CreateShaderVariables(
    ID3D12Device* pd3dDevice,
    ID3D12GraphicsCommandList* pd3dCommandList
) {

}

void CShader::UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList) {

}

void CShader::ReleaseShaderVariables() {

}

void CShader::UpdateShaderVariable(
    ID3D12GraphicsCommandList* pd3dCommandList,
    XMFLOAT4X4* pxmf4x4World
) {

}

void CShader::UpdateShaderVariable(ID3D12GraphicsCommandList* pd3dCommandList, CMaterialColors* pMaterialColors) {

}


void CShader::OnPrepareRender(ID3D12GraphicsCommandList* pd3dCommandList, int nPipelineState) {
    if(m_ppd3dPipelineStates) pd3dCommandList->SetPipelineState(m_ppd3dPipelineStates[nPipelineState]);
}

void CShader::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera, int nPipelineState) {
    OnPrepareRender(pd3dCommandList, nPipelineState);
}



////////////////////////////////////////////////////////////////////////////////////////////////////



CIlluminatedShader::CIlluminatedShader() {

}

CIlluminatedShader::~CIlluminatedShader() {

}


D3D12_INPUT_LAYOUT_DESC CIlluminatedShader::CreateInputLayout() {
    UINT nInputElementDescs = 2;
    D3D12_INPUT_ELEMENT_DESC* pd3dInputElementDescs = new D3D12_INPUT_ELEMENT_DESC[nInputElementDescs];

    pd3dInputElementDescs[0] = {
        "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,
        D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0
    };
    pd3dInputElementDescs[1] = {
        "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 1, 0,
        D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0
    };

    D3D12_INPUT_LAYOUT_DESC d3dInputLayoutDesc;
    d3dInputLayoutDesc.pInputElementDescs = pd3dInputElementDescs;
    d3dInputLayoutDesc.NumElements = nInputElementDescs;

    return d3dInputLayoutDesc;
}

D3D12_SHADER_BYTECODE CIlluminatedShader::CreateVertexShader() {
    return CShader::CompileShaderFromFile(L"Shaders.hlsl", "VSLighting", "vs_5_1", &m_pd3dVertexShaderBlob);
}

D3D12_SHADER_BYTECODE CIlluminatedShader::CreatePixelShader() {
    return CShader::CompileShaderFromFile(L"Shaders.hlsl", "PSLighting", "ps_5_1", &m_pd3dPixelShaderBlob);
}

void CIlluminatedShader::CreateShader(
    ID3D12Device* pd3dDevice,
    ID3D12GraphicsCommandList* pd3dCommandList,
    ID3D12RootSignature* pd3dGraphicsRootSignature
) {
    m_nPipelineStates = 2;
    m_ppd3dPipelineStates = new ID3D12PipelineState*[m_nPipelineStates];

    CShader::CreateShader(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature);

    m_d3dPipelineStateDesc.RasterizerState.FillMode = D3D12_FILL_MODE_WIREFRAME;

    HRESULT hResult = pd3dDevice->CreateGraphicsPipelineState(
        &m_d3dPipelineStateDesc, 
        __uuidof(ID3D12PipelineState), 
        (void**)&m_ppd3dPipelineStates[1]
    );

    if(m_pd3dVertexShaderBlob) m_pd3dVertexShaderBlob->Release();
    if(m_pd3dPixelShaderBlob) m_pd3dPixelShaderBlob->Release();

    if(m_d3dPipelineStateDesc.InputLayout.pInputElementDescs) {
        delete[] m_d3dPipelineStateDesc.InputLayout.pInputElementDescs;
    }
}


void CIlluminatedShader::Render(
    ID3D12GraphicsCommandList* pd3dCommandList, 
    CCamera* pCamera, 
    int nPipelineState
) {
    OnPrepareRender(pd3dCommandList, nPipelineState);
}
