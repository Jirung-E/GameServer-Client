#include "Light.hlsl"
#include "Structures.hlsl"

struct VS_LIGHTING_INPUT {
    float3 position : POSITION;
    float3 normal : NORMAL;
};

struct VS_LIGHTING_OUTPUT {
    float4 position : SV_POSITION;
    float3 positionW : POSITION;
    float3 normalW : NORMAL;
};

VS_LIGHTING_OUTPUT VSLighting(VS_LIGHTING_INPUT input) {
    VS_LIGHTING_OUTPUT output;

    output.normalW = mul(input.normal, (float3x3)gmtxGameObject);
    output.positionW = (float3)mul(float4(input.position, 1.0f), gmtxGameObject);
    output.position = mul(mul(float4(output.positionW, 1.0f), gmtxView), gmtxProjection);
    return output;
}

float4 PSLighting(VS_LIGHTING_OUTPUT input) : SV_TARGET {
    input.normalW = normalize(input.normalW);
    float4 color = Lighting(input.positionW, input.normalW);

    return color;
}

