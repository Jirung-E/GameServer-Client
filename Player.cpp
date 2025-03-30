//-----------------------------------------------------------------------------
// File: CPlayer.cpp
//-----------------------------------------------------------------------------

#include "stdafx.h"
#include "Player.h"
#include "Shader.h"

#include <sstream>



CPlayer::CPlayer() : CGameObject { },
    m_xmf3Position { 0.0f, 0.0f, 0.0f }
{
    OnInitialize();

    MATERIALLOADINFO material_info { };
    m_nMaterials = 1;
    m_ppMaterials = new CMaterial*[1];
    m_ppMaterials[0] = NULL;

    CMaterial* pMaterial = new CMaterial { };
    CMaterialColors* pMaterialColors = new CMaterialColors { &material_info };
    pMaterial->SetMaterialColors(pMaterialColors);
    pMaterial->SetIlluminatedShader();

    SetMaterial(0, pMaterial);
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


CMesh* CPlayer::LoadMeshFromFile(
    ID3D12Device* pd3dDevice, 
    ID3D12GraphicsCommandList* pd3dCommandList, 
    const char* pstrFileName,
    float size
) {
    CMeshLoadInfo* pMeshInfo = new CMeshLoadInfo { };
    pMeshInfo->m_nType |= VERTEXT_POSITION;
    pMeshInfo->m_nType |= VERTEXT_NORMAL;
    //pMeshInfo->m_nType |= VERTEXT_COLOR;

    vector<XMFLOAT3> vertices;
    vector<UINT> indices;

    {
        ifstream obj { "Model/king.obj" };
        if(obj.is_open()) {
            string line;
            while(getline(obj, line)) {
                if(line[0] == 'v') {
                    istringstream iss { line };
                    char c;
                    float x, y, z;
                    iss >> c >> x >> y >> z;
                    vertices.push_back({ x * size, y * size, -z * size });
                }
                else if(line[0] == 'f') {
                    istringstream iss { line };
                    char c;
                    UINT i1, i2, i3;
                    iss >> c >> i1 >> i2 >> i3;
                    indices.push_back(i3 - 1);
                    indices.push_back(i2 - 1);
                    indices.push_back(i1 - 1);
                }
            }
        }
    }

    // 프리미티브별로 노말을 하나씩 가져야 하는데, 이게 지금 안됨.
    // 그래서 프리미티브별로 정점을 가지도록 정점을 복사해야함
    pMeshInfo->m_nVertices = indices.size();
    pMeshInfo->m_pxmf3Positions = new XMFLOAT3[pMeshInfo->m_nVertices];

    pMeshInfo->m_nIndices = pMeshInfo->m_nVertices;
    pMeshInfo->m_pnIndices = new UINT[pMeshInfo->m_nIndices];

    pMeshInfo->m_pxmf3Normals = new XMFLOAT3[pMeshInfo->m_nVertices];

    int fragment_count = pMeshInfo->m_nVertices / 3;
    for(int i=0; i<fragment_count; ++i) {
        XMFLOAT3 v1 = vertices[indices[i*3+0]];
        XMFLOAT3 v2 = vertices[indices[i*3+1]];
        XMFLOAT3 v3 = vertices[indices[i*3+2]];

        XMFLOAT3 e1 = Vector3::Subtract(v2, v1);
        XMFLOAT3 e2 = Vector3::Subtract(v3, v1);
        XMFLOAT3 normal = Vector3::CrossProduct(e1, e2);
        normal = Vector3::Normalize(normal);

        pMeshInfo->m_pxmf3Positions[i*3+0] = v1;
        pMeshInfo->m_pxmf3Positions[i*3+1] = v2;
        pMeshInfo->m_pxmf3Positions[i*3+2] = v3;

        pMeshInfo->m_pxmf3Normals[i*3+0] = normal;
        pMeshInfo->m_pxmf3Normals[i*3+1] = normal;
        pMeshInfo->m_pxmf3Normals[i*3+2] = normal;

        pMeshInfo->m_pnIndices[i*3+0] = i*3+0;
        pMeshInfo->m_pnIndices[i*3+1] = i*3+1;
        pMeshInfo->m_pnIndices[i*3+2] = i*3+2;
    }

    CMesh* mesh = new CMeshIlluminatedFromFile { pd3dDevice, pd3dCommandList, pMeshInfo };
    delete pMeshInfo;

    return mesh;
}
