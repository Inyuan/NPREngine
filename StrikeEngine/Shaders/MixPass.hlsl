#include "Common.hlsl"

Texture2D SceneDepthMap: register(t0);
StructuredBuffer<MaterialData> gMaterialData : register(t0, space1);
RWTexture2D<float4> ProfileMap : register(u0);
Texture2D LightMap : register(t1);
Texture2D GBuffer[5] : register(t2);


cbuffer cbPass : register(b1)
{
    float4x4 gView;
    float4x4 gInvView;
    float4x4 gProj;
    float4x4 gInvProj;
    float4x4 gViewProj;
    float4x4 gInvViewProj;
    float4x4 gShadowTransform;
    float3 gEyePosW;
    float cbPerObjectPad1;
    float2 gRenderTargetSize;
    float2 gInvRenderTargetSize;
    float gNearZ;
    float gFarZ;
    float gTotalTime;
    float gDeltaTime;
    float4 gAmbientLight;

    Light gLights[MaxLights];
};

struct vs_in {
    float4 position : POSITION;
    float2 texcoord : TEXCOORD;
};

struct vs_out {
    float4 position : SV_POSITION;
    float2 texcoord : TEXCOORD;
};

vs_out  VS(vs_in vIn)
{
    vs_out vOut;
    vOut.position = vIn.position;
    vOut.texcoord = vIn.texcoord;
    return vIn;
}


float4 PS(vs_out pIn) : SV_TARGET
{
	uint   meterialId = GBuffer[1][pIn.position.xy].a;
	MaterialData matData = gMaterialData[meterialId];

    float3 profilecolor = matData.ProfileColor;
    float Profilemask = ProfileMap[pIn.position.xy].r;
    float4 outcolor = LightMap[pIn.position.xy];


    uint2 position_1 = pIn.position.xy;
    position_1.x++;
    uint2 position_2 = pIn.position.xy;
    position_2.x--;
    uint2 position_3 = pIn.position.xy;
    position_3.y++;
    uint2 position_4 = pIn.position.xy;
    position_4.y--;
    uint2 position_5 = pIn.position.xy;
    position_5.x++;//(1,1)
    position_5.y++;
    uint2 position_6 = pIn.position.xy;
    position_6.x++;//(1,-1)
    position_6.y--;
    uint2 position_7 = pIn.position.xy;
    position_7.y++;//(-1,1)
    position_7.x--;
    uint2 position_8 = pIn.position.xy;
    position_8.y--;//(-1,-1)
    position_8.x--;


    float4 outcolor_1 = LightMap[position_1];
    float4 outcolor_2 = LightMap[position_2];
    float4 outcolor_3 = LightMap[position_3];
    float4 outcolor_4 = LightMap[position_4];
    float4 outcolor_5 = LightMap[position_5];
    float4 outcolor_6 = LightMap[position_6];
    float4 outcolor_7 = LightMap[position_7];
    float4 outcolor_8 = LightMap[position_8];
    //0Îª±ß

    outcolor += outcolor_1;
    outcolor += outcolor_2;
    outcolor += outcolor_3;
    outcolor += outcolor_4;
    outcolor += outcolor_5;
    outcolor += outcolor_6;
    outcolor += outcolor_7;
    outcolor += outcolor_8;
    outcolor /= 8.0f;

    float procolor_1 = ProfileMap[position_1].r;
    float procolor_2 = ProfileMap[position_2].r;
    float procolor_3 = ProfileMap[position_3].r;
    float procolor_4 = ProfileMap[position_4].r;
    float procolor_5 = ProfileMap[position_5].r;
    float procolor_6 = ProfileMap[position_6].r;
    float procolor_7 = ProfileMap[position_7].r;
    float procolor_8 = ProfileMap[position_8].r;

    Profilemask += procolor_1;
    Profilemask += procolor_2;
    Profilemask += procolor_3;
    Profilemask += procolor_4;
    Profilemask += procolor_5;
    Profilemask += procolor_6;
    Profilemask += procolor_7;
    Profilemask += procolor_8;
    Profilemask /= 8.0f;


    profilecolor = lerp(profilecolor, 1, Profilemask);

    float4 rescolor = { outcolor.rgb * profilecolor,outcolor.a };


    return rescolor;
}