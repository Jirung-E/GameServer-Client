// header.h : include file for standard system include files,
// or project specific include files
//

#pragma once

#include "targetver.h"
#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files
#include <windows.h>
// C RunTime Header Files
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
#include <math.h>

#include <string>
#include <wrl.h>
#include <shellapi.h>

#include <fstream>
#include <vector>

using namespace std;

#include <d3d12.h>
#include <dxgi1_4.h>
#include <D3Dcompiler.h>
#include <DirectXMath.h>
#include <DirectXPackedVector.h>
#include <DirectXColors.h>
#include <DirectXCollision.h>

#include <Mmsystem.h>

#ifdef _DEBUG
#include <dxgidebug.h>
#endif

#pragma comment(lib, "d3dCompiler.lib")
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")

#pragma comment(lib, "dxguid.lib")

#define FRAME_BUFFER_WIDTH  800
#define FRAME_BUFFER_HEIGHT 600

#define _WITH_SWAPCHAIN_FULLSCREEN_STATE
#define _WITH_CB_WORLD_MATRIX_DESCRIPTOR_TABLE


#define RANDOM_COLOR XMFLOAT4 { rand() / float(RAND_MAX), rand() / float(RAND_MAX), rand() / float(RAND_MAX), 1.0f }

#define EPSILON	1.0e-10f


using namespace DirectX;
using namespace DirectX::PackedVector;

using Microsoft::WRL::ComPtr;


extern ID3D12Resource* CreateBufferResource(
    ID3D12Device* pd3dDevice,
    ID3D12GraphicsCommandList* pd3dCommandList,
    void* pData,
    UINT nBytes,
    D3D12_HEAP_TYPE d3dHeapType = D3D12_HEAP_TYPE_UPLOAD,
    D3D12_RESOURCE_STATES d3dResourceStates = D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER,
    ID3D12Resource** ppd3dUploadBuffer = NULL
);


inline bool IsZero(float fValue) { return((fabsf(fValue) < EPSILON)); }
inline bool IsEqual(float fA, float fB) { return(::IsZero(fA - fB)); }
inline float InverseSqrt(float fValue) { return 1.0f / sqrtf(fValue); }
inline void Swap(float* pfS, float* pfT) { float fTemp = *pfS; *pfS = *pfT; *pfT = fTemp; }


namespace Random {
    inline float RandF(float fMin, float fMax) {
        return(fMin + ((float)rand() / (float)RAND_MAX) * (fMax - fMin));
    }

    inline float randint(int fMin, int fMax) {
        return static_cast<float>(fMin + (rand() % (fMax - fMin + 1)));
    }

    inline XMVECTOR RandomUnitVectorOnSphere() {
        XMVECTOR xmvOne = XMVectorSet(1.0f, 1.0f, 1.0f, 1.0f);
        XMVECTOR xmvZero = XMVectorZero();

        while(true) {
            XMVECTOR v = XMVectorSet(RandF(-1.0f, 1.0f), RandF(-1.0f, 1.0f), RandF(-1.0f, 1.0f), 0.0f);
            if(!XMVector3Greater(XMVector3LengthSq(v), xmvOne)) return(XMVector3Normalize(v));
        }
    }

    inline XMFLOAT4 RandomColor() {
        return XMFLOAT4 { rand() / float(RAND_MAX), rand() / float(RAND_MAX), rand() / float(RAND_MAX), 1.0f };
    }
}


////////////////////////////////////////////////////////////////////////////////////////////////////


namespace Vector3 {
    inline XMFLOAT3 XMVectorToFloat3(const XMVECTOR& xmvVector) {
        XMFLOAT3 xmf3Result;
        XMStoreFloat3(&xmf3Result, xmvVector);
        return(xmf3Result);
    }

    inline XMFLOAT3 ScalarProduct(const XMFLOAT3& xmf3Vector, float fScalar, bool bNormalize = true) {
        XMFLOAT3 xmf3Result;
        if(bNormalize)
            XMStoreFloat3(&xmf3Result, XMVector3Normalize(XMLoadFloat3(&xmf3Vector)) * fScalar);
        else
            XMStoreFloat3(&xmf3Result, XMLoadFloat3(&xmf3Vector) * fScalar);
        return(xmf3Result);
    }

    inline XMFLOAT3 Add(const XMFLOAT3& xmf3Vector1, const XMFLOAT3& xmf3Vector2) {
        XMFLOAT3 xmf3Result;
        XMStoreFloat3(&xmf3Result, XMLoadFloat3(&xmf3Vector1) + XMLoadFloat3(&xmf3Vector2));
        return(xmf3Result);
    }

    inline XMFLOAT3 Add(const XMFLOAT3& xmf3Vector1, const XMFLOAT3& xmf3Vector2, float fScalar) {
        XMFLOAT3 xmf3Result;
        XMStoreFloat3(&xmf3Result, 
            XMLoadFloat3(&xmf3Vector1) + (XMLoadFloat3(&xmf3Vector2) * fScalar));
        return(xmf3Result);
    }

