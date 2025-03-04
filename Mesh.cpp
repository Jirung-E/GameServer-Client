#include "Mesh.h"

#include <vector>
#include <fstream>
#include <sstream>

using namespace std;


CMeshLoadInfo::~CMeshLoadInfo() {
	if(m_pxmf3Positions) delete[] m_pxmf3Positions;
	if(m_pxmf4Colors) delete[] m_pxmf4Colors;
	if(m_pxmf3Normals) delete[] m_pxmf3Normals;

	if(m_pnIndices) delete[] m_pnIndices;

	if(m_pnSubSetIndices) delete[] m_pnSubSetIndices;

	for(int i = 0; i < m_nSubMeshes; i++) if(m_ppnSubSetIndices[i]) delete[] m_ppnSubSetIndices[i];
	if(m_ppnSubSetIndices) delete[] m_ppnSubSetIndices;
}


CMeshLoadInfo CMeshLoadInfo::CubeInfo(float fWidth, float fHeight, float fDepth) {
	CMeshLoadInfo pMeshInfo { };
	pMeshInfo.m_nType |= VERTEXT_POSITION;
	pMeshInfo.m_nType |= VERTEXT_NORMAL;

	vector<XMFLOAT3> vertices {
		XMFLOAT3 { -fWidth, -fHeight, -fDepth },
        XMFLOAT3 { fWidth, -fHeight, -fDepth },
        XMFLOAT3 { fWidth, fHeight, -fDepth },
        XMFLOAT3 { -fWidth, fHeight, -fDepth },
        XMFLOAT3 { -fWidth, -fHeight, fDepth },
        XMFLOAT3 { fWidth, -fHeight, fDepth },
        XMFLOAT3 { fWidth, fHeight, fDepth },
        XMFLOAT3 { -fWidth, fHeight, fDepth },
	};
	vector<UINT> indices {
		0, 1, 2, 0, 2, 3,
        1, 5, 6, 1, 6, 2,
        5, 4, 7, 5, 7, 6,
        4, 0, 3, 4, 3, 7,
        3, 2, 6, 3, 6, 7,
        4, 5, 1, 4, 1, 0,
	};
	reverse(indices.begin(), indices.end());

	pMeshInfo.m_nVertices = indices.size();
	pMeshInfo.m_pxmf3Positions = new XMFLOAT3[pMeshInfo.m_nVertices];

	pMeshInfo.m_nIndices = pMeshInfo.m_nVertices;
	pMeshInfo.m_pnIndices = new UINT[pMeshInfo.m_nIndices];

	pMeshInfo.m_pxmf3Normals = new XMFLOAT3[pMeshInfo.m_nVertices];

	int fragment_count = pMeshInfo.m_nVertices / 3;
	for(int i=0; i<fragment_count; ++i) {
		XMFLOAT3 v1 = vertices[indices[i*3+0]];
		XMFLOAT3 v2 = vertices[indices[i*3+1]];
		XMFLOAT3 v3 = vertices[indices[i*3+2]];

		XMFLOAT3 e1 = Vector3::Subtract(v2, v1);
		XMFLOAT3 e2 = Vector3::Subtract(v3, v1);
		XMFLOAT3 normal = Vector3::CrossProduct(e1, e2);
		normal = Vector3::Normalize(normal);

		pMeshInfo.m_pxmf3Positions[i*3+0] = v1;
		pMeshInfo.m_pxmf3Positions[i*3+1] = v2;
		pMeshInfo.m_pxmf3Positions[i*3+2] = v3;

		pMeshInfo.m_pxmf3Normals[i*3+0] = normal;
		pMeshInfo.m_pxmf3Normals[i*3+1] = normal;
		pMeshInfo.m_pxmf3Normals[i*3+2] = normal;

		pMeshInfo.m_pnIndices[i*3+0] = i*3+0;
		pMeshInfo.m_pnIndices[i*3+1] = i*3+1;
		pMeshInfo.m_pnIndices[i*3+2] = i*3+2;
	}

	return pMeshInfo;
}





















