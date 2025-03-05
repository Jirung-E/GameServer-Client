#include "Structures.hlsl"

//--------------------------------------------------------------------------------------
#define MAX_LIGHTS			16 
#define MAX_MATERIALS		512 

#define POINT_LIGHT			1
#define SPOT_LIGHT			2
#define DIRECTIONAL_LIGHT	3


struct LIGHT {
    float4 m_cAmbient;
    float4 m_cDiffuse;
    float4 m_cSpecular;
    float3 m_vPosition;
    float m_fFalloff;
    float3 m_vDirection;
    float m_fTheta; //cos(m_fTheta)
    float3 m_vAttenuation;
    float m_fPhi; //cos(m_fPhi)
    bool m_bEnable;
    int m_nType;
    float m_fRange;
    float padding;
};

cbuffer cbLights : register(b4) {
    LIGHT gLights[MAX_LIGHTS];
    float4 gcGlobalAmbientLight;
    int gnLights;
};

float4 DirectionalLight(int nIndex, float3 vNormal, float3 vToCamera) {
    float3 vToLight = -gLights[nIndex].m_vDirection;
    
    float glossiness = gMaterial.m_cSpecular.a;
    
    float4 ambient = gLights[nIndex].m_cAmbient * gMaterial.m_cAmbient;
    float4 diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float4 specular = float4(0.0f, 0.0f, 0.0f, 0.0f);
    
    float diffuse_light = max(dot(vNormal, vToLight), 0.0f);
    if(diffuse_light > 0.0f) {
        diffuse_light = gLights[nIndex].m_cDiffuse * diffuse_light;
        diffuse = gMaterial.m_cDiffuse * diffuse_light;
        
        if(glossiness != 0.0f) {
            float3 r = reflect(-vToLight, vNormal);
            float specular_light = max(dot(vToCamera, r), 0.0f);
            if(specular_light > 0.0f) {
                specular_light = gLights[nIndex].m_cSpecular * pow(specular_light, glossiness * 255);
                specular = gMaterial.m_cSpecular * specular_light * glossiness;
            }
        }
    }

    return ambient + diffuse + specular;
}

float4 PointLight(int nIndex, float3 vPosition, float3 vNormal, float3 vToCamera) {
    float3 vToLight = gLights[nIndex].m_vPosition - vPosition;
    float fDistance = length(vToLight);
    
    if(fDistance <= gLights[nIndex].m_fRange) {
        float fSpecularFactor = 0.0f;
        vToLight /= fDistance;
        
        float fDiffuseFactor = dot(vToLight, vNormal);
        float fAttenuationFactor = 0.0f;
        
        if(fDiffuseFactor > 0.0f) {
            if(gMaterial.m_cSpecular.a != 0.0f) {
                float3 vHalf = normalize(vToCamera + vToLight);
                fSpecularFactor = pow(max(dot(vHalf, vNormal), 0.0f), gMaterial.m_cSpecular.a * 255);
            }
            
            fAttenuationFactor = 1.0f / dot(gLights[nIndex].m_vAttenuation, float3(1.0f, fDistance, fDistance * fDistance));
        }
        else {
            fDiffuseFactor = 0.0f;
        }

        return ((gLights[nIndex].m_cAmbient * gMaterial.m_cAmbient) + (gLights[nIndex].m_cDiffuse * fDiffuseFactor * gMaterial.m_cDiffuse) + (gLights[nIndex].m_cSpecular * fSpecularFactor * gMaterial.m_cSpecular)) * fAttenuationFactor;
    }
    
    return float4(0.0f, 0.0f, 0.0f, 0.0f);
}

float4 SpotLight(int nIndex, float3 vPosition, float3 vNormal, float3 vToCamera) {
    float3 vToLight = gLights[nIndex].m_vPosition - vPosition;
    float fDistance = length(vToLight);
    
    if(fDistance <= gLights[nIndex].m_fRange) {
        float fSpecularFactor = 0.0f;
        vToLight /= fDistance;
        
        float fDiffuseFactor = dot(vToLight, vNormal);
        float fAlpha = 0.0f;
        float fSpotFactor = 0.0f;
        float fAttenuationFactor = 0.0f;
        
        if(fDiffuseFactor > 0.0f) {
            if(gMaterial.m_cSpecular.a != 0.0f) {
                float3 vHalf = normalize(vToCamera + vToLight);
                fSpecularFactor = pow(max(dot(vHalf, vNormal), 0.0f), gMaterial.m_cSpecular.a);
            }
            fAlpha = max(dot(-vToLight, gLights[nIndex].m_vDirection), 0.0f);
            fSpotFactor = pow(max(((fAlpha - gLights[nIndex].m_fPhi) / (gLights[nIndex].m_fTheta - gLights[nIndex].m_fPhi)), 0.0f), gLights[nIndex].m_fFalloff);
            fAttenuationFactor = 1.0f / dot(gLights[nIndex].m_vAttenuation, float3(1.0f, fDistance, fDistance * fDistance));
        }
        else {
            fDiffuseFactor = 0.0f;
        }

        return ((gLights[nIndex].m_cAmbient * gMaterial.m_cAmbient) + (gLights[nIndex].m_cDiffuse * fDiffuseFactor * gMaterial.m_cDiffuse) + (gLights[nIndex].m_cSpecular * fSpecularFactor * gMaterial.m_cSpecular)) * fAttenuationFactor * fSpotFactor;
    }
    
    return float4(0.0f, 0.0f, 0.0f, 0.0f);
}

float4 Lighting(float3 vPosition, float3 vNormal) {
    float3 vCameraPosition = gvCameraPosition.xyz;
    float3 vToCamera = normalize(vCameraPosition - vPosition);

    float4 cColor = float4(0.0f, 0.0f, 0.0f, 0.0f);
    
	[unroll(MAX_LIGHTS)]
    for(int i = 0; i < gnLights; i++) {
        if(gLights[i].m_bEnable) {
            if(gLights[i].m_nType == DIRECTIONAL_LIGHT) {
                cColor += DirectionalLight(i, vNormal, vToCamera);
            }
            else if(gLights[i].m_nType == POINT_LIGHT) {
                cColor += PointLight(i, vPosition, vNormal, vToCamera);
            }
            else if(gLights[i].m_nType == SPOT_LIGHT) {
                cColor += SpotLight(i, vPosition, vNormal, vToCamera);
            }
        }
    }
    
    cColor += (gcGlobalAmbientLight * gMaterial.m_cAmbient);
    cColor.a = gMaterial.m_cDiffuse.a;

    return cColor;
}

