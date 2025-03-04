#include "Mesh.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

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




















HeightMap::HeightMap(const char* file_name, float x_length, float y_length, float z_length):
	x_length { x_length }, 
	y_length { y_length },
	z_length { z_length }
{
	int number_of_channels;

	stbi_set_flip_vertically_on_load(false);

	unsigned char* data = stbi_load(file_name, &width, &length, &number_of_channels, 1);

	pixels.resize(width * length);

	for(int i=0; i<length; ++i) {
        for(int k=0; k<width; ++k) {
            pixels[k + (length-1-i)*width] = data[k + (i*width)];
        }
    }

	stbi_image_free(data);
}

HeightMap::~HeightMap() {

}


#define _WITH_APPROXIMATE_OPPOSITE_CORNER

float HeightMap::getHeight(float x, float z) {
	if(x < -x_length/2.0f || x > x_length/2.0f || z < -z_length/2.0f || z > z_length/2.0f) {
        return 0.0f;
    }

	float fx = (x + x_length/2.0f) / x_length * width;
	float fz = (z + z_length/2.0f) / z_length * length;

	return GetHeight(fx, fz) * y_length / 255.0f;
}

XMFLOAT3 HeightMap::getNormal(float x, float z) {
	if(x < -x_length/2.0f || x > x_length/2.0f || z < -z_length/2.0f || z > z_length/2.0f) {
		return XMFLOAT3 { 0.0f, 1.0f, 0.0f };
	}

	float fx = (x + x_length/2.0f) / x_length * width;
	float fz = (z + z_length/2.0f) / z_length * length;

	return GetHeightMapNormal(fx, fz);
}


float HeightMap::GetHeight(float fx, float fz) {
	if((fx < 0.0f) || (fz < 0.0f) || (fx >= width) || (fz >= length-1)) {
		return 0.0f;
	}

	int x = (int)fx;
	int z = (int)fz;
	float fxPercent = fx - x;
	float fzPercent = fz - z;
	float fBottomLeft = (float)pixels[x + (z*width)];
	float fBottomRight = (float)pixels[(x + 1) + (z*width)];
	float fTopLeft = (float)pixels[x + ((z + 1)*width)];
	float fTopRight = (float)pixels[(x + 1) + ((z + 1)*width)];

#ifdef _WITH_APPROXIMATE_OPPOSITE_CORNER
	bool bRightToLeft = ((z % 2) != 0);
	if(bRightToLeft) {
		if(fzPercent >= fxPercent)
			fBottomRight = fBottomLeft + (fTopRight - fTopLeft);
		else
			fTopLeft = fTopRight + (fBottomLeft - fBottomRight);
	}
	else {
		if(fzPercent < (1.0f - fxPercent))
			fTopRight = fTopLeft + (fBottomRight - fBottomLeft);
		else
			fBottomLeft = fTopLeft + (fBottomRight - fTopRight);
	}
#endif

	float fTopHeight = fTopLeft * (1 - fxPercent) + fTopRight * fxPercent;
	float fBottomHeight = fBottomLeft * (1 - fxPercent) + fBottomRight * fxPercent;
	float fHeight = fBottomHeight * (1 - fzPercent) + fTopHeight * fzPercent;

	return fHeight;
}

XMFLOAT3 HeightMap::GetHeightMapNormal(int x, int z) {
	if((x < 0.0f) || (z < 0.0f) || (x >= width) || (z >= length)) {
		return XMFLOAT3 { 0.0f, 1.0f, 0.0f };
	}

	int nHeightMapIndex = x + (z * width);
	int xHeightMapAdd = (x < (width - 1)) ? 1 : -1;
	int zHeightMapAdd = (z < (width - 1)) ? width : -width;
	
	float y1 = (float)pixels[nHeightMapIndex] * y_length;
	float y2 = (float)pixels[nHeightMapIndex + xHeightMapAdd] * y_length;
	float y3 = (float)pixels[nHeightMapIndex + zHeightMapAdd] * y_length;
	
	XMFLOAT3 xmf3Edge1 = XMFLOAT3(0.0f, y3 - y1, z_length);
	XMFLOAT3 xmf3Edge2 = XMFLOAT3(x_length, y2 - y1, 0.0f);
	XMFLOAT3 xmf3Normal = Vector3::CrossProduct(xmf3Edge1, xmf3Edge2, true);

	return xmf3Normal;
}