    inline XMFLOAT3 Subtract(const XMFLOAT3& xmf3Vector1, const XMFLOAT3& xmf3Vector2) {
        XMFLOAT3 xmf3Result;
        XMStoreFloat3(&xmf3Result, XMLoadFloat3(&xmf3Vector1) - XMLoadFloat3(&xmf3Vector2));
        return(xmf3Result);
    }

    inline float DotProduct(const XMFLOAT3& xmf3Vector1, const XMFLOAT3& xmf3Vector2) {
        XMFLOAT3 xmf3Result;
        XMStoreFloat3(&xmf3Result, 
            XMVector3Dot(XMLoadFloat3(&xmf3Vector1), XMLoadFloat3(&xmf3Vector2)));
        return(xmf3Result.x);
    }

    inline XMFLOAT3 CrossProduct(const XMFLOAT3& xmf3Vector1, const XMFLOAT3& xmf3Vector2, bool bNormalize = true) {
        XMFLOAT3 xmf3Result;
        if(bNormalize)
            XMStoreFloat3(&xmf3Result, 
                XMVector3Normalize(XMVector3Cross(
                    XMLoadFloat3(&xmf3Vector1), 
                    XMLoadFloat3(&xmf3Vector2))
                )
            );
        else
            XMStoreFloat3(&xmf3Result, 
                XMVector3Cross(XMLoadFloat3(&xmf3Vector1), XMLoadFloat3(&xmf3Vector2)));
        return(xmf3Result);
    }

    inline XMFLOAT3 Normalize(const XMFLOAT3& xmf3Vector) {
        XMFLOAT3 m_xmf3Normal;
        XMStoreFloat3(&m_xmf3Normal, XMVector3Normalize(XMLoadFloat3(&xmf3Vector)));
        return(m_xmf3Normal);
    }

    inline float Length(const XMFLOAT3& xmf3Vector) {
        XMFLOAT3 xmf3Result;
        XMStoreFloat3(&xmf3Result, XMVector3Length(XMLoadFloat3(&xmf3Vector)));
        return(xmf3Result.x);
    }

    inline float Angle(const XMVECTOR& xmvVector1, const XMVECTOR& xmvVector2) {
        XMVECTOR xmvAngle = XMVector3AngleBetweenNormals(xmvVector1, xmvVector2);
        return(XMConvertToDegrees(acosf(XMVectorGetX(xmvAngle))));
    }

    inline float Angle(const XMFLOAT3& xmf3Vector1, const XMFLOAT3& xmf3Vector2) {
        return(Angle(XMLoadFloat3(&xmf3Vector1), XMLoadFloat3(&xmf3Vector2)));
    }

    inline XMFLOAT3 TransformNormal(const XMFLOAT3& xmf3Vector, const XMMATRIX& xmmtxTransform) {
        XMFLOAT3 xmf3Result;
        XMStoreFloat3(&xmf3Result, 
            XMVector3TransformNormal(XMLoadFloat3(&xmf3Vector), xmmtxTransform));
        return(xmf3Result);
    }

    inline XMFLOAT3 TransformCoord(const XMFLOAT3& xmf3Vector, const XMMATRIX& xmmtxTransform) {
        XMFLOAT3 xmf3Result;
        XMStoreFloat3(&xmf3Result, 
            XMVector3TransformCoord(XMLoadFloat3(&xmf3Vector), xmmtxTransform));
        return(xmf3Result);
    }

    inline XMFLOAT3 TransformCoord(const XMFLOAT3& xmf3Vector, const XMFLOAT4X4& xmmtx4x4Matrix) {
        return(TransformCoord(xmf3Vector, XMLoadFloat4x4(&xmmtx4x4Matrix)));
    }
}


////////////////////////////////////////////////////////////////////////////////////////////////////


namespace Vector4 {
    inline XMFLOAT4 Add(const XMFLOAT4& xmf4Vector1, const XMFLOAT4& xmf4Vector2) {
        XMFLOAT4 xmf4Result;
        XMStoreFloat4(&xmf4Result, XMLoadFloat4(&xmf4Vector1) + XMLoadFloat4(&xmf4Vector2));
        return(xmf4Result);
    }

    inline XMFLOAT4 Multiply(const XMFLOAT4& xmf4Vector1, const XMFLOAT4& xmf4Vector2) {
        XMFLOAT4 xmf4Result;
        XMStoreFloat4(&xmf4Result, XMLoadFloat4(&xmf4Vector1) * XMLoadFloat4(&xmf4Vector2));
        return(xmf4Result);
    }

