#ifndef _STRUCTURES_H_
#define _STRUCTURES_H_

struct MATERIAL {
    float4 m_cAmbient;
    float4 m_cDiffuse;
    float4 m_cSpecular; //a = power
    float4 m_cEmissive;
};

cbuffer cbCameraInfo : register(b1) {
    matrix gmtxView : packoffset(c0);
    matrix gmtxProjection : packoffset(c4);
    float3 gvCameraPosition : packoffset(c8);
};

cbuffer cbGameObjectInfo : register(b2) {
    matrix gmtxGameObject : packoffset(c0);
    MATERIAL gMaterial : packoffset(c4);
};

#endif