CMesh* HeightMap::createMesh(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList) {
	CMeshLoadInfo mesh_info { };
    mesh_info.m_nVertices = width * length;
    mesh_info.m_nType = VERTEXT_POSITION | VERTEXT_NORMAL;

	mesh_info.m_pxmf3Positions = new XMFLOAT3[mesh_info.m_nVertices];
	mesh_info.m_pxmf3Normals = new XMFLOAT3[mesh_info.m_nVertices];

	int cnt = 0;
	for(int z=0; z<length; ++z) {
        for(int x=0; x<width; ++x) {
			mesh_info.m_pxmf3Positions[cnt] = XMFLOAT3 {
				((float(x) - width / 2.0f) / width) * x_length, 
				(float(pixels[cnt]) / 255) * y_length,
				((float(z) - length / 2.0f) / length) * z_length
			};
            mesh_info.m_pxmf3Normals[cnt] = GetHeightMapNormal(x, z);
            ++cnt;
        }
    }

	mesh_info.m_nSubMeshes = 1;
	mesh_info.m_pnSubSetIndices	= new int[mesh_info.m_nSubMeshes];
	mesh_info.m_pnSubSetIndices[0] = ((width * 2)*(length - 1)) + ((length - 1) - 1);
	mesh_info.m_ppnSubSetIndices = new UINT*[mesh_info.m_nSubMeshes];
	mesh_info.m_ppnSubSetIndices[0] = new UINT[mesh_info.m_pnSubSetIndices[0]];
	cnt = 0;
	for(int z = 0; z < length - 1; z++) {
		if((z % 2) == 0) {
			for(int x = 0; x < width; x++) {
				if((x == 0) && (z > 0)) mesh_info.m_ppnSubSetIndices[0][cnt++] = (UINT)(x + (z * width));
				mesh_info.m_ppnSubSetIndices[0][cnt++] = (UINT)(x + (z * width));
				mesh_info.m_ppnSubSetIndices[0][cnt++] = (UINT)((x + (z * width)) + width);
			}
		}
		else {
			for(int x = width - 1; x >= 0; x--) {
				if(x == (width - 1)) mesh_info.m_ppnSubSetIndices[0][cnt++] = (UINT)(x + (z * width));
				mesh_info.m_ppnSubSetIndices[0][cnt++] = (UINT)(x + (z * width));
				mesh_info.m_ppnSubSetIndices[0][cnt++] = (UINT)((x + (z * width)) + width);
			}
		}
	}

	CMesh* mesh = new CMeshIlluminatedFromFile { pd3dDevice, pd3dCommandList, &mesh_info };
	mesh->setPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

    return mesh;
}







