#pragma once

#include "stdafx.h"


#define MATERIAL_ALBEDO_MAP			0x01
#define MATERIAL_SPECULAR_MAP		0x02
#define MATERIAL_NORMAL_MAP			0x04
#define MATERIAL_METALLIC_MAP		0x08
#define MATERIAL_EMISSION_MAP		0x10
#define MATERIAL_DETAIL_ALBEDO_MAP	0x20
#define MATERIAL_DETAIL_NORMAL_MAP	0x40


class CShader;


struct MATERIALLOADINFO {
    XMFLOAT4 m_xmf4AlbedoColor = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
    XMFLOAT4 m_xmf4EmissiveColor = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
    XMFLOAT4 m_xmf4SpecularColor = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);

    float m_fGlossiness = 0.0f;
    float m_fSmoothness = 0.0f;
    float m_fSpecularHighlight = 0.0f;
    float m_fMetallic = 0.0f;
    float m_fGlossyReflection = 0.0f;

    UINT m_nType = 0x00;

    //char							m_pstrAlbedoMapName[64] = { '\0' };
    //char							m_pstrSpecularMapName[64] = { '\0' };
    //char							m_pstrMetallicMapName[64] = { '\0' };
    //char							m_pstrNormalMapName[64] = { '\0' };
    //char							m_pstrEmissionMapName[64] = { '\0' };
    //char							m_pstrDetailAlbedoMapName[64] = { '\0' };
    //char							m_pstrDetailNormalMapName[64] = { '\0' };
};


struct MATERIALSLOADINFO {
    int	m_nMaterials = 0;
    MATERIALLOADINFO* m_pMaterials = NULL;
};


class CMaterialColors {
private:
    int	m_nReferences = 0;

public:
    XMFLOAT4 m_xmf4Ambient = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
    XMFLOAT4 m_xmf4Diffuse = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
    XMFLOAT4 m_xmf4Specular = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f); //(r,g,b,a=power)
    XMFLOAT4 m_xmf4Emissive = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);

public:
    CMaterialColors() { }
    CMaterialColors(MATERIALLOADINFO* pMaterialInfo);
    virtual ~CMaterialColors() { }

public:
    void AddRef() { m_nReferences++; }
    void Release() { if(--m_nReferences <= 0) delete this; }
};


class CMaterial {
private:
    int	m_nReferences = 0;

protected:
    static CShader* m_pIlluminatedShader;

public:
    CShader* m_pShader = NULL;

    CMaterialColors* m_pMaterialColors = NULL;

public:
    CMaterial();
    virtual ~CMaterial();

public:
    void AddRef() { m_nReferences++; }
    void Release() { if(--m_nReferences <= 0) delete this; }

    void SetMaterialColors(CMaterialColors* pMaterialColors);
    void SetShader(CShader* pShader);
    void SetIlluminatedShader() { SetShader(m_pIlluminatedShader); }

    void UpdateShaderVariable(ID3D12GraphicsCommandList* pd3dCommandList);

    static void PrepareShaders(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature);
};





const MATERIALLOADINFO BASIC = {
    XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
    XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f),
    XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f),
    0.8f, 0.0f, 0.0f, 0.0f, 0x00
};

const MATERIALLOADINFO METAL = {
    XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
    XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f),
    XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
    1.0f, 0.0f, 0.0f, 0.0f, 0x00
};

const MATERIALLOADINFO MATTE = {
    XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
    XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f),
    XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f),
    0.2f, 0.0f, 0.0f, 0.0f, 0x00
};