CMeshFromFile::CMeshFromFile(
	ID3D12Device* pd3dDevice, 
	ID3D12GraphicsCommandList* pd3dCommandList, 
	CMeshLoadInfo* pMeshInfo
) {
	m_nVertices = pMeshInfo->m_nVertices;
	m_nType = pMeshInfo->m_nType;

	m_pd3dPositionBuffer = ::CreateBufferResource(
		pd3dDevice, pd3dCommandList, 
		pMeshInfo->m_pxmf3Positions, 
		sizeof(XMFLOAT3) * m_nVertices, 
		D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, 
		&m_pd3dPositionUploadBuffer
	);

	{
		float most_left_x = FLT_MAX;
		float most_right_x = -FLT_MAX;
		float most_bottom_y = FLT_MAX;
		float most_top_y = -FLT_MAX;
		float most_near_z = FLT_MAX;
		float most_far_z = -FLT_MAX;

		for(int i=0; i<m_nVertices; ++i) {
			const XMFLOAT3& position = pMeshInfo->m_pxmf3Positions[i];
			if(position.x < most_left_x) most_left_x = position.x;
			if(position.x > most_right_x) most_right_x = position.x;
			if(position.y < most_bottom_y) most_bottom_y = position.y;
			if(position.y > most_top_y) most_top_y = position.y;
			if(position.z < most_near_z) most_near_z = position.z;
			if(position.z > most_far_z) most_far_z = position.z;
		}

		bounding_box.Center = XMFLOAT3 {
			(most_left_x + most_right_x) / 2,
			(most_bottom_y + most_top_y) / 2,
			(most_near_z + most_far_z) / 2,
		};
		bounding_box.Extents = XMFLOAT3 {
            (most_right_x - most_left_x) / 2,
            (most_top_y - most_bottom_y) / 2,
            (most_far_z - most_near_z) / 2,
        };
	}

	m_d3dPositionBufferView.BufferLocation = m_pd3dPositionBuffer->GetGPUVirtualAddress();
	m_d3dPositionBufferView.StrideInBytes = sizeof(XMFLOAT3);
	m_d3dPositionBufferView.SizeInBytes = sizeof(XMFLOAT3) * m_nVertices;

	m_nSubMeshes = pMeshInfo->m_nSubMeshes;
	if(m_nSubMeshes > 0) {
		m_ppd3dSubSetIndexBuffers = new ID3D12Resource*[m_nSubMeshes];
		m_ppd3dSubSetIndexUploadBuffers = new ID3D12Resource*[m_nSubMeshes];
		m_pd3dSubSetIndexBufferViews = new D3D12_INDEX_BUFFER_VIEW[m_nSubMeshes];

		m_pnSubSetIndices = new int[m_nSubMeshes];

		for(int i = 0; i < m_nSubMeshes; i++) {
			m_pnSubSetIndices[i] = pMeshInfo->m_pnSubSetIndices[i];
			m_ppd3dSubSetIndexBuffers[i] = ::CreateBufferResource(
				pd3dDevice, pd3dCommandList, 
				pMeshInfo->m_ppnSubSetIndices[i], 
				sizeof(UINT) * m_pnSubSetIndices[i], 
				D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_INDEX_BUFFER, 
				&m_ppd3dSubSetIndexUploadBuffers[i]
			);

			m_pd3dSubSetIndexBufferViews[i].BufferLocation = 
				m_ppd3dSubSetIndexBuffers[i]->GetGPUVirtualAddress();
			m_pd3dSubSetIndexBufferViews[i].Format = DXGI_FORMAT_R32_UINT;
			m_pd3dSubSetIndexBufferViews[i].SizeInBytes = 
				sizeof(UINT) * pMeshInfo->m_pnSubSetIndices[i];
		}
	}
}

CMeshFromFile::~CMeshFromFile() {
	if(m_pd3dPositionBuffer) m_pd3dPositionBuffer->Release();

	if(m_nSubMeshes > 0) {
		for(int i = 0; i < m_nSubMeshes; i++) {
			if(m_ppd3dSubSetIndexBuffers[i]) m_ppd3dSubSetIndexBuffers[i]->Release();
		}
		if(m_ppd3dSubSetIndexBuffers) delete[] m_ppd3dSubSetIndexBuffers;
		if(m_pd3dSubSetIndexBufferViews) delete[] m_pd3dSubSetIndexBufferViews;

		if(m_pnSubSetIndices) delete[] m_pnSubSetIndices;
	}
}