    inline XMFLOAT4 Multiply(float fScalar, const XMFLOAT4& xmf4Vector) {
        XMFLOAT4 xmf4Result;
        XMStoreFloat4(&xmf4Result, fScalar * XMLoadFloat4(&xmf4Vector));
        return(xmf4Result);
    }
}


////////////////////////////////////////////////////////////////////////////////////////////////////


namespace Matrix4x4 {
    inline XMFLOAT4X4 Identity() {
        XMFLOAT4X4 xmmtx4x4Result;
        XMStoreFloat4x4(&xmmtx4x4Result, XMMatrixIdentity());
        return(xmmtx4x4Result);
    }

    inline XMFLOAT4X4 Multiply(const XMFLOAT4X4& xmmtx4x4Matrix1, const XMFLOAT4X4& xmmtx4x4Matrix2) {
        XMFLOAT4X4 xmmtx4x4Result;
        XMStoreFloat4x4(&xmmtx4x4Result, 
            XMLoadFloat4x4(&xmmtx4x4Matrix1) * XMLoadFloat4x4(&xmmtx4x4Matrix2));
        return(xmmtx4x4Result);
    }

    inline XMFLOAT4X4 Multiply(const XMFLOAT4X4& xmmtx4x4Matrix1, const XMMATRIX& xmmtxMatrix2) {
        XMFLOAT4X4 xmmtx4x4Result;
        XMStoreFloat4x4(&xmmtx4x4Result, XMLoadFloat4x4(&xmmtx4x4Matrix1) * xmmtxMatrix2);
        return(xmmtx4x4Result);
    }

    inline XMFLOAT4X4 Multiply(const XMMATRIX& xmmtxMatrix1, const XMFLOAT4X4& xmmtx4x4Matrix2) {
        XMFLOAT4X4 xmmtx4x4Result;
        XMStoreFloat4x4(&xmmtx4x4Result, xmmtxMatrix1 * XMLoadFloat4x4(&xmmtx4x4Matrix2));
        return(xmmtx4x4Result);
    }

    inline XMFLOAT4X4 Inverse(const XMFLOAT4X4& xmmtx4x4Matrix) {
        XMFLOAT4X4 xmmtx4x4Result;
        XMStoreFloat4x4(&xmmtx4x4Result, 
            XMMatrixInverse(NULL, XMLoadFloat4x4(&xmmtx4x4Matrix)));
        return(xmmtx4x4Result);
    }

    inline XMFLOAT4X4 Transpose(const XMFLOAT4X4& xmmtx4x4Matrix) {
        XMFLOAT4X4 xmmtx4x4Result;
        XMStoreFloat4x4(&xmmtx4x4Result,
            XMMatrixTranspose(XMLoadFloat4x4(&xmmtx4x4Matrix)));
        return(xmmtx4x4Result);
    }

    inline XMFLOAT4X4 PerspectiveFovLH(float FovAngleY, float AspectRatio, float NearZ, float FarZ) {
        XMFLOAT4X4 xmmtx4x4Result;
        XMStoreFloat4x4(&xmmtx4x4Result, 
            XMMatrixPerspectiveFovLH(FovAngleY, AspectRatio, NearZ, FarZ));
        return(xmmtx4x4Result);
    }

    inline XMFLOAT4X4 LookAtLH(const XMFLOAT3& xmf3EyePosition, const XMFLOAT3& xmf3LookAtPosition,
        const XMFLOAT3& xmf3UpDirection) {
        XMFLOAT4X4 xmmtx4x4Result;
        XMStoreFloat4x4(
            &xmmtx4x4Result, 
            XMMatrixLookAtLH(
                XMLoadFloat3(&xmf3EyePosition),
                XMLoadFloat3(&xmf3LookAtPosition), 
                XMLoadFloat3(&xmf3UpDirection)
            )
        );
        return(xmmtx4x4Result);
    }
}


////////////////////////////////////////////////////////////////////////////////////////////////////



namespace Triangle {
    inline bool Intersect(
        const XMFLOAT3& xmf3RayPosition, 
        const XMFLOAT3& xmf3RayDirection, 
        const XMFLOAT3& v0, 
        const XMFLOAT3& v1, 
        const XMFLOAT3& v2, 
        float& fHitDistance
    ) {
        return TriangleTests::Intersects(
            XMLoadFloat3(&xmf3RayPosition), 
            XMLoadFloat3(&xmf3RayDirection), 
            XMLoadFloat3(&v0), 
            XMLoadFloat3(&v1), 
            XMLoadFloat3(&v2), 
            fHitDistance
        );
    }
}

namespace Plane {
    inline XMFLOAT4 Normalize(const XMFLOAT4& xmf4Plane) {
        XMFLOAT4 xmf4Result;
        XMStoreFloat4(&xmf4Result, XMPlaneNormalize(XMLoadFloat4(&xmf4Plane)));
        return xmf4Result;
    }
}
