#pragma once

#include "stdafx.h"

#include <vector>


class CMesh {
private:
    int m_nReferences = 0;

protected:
    D3D12_PRIMITIVE_TOPOLOGY m_d3dPrimitiveTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
    UINT m_nSlot = 0;
    UINT m_nVertices = 0;
    UINT m_nOffset = 0;

    UINT m_nType = 0;

    BoundingBox bounding_box;

public:
    CMesh() { }
    virtual ~CMesh() { }

public:
    void AddRef() { m_nReferences++; }
    void Release() { if(--m_nReferences <= 0) delete this; }

    virtual void ReleaseUploadBuffers() { }

    UINT GetType() { return m_nType; }
    virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList) { }
    virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, int nSubSet) { }

    BoundingBox GetBoundingBox() { return bounding_box; }

    void setPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY primitive_topology) { m_d3dPrimitiveTopology = primitive_topology; }
};





















#define VERTEXT_POSITION			0x01
#define VERTEXT_COLOR				0x02
#define VERTEXT_NORMAL				0x04






class CMeshLoadInfo {
public:
    char m_pstrMeshName[256] = { 0 };

    UINT m_nType = 0x00;

    XMFLOAT3 m_xmf3AABBCenter = XMFLOAT3 { 0.0f, 0.0f, 0.0f };
    XMFLOAT3 m_xmf3AABBExtents = XMFLOAT3 { 0.0f, 0.0f, 0.0f };

    int m_nVertices = 0;
    XMFLOAT3* m_pxmf3Positions = nullptr;
    XMFLOAT4* m_pxmf4Colors = nullptr;
    XMFLOAT3* m_pxmf3Normals = nullptr;

    int m_nIndices = 0;
    UINT* m_pnIndices = nullptr;

    int m_nSubMeshes = 0;
    int* m_pnSubSetIndices = nullptr;
    UINT** m_ppnSubSetIndices = nullptr;

public:
    CMeshLoadInfo() { }
    ~CMeshLoadInfo();

public:
    static CMeshLoadInfo CubeInfo(float fWidth = 2.0f, float fHeight = 2.0f, float fDepth = 2.0f);
};




















class CMeshFromFile : public CMesh {
protected:
    ID3D12Resource* m_pd3dPositionBuffer = NULL;
    ID3D12Resource* m_pd3dPositionUploadBuffer = NULL;
    D3D12_VERTEX_BUFFER_VIEW		m_d3dPositionBufferView;

    int m_nSubMeshes = 0;
    int* m_pnSubSetIndices = NULL;

    ID3D12Resource** m_ppd3dSubSetIndexBuffers = NULL;
    ID3D12Resource** m_ppd3dSubSetIndexUploadBuffers = NULL;
    D3D12_INDEX_BUFFER_VIEW* m_pd3dSubSetIndexBufferViews = NULL;

public:
    CMeshFromFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, CMeshLoadInfo* pMeshInfo);
    virtual ~CMeshFromFile();

public:
    virtual void ReleaseUploadBuffers();

    virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, int nSubSet);
};





















class CMeshIlluminatedFromFile : public CMeshFromFile {
public:
    CMeshIlluminatedFromFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, CMeshLoadInfo* pMeshInfo);
    virtual ~CMeshIlluminatedFromFile();

    virtual void ReleaseUploadBuffers();

protected:
    ID3D12Resource* m_pd3dNormalBuffer = NULL;
    ID3D12Resource* m_pd3dNormalUploadBuffer = NULL;
    D3D12_VERTEX_BUFFER_VIEW		m_d3dNormalBufferView;

public:
    virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, int nSubSet);
};














class CMeshIlluminated : public CMesh {

};