//CHeightMapGridMesh::CHeightMapGridMesh(ID3D12Device* pd3dDevice,
//	ID3D12GraphicsCommandList* pd3dCommandList, int xStart, int zStart, int nWidth, int nLength, 
//	XMFLOAT3 xmf3Scale, XMFLOAT4 xmf4Color, void* pContext
//) : CMesh(pd3dDevice, pd3dCommandList) {
//	m_nVertices = nWidth * nLength;
//	m_nStride = sizeof(XMFLOAT3);
//	m_d3dPrimitiveTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
//	m_nWidth = nWidth;
//	m_nLength = nLength;
//	m_xmf3Scale = xmf3Scale;
//	CDiffusedVertex* pVertices = new CDiffusedVertex[m_nVertices];
//	
//	float fHeight = 0.0f, fMinHeight = +FLT_MAX, fMaxHeight = -FLT_MAX;
//	for(int i = 0, z = zStart; z < (zStart + nLength); z++) {
//		for(int x = xStart; x < (xStart + nWidth); x++, i++) {
//			XMFLOAT3 xmf3Position = XMFLOAT3((x*m_xmf3Scale.x), OnGetHeight(x, z, pContext),
//			(z*m_xmf3Scale.z));
//			XMFLOAT4 xmf3Color = Vector4::Add(OnGetColor(x, z, pContext), xmf4Color);
//			pVertices[i] = CDiffusedVertex(xmf3Position, xmf3Color);
//			if(fHeight < fMinHeight) fMinHeight = fHeight;
//			if(fHeight > fMaxHeight) fMaxHeight = fHeight;
//		}
//	}
//	m_pd3dVertexBuffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, pVertices,
//		m_nStride * m_nVertices, D3D12_HEAP_TYPE_DEFAULT,
//		D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &m_pd3dVertexUploadBuffer);
//	m_d3dVertexBufferView.BufferLocation = m_pd3dVertexBuffer->GetGPUVirtualAddress();
//	m_d3dVertexBufferView.StrideInBytes = m_nStride;
//	m_d3dVertexBufferView.SizeInBytes = m_nStride * m_nVertices;
//	delete[] pVertices;
//
//	m_nIndices = ((nWidth * 2)*(nLength - 1)) + ((nLength - 1) - 1);
//	UINT* pnIndices = new UINT[m_nIndices];
//	for(int j = 0, z = 0; z < nLength - 1; z++) {
//		if((z % 2) == 0) {
//			for (int x = 0; x < nWidth; x++) {
//				if ((x == 0) && (z > 0)) pnIndices[j++] = (UINT)(x + (z * nWidth));
//				pnIndices[j++] = (UINT)(x + (z * nWidth));
//				pnIndices[j++] = (UINT)((x + (z * nWidth)) + nWidth);
//			}
//		}
//		else {
//			for (int x = nWidth - 1; x >= 0; x--)
//			{
//				if (x == (nWidth - 1)) pnIndices[j++] = (UINT)(x + (z * nWidth));
//				pnIndices[j++] = (UINT)(x + (z * nWidth));
//				pnIndices[j++] = (UINT)((x + (z * nWidth)) + nWidth);
//			}
//		}
//	}
//	m_pd3dIndexBuffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, pnIndices,
//		sizeof(UINT) * m_nIndices, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_INDEX_BUFFER,
//		&m_pd3dIndexUploadBuffer);
//	m_d3dIndexBufferView.BufferLocation = m_pd3dIndexBuffer->GetGPUVirtualAddress();
//	m_d3dIndexBufferView.Format = DXGI_FORMAT_R32_UINT;
//	m_d3dIndexBufferView.SizeInBytes = sizeof(UINT) * m_nIndices;
//	delete[] pnIndices;
//}
//
//CHeightMapGridMesh::~CHeightMapGridMesh() {
//}
//
//
//float CHeightMapGridMesh::OnGetHeight(int x, int z, void* pContext) {
//	CHeightMapImage* pHeightMapImage = (CHeightMapImage*)pContext;
//	BYTE* pHeightMapPixels = pHeightMapImage->GetHeightMapPixels();
//	XMFLOAT3 xmf3Scale = pHeightMapImage->GetScale();
//	int nWidth = pHeightMapImage->GetHeightMapWidth();
//	float fHeight = pHeightMapPixels[x + (z*nWidth)] * xmf3Scale.y;
//
//	return(fHeight);
//}
//
//XMFLOAT4 CHeightMapGridMesh::OnGetColor(int x, int z, void* pContext) {
//	XMFLOAT3 xmf3LightDirection = XMFLOAT3(-1.0f, 1.0f, 1.0f);
//	xmf3LightDirection = Vector3::Normalize(xmf3LightDirection);
//	CHeightMapImage* pHeightMapImage = (CHeightMapImage*)pContext;
//	XMFLOAT3 xmf3Scale = pHeightMapImage->GetScale();
//	XMFLOAT4 xmf4IncidentLightColor(0.9f, 0.8f, 0.4f, 1.0f);
//	float fScale = Vector3::DotProduct(pHeightMapImage->GetHeightMapNormal(x, z),
//		xmf3LightDirection);
//	fScale += Vector3::DotProduct(pHeightMapImage->GetHeightMapNormal(x + 1, z),
//		xmf3LightDirection);
//	fScale += Vector3::DotProduct(pHeightMapImage->GetHeightMapNormal(x + 1, z + 1),
//		xmf3LightDirection);
//	fScale += Vector3::DotProduct(pHeightMapImage->GetHeightMapNormal(x, z + 1),
//		xmf3LightDirection);
//	fScale = (fScale / 4.0f) + 0.05f;
//	if(fScale > 1.0f) fScale = 1.0f;
//	if(fScale < 0.25f) fScale = 0.25f;
//	XMFLOAT4 xmf4Color = Vector4::Multiply(fScale, xmf4IncidentLightColor);
//
//	return(xmf4Color);
//}