void CMeshFromFile::ReleaseUploadBuffers() {
	CMesh::ReleaseUploadBuffers();

	if(m_pd3dPositionUploadBuffer) m_pd3dPositionUploadBuffer->Release();
	m_pd3dPositionUploadBuffer = NULL;

	if((m_nSubMeshes > 0) && m_ppd3dSubSetIndexUploadBuffers) {
		for(int i = 0; i < m_nSubMeshes; i++) {
			if(m_ppd3dSubSetIndexUploadBuffers[i]) m_ppd3dSubSetIndexUploadBuffers[i]->Release();
		}
		if(m_ppd3dSubSetIndexUploadBuffers) delete[] m_ppd3dSubSetIndexUploadBuffers;
		m_ppd3dSubSetIndexUploadBuffers = NULL;
	}
}

void CMeshFromFile::Render(ID3D12GraphicsCommandList* pd3dCommandList, int nSubSet) {
	pd3dCommandList->IASetPrimitiveTopology(m_d3dPrimitiveTopology);
	pd3dCommandList->IASetVertexBuffers(m_nSlot, 1, &m_d3dPositionBufferView);
	if((m_nSubMeshes > 0) && (nSubSet < m_nSubMeshes)) {
		pd3dCommandList->IASetIndexBuffer(&(m_pd3dSubSetIndexBufferViews[nSubSet]));
		pd3dCommandList->DrawIndexedInstanced(m_pnSubSetIndices[nSubSet], 1, 0, 0, 0);
	}
	else {
		pd3dCommandList->DrawInstanced(m_nVertices, 1, m_nOffset, 0);
	}
}

















CMeshIlluminatedFromFile::CMeshIlluminatedFromFile(
	ID3D12Device* pd3dDevice, 
	ID3D12GraphicsCommandList* pd3dCommandList, 
	CMeshLoadInfo* pMeshInfo
) : 
	CMeshFromFile { pd3dDevice, pd3dCommandList, pMeshInfo }
{
	m_pd3dNormalBuffer = ::CreateBufferResource(
		pd3dDevice, pd3dCommandList, 
		pMeshInfo->m_pxmf3Normals, 
		sizeof(XMFLOAT3) * m_nVertices, 
		D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, 
		&m_pd3dNormalUploadBuffer
	);

	m_d3dNormalBufferView.BufferLocation = m_pd3dNormalBuffer->GetGPUVirtualAddress();
	m_d3dNormalBufferView.StrideInBytes = sizeof(XMFLOAT3);
	m_d3dNormalBufferView.SizeInBytes = sizeof(XMFLOAT3) * m_nVertices;
}

CMeshIlluminatedFromFile::~CMeshIlluminatedFromFile() {
	if(m_pd3dNormalBuffer) m_pd3dNormalBuffer->Release();
}


void CMeshIlluminatedFromFile::ReleaseUploadBuffers() {
	CMeshFromFile::ReleaseUploadBuffers();

	if(m_pd3dNormalUploadBuffer) m_pd3dNormalUploadBuffer->Release();
	m_pd3dNormalUploadBuffer = NULL;
}

void CMeshIlluminatedFromFile::Render(ID3D12GraphicsCommandList* pd3dCommandList, int nSubSet) {
	pd3dCommandList->IASetPrimitiveTopology(m_d3dPrimitiveTopology);
	D3D12_VERTEX_BUFFER_VIEW pVertexBufferViews[2] = { m_d3dPositionBufferView, m_d3dNormalBufferView };
	pd3dCommandList->IASetVertexBuffers(m_nSlot, 2, pVertexBufferViews);
	if((m_nSubMeshes > 0) && (nSubSet < m_nSubMeshes)) {
		pd3dCommandList->IASetIndexBuffer(&(m_pd3dSubSetIndexBufferViews[nSubSet]));
		pd3dCommandList->DrawIndexedInstanced(m_pnSubSetIndices[nSubSet], 1, 0, 0, 0);
	}
	else {
		pd3dCommandList->DrawInstanced(m_nVertices, 1, m_nOffset, 0);
	}
}
